#include "rules/RuleEngine.h"
#include "rules/PieceRules.h"

RuleEngine::RuleEngine() {
    rules['R'] = std::unique_ptr<IPieceRules>(new RookRules());
    rules['B'] = std::unique_ptr<IPieceRules>(new BishopRules());
    rules['Q'] = std::unique_ptr<IPieceRules>(new QueenRules());
    rules['N'] = std::unique_ptr<IPieceRules>(new KnightRules());
    rules['K'] = std::unique_ptr<IPieceRules>(new KingRules());
    rules['P'] = std::unique_ptr<IPieceRules>(new PawnRules());
}

MoveValidation RuleEngine::validateMove(const Board& board, const Position& from, const Position& to) const {
    try {
        if (!board.isWithinBounds(from) || !board.isWithinBounds(to)) {
            return { false, "outside_board" };
        }

        const std::string token = board.getCell(from);
        if (token == ".") {
            return { false, "empty_source" };
        }

        if (board.isFriendly(to, token[0])) {
            return { false, "friendly_destination" };
        }

        const Piece piece = Piece::fromToken(token, from);
        const auto it = rules.find(piece.kind);
        if (it == rules.end() || !it->second->isValidMove(from, to, board)) {
            return { false, "illegal_piece_move" };
        }

        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "invalid_piece" };
    }
}
