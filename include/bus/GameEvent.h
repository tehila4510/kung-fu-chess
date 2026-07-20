#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <string>

enum class GameEventType {
    MoveMade,
    JumpMade,
    PieceCaptured,
    PiecePromoted,
    PieceSelected,
    SelectionCleared,
    GameEnded,
    ScoreUpdated,
    GameStarted
};

struct GameEvent {
    GameEventType type = GameEventType::MoveMade;
    char color = '?';
    std::string from;
    std::string to;
    std::string piece;
    std::string capturedPiece;
    int whiteScore = 0;
    int blackScore = 0;
    std::string reason;
    long long timeMs = 0;
};

const char* toString(GameEventType type);

// Material points for a captured piece token (e.g. "bN" -> 3). King is 0.
int capturePoints(const std::string& capturedPieceToken);

#endif
