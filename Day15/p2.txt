#include<iostream>
#include<fstream>
#include<algorithm>
#include<climits>

struct Node {
    int x, y, bx, by;
    int distance;
    Node *next;
};

int main() {
    auto str = std::ifstream{ "p1.txt" };

    Node *startNode = nullptr;
    Node *curNode;
    int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN; 
    while(str.peek() != std::char_traits<char>::eof()) {
        str.ignore(12);
        int x, y;
        str >> x;
        str.ignore(4);
        str >> y;
        str.ignore(25);
        int bx, by;
        str >> bx;
        str.ignore(4);
        str >> by;
        str.ignore(1);

        if(!startNode) startNode = curNode = new Node;
        else curNode = curNode->next = new Node;
        curNode->x = x;
        curNode->y = y;
        curNode->bx = bx;
        curNode->by = by;
        auto const dist = curNode->distance = std::abs(x - bx) + std::abs(y - by);

        minX = std::min({ minX, x - dist });
        minY = std::min({ minY, y - dist });
        maxX = std::max({ maxX, x + dist });
        maxY = std::max({ maxY, y + dist });
    }
    curNode->next = nullptr;

    auto unsuitable = 0;
    auto const targetRow = 2000000;
    auto const y = targetRow;
    //for(int y = minY; y <= maxY; y++) { 
    //    std::cout << y << '\t';
        for(int x = minX; x <= maxX; x++) {
            curNode = startNode;
            bool beacon = false, suitable = true;
            while(curNode) {
                if(std::abs(x - curNode->x) + std::abs(y - curNode->y)
                    <= curNode->distance
                ) {
                    suitable = false;
                    if(curNode->bx == x & curNode->by == y) beacon = true;
                }
                curNode = curNode->next;
            }
            if(beacon | suitable) {
                //std::cout << '.';
            }
            else {
                unsuitable++;
                //std::cout << '#';
            }
        } 
    //    break; std::cout << '\n';
    //}
    std::cout << unsuitable << '\n';
}
