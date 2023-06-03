#include<iostream>
#include<fstream>

static int scoreForOutcome(char opponent, char outcome) {
    int scoresForPick[] = { 1, 2, 3 };
    int offsets[] = { -1, 0, 1 };

    opponent -= 'A';
    outcome  -= 'X';
    auto const response = (opponent + offsets[outcome] + 3) % 3;

    return outcome * 3 + scoresForPick[response];
}

int main() {
    auto stream = std::ifstream{"pract.txt"};

    //std::cout << (scoreForOutcome('A', 'Y') + scoreForOutcome('B', 'X') + scoreForOutcome('C', 'Z')) << '\n';

    int score = 0;
    while(stream.peek() != -1) {
        auto const opponent = (char) stream.get();
        stream.get();
        auto const outcome = (char) stream.get();
        stream.get();
        score += scoreForOutcome(opponent, outcome);
    }

    std::cout << score << '\n';

	return 0;
}

