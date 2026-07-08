#include "Game.h"
#include "MoveRules.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>

bool Game::isValidPlayerColor(char playerColor) {
    return playerColor == 'w' || playerColor == 'b';
}

int Game::colorToIndex(char playerColor) {
    return playerColor == 'w' ? 0 : 1;
}

Game::PlayerSelection& Game::selectionFor(char playerColor) {
    return selections[colorToIndex(playerColor)];
}

const Game::PlayerSelection& Game::selectionFor(char playerColor) const {
    return selections[colorToIndex(playerColor)];
}

bool Game::setup(const std::vector<std::string>& lines, size_t& index) {
    return board.loadFromLines(lines, index);
}

char Game::resolveClickColor(const Position& pos) const {
    if (isPieceSelected('w')) {
        return 'w';
    }
    if (isPieceSelected('b')) {
        return 'b';
    }
    if (!board.isEmpty(pos)) {
        return board.getCell(pos)[0];
    }
    return '\0';
}

bool Game::hasMoveInFlight() const {
    return activeMove.isActive && gameClockMs < activeMove.arrivalTime;
}

void Game::handleClick(int x, int y) {
    if (isGameOver || hasMoveInFlight()) {
        return;
    }
    Position pos = board.pixelToCell(x, y);
    const char playerColor = resolveClickColor(pos);
    if (playerColor == '\0') {
        return;
    }
    handlePlayerClick(pos, playerColor);
}

void Game::handleClick(int x, int y, char playerColor) {
    if (isGameOver || !isValidPlayerColor(playerColor)) {
        return;
    }
    handlePlayerClick(board.pixelToCell(x, y), playerColor);
}

void Game::handlePlayerClick(const Position& pos, char playerColor) {
    if (isGameOver || !isValidPlayerColor(playerColor)) {
        return;
    }

    if (hasMoveInFlight()) {
        return;
    }

    if (!board.isWithinBounds(pos)) {
        return;
    }

    PlayerSelection& sel = selectionFor(playerColor);

    if (!sel.active) {
        if (board.isEmpty(pos) || !board.isFriendly(pos, playerColor)) {
            return;
        }
        handleSelectNew(pos, playerColor);
    } else {
        if (board.isFriendly(pos, playerColor)) {
            handleSelectNew(pos, playerColor);
        } else {
            handleMoveRequest(sel.pos, pos, playerColor);
        }
    }
}

void Game::handleSelectNew(const Position& pos, char playerColor) {
    PlayerSelection& sel = selectionFor(playerColor);
    sel.active = true;
    sel.pos = pos;
}

void Game::handleMoveRequest(const Position& from, const Position& to, char playerColor) {
    if (!isValidMove(board, from, to)) {
        return;
    }

    std::string piece = board.getCell(from);

    activeMove.from = from;
    activeMove.to = to;
    activeMove.piece = piece;

    const int dr = std::abs(to.row - from.row);
    const int dc = std::abs(to.col - from.col);
    const long long durationMs = static_cast<long long>(std::max(dr, dc)) * 1000;
    activeMove.arrivalTime = gameClockMs + durationMs;
    activeMove.isActive = true;

    PlayerSelection& sel = selectionFor(playerColor);
    sel.active = false;
    sel.pos = {-1, -1};
}

void Game::handleWait(int ms) {
    gameClockMs += ms;

    if (activeMove.isActive && gameClockMs >= activeMove.arrivalTime) {
        const std::string capturedPiece = board.getCell(activeMove.to);
        board.setCell(activeMove.to, activeMove.piece);
        board.setCell(activeMove.from, ".");
        if (capturedPiece.size() == 2 && capturedPiece[1] == 'K') {
            isGameOver = true;
            selections[0].active = false;
            selections[1].active = false;
        }
        activeMove.isActive = false;
    }

}

void Game::handlePrintBoard() const {
    board.print();
}

bool Game::isPieceSelected(char playerColor) const {
    if (!isValidPlayerColor(playerColor)) {
        return false;
    }
    return selectionFor(playerColor).active;
}

Position Game::getSelectedPosition(char playerColor) const {
    if (!isValidPlayerColor(playerColor)) {
        return {-1, -1};
    }
    return selectionFor(playerColor).pos;
}
