#include<iostream>
#include<fstream>
#include<stdint.h>

int main() {
    auto stream = std::ifstream{ "p1.txt" };

    auto rows = 0;
    auto cols = 0;
    while(stream.peek() != std::char_traits<char>::eof()) {
        auto const c = stream.get();
        if(c == '\n') rows++;
        if(rows == 0) cols++;
    }
    
    if(rows <= 2 | cols <= 2) /*possible but unlikely*/ {
        return 1; 
    }

    auto const treesMatrix = new uint8_t[rows*cols];

    stream.clear();
    stream.seekg(0, stream.beg);
    for(auto i = 0; i < rows*cols; i++) {
        if(i != 0 & i % cols == 0) stream.ignore(1);
        treesMatrix[i] = stream.get() - '0';
    }
    stream.close();

    /*for(int i = 0; i < rows*cols; i++) {
        if(i!=0 & i % cols == 0) std::cout << '\n';
        std::cout << (int)treesMatrix[i];
    }*/

    constexpr int dirsC = 4;
    constexpr int dirs[] = { -1,0, 0,-1, 1,0, 0,1 };

    auto highestScore = 0;
    for(auto r = 1; r < rows-1; r++)
    for(auto c = 1; c < cols-1; c++) {
        auto const cur = treesMatrix[r*cols + c];
        
        auto score = 1;
        for(auto i = 0; i < dirsC; i++) {
            auto const ro = dirs[i*2 + 0];
            auto const co = dirs[i*2 + 1];

            auto cr = r + ro;
            auto cc = c + co;
            auto dist = 0;
            while(cc >= 0 & cc < cols & cr >= 0 & cr < rows) {
                dist++;
                if(treesMatrix[cr*cols + cc] >= cur) break;
                cr += ro;
                cc += co;
            }
            score *= dist;
        }
        if(highestScore < score) highestScore = score;
    }

    std::cout << highestScore << '\n';

    return 0;
}
