#ifndef MODEL_BOARD_H
#define MODEL_BOARD_H
#include "model/Cell.h"
#include "model/Position.h"
#include <vector>
#include <string>

class Board {
    std::vector<Cell> grid;
    int rows = 0, cols = 0;

    int getIndex(const Position& pos) const;
public:
    Board() = default;
    explicit Board(std::vector<std::vector<std::string>> initialGrid);

    bool isWithinBounds(const Position& p) const;
    const Cell& getCell(const Position& pos) const;
    void setCell(const Position& p, const std::string v);
    int getRowCount() const { return rows; }
    int getColCount() const { return cols; }
};
#endif
