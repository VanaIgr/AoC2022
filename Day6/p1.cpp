#include<iostream>
#include<fstream>
#include<stdint.h>
#include<bit>

int main() {
    constexpr auto uniqueChars = 4;

    auto stream = std::ifstream{"p1.txt"};

    uint32_t window = 0; //32 bits for checking presence of the leters of the alphabet
    char prev[uniqueChars] = {};
    auto lastPrev = 0;
    stream.read(&prev[0], uniqueChars);
    if(!stream.good()) return 1;
    for(int i = 0; i < uniqueChars; i++) window ^= uint32_t(1) << (prev[i] - 'a');

    while(true) {
        auto const curI = stream.get();
        if(char(curI) != curI || !stream.good()) return 1;
        auto const cur = char(curI);

        window ^= uint32_t(1) << (prev[lastPrev] - 'a');
        window ^= uint32_t(1) << (cur - 'a');
        prev[lastPrev] = cur;
        lastPrev = (lastPrev+1) % uniqueChars;

        if(std::popcount(window) == uniqueChars) {
            std::cout << stream.tellg();
            return 0;
        }
    }
}
