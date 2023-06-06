#include<iostream>
#include<fstream>
#include<stdint.h>
#include<utility>
#include<algorithm>
#include<cassert>
#include<limits>

struct Item {
    union {
        int initialWorry;
        int *worryMod; //worry level mod each monkey's check value
    };
    Item *next;
};

enum Operation {
    plus, times
};

struct Monkey {
    Item *items;
    Monkey *next;
    int operand; //-1 if old worry value is used instead
    int checkValue;
    int mIfTrue;
    int mIfFalse;
    int totalInspected;
    Operation op;
};

std::ostream &operator<<(std::ostream &o, Monkey const &it) {
    o << "Items: ";
    //auto curItem = it.items; //to hard to print with worry levens now being different for each monkey
    //while(curItem) {
    //    std::cout << curItem->worry << ' ';
    //    curItem = curItem->next;
    //}
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
    auto monkeyCount = 0;
    auto startMonkey = Monkey{};
    auto curMonkey = (Monkey*){};

    //read monkeys from file
    auto str = std::ifstream{ "p1.txt" };
    while(str.peek() != std::char_traits<char>::eof()) {
        if(curMonkey == nullptr) curMonkey = &startMonkey;
        else {
            auto const newMonkey = new Monkey{};
            curMonkey = curMonkey->next = newMonkey;
        }
        monkeyCount++;

        while(str.get() != '\n');
        str.ignore(16);
        auto nextItem = &curMonkey->items;
        while(str.get() != '\n') {
            str.ignore(1);
            auto item = readNumber(str);
            if(item < 0) return 101;
            *nextItem = new Item{};
            (**nextItem).initialWorry = item;
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

    //fill worry levels mod monkeys' check value
    curMonkey = &startMonkey;
    while(curMonkey) {
        auto item = curMonkey->items;
        while(item) {
            auto otherMonkey = &startMonkey;
            auto const init = item->initialWorry;
            auto worryMod = item->worryMod = new int[monkeyCount];
            while(otherMonkey) {
                *worryMod = init % otherMonkey->checkValue;
                otherMonkey = otherMonkey->next;
                worryMod++;
            }
            item = item->next;
        }
        curMonkey = curMonkey->next;
    }

    //calculate n rounds of monkeys passing items to each other
    for(int i = 0; i < 10'000; i++) {
        curMonkey = &startMonkey;

        while(curMonkey) {
            //throw all items to other monkeys
            auto item = curMonkey->items;
            curMonkey->items = nullptr;
            while(item) {
                auto const nextItem = item->next;

                //update worry levels for item for each monkey and remenber the value of current one
                auto const calcNewW = [&](int w) -> int {
                    if(curMonkey->operand == -1) {
                        if(curMonkey->op == plus) return w + w;
                        else return w * w;
                    }
                    else { 
                        if(curMonkey->op == plus) return w + curMonkey->operand;
                        else return w * curMonkey->operand;
                    }
                };

                auto newW = -1;
                auto worryMod = item->worryMod;
                auto otherMonkey = &startMonkey;
                while(otherMonkey) {
                    auto const mod = otherMonkey->checkValue;
                    *worryMod = calcNewW(*worryMod) % mod;

                    if(otherMonkey == curMonkey) newW = *worryMod;

                    otherMonkey = otherMonkey->next;
                    worryMod++;
                }
                if(newW == -1) exit(25);

                //move item to the end oth other monkey's items list
                auto const monkeyToThrow = newW == 0ll ? curMonkey->mIfTrue : curMonkey-> mIfFalse;
                
                auto newItemPlace = &monkeyAt(&startMonkey, monkeyToThrow).items; 
                while(*newItemPlace) newItemPlace = &(**newItemPlace).next;
                *newItemPlace = item;
                (**newItemPlace).next = nullptr;

                //move to nextItem
                curMonkey->totalInspected++;
                item = nextItem;
            }
            curMonkey = curMonkey->next;
        }
    }

    //calculate top n monkeys by total number of inspected items
    constexpr int topC = 2;
    int topMonkey[topC]; //additional info abount index of top monkeys
    using count_t = decltype(Monkey::totalInspected);
    count_t topCount[topC];
    for(int i = 0; i < topC; i++) {
        topMonkey[i] = -1;
        topCount[i] = std::numeric_limits<count_t>::lowest();
    }

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

    //calculate monkey business
    auto result = uint64_t{1};
    for(int i = 0; i < topC; i++) {
        std::cout << topMonkey[i] << ' ' << topCount[i] << '\n';
        result *= topCount[i];
    }
    std::cout << result << '\n';

    return 0;
}
