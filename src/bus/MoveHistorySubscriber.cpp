#include "bus/MoveHistorySubscriber.h"

#include <iomanip>
#include <sstream>

namespace {

const char* pieceKindName(char kind) {
    switch (kind) {
        case 'K':
            return "King";
        case 'Q':
            return "Queen";
        case 'R':
            return "Rook";
        case 'B':
            return "Bishop";
        case 'N':
            return "Knight";
        case 'P':
            return "Pawn";
        default:
            return "Piece";
    }
}

std::string pieceDisplayName(const std::string& token) {
    if (token.size() != 2) {
        return token.empty() ? "Piece" : token;
    }
    return pieceKindName(token[1]);
}

std::string formatTimeSeconds(long long timeMs) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1)
        << (static_cast<double>(timeMs) / 1000.0) << 's';
    return oss.str();
}

}  // namespace

const std::vector<MoveHistoryEntry>& MoveHistorySubscriber::whiteEntries() const {
    return white_;
}

const std::vector<MoveHistoryEntry>& MoveHistorySubscriber::blackEntries() const {
    return black_;
}

void MoveHistorySubscriber::append(MoveHistoryEntry entry) {
    std::vector<MoveHistoryEntry>& side =
        (entry.color == 'B') ? black_ : white_;
    side.push_back(std::move(entry));
    while (side.size() > kMaxEntriesPerSide) {
        side.erase(side.begin());
    }
}

std::string MoveHistorySubscriber::formatEntry(const MoveHistoryEntry& entry) {
    std::ostringstream oss;
    oss << formatTimeSeconds(entry.timeMs) << "  ";
    if (entry.isCapture) {
        oss << "capture " << entry.capturedPiece;
        if (!entry.to.empty()) {
            oss << " on " << entry.to;
        }
        return oss.str();
    }
    if (entry.isJump) {
        oss << "JUMP " << pieceDisplayName(entry.piece);
        if (!entry.from.empty()) {
            oss << ' ' << entry.from;
        }
        return oss.str();
    }
    oss << pieceDisplayName(entry.piece);
    if (!entry.from.empty() && !entry.to.empty()) {
        oss << ' ' << entry.from << '-' << entry.to;
    }
    return oss.str();
}

void MoveHistorySubscriber::onEvent(const GameEvent& event) {
    if (event.type == GameEventType::MoveMade ||
        event.type == GameEventType::JumpMade) {
        MoveHistoryEntry entry;
        entry.color = event.color;
        entry.timeMs = event.timeMs;
        entry.piece = event.piece;
        entry.from = event.from;
        entry.to = event.to;
        entry.isJump = (event.type == GameEventType::JumpMade);
        append(std::move(entry));
        return;
    }

    if (event.type == GameEventType::PieceCaptured) {
        MoveHistoryEntry entry;
        entry.color = event.color;
        entry.timeMs = event.timeMs;
        entry.piece = event.piece;
        entry.to = event.to;
        entry.isCapture = true;
        entry.capturedPiece = event.capturedPiece;
        append(std::move(entry));
    }
}
