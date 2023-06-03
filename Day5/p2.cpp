#include<algorithm>
#include<iostream>
#include<fstream>

static int readNumber(std::istream &stream) {
    int number = 0;
    for(int input; (input = stream.get()) && input >= '0' && input <= '9';) {
        number = number*10 + (input-'0');
    }
    return number;
}

int main() {
    auto stacksS = std::ifstream{"p1_stack.txt"};
    auto stepsS = std::ifstream{"p1_instructions.txt"};

    auto stacksInitDepth = 0;
    auto stacksCount = 0;
    auto elementsCount = 0;
    while(stacksS.peek() != std::char_traits<char>::eof()) {
        if(stacksInitDepth == 0) stacksCount++;
        if(stacksS.get() == '[') elementsCount++;
        stacksS.ignore(2);
        if(stacksS.get() != ' ') stacksInitDepth++;
    }
    stacksInitDepth--; //last row are stacks numbers and not stack elements

    stacksS.seekg(0, stacksS.beg);
    stacksS.clear();

    int  *stacksCounts = new int[stacksCount]{};
    char *stacks = new char[stacksCount * elementsCount]{};
    auto const stack = [&](int stack) -> char* {
        return stacks + elementsCount*stack;
    };

    { //populate stacks
        for(int i = stacksInitDepth-1; i >= 0; i--) {
            for(int j = 0; j < stacksCount; j++) {
                if(stacksS.get() == '[') {
                    stack(j)[i] = stacksS.get();
                    stacksCounts[j] = std::max(stacksCounts[j], i+1);
                }
                else stacksS.ignore(1);
                stacksS.ignore(2);
            }
        }
    }
    
    /*std::cout << "!\n";
    for(int i = 0; i < stacksInitDepth; i++) {
        for(int j = 0; j < stacksCount && ((std::cout << ' '), true); j++) {
            if(stack(j)[i] != 0) std::cout << '[' << stack(j)[i] << ']';
            else std::cout << "   ";
        }
        std::cout << '\n';
    }*/

    while(stepsS.peek() != std::char_traits<char>::eof()) {
        stepsS.ignore(5);
        auto const count = readNumber(stepsS);
        stepsS.ignore(5);
        auto const from = readNumber(stepsS) - 1;
        stepsS.ignore(3);
        auto const to = readNumber(stepsS) - 1;

        auto &fromC = stacksCounts[from];
        auto &toC = stacksCounts[to];
        for(int i = 0; i < count; i++) { 
            stack(to)[toC + i] = stack(from)[fromC-count + i];
        }
        fromC -= count;
        toC += count;
    }

    for(int i = 0; i < elementsCount; i++) {
        bool printed = false;
        for(int j = 0; j < stacksCount && ((std::cout << ' '), true); j++) {
            if(stacksCounts[j] > i & stack(j)[i] != '0') {
                std::cout << '[' << stack(j)[i] << ']';
                printed = true;
            }
            else std::cout << "   ";
        }
        std::cout << '\n';
        if(!printed) break;
    }

    for(int i = 0; i < stacksCount; i++) std::cout << (stack(i)[stacksCounts[i]-1]);
    std::cout << '\n';
    
    return 0;
}
