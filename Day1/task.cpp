#include<iostream>
#include<fstream>

int getNextInput(std::istream &s) {
    int input;
    while((input = s.get()) == 13);
    return input;
}

int main() {
    auto &&stream = std::ifstream{"input.txt"};
    int curTotal = 0, curNumber = 0;
    auto input = getNextInput(stream);

    constexpr int topn = 3;
    int maxTotals[topn];
    for(int i = 0; i < topn; i++) maxTotals[i] = -1;

    auto const updateTotals = [&]() {
        for(int i = 0; i < topn; i++) {
            if(curTotal > maxTotals[i]) {
                auto tmp = maxTotals[i];
                maxTotals[i] = curTotal;
                curTotal = tmp;
            }
        }
    };

    for(;;) {
        if(input == -1) {
            curTotal += curNumber;
            updateTotals();

            int sum = 0;
            for(int i = 0; i < topn; i++) {
                sum += maxTotals[i];
            }
            std::cout << sum << '\n';
            return 0;
        }
        else {
            auto const c = char(input);
            auto const nextInput = getNextInput(stream);
            if(c == '\n') {
                curTotal += curNumber;
                curNumber = 0;

                if(nextInput == '\n') {
                    updateTotals();
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
