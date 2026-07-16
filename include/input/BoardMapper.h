#ifndef BOARD_MAPPER_H
#define BOARD_MAPPER_H
#include "model/Position.h"
#include <optional>
#include <utility>

class BoardMapper {
    int cellWidth = 100;
    int cellHeight = 100;
    int widthCells = 8;
    int heightCells = 8;
public:
    BoardMapper(int cellSize, int widthCells, int heightCells);
    BoardMapper(int cellWidth, int cellHeight, int widthCells, int heightCells);
    std::optional<Position> pixelToCell(int x, int y) const;
    std::pair<int, int> cellCenterPixel(const Position& cell) const;
    int getCellSize() const;
    int getCellWidth() const;
    int getCellHeight() const;
    int getWidthCells() const;
    int getHeightCells() const;
};
#endif
