#ifndef PIECE_RULES_H
#define PIECE_RULES_H
#include "model/Board.h"
#include "model/Piece.h"
#include "model/Position.h"
#include <set>
#include <utility>
#include <vector>

class IPieceRules {
public:
    virtual ~IPieceRules() = default;
    virtual bool isValidMove(const Position& from, const Position& to, const Board& board) const = 0;
};

namespace piece_rules {

using Step = std::pair<int, int>;

bool inBounds(const Board& board, int row, int col);

void addSlidingMoves(const Board& board, const Piece& piece,
                     const std::vector<Step>& dirs, std::set<Position>& out);

void addStepMoves(const Board& board, const Piece& piece,
                  const std::vector<Step>& offsets, std::set<Position>& out);

bool isPawnInitialRow(int row, char color, int rows);

const std::vector<Step>& rookDirs();
const std::vector<Step>& bishopDirs();

} // namespace piece_rules

class RookRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

class BishopRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

class QueenRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

class KnightRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

class KingRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

class PawnRules : public IPieceRules {
public:
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
};

#endif
