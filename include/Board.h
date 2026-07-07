#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>

class Board {
private:
    std::vector<std::vector<std::string>> grid;
    int rows = 0;
    int cols = 0;

public:
    bool loadFromLines(const std::vector<std::string>& lines, size_t& index);
    void print() const;
};

#endif