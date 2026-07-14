#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H
#include "model/Board.h"
#include "model/Piece.h"
#include "rules/PieceRules.h"
#include <memory>
#include <string>
#include <unordered_map>

struct MoveValidation {
    bool is_valid;
    std::string reason; // "ok" | "outside_board" | "empty_source" | "friendly_destination" | "illegal_piece_move"
};

class RuleEngine {
public:
    RuleEngine();

    MoveValidation validateMove(const Board& board, const Position& from, const Position& to) const;

private:
    std::unordered_map<char, std::unique_ptr<IPieceRules>> rules;
};
#endif
