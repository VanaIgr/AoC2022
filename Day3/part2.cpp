#include<iostream>
#include<fstream>

int calcPos(int input) {
    if(input >= 'a' && input <= 'z') return input - 'a';
    else return ('z'+1 - 'a') + (input - 'A');
}


int main() {
    auto stream = std::ifstream{"part1.txt"};

    constexpr int countInGroup = 3;
    int curInGroup = 0;
    unsigned char presentInGroup[52]; //count of present poisitons in a group

    int sumOfPositions = 0;
    while(stream.peek() != std::char_traits<char>::eof()) {
        if(curInGroup == 0) for(int i = 0; i < 52; i++) presentInGroup[i] = 0;
        
        int posPresentInEntireGroup = -1;
        for(int input; (input = stream.get()) && input != '\n';) {
            auto const pos = calcPos(input);
            if(presentInGroup[pos] == curInGroup) {
                if(curInGroup == countInGroup-1) {
                    posPresentInEntireGroup = pos;
                }
                else presentInGroup[pos]++;
            }
        }
    
        if(++curInGroup == countInGroup) {
            sumOfPositions += 1 + posPresentInEntireGroup;
            curInGroup = 0;
        }
    }

    std::cout << sumOfPositions << '\n';

    return 0;
}
