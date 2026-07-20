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

BoardMapper::BoardMapper(int cellWidth, int cellHeight, int widthCells, int heightCells,
                         int originX, int originY)
    : cellWidth(cellWidth),
      cellHeight(cellHeight),
      widthCells(widthCells),
      heightCells(heightCells),
      originX(originX),
      originY(originY) {
    validateDimensions(cellWidth, cellHeight, widthCells, heightCells);
}

std::optional<Position> BoardMapper::pixelToCell(int x, int y) const {
    const int local_x = x - originX;
    const int local_y = y - originY;
    if (local_x < 0 || local_y < 0) {
        return std::nullopt;
    }
    const int col = local_x / cellWidth;
    const int row = local_y / cellHeight;
    if (col >= widthCells || row >= heightCells) {
        return std::nullopt;
    }
    return Position{row, col};
}

std::pair<int, int> BoardMapper::cellCenterPixel(const Position& cell) const {
    const int center_x = originX + cell.col * cellWidth + cellWidth / 2;
    const int center_y = originY + cell.row * cellHeight + cellHeight / 2;
    return { center_x, center_y };
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

int BoardMapper::getOriginX() const {
    return originX;
}

int BoardMapper::getOriginY() const {
    return originY;
}
