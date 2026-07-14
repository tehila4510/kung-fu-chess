#include "rules/PieceRules.h"

#include <optional>

namespace piece_rules {

bool inBounds(const Board& board, int row, int col) {
    return board.isWithinBounds(Position{ row, col });
}

void addSlidingMoves(const Board& board, const Piece& piece,
                     const std::vector<Step>& dirs, std::set<Position>& out) {
    for (const Step& d : dirs) {
        int row = piece.position.row + d.first;
        int col = piece.position.col + d.second;
        while (inBounds(board, row, col)) {
            Position p{ row, col };
            const Cell& cell = board.getCell(p);
            if (cell.isEmpty()) {
                out.insert(p);
            } else {
                if (cell.getColor() != piece.color) {
                    out.insert(p);
                }
                break;
            }
            row += d.first;
            col += d.second;
        }
    }
}

void addStepMoves(const Board& board, const Piece& piece,
                  const std::vector<Step>& offsets, std::set<Position>& out) {
    for (const Step& o : offsets) {
        int row = piece.position.row + o.first;
        int col = piece.position.col + o.second;
        if (!inBounds(board, row, col)) {
            continue;
        }
        Position p{ row, col };
        const Cell& cell = board.getCell(p);
        if (cell.isEmpty() || cell.getColor() != piece.color) {
            out.insert(p);
        }
    }
}

bool isPawnInitialRow(int row, char color, int rows) {
    if (color == 'w') {
        return row == rows - 2;
    }
    return row == 1;
}

const std::vector<Step>& rookDirs() {
    static const std::vector<Step> dirs{ { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
    return dirs;
}

const std::vector<Step>& bishopDirs() {
    static const std::vector<Step> dirs{ { -1, -1 }, { -1, 1 }, { 1, -1 }, { 1, 1 } };
    return dirs;
}

std::optional<Piece> pieceAt(const Board& board, const Position& from) {
    try {
        return Piece::fromToken(board.getCell(from).getContent(), from);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

} // namespace piece_rules

bool RookRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }
    std::set<Position> legal;
    piece_rules::addSlidingMoves(board, *piece, piece_rules::rookDirs(), legal);
    return legal.find(to) != legal.end();
}

bool BishopRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }
    std::set<Position> legal;
    piece_rules::addSlidingMoves(board, *piece, piece_rules::bishopDirs(), legal);
    return legal.find(to) != legal.end();
}

bool QueenRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }
    std::set<Position> legal;
    piece_rules::addSlidingMoves(board, *piece, piece_rules::rookDirs(), legal);
    piece_rules::addSlidingMoves(board, *piece, piece_rules::bishopDirs(), legal);
    return legal.find(to) != legal.end();
}

bool KnightRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }
    const std::vector<piece_rules::Step> offsets{
        { -2, -1 }, { -2, 1 }, { -1, -2 }, { -1, 2 },
        { 1, -2 }, { 1, 2 }, { 2, -1 }, { 2, 1 }
    };
    std::set<Position> legal;
    piece_rules::addStepMoves(board, *piece, offsets, legal);
    return legal.find(to) != legal.end();
}

bool KingRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }
    const std::vector<piece_rules::Step> offsets{
        { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, -1 },
        { 0, 1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }
    };
    std::set<Position> legal;
    piece_rules::addStepMoves(board, *piece, offsets, legal);
    return legal.find(to) != legal.end();
}

bool PawnRules::isValidMove(const Position& from, const Position& to, const Board& board) const {
    const std::optional<Piece> piece = piece_rules::pieceAt(board, from);
    if (!piece) {
        return false;
    }

    std::set<Position> legal;

    const int forward = piece->isWhite() ? -1 : 1;
    const int rows = board.getRowCount();
    const int row = piece->position.row;
    const int col = piece->position.col;

    Position oneForward{ row + forward, col };
    if (piece_rules::inBounds(board, oneForward.row, oneForward.col)
        && board.getCell(oneForward).isEmpty()) {
        legal.insert(oneForward);

        Position twoForward{ row + 2 * forward, col };
        if (piece_rules::isPawnInitialRow(row, piece->color, rows)
            && piece_rules::inBounds(board, twoForward.row, twoForward.col)
            && board.getCell(twoForward).isEmpty()) {
            legal.insert(twoForward);
        }
    }

    for (int dc : { -1, 1 }) {
        Position capture{ row + forward, col + dc };
        const Cell& captureCell = board.getCell(capture);
        if (piece_rules::inBounds(board, capture.row, capture.col)
            && !captureCell.isEmpty()
            && captureCell.getColor() != piece->color) {
            legal.insert(capture);
        }
    }

    return legal.find(to) != legal.end();
}
