#include<iostream>
#include<fstream>

int readNumber(std::istream &stream) {
    bool sign = false;
    int number = 0;
    for(int input; (input = stream.get()) && ((input >= '0' & input <= '9') | input == '-');) {
        if(input == '-') sign = !sign;
        else number = number*10 + (input-'0');
    }
    return number * (sign ? -1 : 1);
}

int main() {
    auto str = std::ifstream{"p1.txt"};

    //cycle and register start with 1, but the display is 0-indexed
    auto cycle = 1;
    auto x = 1;
    auto const nextCycle = [&]() {
        if(cycle != 1 & (cycle-1) % 40 == 0) {
            std::cout << '\n';
        }
        auto curX = (cycle - 1) % 40;
        if(curX >= x-1 & curX <= x+1) std::cout << '#';
        else std::cout << ' ';
        if(cycle == 240) exit(0);
        cycle++;
    };

    while(str.peek() != std::char_traits<char>::eof()) {
        auto const c = str.get();
        
        if(c == 'n') {
            str.ignore(4);
            nextCycle();
        }
        else {
            str.ignore(4);
            auto const n = readNumber(str);
            nextCycle();
            nextCycle();
            x += n;
        }
    }

    return 0;
}
