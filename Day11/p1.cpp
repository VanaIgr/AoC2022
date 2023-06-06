#include<iostream>
#include<fstream>

struct Item {
    int worry;
    Item *next;
};

enum Operation {
    plus, times
};

struct Monkey {
    Item *items;
    Operation op;
    int operand; //-1 for the same value
    int checkValue;
    int mIfTrue;
    int mIfFalse;
    int totalInspected;
    Monkey *next;
};

std::ostream &operator<<(std::ostream &o, Monkey const &it) {
    o << "Items: ";
    auto curItem = it.items;
    while(curItem) {
        std::cout << curItem->worry << ' ';
        curItem = curItem->next;
    }
    std::cout << '\n';

    std::cout << "Operation: " << (it.op == plus ? '+' : '*') << ' ' << it.operand << '\n';

    std::cout << "Test divisibility by: " << it.checkValue << '\n';
    std::cout << "  true: " << it.mIfTrue << '\n';
    std::cout << "  false: " << it.mIfFalse << '\n';

    return o;
}

int readNumber(std::istream &stream) {
    int number = 0;
    for(int input; (input = stream.peek()) && input >= '0' && input <= '9';) {
        number = number*10 + (stream.get()-'0');
    }
    return number;
}

Monkey &monkeyAt(Monkey *cur, int offset) {
    while(offset--) cur = cur->next;
    return *cur;
}

int main() {
    auto str = std::ifstream{ "p1.txt" };

    auto startMonkey = Monkey{};
    auto curMonkey = (Monkey*){};

    while(str.peek() != std::char_traits<char>::eof()) {
        if(curMonkey == nullptr) curMonkey = &startMonkey;
        else {
            auto const newMonkey = new Monkey{};
            curMonkey = curMonkey->next = newMonkey;
        }
        while(str.get() != '\n');
        str.ignore(16);
        auto nextItem = &curMonkey->items;
        while(str.get() != '\n') {
            str.ignore(1);
            auto item = readNumber(str);
            *nextItem = new Item{ item, nullptr };
            nextItem = &(**nextItem).next;
        }

        str.ignore(23);
        auto const opC = str.get();
        if(opC == '+') curMonkey->op = plus;
        else curMonkey->op = times;
        str.ignore(1);
        if(str.peek() == 'o') {
            str.ignore(3);
            curMonkey->operand = -1;
        }
        else curMonkey->operand = readNumber(str);
        str.ignore(1);

        str.ignore(21);
        curMonkey->checkValue = readNumber(str);
        str.ignore(1);
        
        str.ignore(29);
        curMonkey->mIfTrue = readNumber(str);
        str.ignore(1);

        str.ignore(30);
        curMonkey->mIfFalse = readNumber(str);
        str.ignore(1);

        str.ignore(1);
    }
    str.close();

    for(int i = 0; i < 20; i++) {
        curMonkey = &startMonkey;

        while(curMonkey) {
            auto item = curMonkey->items;
            curMonkey->items = nullptr;
            while(item) {
                auto const nextItem = item->next;

                auto const newW = [&]() {
                    auto const w = item->worry;
                    auto const oper = curMonkey->operand == -1 ? w : curMonkey->operand;
                    if(curMonkey->op == plus) return w + oper;
                    else return w * oper;
                }() / 3;

                auto const monkeyToThrow = [&]() {
                    return newW % curMonkey->checkValue == 0
                        ? curMonkey->mIfTrue
                        : curMonkey-> mIfFalse;
                }();
                
                auto newItem = &monkeyAt(&startMonkey, monkeyToThrow).items; 
                while(*newItem) newItem = &(**newItem).next;
                item->worry = newW;
                item->next = nullptr;
                *newItem = item;

                item = nextItem;
                curMonkey->totalInspected++;
            }
            curMonkey = curMonkey->next;
        }
    }

    constexpr int topC = 2;
    int topMonkey[topC];
    int topCount[topC];
    for(int i = 0; i < topC; i++) topMonkey[i] = topCount[i] = -1;

    curMonkey = &startMonkey;
    auto curMonkeyI = 0;
    while(curMonkey) {
        auto index = curMonkeyI;
        auto count = curMonkey->totalInspected;

        for(int i = 0; i < topC; i++) {
            if(count > topCount[i]) {
                std::swap(count, topCount[i]);
                std::swap(index, topMonkey[i]);
            }
        }
        
        curMonkey = curMonkey->next;
        curMonkeyI++;
    }

    auto result = 1;
    for(int i = 0; i < topC; i++) {
        //std::cout << topMonkey[i] << ' ' << topCount[i] << '\n';
        result *= topCount[i];
    }
    std::cout << result << '\n';

    return 0;
}
