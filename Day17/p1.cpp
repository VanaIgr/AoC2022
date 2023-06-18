#include<iostream>
#include<fstream>
#include<stdint.h>
#include<cassert>
#include<algorithm>
#include<cstring>

static constexpr auto rocksC = 2022;
static constexpr auto tallestRock = 4;
static constexpr auto maxHeight = rocksC * tallestRock + (tallestRock + 3) + 10/*don't die if I made off by 1 error*/;
static constexpr auto width = 7;

using pattern_t = uint16_t; //4*4 rock, bits start from bottom-left
static constexpr auto rockSize = 4;
static constexpr auto rockMask = (1 << rockSize) - 1;

struct Chamber {
    static constexpr auto memcpyOffset = 1;
    uint8_t *data;
    int height;

    Chamber() 
        : data{ new uint8_t[memcpyOffset + (maxHeight*width + 8-1) / 8] },
        height{}
    {}

    bool occupiedAt(int const leftX, int const botY, pattern_t const p) {
        if(leftX < 0 | botY < 0) return true;
        int widthToCheck = std::min(rockSize, width-1 - rockSize)/*
            everything else is out of bounds, and pattern must be empty there
        */;

        for(int yo = 0; yo < 4; yo++) {
            auto const startI = (botY+yo)*width + leftX;
            auto const blockI = unsigned(startI) / 8;
            auto const offset = startI - blockI*8;

            uint16_t region;
            static_assert(16 >= width*2 & memcpyOffset >= 1);
            std::memcpy(&region, &data[blockI], 2);

            auto const remCellsMask = (1u << std::max(width - leftX, 0)) - 1;
            auto const curRow = (region >> offset) & remCellsMask;
            auto const curP   = (p >> (4*yo)) & rockMask &  remCellsMask;
            auto const restP  = (p >> (4*yo)) & rockMask & ~remCellsMask;

            if((curRow & curP) != 0 | restP != 0) return true;
        }

        return false;
    }

    void occupyAt(int const leftX, int const botY, pattern_t const p) {
        assert(leftX >= 0 & botY >= 0);

        for(int yo = 0; yo < 4; yo++) {
            auto const startI = (botY+yo)*width + leftX;
            auto const blockI = unsigned(startI) / 8;
            auto const offset = startI - blockI*8;

            auto const remCellsMask = (1u << std::max(width - leftX, 0)) - 1;
            auto const curP  = (p >> (rockSize*yo)) & rockMask;
            auto const restP = (p >> (rockSize*yo)) & rockMask & ~remCellsMask;
            assert(restP == 0);

            if(curP != 0) {
                uint16_t region;
                static_assert(16 >= width*2 & memcpyOffset >= 1);
                std::memcpy(&region, &data[blockI], 2);
                region |= curP << offset;

                std::memcpy(&data[blockI], &region, 2);

                height = std::max(height, botY + yo + 1);
            }
        }
    }

    friend std::ostream &operator<<(std::ostream &o, Chamber const &it) {
        for(int y = it.height; y >= 0; y--) {
            auto const startI = y*width;
            auto const blockI = unsigned(startI) / 8;
            auto const offset = startI - blockI*8;

            uint16_t region;
            static_assert(16 >= width*2 & memcpyOffset >= 1);
            std::memcpy(&region, &it.data[blockI], 2);

            o << '|';
            for(int x = 0; x < width; x++) {
                o << (
                    ((region >> (offset+x)) & 1)
                    ? '#' : '.'
                );
            }
            o << '|';
            o << '\n';
        }

        o << '+';
        for(int x = 0; x < width; x++) {
            o << '-';
        }
        o << '+';

        return o;
    }
};
    
static constexpr auto patternsC = 5;
static constexpr pattern_t patterns[] = {
    0b0000'0000'0000'1111u,
    0b0000'0010'0111'0010u,
    0b0000'0100'0100'0111u,
    0b0001'0001'0001'0001u,
    0b0000'0000'0011'0011u
};

int main() {
    auto str = std::ifstream{ "p1.txt" };
    str.seekg(0, str.end);
    auto const strC_ = str.tellg();
    str.seekg(0, str.beg);
    assert(strC_ != decltype(strC_){-1});
    int strC = strC_;

    //left is default
    auto const dirs = new uint64_t[(strC+64-1) / 64]{}; 
    auto dirC = 0;
    auto const setDirRight = [&](unsigned const i) {
        dirs[i/64] |= uint64_t{1} << (i%64);
    };
    auto const isRight = [&](unsigned const i) -> bool { return (dirs[i/64] >> (i%64)) & 1; };

    while(true) {
        auto const c = str.get();
        if(c == '>') setDirRight(dirC);
        else if(c != '<') break;
        dirC++;
    }

    /*
    for(int i = 0; i < dirC; i++) {
        std::cout << (isRight(i) ? '>' : '<');
    }
    std::cout << '\n';
    */

    Chamber chamber{};

    auto curPatternI = 0ull;
    auto curDirI = 0ull;
    for(int i = 0; i < rocksC; i++) {
        auto const p = patterns[curPatternI % patternsC];
        auto x = 2;
        auto y = chamber.height + 3;

        while(true) {
            auto const curDir = isRight(curDirI % dirC) ? 1 : -1;
            curDirI++;

            if(!chamber.occupiedAt(x + curDir, y, p)) {
                x = x + curDir;
            }

            if(!chamber.occupiedAt(x, y - 1, p)) {
                y = y -1;
            }
            else {
                chamber.occupyAt(x, y, p);
                break;
            }
        }

        curPatternI++;
    }

    std::cout << "height: " << chamber.height << '\n';
    //std::cout << chamber << '\n';

    return 0;
}
