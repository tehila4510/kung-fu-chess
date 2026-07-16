#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H
#include "model/Board.h"
#include "model/Piece.h"
#include "rules/PieceRules.h"
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

enum class MoveResult {
    Ok,
    OutsideBoard,
    EmptySource,
    FriendlyDestination,
    IllegalPieceMove,
    InvalidPiece
};

std::string toString(MoveResult reason);

struct MoveValidation {
    bool is_valid;
    MoveResult reason;
};

class RuleEngine {
public:
    RuleEngine();

    MoveValidation validateMove(const Board& board, const Position& from, const Position& to) const;
    MoveValidation validateMove(const Board& board, const Position& from, const Position& to,
                                const std::vector<AirborneOccupant>& airborne) const;
    std::set<Position> legalMoves(const Board& board, const Position& from) const;
    std::set<Position> legalMoves(const Board& board, const Position& from,
                                  const std::vector<AirborneOccupant>& airborne) const;

private:
    std::unordered_map<char, std::unique_ptr<IPieceRules>> rules;
};
#endif
