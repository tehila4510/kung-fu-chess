#include "input/BoardMapper.h"

#include <stdexcept>

BoardMapper::BoardMapper(int cellSize, int widthCells, int heightCells)
    : cellSize(cellSize), widthCells(widthCells), heightCells(heightCells) {
    if (cellSize <= 0) {
        throw std::invalid_argument("Cell size must be positive");
    }
    if (widthCells <= 0 || heightCells <= 0) {
        throw std::invalid_argument("Board dimensions must be positive");
    }
}

std::optional<Position> BoardMapper::pixelToCell(int x, int y) const {
    if (x < 0 || y < 0) {
        return std::nullopt;
    }
    const int col = x / cellSize;
    const int row = y / cellSize;
    if (col >= widthCells || row >= heightCells) {
        return std::nullopt;
    }
    return Position{ row, col };
}

int BoardMapper::getCellSize() const {
    return cellSize;
}

int BoardMapper::getWidthCells() const {
    return widthCells;
}

int BoardMapper::getHeightCells() const {
    return heightCells;
}
