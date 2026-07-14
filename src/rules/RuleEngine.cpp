#include "rules/RuleEngine.h"
#include "rules/PieceRules.h"

RuleEngine::RuleEngine() {
    rules['R'] = std::make_unique<RookRules>();
rules['B'] = std::make_unique<BishopRules>();
rules['Q'] = std::make_unique<QueenRules>();
rules['N'] = std::make_unique<KnightRules>();
rules['K'] = std::make_unique<KingRules>();
rules['P'] = std::make_unique<PawnRules>();
}

MoveValidation RuleEngine::validateMove(const Board& board, const Position& from, const Position& to) const {
    try {
        if (!board.isWithinBounds(from) || !board.isWithinBounds(to)) {
            return { false, "outside_board" };
        }

        const Cell& fromCell = board.getCell(from);
        if (fromCell.isEmpty()) {
            return { false, "empty_source" };
        }

        const Cell& toCell = board.getCell(to);
        if (!toCell.isEmpty() && toCell.getColor() == fromCell.getColor()) {
            return { false, "friendly_destination" };
        }

        const Piece piece = Piece::fromToken(fromCell.getContent(), from);
        const auto it = rules.find(piece.kind);
        if (it == rules.end() || !it->second->isValidMove(from, to, board)) {
            return { false, "illegal_piece_move" };
        }

        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "invalid_piece" };
    }
}
