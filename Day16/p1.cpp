#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<stdint.h>
#include<cassert>
#include<chrono>

//aka std::vector
template<typename T>
struct Vector {
    int count, cap;
    T *data;

    Vector() : count{}, cap{ 8 }, data{ new T[cap] } {}

    void add(int value) {
        if(count == cap) data = (T*)realloc(data, cap*sizeof(T) * 2);
        data[count++] = value;
    }

    void compact() {
        data = (T*)realloc(data, cap = count);
    }
};

struct Room {
    Vector<int> connectedRooms;
    int rate;
};

static std::ostream &operator<<(std::ostream &o, Room it) {
    o << "Room{ rate=" << it.rate << ", connectedRooms={ ";
    for(int i = 0; i < it.connectedRooms.count; i++) {
        if(i != 0) o << ", ";
        o << it.connectedRooms.data[i];
    }
    o << " }";
    return o;
}

struct Map {
    struct alignas(64) Node {
        uint64_t data[8] /*
            (59 bist for state + 5 bits for remaining steps) * 7
            + (2 bist of next node index + 6 bits of room index) * 7
            + 5 bits of next node index
            + 3 bits of count 
            <=>
            uint32_t nextNode : 19; // = index of next or 0 for no next node (next node can never be at index 0)
            uint8_t count : 3;
            uint64_t[7] state;
            uint8_t[7] roomNumber;
            uint8_t[7] remainingSteps;
        */;
    };

    unsigned *indicesO;
    Node *nodes;
    int *pressures;
    int nodesCount;

    mutable int maxChain, entriesCount, accessCount, successCount; //stats

    static constexpr auto indicesCapacity = 37871; //100069; //9598th prime
    static constexpr auto nodesCapacity = 500'000;
    static_assert(nodesCapacity < (1 << 19) - 1); //address of next node is only 19 bits & includes 1 invalid state

    Map() : 
        indicesO{ new unsigned[indicesCapacity]{} }, 
        nodes{ new Node[nodesCapacity]{} },
        pressures{ new int[nodesCapacity*8] },
        nodesCount{},
        maxChain{},
        entriesCount{},
        accessCount{},
        successCount{}
    {
        if(uintptr_t(nodes) & 0b00111111 != 0) exit(43); //unaligned at cache line
    }

    static unsigned calcHash(
        uint64_t const state, 
        int const roomIndex,
        int const remainingSteps
    ) {
        return unsigned(
            uint64_t(state ^ (uint64_t(roomIndex) << 5) ^ remainingSteps)
            % indicesCapacity
        );
    }

    int getPressure(
        uint64_t const state, 
        int const roomIndex,
        int const remainingSteps,
        unsigned hash
    ) const {
        accessCount++;
        auto index = indicesO[hash] - 1u;
        if(index == -1u) return -1;
        while(true) {
            auto const &node = nodes[index];
            auto const count = node.data[7] >> 61;

            for(int i = 0; i < count; i++) {
                auto const data = node.data[i];
                auto const curState = data >> 5;
                auto const curRemSteps = data & 0b1'1111;
                auto const curRoomIndex = (node.data[7] >> (8*i + 2)) & 0b11'1111; 

                if(curState != state) continue;
                if(curRemSteps != remainingSteps) continue;
                if(curRoomIndex != roomIndex) continue;
                
                successCount++;
                return pressures[index*8 + i];
            }

            auto const nextNodeIndex = getNextNodeIndex(node);
            if(nextNodeIndex == 0) return -2;
            index = nextNodeIndex;
        }
    }

    static unsigned getNextNodeIndex(Node const & node) {
        auto nextNodeIndex = 0u;
        for(int i = 0; i < 7; i++) {
            nextNodeIndex = (nextNodeIndex << 2) | ((node.data[7] >> (8*i)) & 0b11);
        }
        return (nextNodeIndex << 5) | ((node.data[7] >> (7*8)) & 0b1'1111);
    }

    void putPressure(
        int pressure,
        uint64_t const state, 
        int const roomIndex,
        int const remainingSteps,
        unsigned hash
    ) {
        assert(state <= (1ull << 59) -1);
        assert(roomIndex >= 0 & roomIndex <= 59);
        assert(remainingSteps >= 0 & remainingSteps <= 30);
        assert(pressure >= 0);
        auto index = indicesO[hash] - 1u;
        auto curChainLen = 1;
        entriesCount++;
        if(index == -1u) {
            if(nodesCount == nodesCapacity) exit(28);
            indicesO[hash] = 1 + (index = nodesCount++);
            curChainLen = 0;
        }
        while(true) {
            auto &node = nodes[index];
            auto const count = node.data[7] >> 61;
            
            if(count == 7) {
                auto nextNodeIndex = getNextNodeIndex(node);
                if(nextNodeIndex != 0) {
                    index = nextNodeIndex;
                    curChainLen++;
                    continue;
                }
                if(nodesCount == nodesCapacity) exit(28);
                nextNodeIndex = index = nodesCount++;
                node.data[7] |= uint64_t(nextNodeIndex & 0b1'1111) << (7*8);
                nextNodeIndex = nextNodeIndex >> 5;
                for(int i = 6; i >= 0; i--) {
                    node.data[7] |= uint64_t(nextNodeIndex & 0b11) << (8*i);
                    nextNodeIndex = nextNodeIndex >> 2;
                }
                assert(nextNodeIndex == 0);
                curChainLen++;
                continue;
            }
            
            node.data[count] |= (state << 5) | remainingSteps;
            node.data[7] |= (uint64_t(roomIndex) << (8*count + 2));
            node.data[7] &= ~(uint64_t(0b111) << 61);
            node.data[7] |= (uint64_t(roomIndex) << (8*count + 2));
            node.data[7] |= ((count+1) << 61);
            pressures[index*8 + count] = pressure;
            if(curChainLen > maxChain) maxChain = curChainLen;
            return;
        }
    }
};

static int iterations = 0;


static int *offsetsO_;
void printOffset(int o) {
    for(int i = 0; i < 26*26; i++) {
        if(o == offsetsO_[i] - 1) {
            std::cout << char(i/26 + 'A') << char(i%26 + 'A');
            return;
        }
    }
    std::cout << "-1";
}

namespace MaxPressure {
    static Room const *rooms;
    static int count;
    static Map map;
    static uint8_t lastPath[30];

    static int maxPressure(
        uint8_t const curRoom, uint64_t const curState,
        uint8_t const stepsRemaining, uint8_t const lastPathEntries
    ) {
        if(stepsRemaining == 0) return 0;

        auto const hash = decltype(map)::calcHash(curState, curRoom, stepsRemaining);
        auto const pressureCand = map.getPressure(curState, curRoom, stepsRemaining, hash);
        if(pressureCand >= 0) return pressureCand;

        auto const prevLastEntry = lastPath[lastPathEntries];
        lastPath[lastPathEntries] = curRoom;

        auto const &room = rooms[curRoom];
        auto const connectedCount = room.connectedRooms.count;
        auto const connected = room.connectedRooms.data;
        auto const rate = room.rate;

        int max;

        //turn valve in current room
        if(rate != 0 & ((curState >> curRoom)&1) == 0) {
            max = maxPressure(
                curRoom, curState | (uint64_t(1) << curRoom),
                stepsRemaining-1, 0
            ) + rate * (stepsRemaining-1);
        }
        else max = 0;

        for(int nextRoomI = 0; nextRoomI < connectedCount; nextRoomI++) {
            auto const nextRoom = connected[nextRoomI];

            auto alreadyVisited = false;
            for(auto i = 0; i < lastPathEntries; i++) {
                if(lastPath[i] == nextRoom) {
                    alreadyVisited = true;
                    break;
                }
            }
            if(alreadyVisited) continue;

            auto const maxCand = maxPressure(
                connected[nextRoomI], curState,
                stepsRemaining-1, lastPathEntries+1
            );
            if(maxCand > max) max = maxCand;
        }

        iterations++;

        if(max != 0) map.putPressure(max, curState, curRoom, stepsRemaining, hash);
        lastPath[lastPathEntries] = prevLastEntry;
        return max;
    }

    static int maxPressure(Room const *const _rooms, int const _count, uint8_t startingRoom) {
        if(_count > 59) exit(20);

        rooms = _rooms;
        count = _count;

        return maxPressure(startingRoom, uint64_t(0), 30, 0);
    }
};


#include<random>

int main() {
#if 0
    std::mt19937 gen(7079);
    std::uniform_int_distribution<uint64_t> state(0, (1ull << 59) - 1);
    std::uniform_int_distribution<uint8_t> roomIndex(0, 59);
    std::uniform_int_distribution<uint8_t> remainingSteps(0, 30);
    std::uniform_int_distribution<int> pressure(0);
   
    /*
        uint64_t const state, 
        int const roomIndex,
        int const remainingSteps,
    */

    auto map = Map{};
    for(int iter = 0; iter < 100'000; ++iter) {
        auto const s = state(gen);
        auto const i = roomIndex(gen);
        auto const r = remainingSteps(gen);
        auto const p = pressure(gen);
        auto const hash = Map::calcHash(s, i, r);
        
        map.putPressure(p, s, i, r, hash);
        auto const p2 = map.getPressure(s, i, r, hash);

        if(p != p2) {
            printf("iteration %d:\n\tstate = 0x", iter);
            for(int j = 0; j < 64; j++) {
                std::cout << ((s >> (64-j-1)) & 1);
                if(j != 0 & j % 8 == 0) std::cout << '\'';
            }
            std::cout << "\n\tindex = " << (int) i;
            std::cout << "\n\tremainingSteps = " << (int) r;
            std::cout << "\n\texpected = " << (int) p;
            std::cout << "\n\tactual   = " << (int) p2 << '\n';
            exit(25);
        }
    }

    std::cout << "done\n";
    return 0;
#else

    static constexpr auto letters = 26;

    auto str = std::ifstream{ "p1.txt" };

    auto rooms = new Room[letters*letters];
    auto roomCount = 0;

    auto offsetsO = new int[letters*letters]{}; //offseted by 1 so that invalid index is 0
    auto const getOffset = [&](char f, char s) {
        auto const index = ((s - 'A')*letters) + (f - 'A');
        if(offsetsO[index] == 0) offsetsO[index] = roomCount++ + 1;
        return offsetsO[index] - 1;
    };

    while(str.peek() != std::char_traits<char>::eof()) {
        str.ignore(6);
        char rf, rs;
        str >> rf >> rs;
        auto const offset = getOffset(rf, rs);
        str.ignore(15);
        int rate;
        str >> rate;
        str.ignore(24);

        auto otherRooms = Vector<int>();
        while(str.peek() != '\n') {
            str >> rf >> rs;
            auto const otherOffset = getOffset(rf, rs);
            otherRooms.add(otherOffset);
            if(str.get() == '\n') break;
            str.ignore(1);
        }
        otherRooms.compact();

        rooms[offset] = { otherRooms, rate };
    }
    auto const startingRoom = getOffset('A', 'A');

    offsetsO_ = offsetsO;
    //delete[] offsetsO;

    std::cout << "rooms: " << roomCount << '\n';
    for(int i = 0; i < roomCount; i++) {
        std::cout << "#" << i << " = " << rooms[i] << '\n';
    }
    std::cout << "------------------------------------------\n";

    auto const start = std::chrono::high_resolution_clock::now();
    auto const mp = MaxPressure::maxPressure(rooms, roomCount, startingRoom);
    auto const end = std::chrono::high_resolution_clock::now();
    auto const time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);
    auto const &map = MaxPressure::map;
    std::cout << "maxPressure(" << time << "ms, " << iterations << " iters): " << mp << '\n';
    std::cout << "map max chain is " << map.maxChain << " steps,\nused "
        << map.entriesCount << " entries in "
        << map.nodesCount << " nodes.\n";
    std::cout << "accessed " << map.accessCount << " times, " << map.successCount << " successfully\n";

    return 0;
#endif
}
