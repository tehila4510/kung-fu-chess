#include "bus/GameEvent.h"

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
    switch (capturedPieceToken[1]) {
        case 'P':
            return 1;
        case 'N':
        case 'B':
            return 3;
        case 'R':
            return 5;
        case 'Q':
            return 9;
        case 'K':
            return 0;
        default:
            return 0;
    }
}
