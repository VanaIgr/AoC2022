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

    auto sum = 0;
    auto cycle = 1;
    auto x = 1;
    auto const nextCycle = [&]() {
        if(cycle >= 20 & cycle <= 220 & (cycle-20) % 40 == 0) {
            sum += x * cycle;
        }
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

    std::cout << sum << '\n';

    return 0;
}
