#include<iostream>
#include<fstream>
#include<algorithm>
#include<stdint.h>
#include<cassert>
#include<chrono>

#include"map.h"

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

    void compact() { data = (T*)realloc(data, cap = count); }
};

struct Room {
    Vector<int> connectedRooms;
    int rate;
    int roomsIndex;
    int origOffset;
};

static std::ostream &operator<<(std::ostream &o, Room const &it) {
    o << "Room{ rate=" << it.rate << ", connectedRooms={ ";
    for(int i = 0; i < it.connectedRooms.count; i++) {
        if(i != 0) o << ", ";
        o << it.connectedRooms.data[i];
    }
    o << " }, origOffset=" << it.origOffset << ", roomsIndex=" << it.roomsIndex << " }";
    return o;
}

static int iterations = 0;


static constexpr uint32_t roundUpPow2(uint32_t value) {
    auto const v = value - 1;
    return 1 + (v | (v >> 1) | (v >> 2)
        | (v >> 4) | (v >> 8) | (v >> 16));
}

using room_t = uint8_t;

template<int count_>
struct Rooms {
    static constexpr auto count = count_;
    static_assert(count < 256);

    static constexpr auto c = roundUpPow2(count);
    uint8_t connectedRooms[count*c];
    uint8_t stepsToConnected[count*c];
    int rate[count];
    int origIndices[count];

    template<typename Iter>
    void iterRooms(room_t const r, Iter &&iter) const {
        auto const conn = getConnectedRooms(r);
        for(int i = 0;; i++) {
            auto const nextI = *(conn + i);
            if(nextI == uint8_t{}) break; //at least 1 slot is empty, bc the room cannot connect to itself
            iter(nextI - 1);
        }
    }

    auto getConnectedRooms(room_t const r) const {
        return &connectedRooms[r * c];
    }

    auto getConnectedRooms(room_t const r) {
        return &connectedRooms[r * c];
    }

    auto getStepsToConnectedFor(room_t const r) {
        return &stepsToConnected[r * c];
    }
    void printName(room_t const room) const {
        auto const i = origIndices[room];
        std::cout << char(i/26 + 'A') << char(i%26 + 'A');
    }
};

static Rooms<16> rooms;
static Map<int> map(-1);
static uint8_t lastPaths[2][32];

struct MaxPressure {
    struct Data {
        uint8_t curRoom;
        int8_t stepsRem;
        uint8_t lastPathC, lastPathIndex;
    };

    static int maxPressure(
        uint16_t const curState, 
        Data df, Data ds
    ) {
        if(df.stepsRem <= 0 & df.stepsRem <= 0) return 0;

        //df must have more time than ds
        if(df.stepsRem < ds.stepsRem) std::swap(df, ds);
        
        auto const hash = decltype(map)::calcHash(curState, df.curRoom, ds.curRoom, df.stepsRem, ds.stepsRem);
        auto const pState = decltype(map)::packState(curState, df.curRoom, ds.curRoom, df.stepsRem, ds.stepsRem); 
        auto const mapCand = map.get(pState, hash);
        if(mapCand >= 0) return mapCand;

        auto const lastPathF = lastPaths[df.lastPathIndex];
        //auto const prevLastEntryF = lastPathF[df.lastPathC];
        lastPathF[df.lastPathC] = df.curRoom;
        
        int max;

        //turn valve in current room
        if(((curState >> df.curRoom) & 1) == 0) {
            auto newDf = df;
            newDf.lastPathC++;
            newDf.stepsRem = df.stepsRem - 1;

            max = maxPressure(
                curState | (uint16_t(1) << df.curRoom),
                newDf, ds
            ) + rooms.rate[df.curRoom] * newDf.stepsRem;
        }
        else max = 0;

        auto const nextRooms = rooms.getConnectedRooms(df.curRoom);
        auto const stepsToRooms = rooms.getStepsToConnectedFor(df.curRoom);
        for(int i = 0;; i++) {
            auto const nextRoomOffI = nextRooms[i];
            if(nextRoomOffI == 0) break;

            auto alreadyVisited = false;
            for(auto i = 0; i < df.lastPathC; i++) {
                if(lastPathF[i] == nextRoomOffI-1) {
                    alreadyVisited = true;
                    break;
                }
            }
            if(alreadyVisited) continue;

            auto newDf = df;
            newDf.lastPathC++;
            newDf.stepsRem = df.stepsRem - stepsToRooms[i];
            newDf.curRoom = nextRoomOffI-1;

            auto const maxCand = maxPressure(
                curState, newDf, ds
            );
            if(maxCand > max) max = maxCand;
        }

        iterations++;
        if(iterations % (65536*8) == 0) std::cout << "iter: " << iterations << '\n';

        if(max != 0) map.put(max, pState, hash);
        //lastPathF[df.lastPathC] = prevLastEntryF;
        return max;
    }

    static int maxPressure(uint8_t startingRoom) {
        auto state = uint16_t{};
        if(rooms.rate[startingRoom] == 0) state = 1u << startingRoom;

        //uint8_t curRoom, stepsRem, lastPathC, lastPathIndex;
        return maxPressure(
            uint64_t(0), 
            Data{ startingRoom, 26, 0, 0 },
            Data{ startingRoom, 26, 0, 1 }
        );
    }
};

#if 1

static int connectedRoomsRoomsArrPath[64];
static int connectedRoomsPathC = 0;
static void fillConnectedRooms(
    Room const *const roomsArr,
    int const roomCount,
    room_t const startRoomRoomsI,
    int const curRoomArrI,
    int const steps
) {
    assert(64 >= roomCount);
    for(int i = 0; i < connectedRoomsPathC; i++) {
        if(connectedRoomsRoomsArrPath[i] == curRoomArrI) return;
    }
    if(steps >= roomCount) return;

    auto const &curRoom = roomsArr[curRoomArrI];
    for(int i = 0; i < curRoom.connectedRooms.count; i++) {
        auto const nextRoomArrI = curRoom.connectedRooms.data[i];
        auto const &nextRoom = roomsArr[nextRoomArrI];

        connectedRoomsRoomsArrPath[connectedRoomsPathC++] = curRoomArrI;
        fillConnectedRooms(
            roomsArr, roomCount,
            startRoomRoomsI, nextRoomArrI,
            steps + 1
        );
        connectedRoomsPathC--;

        if(nextRoom.rate == 0) continue; 

        auto const nextRoomRoomsI = nextRoom.roomsIndex;
        if(nextRoomRoomsI == startRoomRoomsI) continue;
        
        auto const r = rooms.getConnectedRooms(startRoomRoomsI);
        auto const s = rooms.getStepsToConnectedFor(startRoomRoomsI);
        for(int j = 0;; j++) {
            if(r[j] == nextRoomRoomsI + 1) {
                if(s[j] > steps + 1) {
                    r[j] = nextRoomRoomsI + 1;
                    s[j] = steps + 1;
                }
                break;
            }
            else if(r[j] == 0) {
                r[j] = nextRoomRoomsI + 1;
                s[j] = steps + 1;
                break;
            }
        }
    }
}

static void fillConnectedRooms(
    Room const *const roomsArr,
    int const roomCount,
    room_t const startRoomRoomsI,
    int const curRoomArrI
) {
    fillConnectedRooms(roomsArr, roomCount, startRoomRoomsI, curRoomArrI, 0);
    assert(connectedRoomsPathC == 0);
}

int main() {
    static constexpr auto letters = 26;

    auto str = std::ifstream{ "p1.txt" };

    auto roomsArr = new Room[letters*letters];
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
        auto const index = ((rs - 'A')*letters) + (rf - 'A');
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

        roomsArr[offset] = { otherRooms, rate, 0, index };
    }
    
    int roomsRoomArrI[decltype(rooms)::count];
    auto roomsRoomsC = 0;
    auto startRoom  = -1;
    for(int i = 0; i < roomCount; i++) {
        if(roomsArr[i].rate != 0 | roomsArr[i].origOffset == 0) {
            if(roomsArr[i].origOffset == 0) startRoom = roomsRoomsC;
            rooms.rate[roomsRoomsC] = roomsArr[i].rate;
            rooms.origIndices[roomsRoomsC] = roomsArr[i].origOffset;
            roomsRoomArrI[roomsRoomsC] = i;
            roomsArr[i].roomsIndex = roomsRoomsC;
            roomsRoomsC++;
        }
    }
    assert(roomsRoomsC <= decltype(rooms)::count);

    for(int i = 0; i < roomsRoomsC; i++) {
        fillConnectedRooms(
            roomsArr, roomCount,
            i, roomsRoomArrI[i]
        );
    }

    std::cout << "Rooms: " << roomsRoomsC << '\n';

    auto const start = std::chrono::high_resolution_clock::now();
        auto const mp = MaxPressure::maxPressure(startRoom);
    auto const end = std::chrono::high_resolution_clock::now();

    auto const time = (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);
    std::cout << "maxPressure(" << time << "ms, " << iterations << " iters): " << mp << '\n';
    std::cout << "map max chain is " << map.maxChain << " steps,\nused "
        << map.entriesCount << " entries in "
        << map.nodesCount << " nodes.\n";
    std::cout << "accessed " << map.accessCount << " times, " << map.successCount << " successfully\n";
    std::cout << "didn't fin in cache: " << map.didntPut << " entries\n";

    return 0;
    /*std::cout << "rooms: " << roomCount << '\n';
    for(int i = 0; i < roomCount; i++) {
        std::cout << "#" << i << " = " << roomsArr[i] << '\n';
    }
    std::cout << "------------------------------------------\n";


    for(int i = 0; i < roomsRoomsC; i++) {
        auto const steps = rooms.getStepsToConnectedFor(i);
        auto const conn = rooms.getConnectedRooms(i);
        
        std::cout << "Room ";
        rooms.printName(i);
        std::cout << ":\n";

        for(int i = 0;; i++) {
            auto const nextI = *(conn + i);
            if(nextI == uint8_t{}) break; //at least 1 slot is empty, bc the room cannot connect to itself

            std::cout << "  neighbour ";
            rooms.printName(nextI - 1);
            std::cout << " - " << (int) steps[i] << '\n';
        }
    }*/

    return 0;

    /*std::cout << "rooms: " << roomCount << '\n';
    for(int i = 0; i < roomCount; i++) {
        std::cout << "#" << i << " = " << roomsArr[i] << '\n';
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
    */

    return 0;
}
#else

#include<random>

int main() {
    std::mt19937 gen{ 7079 };
    std::uniform_int_distribution<uint16_t> state(0, (1ull << 14) - 1);
    std::uniform_int_distribution<uint8_t> roomIndex(0, 14);
    std::uniform_int_distribution<uint8_t> remainingSteps(0, 30);
    std::uniform_int_distribution<int> pressure(0);
   
    auto map = Map{};
    for(int iter = 0; iter < 100'000; ++iter) {
        auto const s = state(gen);
        auto const i = roomIndex(gen);
        auto const i2 = roomIndex(gen);
        auto const r = remainingSteps(gen);
        auto const r2 = remainingSteps(gen);
        auto g2 = std::mt19937(s + i + i2 + r + r2);
        auto const p = pressure(g2);
        auto const hash = Map::calcHash(s, i, i2, r, r2);
        auto const pState = Map::packState(s, i, i2, r, r2);
        
        map.putPressure(p, pState, hash);
        auto const p2 = map.getPressure(pState, hash);

        if(p != p2) {
            printf("iteration %d:\n\tstate = 0x", iter);
            for(int j = 0; j < 14; j++) {
                std::cout << ((s >> (14-j-1)) & 1);
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
}
#endif
