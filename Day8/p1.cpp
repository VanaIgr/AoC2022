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
    stream.clear();
    stream.seekg(0, stream.beg);

    if(rows <= 2 | cols <= 2) /*possible but unlikely*/ {
        return 1; 
    }

    auto const treesMatrix = new uint8_t[rows*cols];
    auto const treesVisible = new bool[rows * cols]{};

    for(auto r = 0; r < rows; r++) treesVisible[r*cols] = treesVisible[r*cols + cols-1] = true;
    for(auto c = 0; c < cols; c++) treesVisible[c] = treesVisible[(rows-1) * cols + c] = true;

    for(auto i = 0; i < rows*cols; i++) {
        if(i != 0 & i % cols == 0) stream.ignore(1);
        treesMatrix[i] = stream.get() - '0';
    }
    stream.close();

    /*for(int r = 0; r < rows; r++) {
    for(int c = 0; c < cols; c++) {
        std::cout << (int) treesMatrix[r*cols + c];
    }
    std::cout << '\n';
    }*/

    for(auto dirI = 0; dirI < 2; dirI++) {
        auto const startCol = dirI ? cols-1 : 0;
        auto const dir = dirI ? -1 : 1;

        for(auto r = 1; r < rows-1; r++) {
            auto max = treesMatrix[r*cols + startCol];
            for(auto co = 1; co < cols-1; co++) {
                auto const index = r*cols + (startCol + co*dir);
                auto const curTree = treesMatrix[index];
                if(curTree > max) {
                    treesVisible[index] = true;
                    max = curTree;
                }
            }
        }
    }
 
    for(auto dirI = 0; dirI < 2; dirI++) {
        auto const startRow = dirI ? rows-1 : 0;
        auto const dir = dirI ? -1 : 1;

        for(auto c = 1; c < cols-1; c++) {
            auto max = treesMatrix[startRow*cols + c];
            for(auto ro = 1; ro < rows-1; ro++) {
                auto const index = (startRow + ro*dir)*cols + c;
                auto const curTree = treesMatrix[index];
                if(curTree > max) {
                    treesVisible[index] = true;
                    max = curTree;
                }
            }
        }
    }   

    /*for(int i = 0; i < rows*cols; i++) {
        if(i != 0 & i % cols == 0) std::cout << '\n';
        std::cout << treesVisible[i];
    }*/

    auto count = 0;
    for(int i = 0; i < rows*cols; i++) {
        if(treesVisible[i]) count++; 
    }

    std::cout << count << '\n';

    return 0;
}
