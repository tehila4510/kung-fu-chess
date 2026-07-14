#include "model/Board.h"

#include <stdexcept>

namespace {

void validateRectangularGrid(const std::vector<std::vector<std::string>>& grid) {
    if (grid.empty()) {
        return;
    }
    const size_t width = grid.front().size();
    for (size_t i = 1; i < grid.size(); ++i) {
        if (grid[i].size() != width) {
            throw std::invalid_argument("Board grid rows must have equal width");
        }
    }
}

void validateWithinBoundsOrThrow(const Board& board, const Position& pos) {
    if (!board.isWithinBounds(pos)) {
        throw std::out_of_range(
            "Position (" + std::to_string(pos.row) + ", " + std::to_string(pos.col) +
            ") is outside board bounds");
    }
}

} // namespace

Board::Board(std::vector<std::vector<std::string>> initialGrid)
    : grid(std::move(initialGrid)),
      rows(static_cast<int>(grid.size())),
      cols(grid.empty() ? 0 : static_cast<int>(grid.front().size())) {
    validateRectangularGrid(grid);
}

bool Board::isWithinBounds(const Position& pos) const {
    return pos.row >= 0 && pos.row < rows && pos.col >= 0 && pos.col < cols;
}

std::string Board::getCell(const Position& pos) const {
    validateWithinBoundsOrThrow(*this, pos);
    return grid[pos.row][pos.col];
}

void Board::setCell(const Position& pos, const std::string& val) {
    validateWithinBoundsOrThrow(*this, pos);
    grid[pos.row][pos.col] = val;
}

bool Board::isEmpty(const Position& pos) const {
    validateWithinBoundsOrThrow(*this, pos);
    return grid[pos.row][pos.col] == ".";
}

bool Board::isFriendly(const Position& pos, char color) const {
    validateWithinBoundsOrThrow(*this, pos);
    if (grid[pos.row][pos.col] == ".") {
        return false;
    }
    return grid[pos.row][pos.col][0] == color;
}
