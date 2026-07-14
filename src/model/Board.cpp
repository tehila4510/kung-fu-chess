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

Board::Board(std::vector<std::vector<std::string>> initialGrid) {
    validateRectangularGrid(initialGrid);
    rows = static_cast<int>(initialGrid.size());
    cols = initialGrid.empty() ? 0 : static_cast<int>(initialGrid.front().size());
    grid.reserve(static_cast<size_t>(rows) * static_cast<size_t>(cols));
    for (const std::vector<std::string>& row : initialGrid) {
        for (const std::string& token : row) {
            grid.emplace_back(token);
        }
    }
}

int Board::getIndex(const Position& pos) const {
    return pos.row * cols + pos.col;
}

bool Board::isWithinBounds(const Position& pos) const {
    return pos.row >= 0 && pos.row < rows && pos.col >= 0 && pos.col < cols;
}

const Cell& Board::getCell(const Position& pos) const {
    validateWithinBoundsOrThrow(*this, pos);
    return grid[static_cast<size_t>(getIndex(pos))];
}

void Board::setCell(const Position& pos, const std::string val) {
    validateWithinBoundsOrThrow(*this, pos);
    grid[static_cast<size_t>(getIndex(pos))] = Cell(std::move(val));
}
