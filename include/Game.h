#ifndef GAME_H
#define GAME_H

#include "Board.h"

class Game {
private:
    struct PlayerSelection {
        bool active = false;
        Position pos = {-1, -1};
    };

    struct ActiveMove {
        Position from;
        Position to;
        std::string piece;
        long long arrivalTime;
        bool isActive = false;
    };
    Board board;
    long long gameClockMs = 0;

    PlayerSelection selections[2]; // 0=white, 1=black
    ActiveMove activeMove;
    
    static bool isValidPlayerColor(char playerColor);
    static int colorToIndex(char playerColor);
    PlayerSelection& selectionFor(char playerColor);
    const PlayerSelection& selectionFor(char playerColor) const;

    void handleSelectNew(const Position& pos, char playerColor);
    void handleMoveRequest(const Position& from, const Position& to, char playerColor);
    char resolveClickColor(const Position& pos) const;
    bool hasMoveInFlight() const;

public:
    bool setup(const std::vector<std::string>& lines, size_t& index);
    void handleClick(int x, int y);
    void handleClick(int x, int y, char playerColor);
    void handlePlayerClick(const Position& pos, char playerColor);
    void handleWait(int ms);
    void handlePrintBoard() const;

    const Board& getBoard() const { return board; }
    long long getGameClockMs() const { return gameClockMs; }
    bool isPieceSelected(char playerColor) const;
    Position getSelectedPosition(char playerColor) const;
};

#endif
