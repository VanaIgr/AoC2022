#include<iostream>
#include<fstream>

int readNumber(std::istream &stream) {
    int number = 0;
    for(int input; (input = stream.get()) && input >= '0' && input <= '9';) {
        number = number*10 + (input-'0');
    }
    return number;
}

int main() {
	auto stream = std::ifstream{"part1.txt"};
    	
    int overlap = 0;
	while(stream.peek() != std::char_traits<char>::eof()) {
        int n1 = readNumber(stream);
        int n2 = readNumber(stream);
        int n3 = readNumber(stream);
        int n4 = readNumber(stream);

        if((n1 <= n4 && n2 >= n3)
            | (n3 <= n1 && n4 >= n2)
        ) overlap++;
    }

    std::cout << overlap << '\n';

    return 0;
}
