#include<iostream>
#include<fstream>

int getNextInput(std::istream &s) {
    int input;
    while((input = s.get()) == 13);
    return input;
}

int main() {
    auto &&stream = std::ifstream{"pract_input.txt"};
    int maxTotal = -1, curTotal = 0, curNumber = 0;
    auto input = getNextInput(stream);
    for(;;) {
        if(input == -1) {
            curTotal += curNumber;
            std::cout << (
                curTotal > maxTotal 
                ? curTotal
                : maxTotal
            );
            return 0;
        }
        else {
            auto const c = char(input);
            auto const nextInput = getNextInput(stream);
            if(c == '\n') {
                curTotal += curNumber;
                curNumber = 0;

                if(nextInput == '\n') {
                    if(curTotal > maxTotal) {
                        maxTotal = curTotal;
                    }
                    curTotal = 0;
                }
            }
            else if(c >= '0' && c <= '9') {
                curNumber = curNumber*10 + (c - '0');
            }

            input = nextInput;
        }
    }
}

