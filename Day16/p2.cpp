#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<stdint.h>
#include<cassert>
#include<chrono>
#include<bitset>

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
            (5 bits for remaining steps + 59 bist for state) * 6
            + (6 bits of first room index + 2 bits of padding) * 6 + 16 bits for next node inde
            + (6 bits of second room index + 2 bist of padding) * 6 + 13 bits for next node index + 3 bits of count
            <=>
            uint32_t count : 3
            uint32_t nextNode : 29; // = index of next or 0 for no next node (next node can never be at index 0)
            uint64_t[6] state;
            uint8_t[6] fRoomNumber;
            uint8_t[6] sRoomNumber;
            uint8_t[6] remainingSteps;
        */;
    };

    static constexpr auto invalidIndex = -1u;

    unsigned *indicesO;
    Node *nodes;
    int *pressures;
    int nodesCount;

    mutable int maxChain, entriesCount, 
            accessCount, successCount,
            didntPut; //stats

    static constexpr auto indicesCapacity = 1000669; //78551st prime    //100069; //9598th prime
    static constexpr auto nodesCapacity = 5'000'000;


    Map() : 
        indicesO{ new unsigned[indicesCapacity]{} }, 
        nodes{ new Node[nodesCapacity]{} },
        pressures{ new int[nodesCapacity*8] },
        nodesCount{},
        maxChain{},
        entriesCount{},
        accessCount{},
        successCount{},
        didntPut{}
    {
        assert((uintptr_t(nodes) & 0b00111111) == 0);  //unaligned at cache line
    }

    static size_t calcHash(
        uint64_t const state, 
        int const fRoomIndex,
        int const sRoomIndex,
        int const remainingSteps
    ) {
#if 1
        std::bitset<59+6+6+5> bits{};
        bits = state;
        bits << 5;
        bits |= remainingSteps;
        bits <<= 6;
        bits |= sRoomIndex;
        bits <<= 6;
        bits |= fRoomIndex;
        return std::hash<decltype(bits)>{}(bits) % indicesCapacity;
#else
        return unsigned(
            uint64_t(state ^ (uint64_t(fRoomIndex) << 5) ^ remainingSteps ^ (uint64_t(sRoomIndex) << 11))
            % indicesCapacity
        );
#endif
    }

    int getPressure(
        uint64_t const state, 
        int const fRoomIndex,
        int const sRoomIndex,
        int const remainingSteps,
        size_t const hash
    ) const {
        accessCount++;
        auto index = indicesO[hash] - 1;
        if(index == invalidIndex) return -1;
        while(true) {
            auto const &node = nodes[index];
            auto const count = node.data[7] >> 61;

            for(int i = 0; i < count; i++) {
                auto const data = node.data[i];
                auto const curState = data >> 5;
                auto const curRemSteps = data & 0b1'1111;
                auto const curFRoomIndex = (node.data[6] >> (8*i)) & 0b11'1111; 
                auto const curSRoomIndex = (node.data[7] >> (8*i)) & 0b11'1111; 

                if(curState != state) continue;
                if(curRemSteps != remainingSteps) continue;
                if(curFRoomIndex != fRoomIndex) continue;
                if(curSRoomIndex != sRoomIndex) continue;
                
                successCount++;
                return pressures[index*8 + i];
            }

            auto const nextNodeIndex = getNextNodeIndex(node);
            if(nextNodeIndex == 0) return -2;
            index = nextNodeIndex;
        }
    }

    static unsigned getNextNodeIndex(Node const & node) {
        return (node.data[6] >> (6*8)) | (((node.data[7] >> (6*8)) & 0b00011111'11111111) << 16);
    }

    void putPressure(
        int pressure,
        uint64_t const state, 
        int const fRoomIndex,
        int const sRoomIndex,
        int const remainingSteps,
        size_t hash
    ) {
        assert(state <= (1ull << 59) -1);
        assert(fRoomIndex >= 0 & fRoomIndex <= 59);
        assert(sRoomIndex >= 0 & sRoomIndex <= 59);
        assert(remainingSteps >= 0 & remainingSteps <= 30);
        assert(pressure >= 0);
        auto index = indicesO[hash] - 1u;
        auto curChainLen = 1;
        entriesCount++;
        if(index == invalidIndex) {
            if(nodesCount == nodesCapacity) { didntPut++; return; }
            indicesO[hash] = 1 + (index = nodesCount++);
            curChainLen = 0;
        }
        while(true) {
            auto &node = nodes[index];
            auto const count = node.data[7] >> 61;

            if(count == 6) {
                auto nextNodeIndex = getNextNodeIndex(node);
                if(nextNodeIndex != 0) {
                    index = nextNodeIndex;
                    curChainLen++;
                    continue;
                }
                if(nodesCount == nodesCapacity) { didntPut++; return; }
                nextNodeIndex = index = nodesCount++;
                node.data[6] |= uint64_t(nextNodeIndex) << (6*8);
                nextNodeIndex = (nextNodeIndex >> 16) & 0b11111'11111111;
                node.data[7] |= uint64_t(nextNodeIndex) << (6*8);
                assert(getNextNodeIndex(node) == index);
                curChainLen++;
                continue;
            }
            
            node.data[count] |= (state << 5) | remainingSteps;
            node.data[6] |= uint64_t(fRoomIndex) << (8*count);
            node.data[7] &= ~(uint64_t(0b111) << 61);
            node.data[7] |= (uint64_t(sRoomIndex) << (8*count)) | (uint64_t(count+1) << 61);
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

static Room const *rooms;
static int count;
static Map map;
static uint8_t lastPathF[32];
static uint8_t lastPathS[32];

struct MaxPressure {
    static int updateSecondRoom(
        uint8_t const firstRoom, uint8_t curRoom,
        int const lastEntriesF, int const lastEntriesS,
        uint64_t const curState, uint8_t const stepsRemaining
    ) {
        auto const &room = rooms[curRoom];
        auto const connectedCount = room.connectedRooms.count;
        auto const connected = room.connectedRooms.data;
        auto const rate = room.rate;

        auto const hash = decltype(map)::calcHash(curState, firstRoom, curRoom, stepsRemaining);
        auto const pressureCand = map.getPressure(curState, firstRoom, curRoom, stepsRemaining, hash);
        if(pressureCand >= 0) return pressureCand;

        int max;
        //turn valve in current room
        if(rate != 0 & ((curState >> curRoom)&1) == 0) {
            max = maxPressure(
                firstRoom, curRoom,
                curState | (uint64_t(1) << curRoom), stepsRemaining-1, 
                lastEntriesF, 0
            ) + rate * (stepsRemaining-1);
        }
        else max = 0;

        for(int nextRoomI = 0; nextRoomI < connectedCount; nextRoomI++) {
            auto const nextRoom = connected[nextRoomI];

            auto alreadyVisited = false;
            for(auto i = 0; i < lastEntriesS; i++) {
                if(lastPathS[i] == nextRoom) {
                    alreadyVisited = true;
                    break;
                }
            }
            if(alreadyVisited) continue;

            auto const maxCand = maxPressure(
                firstRoom, connected[nextRoomI], 
                curState, stepsRemaining-1, 
                lastEntriesF, lastEntriesS
            );
            if(maxCand > max) max = maxCand;
        }
        
        if(max != 0) map.putPressure(max, curState, firstRoom, curRoom, stepsRemaining, hash);

        return max;
    };

    static int maxPressure(
        uint8_t const curRoomF, uint8_t const curRoomS, 
        uint64_t const curState, uint8_t const stepsRemaining, 
        uint8_t const lastPathEntriesF, uint8_t const lastPathEntriesS
    ) {
        if(stepsRemaining == 0) return 0;

        auto const hash = decltype(map)::calcHash(curState, curRoomF, curRoomS, stepsRemaining);
        auto const pressureCand = map.getPressure(curState, curRoomF, curRoomS, stepsRemaining, hash);
        if(pressureCand >= 0) return pressureCand;

        auto const prevLastEntryF = lastPathF[lastPathEntriesF];
        lastPathF[lastPathEntriesF] = curRoomF;
        auto const prevLastEntryS = lastPathS[lastPathEntriesS];
        lastPathS[lastPathEntriesS] = curRoomS;


        auto const curRoom = curRoomF;
        auto const &room = rooms[curRoom];
        auto const connectedCount = room.connectedRooms.count;
        auto const connected = room.connectedRooms.data;
        auto const rate = room.rate;

        int max;
        //turn valve in current room
        if(rate != 0 & ((curState >> curRoom)&1) == 0) {
            max = updateSecondRoom(
                curRoom, curRoomS,
                0, lastPathEntriesS,
                curState | (uint64_t(1) << curRoom),
                stepsRemaining
            ) + rate * (stepsRemaining-1);
        }
        else max = 0;

        for(int nextRoomI = 0; nextRoomI < connectedCount; nextRoomI++) {
            auto const nextRoom = connected[nextRoomI];

            auto alreadyVisited = false;
            for(auto i = 0; i < lastPathEntriesF; i++) {
                if(lastPathF[i] == nextRoom) {
                    alreadyVisited = true;
                    break;
                }
            }
            if(alreadyVisited) continue;

            auto const maxCand = updateSecondRoom(
                nextRoom, curRoomS,
                lastPathEntriesF+1, lastPathEntriesS,
                curState, stepsRemaining
            );
            if(maxCand > max) max = maxCand;
        }

        iterations++;
        if(iterations % (65536*8) == 0) std::cout << "iter: " << iterations << '\n';

        if(max != 0) map.putPressure(max, curState, curRoomF, curRoomS, stepsRemaining, hash);
        lastPathF[lastPathEntriesF] = prevLastEntryF;
        lastPathS[lastPathEntriesS] = prevLastEntryS;
        return max;
    }

    static int maxPressure(Room const *const _rooms, int const _count, uint8_t startingRoom) {
        assert(_count <= 59);

        rooms = _rooms;
        count = _count;

        return maxPressure(
            startingRoom, startingRoom, 
            uint64_t(0), 26, 
            0, 0
        );
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
   
    auto map = Map{};
    for(int iter = 0; iter < 100'000; ++iter) {
        auto const s = state(gen);
        auto const i = roomIndex(gen);
        auto const i2 = roomIndex(gen);
        auto const r = remainingSteps(gen);
        auto const p = pressure(gen);
        auto const hash = Map::calcHash(s, i, i2, r);
        
        map.putPressure(p, s, i, i2, r, hash);
        auto const p2 = map.getPressure(s, i, i2, r, hash);

        if(p != p2) {
            printf("iteration %d:\n\tstate = 0x", iter);
            for(int j = 0; j < 64; j++) {
                std::cout << ((s >> (64-j-1)) & 1);
                if(j != 0 & j % 8 == 0) std::cout << '\'';
            }
            std::cout << "\n\tindex = " << (int) i;
            std::cout << "\n\tindex2 = " << (int) i2;
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
    std::cout << "maxPressure(" << time << "ms, " << iterations << " iters): " << mp << '\n';
    std::cout << "map max chain is " << map.maxChain << " steps,\nused "
        << map.entriesCount << " entries in "
        << map.nodesCount << " nodes.\n";
    std::cout << "accessed " << map.accessCount << " times, " << map.successCount << " successfully\n";
    std::cout << "didn't fin in cache: " << map.didntPut << " entries\n";

    return 0;
#endif
}
