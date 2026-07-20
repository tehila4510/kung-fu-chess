#include "bus/GameEvent.h"

#include <unordered_map>

namespace {

const std::unordered_map<char, int>& capturePointsByKind() {
    static const std::unordered_map<char, int> kPoints = {
        {'P', 1},
        {'N', 3},
        {'B', 3},
        {'R', 5},
        {'Q', 9},
        {'K', 0},
    };
    return kPoints;
}

}  // namespace

const char* toString(GameEventType type) {
    switch (type) {
        case GameEventType::MoveMade:         return "MoveMade";
        case GameEventType::JumpMade:         return "JumpMade";
        case GameEventType::PieceCaptured:    return "PieceCaptured";
        case GameEventType::PiecePromoted:    return "PiecePromoted";
        case GameEventType::PieceSelected:    return "PieceSelected";
        case GameEventType::SelectionCleared: return "SelectionCleared";
        case GameEventType::GameEnded:        return "GameEnded";
        case GameEventType::ScoreUpdated:     return "ScoreUpdated";
        case GameEventType::GameStarted:      return "GameStarted";
    }
    return "Unknown";
}

int capturePoints(const std::string& capturedPieceToken) {
    if (capturedPieceToken.size() != 2) {
        return 0;
    }
    const auto& points = capturePointsByKind();
    const auto it = points.find(capturedPieceToken[1]);
    return (it != points.end()) ? it->second : 0;
}
