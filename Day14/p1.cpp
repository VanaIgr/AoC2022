#include<iostream>
#include<fstream>
#include<climits>
#include<algorithm>

struct Position { int x, y; };

static int sign(int n) {
    return (n > 0) - (n < 0);
}

static constexpr char const *chars = ".#o~";

int main() {
    auto str = std::ifstream{"p1.txt"};

    auto const sandPos = Position{ 500, 0 };
    auto minPos = sandPos;
    auto maxPos = sandPos;

    while(str.peek() != std::char_traits<char>::eof()) {
        int x, y;
        str >> x;
        str.ignore(1);
        str >> y;

        minPos = Position{ std::min(minPos.x, x), std::min(minPos.y, y) };
        maxPos = Position{ std::max(maxPos.x, x), std::max(maxPos.y, y) };

        while(str.peek() != '\n') {
            str.ignore(4);
            str >> x;
            str.ignore(1);
            str >> y;
            minPos = Position{ std::min(minPos.x, x), std::min(minPos.y, y) };
            maxPos = Position{ std::max(maxPos.x, x), std::max(maxPos.y, y) };
        } 
        str.ignore(1);
    }
    str.clear();
    str.seekg(0, str.beg);

    auto const size = Position{ maxPos.x - minPos.x + 1, maxPos.y - minPos.y + 1 };
    auto const area = new uint8_t[size.y*size.x]{};

    auto const inBounds = [&](int const x, int const y) {
        return x >= 0 & x < size.x & y >= 0 & y < size.y;
    };
    auto const idx = [&](int const x, int const y) { return y * size.x + x; };
    auto const isAir = [&](int const x, int const y) { return !inBounds(x, y) || !area[idx(x, y)]; };

    while(str.peek() != std::char_traits<char>::eof()) {
        int x1, y1;
        str >> x1;
        str.ignore(1);
        str >> y1;

        while(str.peek() != '\n') {
            int x2, y2;
            str.ignore(4);
            str >> x2;
            str.ignore(1);
            str >> y2;

            auto const len = x2-x1 + y2-y1;
            for(int i = 0;; i += sign(len)) {
                auto const x = x1 + (i & -(x2!=x1));
                auto const y = y1 + (i & -(y2!=y1));
                area[idx(x - minPos.x, y - minPos.y)] = 1;
                if(i == len) break;
            }

            x1 = x2;
            y1 = y2;
        } 
        str.ignore(1);
    }

    auto steps = 0;
    while(true) {
        auto cx = sandPos.x - minPos.x;
        auto cy = sandPos.y - minPos.y;

        while(true) {
            if(!isAir(cx, cy)) exit(94); 
            if(!inBounds(cx, cy)) goto finish;
            auto const sx = cx;
            auto const sy = cy;

            cx = sx;
            cy = sy+1;
            if(isAir(sx, sy+1)) continue;
            
            cx = sx-1;
            cy = sy+1;
            if(isAir(sx-1, sy+1)) continue;

            cx = sx+1;
            cy = sy+1;
            if(isAir(sx+1, sy+1)) continue;

            cx = sx;
            cy = sy;
            break; //nowhere to move
        }

        steps++;
        area[idx(cx, cy)] = 2;
    }
    finish:

    std::cout << steps << '\n';

    {
        auto cx = sandPos.x - minPos.x;
        auto cy = sandPos.y - minPos.y;

        while(true) {
            if(!inBounds(cx, cy)) break;
            auto const sx = cx;
            auto const sy = cy;

            area[idx(cx, cy)] = 3;

            cx = sx;
            cy = sy+1;
            if(isAir(sx, sy+1)) continue;
            
            cx = sx-1;
            cy = sy+1;
            if(isAir(sx-1, sy+1)) continue;

            cx = sx+1;
            cy = sy+1;
            if(isAir(sx+1, sy+1)) continue;

            cx = sx;
            cy = sy;
            break;
        }
    }

    for(int y = 0; y < size.y; y++) {
    for(int x = 0; x < size.x; x++) {
        auto const i = area[y*size.x + x];
        std::cout << chars[i];
    } std::cout << '\n'; }

    return 0;
}
