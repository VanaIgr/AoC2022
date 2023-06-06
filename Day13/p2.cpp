#include<iostream>
#include<fstream>

struct Item {
    union {
        Item *inner;
        int value;
    };
    Item *next;
    bool isValue;
};

static bool _a;
static Item *readList(std::istream &s) {
    if(s.get() != '[') exit(100);
    if(s.peek() == ']') {
        if(_a) std::cout << "return: empty list\n";
        s.ignore(1);
        return new Item{};
    }
    auto const start = new Item{};
    auto cur = start;
    while(true) {
        if(s.peek() == '[') {
            if(_a) std::cout << "reading inner list\n";
            cur->isValue = false;
            cur->inner = readList(s);
        }
        else {
            if(_a) std::cout << "reading value\n";
            int n;
            s >> n;
            //if(n == 9) _a = true;
            cur->value = n;
            cur->isValue = true;
        }

        if(s.get() == ']') {
            if(_a) std::cout << "return: end of list\n";
            return start;
        }
        else {
            if(_a) std::cout << "preparing for next item\n";
            cur = cur->next = new Item{};
        }
    }
}

static void deleteList(Item *list) {
    if(!list) return;
    if(!list->isValue) deleteList(list->inner);
    deleteList(list->next);
    delete list;
}

enum CheckResult : uint8_t {
    cont, correct, incorrect
};

static CheckResult checkLists(Item const *l, Item const *r) {
    while(true) {
        if(!l & !r) return cont;
        else if(!l) return correct;
        else if(!r) return incorrect;

        if(l->isValue & r->isValue) {
            if(l->value < r->value) return correct;
            else if(l->value > r->value) return incorrect;
            //l->value == r->value
        }
        else {
            Item i1{};
            if(l->isValue) {
                i1.isValue = true;
                i1.value = l->value;
            }
            auto const lp = l->isValue ? &i1 : l->inner;

            Item i2{};
            if(r->isValue) {
                i2.isValue = true;
                i2.value = r->value;
            }
            auto const rp = r->isValue ? &i2 : r->inner;

            auto const res = checkLists(lp, rp);
            if(res != cont) return res;
        }

        l = l->next;
        r = r->next;
    }
}

static std::ostream &operator<<(std::ostream &o, Item &it) {
    o << '[';
    auto *cur = &it;
    while(cur) {
        if(cur->isValue) o << cur->value;
        else if(cur->inner) o << *cur->inner;

        if(cur->next != nullptr) o << ',';
        cur = cur->next;
    }
    o << ']';
    return o;
}

int main() {
    auto str = std::ifstream{ "p1.txt" };

    auto listList = Item{};
    auto lastList = &listList;
    int listCount = 0;

    while(true) {
        if(str.peek() != '[') break;
        auto const fList = readList(str);
        str.ignore(1);

        if(str.peek() != '[') exit(101);
        auto const sList = readList(str);
        str.ignore(2);

        lastList->inner = fList;
        lastList = lastList->next = new Item{};
        lastList->inner = sList;
        lastList = lastList->next = new Item{};
        listCount += 2;
    }

    auto const fMarker = new Item{};
    fMarker->inner = new Item{};
    fMarker->inner->inner = new Item{};
    fMarker->inner->isValue = true;
    fMarker->inner->value = 2;
    
    lastList->inner = fMarker; 


    auto const sMarker = new Item{};
    sMarker->inner = new Item{};
    sMarker->inner->inner = new Item{};
    sMarker->inner->isValue = true;
    sMarker->inner->value = 6;

    lastList = lastList->next = new Item{};
    lastList->inner = sMarker; 

    listCount += 2;

    auto const listsArr = new Item*[listCount];
    auto const checksArr = new CheckResult[listCount*listCount]{};

    lastList = &listList;
    for(int i = 0; i < listCount; i++) {
        listsArr[i] = lastList->inner;
        lastList = lastList->next;
    }

    for(int i = 0; i < listCount-1; i++) {
        for(int j = i+1; j < listCount; j++) {
            auto &status = checksArr[i * listCount + j];
            if(status == cont) status = checkLists(listsArr[i], listsArr[j]);
            if(status == cont) exit(160);
            if(status == incorrect) std::swap(listsArr[i], listsArr[j]);
        }
    }

    int res = 1;
    for(int i = 0; i < listCount; i++) {
        if(listsArr[i] == fMarker | listsArr[i] == sMarker) {
            res *= i+1;
        }
    }
    std::cout << res << '\n';

    return 0;
}
