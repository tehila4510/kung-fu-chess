#include "realtime/RealTimeArbiter.h"

#include <algorithm>
#include <cmath>
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

bool isFleeingFrom(
    const std::optional<Motion> active[],
    int colorCount,
    const Position& at,
    const std::string& occupant) {
    for (int i = 0; i < colorCount; ++i) {
        const auto& slot = active[i];
        if (slot && slot->piece == occupant && slot->from == at && slot->from != slot->to) {
            return true;
        }
    }
    return false;
}

} // namespace

long long RealTimeArbiter::elapsedMs() const {
    return clockMs;
}

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

bool RealTimeArbiter::isResting(const Position& at) const {
    return rests.find(at) != rests.end();
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

    for (auto it = rests.begin(); it != rests.end();) {
        if (clockMs >= it->second.endTime) {
            it = rests.erase(it);
        } else {
            ++it;
        }
    }

    std::vector<ArrivalEvent> arrivals;
    const int lastRow = board.getRowCount() - 1;

    struct PendingArrival {
        int slotIndex;
        long long arrivalTime;
        long long startSeq;
        bool isJump;
    };

    std::vector<PendingArrival> pending;
    pending.reserve(kColorCount);

    for (int i = 0; i < kColorCount; ++i) {
        const auto& slot = active[i];
        if (slot && clockMs >= slot->arrivalTime) {
            pending.push_back(PendingArrival{
                i,
                slot->arrivalTime,
                slot->startSeq,
                slot->from == slot->to,
            });
        }
    }

    std::sort(pending.begin(), pending.end(), [](const PendingArrival& a, const PendingArrival& b) {
        if (a.arrivalTime != b.arrivalTime) {
            return a.arrivalTime < b.arrivalTime;
        }
        return a.startSeq > b.startSeq;
    });

    for (const PendingArrival& item : pending) {
        auto& slot = active[static_cast<size_t>(item.slotIndex)];
        if (!slot) {
            continue;
        }

        const Motion motion = *slot;
        slot.reset();

        std::string placedPiece = motion.piece;
        std::string capturedPiece = ".";

        if (item.isJump) {
            const std::string occupant = board.getCell(motion.to).getContent();
            capturedPiece =
                (occupant == "." || occupant == motion.piece) ? "." : occupant;

            board.setCell(motion.to, motion.piece);
        } else {
            const std::string occupant = board.getCell(motion.to).getContent();
            if (occupant != "." && occupant != motion.piece
                && !isFleeingFrom(active, kColorCount, motion.to, occupant)) {
                capturedPiece = occupant;
            }
            placedPiece = promotePawnIfNeeded(motion.piece, motion.to, lastRow);

            board.setCell(motion.to, placedPiece);
            // Only vacate origin if we still own it; a pursuer may have landed there.
            if (board.getCell(motion.from).getContent() == motion.piece) {
                board.setCell(motion.from, ".");
            }
        }

        const RestKind kind = item.isJump ? RestKind::Short : RestKind::Long;
        const long long durationMs =
            kind == RestKind::Short ? kShortRestMs : kLongRestMs;
        rests[motion.to] = Rest{
            motion.to,
            placedPiece,
            kind,
            clockMs + durationMs,
            durationMs,
        };

        const bool promoted = !item.isJump && placedPiece != motion.piece;
        arrivals.push_back(ArrivalEvent{ motion.to, placedPiece, capturedPiece, promoted });
    }

    return arrivals;
}

std::vector<MotionView> RealTimeArbiter::activeMotions() const {
    std::vector<MotionView> views;
    views.reserve(kColorCount);

    for (const auto& slot : active) {
        if (!slot) {
            continue;
        }

        const Motion& motion = *slot;
        const long long duration_ms =
            motion.from == motion.to
                ? static_cast<long long>(kJumpDurationMs)
                : static_cast<long long>(motion.from.chebyshevDistanceTo(motion.to)) * 1000;
        const long long start_ms = motion.arrivalTime - duration_ms;

        double progress = 0.0;
        if (duration_ms > 0) {
            progress = static_cast<double>(clockMs - start_ms) /
                       static_cast<double>(duration_ms);
            progress = std::clamp(progress, 0.0, 1.0);
        }

        views.push_back(MotionView{motion.piece, motion.from, motion.to, progress});
    }

    return views;
}

std::vector<RestView> RealTimeArbiter::activeRests() const {
    std::vector<RestView> views;
    views.reserve(rests.size());

    for (const auto& entry : rests) {
        const Rest& rest = entry.second;
        double remaining = 0.0;
        if (rest.durationMs > 0) {
            remaining = static_cast<double>(rest.endTime - clockMs) /
                        static_cast<double>(rest.durationMs);
            remaining = std::clamp(remaining, 0.0, 1.0);
        }
        views.push_back(RestView{ rest.piece, rest.at, rest.kind, remaining });
    }

    return views;
}
