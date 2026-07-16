#include "input/BoardMapper.h"

#include <stdexcept>

namespace {

void validateDimensions(int cellWidth, int cellHeight, int widthCells, int heightCells) {
    if (cellWidth <= 0 || cellHeight <= 0) {
        throw std::invalid_argument("Cell dimensions must be positive");
    }
    if (widthCells <= 0 || heightCells <= 0) {
        throw std::invalid_argument("Board dimensions must be positive");
    }
}

}  // namespace

BoardMapper::BoardMapper(int cellSize, int widthCells, int heightCells)
    : BoardMapper(cellSize, cellSize, widthCells, heightCells) {}

BoardMapper::BoardMapper(int cellWidth, int cellHeight, int widthCells, int heightCells)
    : cellWidth(cellWidth),
      cellHeight(cellHeight),
      widthCells(widthCells),
      heightCells(heightCells) {
    validateDimensions(cellWidth, cellHeight, widthCells, heightCells);
}

std::optional<Position> BoardMapper::pixelToCell(int x, int y) const {
    if (x < 0 || y < 0) {
        return std::nullopt;
    }
    const int col = x / cellWidth;
    const int row = y / cellHeight;
    if (col >= widthCells || row >= heightCells) {
        return std::nullopt;
    }
    return Position{row, col};
}

int BoardMapper::getCellSize() const {
    return cellWidth;
}

int BoardMapper::getCellWidth() const {
    return cellWidth;
}

int BoardMapper::getCellHeight() const {
    return cellHeight;
}

int BoardMapper::getWidthCells() const {
    return widthCells;
}

int BoardMapper::getHeightCells() const {
    return heightCells;
}
