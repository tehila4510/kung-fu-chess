#include "MoveRules.h"
#include <cstdlib>

static bool isPathClear(const Board& board, const Position& from, const Position& to) {
    int dr = to.row - from.row;
    int dc = to.col - from.col;
    int stepRow = (dr > 0) - (dr < 0);
    int stepCol = (dc > 0) - (dc < 0);
    int row = from.row + stepRow;
    int col = from.col + stepCol;

    while (row != to.row || col != to.col) {
        if (!board.isEmpty({row, col})) {
            return false;
        }
        row += stepRow;
        col += stepCol;
    }
    return true;
}

static bool isValidKing(int dr, int dc) {
    return std::abs(dr) <= 1 && std::abs(dc) <= 1 && (dr != 0 || dc != 0);
}

static bool isValidRook(int dr, int dc) {
    return (dr == 0 && dc != 0) || (dc == 0 && dr != 0);
}

static bool isValidBishop(int dr, int dc) {
    return std::abs(dr) == std::abs(dc) && dr != 0;
}

static bool isValidQueen(int dr, int dc) {
    return isValidRook(dr, dc) || isValidBishop(dr, dc);
}

static bool isValidKnight(int dr, int dc) {
    int adr = std::abs(dr);
    int adc = std::abs(dc);
    return (adr == 2 && adc == 1) || (adr == 1 && adc == 2);
}

static bool isValidPawnMove(const Board& board, const Position& from, const Position& to, char color) {
    const int dr = to.row - from.row;
    const int dc = to.col - from.col;
    const int forward = (color == 'w') ? -1 : 1;

    if (dc == 0 && dr == forward && board.isEmpty(to)) {
        return true;
    }

    if (std::abs(dc) == 1 && dr == forward && !board.isEmpty(to)) {
        return true;
    }

    return false;
}

bool isValidMove(const Board& board, const Position& from, const Position& to) {
    if (!board.isWithinBounds(from) || !board.isWithinBounds(to)) {
        return false;
    }

    const std::string piece = board.getCell(from);
    if (piece == ".") {
        return false;
    }

    if (board.isFriendly(to, piece[0])) {
        return false;
    }

    const int dr = to.row - from.row;
    const int dc = to.col - from.col;
    const char type = piece[1];

    switch (type) {
        case 'P':
            return isValidPawnMove(board, from, to, piece[0]);
        case 'K':
            return isValidKing(dr, dc);
        case 'R':
            return isValidRook(dr, dc) && isPathClear(board, from, to);
        case 'B':
            return isValidBishop(dr, dc) && isPathClear(board, from, to);
        case 'Q':
            return isValidQueen(dr, dc) && isPathClear(board, from, to);
        case 'N':
            return isValidKnight(dr, dc);
        default:
            return true;
    }
}
