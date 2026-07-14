#include "realtime/RealTimeArbiter.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace {

std::string promotePawnIfNeeded(const std::string& piece, const Position& to, int lastRow) {
    if (piece.size() == 2 && piece[1] == 'P') {
        const char color = piece[0];
        if ((color == 'w' && to.row == 0) || (color == 'b' && to.row == lastRow)) {
            return std::string(1, color) + 'Q';
        }
    }
    return piece;
}

int colorIndex(char color) {
    if (color != 'w' && color != 'b') {
        throw std::invalid_argument("Piece color must be 'w' or 'b'");
    }
    return color == 'w' ? 0 : 1;
}

void validatePieceTokenOrThrow(const std::string& piece) {
    if (piece.size() != 2) {
        throw std::invalid_argument("Motion piece token must be exactly two characters");
    }
    colorIndex(piece[0]);
    static const std::string kinds = "KQRBNP";
    if (kinds.find(piece[1]) == std::string::npos) {
        throw std::invalid_argument("Invalid motion piece kind");
    }
}

} // namespace

bool RealTimeArbiter::hasActiveMotion() const {
    return active[0].has_value() || active[1].has_value();
}

bool RealTimeArbiter::hasActiveMotion(char color) const {
    return active[colorIndex(color)].has_value();
}

bool RealTimeArbiter::hasActiveTravel(char color) const {
    const auto& slot = active[colorIndex(color)];
    return slot.has_value() && slot->from != slot->to;
}

void RealTimeArbiter::startMotion(const std::string& piece, const Position& from, const Position& to) {
    validatePieceTokenOrThrow(piece);
    if (!from.isValid() || !to.isValid()) {
        throw std::out_of_range("Motion endpoints must have non-negative coordinates");
    }

    const long long durationMs = static_cast<long long>(from.chebyshevDistanceTo(to)) * 1000;
    active[colorIndex(piece[0])] = Motion{ from, to, piece, clockMs + durationMs, nextStartSeq++ };
}

void RealTimeArbiter::startJump(const std::string& piece, const Position& at) {
    validatePieceTokenOrThrow(piece);
    if (!at.isValid()) {
        throw std::out_of_range("Jump position must have non-negative coordinates");
    }

    active[colorIndex(piece[0])] = Motion{ at, at, piece, clockMs + kJumpDurationMs, nextStartSeq++ };
}

std::vector<ArrivalEvent> RealTimeArbiter::advanceTime(int ms, Board& board) {
    if (ms < 0) {
        throw std::invalid_argument("advanceTime milliseconds must be non-negative");
    }

    clockMs += ms;

    std::vector<ArrivalEvent> arrivals;
    const int lastRow = board.getRowCount() - 1;

    auto isJumpLandingHere = [&](const Position& p) {
        for (const auto& slot : active) {
            if (slot && slot->from == slot->to && slot->to == p && clockMs >= slot->arrivalTime) {
                return true;
            }
        }
        return false;
    };

    std::array<int, kColorCount> travelOrder{};
    int travelCount = 0;
    for (int i = 0; i < kColorCount; ++i) {
        const auto& slot = active[i];
        if (slot && slot->from != slot->to && clockMs >= slot->arrivalTime) {
            travelOrder[static_cast<size_t>(travelCount++)] = i;
        }
    }
    std::sort(travelOrder.begin(), travelOrder.begin() + travelCount, [&](int a, int b) {
        return active[a]->startSeq > active[b]->startSeq;
    });

    for (int i = 0; i < travelCount; ++i) {
        auto& slot = active[static_cast<size_t>(travelOrder[static_cast<size_t>(i)])];
        const Motion motion = *slot;
        slot.reset();

        std::string capturedPiece = board.getCell(motion.to).getContent();
        if (isJumpLandingHere(motion.to)) {
            capturedPiece = ".";
        }
        const std::string pieceToPlace = promotePawnIfNeeded(motion.piece, motion.to, lastRow);

        board.setCell(motion.to, pieceToPlace);
        board.setCell(motion.from, ".");

        arrivals.push_back(ArrivalEvent{ motion.to, capturedPiece });
    }

    for (auto& slot : active) {
        if (!slot || slot->from != slot->to || clockMs < slot->arrivalTime) {
            continue;
        }

        const Motion motion = *slot;
        slot.reset();

        const std::string occupant = board.getCell(motion.to).getContent();
        const std::string capturedPiece = (occupant == "." || occupant == motion.piece) ? "." : occupant;

        board.setCell(motion.to, motion.piece);

        arrivals.push_back(ArrivalEvent{ motion.to, capturedPiece });
    }

    return arrivals;
}
