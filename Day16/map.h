#include<stdint.h>
#include<cassert>
#include<bitset>
#include<new>

template<typename Value_>
struct Map {
    using Value = Value_;

    struct alignas(64) Node {
        uint32_t data[16] /*
              (16 bits for state
              + 5 bits for first remaining steps + 5 bits for second remaining steps
              + 4 bits for first room index + 2 bits for second room index) * 14
            + (2 bits for second room index * 14 + 4 bits for count)
            + 32 bits for index of next node
            <=>
            uint32_t count : 4; //0 if empty!
            uint32_t nextNode : 32;
            uint16_t[14] state;
            uint8_t[14] fRoomNumber;
            uint8_t[14] sRoomNumber;
            uint8_t[14] fRemSteps;
            uint8_t[14] sRemSteps;
        */;
    };
    using index_t = uint32_t;
    static constexpr auto valuesInNodeCount = 14;
    static constexpr auto invalidIndex = index_t{};

    using pstate_t = uint64_t;
    static pstate_t packState(
        uint16_t const state, 
        int const fRoomIndex,
        int const sRoomIndex,
        int const fRemSteps,
        int const sRemSteps
    ) {
        assert(state <= (1ull << 16) - 1);
        assert(fRoomIndex >= 0 & fRoomIndex < 16);
        assert(sRoomIndex >= 0 & sRoomIndex < 16);
        assert(fRemSteps >= 0 & fRemSteps < 32);
        assert(sRemSteps >= 0 & sRemSteps < 32);
        return pstate_t(state)
            | (pstate_t(fRemSteps)  << (16+ 0))
            | (pstate_t(sRemSteps)  << (16+ 5))
            | (pstate_t(fRoomIndex) << (16+10))
            | (pstate_t(sRoomIndex) << (16+14));
    }

    Node *nodes;
    Value *values;
    int nodesCount;

    Value invalid;

    mutable int maxChain, entriesCount, 
            accessCount, successCount,
            didntPut; //stats

    static constexpr auto hashCapacity = 1000669; //78551st prime    //100069; //9598th prime
    static constexpr auto nodesCapacity = 5'000'000;


    Map(Value invalid_) : 
        nodes{ new (std::align_val_t(64)) Node[nodesCapacity]{} },
        values{ new Value[nodesCapacity*valuesInNodeCount] },
        nodesCount{ hashCapacity },
        invalid(invalid_),
        maxChain{},
        entriesCount{},
        accessCount{},
        successCount{},
        didntPut{}
    {
        assert((uintptr_t(nodes) & 0b00111111) == 0);  //unaligned at cache line
    }

    static index_t calcHash(
        uint16_t const state, 
        int const fRoomIndex,
        int const sRoomIndex,
        int const fRemSteps,
        int const sRemSteps
    ) {
        std::bitset<16 + 5 + 5 + 4 + 4> bits{};
        bits = state;
        bits <<= 5;
        bits |= fRemSteps;
        bits <<= 5;
        bits |= sRemSteps;
        bits <<= 4;
        bits |= sRoomIndex;
        bits <<= 4;
        bits |= fRoomIndex;
        return (index_t) (std::hash<decltype(bits)>{}(bits) % hashCapacity) + 1;
    }


    Value get(
        pstate_t const pState,
        index_t const hash
    ) const {
        auto index = hash;
        assert(index != invalidIndex);
        accessCount++;
        while(true) {
            auto const &node = nodes[index - 1];
            auto const count = node.data[14] >> 28;

            auto const auxData = node.data[14];
            for(int i = 0; i < count; i++) {
                auto const data = node.data[i];
                auto const curPState = pstate_t(data) | (pstate_t((auxData >> (2*i)) & 0b11) << 32);
                if(pState != curPState) continue;
                
                successCount++;
                return values[index*valuesInNodeCount + i];
            }

            auto const nextNodeIndex = (index_t) node.data[15];
            if(nextNodeIndex == invalidIndex) return invalid;
            index = nextNodeIndex;
        }
    }

    void put(
        Value value,
        pstate_t const pState,
        index_t const hash 
    ) {
        auto index = hash;
        assert(index != invalidIndex);
        auto curChainLen = 1;
        while(true) {
            auto &node = nodes[index - 1];
            auto const count = node.data[14] >> 28;

            if(count == 6) {
                auto nextNodeIndex = node.data[15];
                if(nextNodeIndex != invalidIndex) {
                    index = nextNodeIndex;
                    curChainLen++;
                    continue;
                }
                if(nodesCount == nodesCapacity) { didntPut++; return; }
                nextNodeIndex = index = nodesCount++;
                node.data[15] = nextNodeIndex;
                curChainLen++;
                continue;
            }
            
            node.data[count] = pState;
            node.data[14] = (node.data[14] & ~(0b1111u << 28)) | ((count+1) << 28)
                | ((pState >> 32) & 0b11) << (2*count);
            static_assert(invalidIndex == 0) /*node.data[15] = invalidIndex;*/;
            values[index*valuesInNodeCount + count] = value;

            entriesCount++;
            if(curChainLen > maxChain) maxChain = curChainLen;
            return;
        }
    }
};
