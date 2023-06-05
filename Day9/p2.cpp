#include<iostream>
#include<fstream>
#include<stdint.h>
#include<cmath>

int readNumber(std::istream &stream) {
    int number = 0;
    for(int input; (input = stream.get()) && input >= '0' && input <= '9';) {
        number = number*10 + (input-'0');
    }
    return number;
}

int sign(int n) { if(n >= 1) return 1; if(n <= -1) return -1; return 0; }

int main() {
    auto str = std::ifstream{ "p1.txt" };

    auto commandsC = 0;
    while(str.peek() != std::char_traits<char>::eof()) {
        if(str.get() == '\n') commandsC++;
    }
    str.clear();
    str.seekg(0, str.beg);

    auto const commands = new int8_t[commandsC*2]; //2 offsets for each command

    auto maxPosC = 0;
    for(auto i = 0; i < commandsC; i++) {
        auto const c = str.get();
        str.ignore(1);
        auto const n = readNumber(str);
        switch(c) {
            break; case 'U': commands[i*2] = 0; commands[i*2+1] = 1;
            break; case 'D': commands[i*2] = 0; commands[i*2+1] = -1;
            break; case 'L': commands[i*2] = -1; commands[i*2+1] = 0;
            break; case 'R': commands[i*2] = 1; commands[i*2+1] = 0;
        }
        commands[i*2] *= n;
        commands[i*2 + 1] *= n;
        maxPosC += n;
    }

    /*std::cout << commandsC << '\n';
    for(int i = 0; i < commandsC; i++) {
        std::cout << (int)commands[i*2] << ' ' << (int)commands[i*2 + 1] << '\n';
    }*/

    auto const positions = new int[(maxPosC+1)*2]; //2 coords for each position + starting poisiton
    auto posC = 0;

    auto const snakeSegments = 10;
    int snake[snakeSegments*2] = {};

    auto &lastTailX = snake[(snakeSegments-1) * 2];
    auto &lastTailY = snake[(snakeSegments-1) * 2 + 1];

    positions[0] = lastTailX;
    positions[1] = lastTailY;
    posC++;

    for(auto c = 0; c < commandsC; c++) {
        auto const mvX = commands[c*2];
        auto const mvY = commands[c*2 + 1];
       
        for(int i = 0; i < std::max(std::abs(mvX), std::abs(mvY)); i++) {
            snake[0] += sign(mvX);
            snake[1] += sign(mvY);

            auto const lastTailXC = lastTailX;
            auto const lastTailYC = lastTailY;

            for(int t = 1; t < snakeSegments; t++) {
                auto &headX = snake[(t-1)*2];
                auto &headY = snake[(t-1)*2 + 1];

                auto &tailX = snake[t*2];
                auto &tailY = snake[t*2 + 1];

                auto const dx = std::abs(headX - tailX) > 1;
                auto const dy = std::abs(headY - tailY) > 1;
                if(dx | dy) {
                    tailX += sign(headX - tailX);
                    tailY += sign(headY - tailY);

                }
            }

            if(lastTailXC == lastTailX & lastTailYC == lastTailY) continue;

            auto inList = false;
            for(int pos = 0; pos < posC; pos++) {
                if(positions[pos*2] == lastTailX & positions[pos*2+1] == lastTailY) {
                    inList = true;
                    break;
                }
            }

            if(!inList) {
                positions[posC*2] = lastTailX;
                positions[posC*2 + 1] = lastTailY;
                posC++;
            }
        }
    }

    std::cout << posC << '\n';
}
