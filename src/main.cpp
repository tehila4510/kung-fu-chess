#include <iostream>
#include <vector>
#include <string>
#include "Board.h"

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(std::cin, line)) {
        lines.push_back(line);
    }

    Board board;
    size_t index = 0;
    if (!board.loadFromLines(lines, index)) {
        return 0;
    }

    board.print();
    return 0;
}