#ifndef MOVE_HISTORY_SUBSCRIBER_H
#define MOVE_HISTORY_SUBSCRIBER_H

#include "bus/IGameEventListener.h"

#include <string>
#include <vector>

struct MoveHistoryEntry {
    char color = '?';
    long long timeMs = 0;
    std::string piece;
    std::string from;
    std::string to;
    bool isJump = false;
    bool isCapture = false;
    std::string capturedPiece;
};

// Accumulates White/Black move lines from GameEvent for the graphics HUD.
class MoveHistorySubscriber : public IGameEventListener {
    std::vector<MoveHistoryEntry> white_;
    std::vector<MoveHistoryEntry> black_;

    void append(MoveHistoryEntry entry);

public:
    static constexpr size_t kMaxEntriesPerSide = 20;

    MoveHistorySubscriber() = default;

    void onEvent(const GameEvent& event) override;

    const std::vector<MoveHistoryEntry>& whiteEntries() const;
    const std::vector<MoveHistoryEntry>& blackEntries() const;

    static std::string formatEntry(const MoveHistoryEntry& entry);
};

#endif
