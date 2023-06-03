#include<iostream>
#include<fstream>

int calcPos(int input) {
    if(input >= 'a' && input <= 'z') return input - 'a';
    else return ('z'+1 - 'a') + (input - 'A');
}


int main() {
    auto stream = std::ifstream{"part1.txt"};


    int sumOfPositions = 0;
    while(stream.peek() != std::char_traits<char>::eof()) {
        bool presentInC1[52] = {};

        auto const lineStartPos = stream.tellg();
        while(!stream.eof() && stream.get() != '\n');
        auto const lineEndPos = stream.tellg();
        auto const middle = (lineEndPos - lineStartPos) >> 1;
        stream.seekg(lineStartPos, stream.beg);

        while(stream.tellg() != lineStartPos + middle) {
            auto const input = stream.get(); 
            auto const pos = calcPos(input);
            presentInC1[pos] = true;
        }
 
        int posOfPresentInC2 = -1;
        while(stream.tellg() != lineEndPos) {
            auto const input = stream.get();
            auto const position = calcPos(input);
            if(presentInC1[position]) {
                posOfPresentInC2 = position;
                break;
            }
        }
        stream.seekg(lineEndPos, stream.beg);

        sumOfPositions += 1 + posOfPresentInC2;
    }

    std::cout << sumOfPositions << '\n';

    return 0;
}
