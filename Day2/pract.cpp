#include<iostream>
#include<fstream>

static int outcome(char opponent, char response) {
    int scoresForPick[] = { 1, 2, 3 };

    opponent -= 'A';
    response -= 'X';

    int result = 0;
    if(response == (opponent+1)%3) result = 6;
    else if(response == opponent) result = 3;
    
    return result + scoresForPick[response];
}

int main() {
    auto stream = std::ifstream{"pract.txt"};

    int score = 0;
    while(stream.peek() != -1) {
        auto const opponent = (char) stream.get();
        stream.get();
        auto const response = (char) stream.get();
        stream.get();
        score += outcome(opponent, response);
    }

    std::cout << score << '\n';

	return 0;
}

