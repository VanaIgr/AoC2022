#include<iostream>
#include<fstream>
#include<algorithm>

struct Position {
    int x, y;
};

int main() {
    auto str = std::ifstream{ "p1.txt" };

    auto posC = 0;
    while(str.peek() != std::char_traits<char>::eof()) {
        if(str.get() == '\n') posC++;
    }
    str.clear();
    str.seekg(0, str.beg);

    auto const linesC = posC*2;
    //the 'b in y = +-x + b;
    auto const pLines = new int[linesC];
    auto const nLines = new int[linesC];
    struct Point { Position sensor, beacon; int distance; };
    auto const points = new Point[posC];

    for(auto i = 0; i < posC; i++) {
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

        auto const distance = std::abs(x-bx) + std::abs(y-by);

        pLines[i*2 + 0] = y + distance+1 - x;
        pLines[i*2 + 1] = y - distance-1 - x;
        nLines[i*2 + 0] = y + distance+1 + x;
        nLines[i*2 + 1] = y - distance-1 + x;

        points[i] = Point{ {x, y}, {bx, by}, distance };
    }
    str.close();

    //https://oeis.org/A002620
    auto const totalLines = linesC*2;
    auto const intersectionsMaxC = (totalLines*totalLines)/4;
    auto const intersections = new Position[intersectionsMaxC];
    auto intersectionsC = 0;

    /*
     * y = x + po;
     * y = -x + no;
     *
     * + -> 2y = po + no; y = (po+no)/2;
     * - -> 0 = 2x + po - no; 2x = no - po; x = (no - po) / 2;
    */
    auto const maxV = 4'000'000;
    for(auto pI = 0; pI < linesC; pI++) {
        auto const po = pLines[pI];
        for(auto nI = 0; nI < linesC; nI++) {
            auto const no = nLines[nI];

            /*
                if diference is odd, we don't need to consider the interseciton
                because it doesn't yield any points.
            */
            if((po ^ no) & 1) continue; 

            auto const x = (no - po) >> 1;
            auto const y = (no + po) >> 1; 
            if(x < 0 | x > maxV | y < 0 | y > maxV) continue;

            for(auto i = 0; i < intersectionsC; i++) {
                if(intersections[i].x == x & intersections[i].y == y) goto end;
            }

            intersections[intersectionsC] = { x, y };
            intersectionsC++;

            end:
            ;
        }
    }

    auto foundPos = Position{ -1, -1 };
    for(auto i = 0; i < intersectionsC; i++) {
        auto const in = intersections[i];

        for(auto pI = 0; pI < posC; pI++) {
            auto const sensor   = points[pI].sensor;
            auto const beacon   = points[pI].beacon;
            auto const distance = points[pI].distance;
            if(std::abs(sensor.x - in.x) + std::abs(sensor.y - in.y) <= distance) {
                goto notTheBeacon;
            }
            if(in.x == beacon.x & in.y == beacon.y) {
                goto notTheBeacon;
            }
        }

        foundPos = { in.x, in.y };
        break;

        notTheBeacon:
        ;
    }

    auto const result = foundPos.x*(long long)maxV + foundPos.y;
    std::cout << result << '\n';
}
