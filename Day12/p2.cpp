#include<iostream>
#include<fstream>
#include<stdint.h>

struct Position {
    int x, y;
};

int main() {
    auto str = std::ifstream{ "p1.txt" };

    int width = 0;
    int height = 0;
    while(str.peek() != std::char_traits<char>::eof()) {
        auto const c = str.get();
        if(c == '\n') height++;
        if(height == 0) width++;
    }
    str.clear();
    str.seekg(0, str.beg);


    auto const size = width * height;
    auto const values = new uint8_t[size];
    auto const stepsToGet = new uint16_t[size];
    auto startingPositions = new Position[size];
    auto startingPositionsC = 0;
    auto posToTest = new Position[size];
    auto posToTestNext = new Position[size];
    auto posToTestC = 0;
    auto posToTestNextC = 0;

    for(int i = 0; i < size; i++) stepsToGet[i] = 0xFFFF;

    auto curFillI = 0;
    while(str.peek() != std::char_traits<char>::eof()) {
        auto const c = str.get();
        if(c == '\n') continue;
        
        auto &v = values[curFillI];
        if(c == 'a' | c == 'S') {
            startingPositions[startingPositionsC++] = Position{ curFillI % width, curFillI / width };
            v = 0;
        }
        else if(c > 'a' && c <= 'z') v = c - 'a';
        else if(c == 'E') {
            posToTest[posToTestC++] = Position{ curFillI % width, curFillI / width };
            stepsToGet[curFillI] = 0;
            v = 'z' - 'a';
        }
        else exit(100);

        curFillI++;
    }

    /*for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
        auto const i = y*width + x;
        std::cout << (char)(values[i] == endValue ? 'E' : values[i] + 'a');
    } std::cout << '\n';
    }*/

    static constexpr int offsetsC = 4;
    static constexpr Position offsets[] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };

    while(posToTestC != 0) {
        /*for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                std::cout << stepsToGet[y*width+x] << '\t';
            }
            std::cout << '\n';
        }
        std::cout << '\n';*/

        for(int i = 0; i < posToTestC; i++) {
            auto const pos = posToTest[i];
            auto const v = values[pos.y*width + pos.x];
            auto const steps = stepsToGet[pos.y*width + pos.x];
           
            for(int oi = 0; oi < offsetsC; oi++) {
                auto const o = offsets[oi];
                auto const np = Position{ pos.x + o.x, pos.y + o.y };
                auto const ni = np.y*width + np.x; 
                if(np.x < 0 | np.y < 0 | np.x >= width | np.y >= height) continue;
                
                auto const nv = values[ni];
                if(nv+1 < v) continue;
                
                auto &ns = stepsToGet[ni];
                if(ns <= steps+1) continue;
                
                ns = steps+1;
                posToTestNext[posToTestNextC++] = np;
            }
        }

        std::swap(posToTest, posToTestNext);
        posToTestC = posToTestNextC;
        posToTestNextC = 0;
    }

    if(startingPositionsC <= 0) exit(150);
    auto begMinPos = startingPositions[0];
    auto minSteps = stepsToGet[begMinPos.y*width + begMinPos.x];
    for(int i = 0; i < startingPositionsC; i++) {
        auto const curPos = startingPositions[i];
        auto const ci = curPos.y*width + curPos.x;
        if(minSteps > stepsToGet[ci]) minSteps = stepsToGet[ci];
    }

    std::cout << minSteps << '\n';

    return 0;
}
