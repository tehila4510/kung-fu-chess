#include "rules/RuleEngine.h"
#include "rules/PieceRules.h"

#include <optional>

std::string toString(MoveResult reason) {
    switch (reason) {
        case MoveResult::Ok: return "ok";
        case MoveResult::OutsideBoard: return "outside_board";
        case MoveResult::EmptySource: return "empty_source";
        case MoveResult::FriendlyDestination: return "friendly_destination";
        case MoveResult::IllegalPieceMove: return "illegal_piece_move";
        case MoveResult::InvalidPiece: return "invalid_piece";
    }
    return "unknown";
}

RuleEngine::RuleEngine() {
    rules['R'] = std::make_unique<RookRules>();
    rules['B'] = std::make_unique<BishopRules>();
    rules['Q'] = std::make_unique<QueenRules>();
    rules['N'] = std::make_unique<KnightRules>();
    rules['K'] = std::make_unique<KingRules>();
    rules['P'] = std::make_unique<PawnRules>();
}

MoveValidation RuleEngine::validateMove(const Board& board, const Position& from, const Position& to) const {
    return validateMove(board, from, to, {});
}

MoveValidation RuleEngine::validateMove(const Board& board, const Position& from, const Position& to,
                                        const std::vector<AirborneOccupant>& airborne) const {
    if (!board.isWithinBounds(from) || !board.isWithinBounds(to)) {
        return { false, MoveResult::OutsideBoard };
    }

    const Cell& fromCell = board.getCell(from);
    if (fromCell.isEmpty()) {
        return { false, MoveResult::EmptySource };
    }

    const Cell& toCell = board.getCell(to);
    if (!toCell.isEmpty() && toCell.getColor() == fromCell.getColor()) {
        return { false, MoveResult::FriendlyDestination };
    }

    for (const AirborneOccupant& occupant : airborne) {
        if (occupant.at == to && occupant.piece.size() == 2
            && occupant.piece[0] == fromCell.getColor()) {
            return { false, MoveResult::FriendlyDestination };
        }
    }

    const std::optional<Piece> piece = Piece::tryFromToken(fromCell.getContent(), from);
    if (!piece) {
        return { false, MoveResult::InvalidPiece };
    }

    const auto it = rules.find(piece->kind);
    if (it == rules.end() || !it->second->isValidMove(from, to, board, airborne)) {
        return { false, MoveResult::IllegalPieceMove };
    }

    return { true, MoveResult::Ok };
}

std::set<Position> RuleEngine::legalMoves(const Board& board, const Position& from) const {
    return legalMoves(board, from, {});
}

std::set<Position> RuleEngine::legalMoves(const Board& board, const Position& from,
                                          const std::vector<AirborneOccupant>& airborne) const {
    if (!board.isWithinBounds(from)) {
        return {};
    }

    const Cell& fromCell = board.getCell(from);
    if (fromCell.isEmpty()) {
        return {};
    }

    const std::optional<Piece> piece = Piece::tryFromToken(fromCell.getContent(), from);
    if (!piece) {
        return {};
    }

    const auto it = rules.find(piece->kind);
    if (it == rules.end()) {
        return {};
    }

    return it->second->collectLegalMoves(from, board, airborne);
}
