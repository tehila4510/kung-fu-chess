#include "engine/GameEngine.h"

#include <stdexcept>

static bool isKing(const std::string& piece) {
    return piece.size() == 2 && piece[1] == 'K';
}

void GameEngine::setup(Board initialBoard) {
    gameState.initialize(std::move(initialBoard));
}

MoveResult GameEngine::requestMove(const Position& from, const Position& to) {
    try {
        if (gameState.isGameOver()) {
            return { false, "game_over" };
        }

        Board& board = gameState.getBoard();
        const Cell& moverCell = board.getCell(from);
        const std::string& mover = moverCell.getContent();
        if (mover.size() == 2) {
            const char color = mover[0];
            if (arbiter.hasActiveMotion(color)) {
                return { false, "move_in_flight" };
            }
            const char opponent = color == 'w' ? 'b' : 'w';
            if (arbiter.hasActiveTravel(opponent)) {
                return { false, "common_route" };
            }
        }

        const MoveValidation validation = ruleEngine.validateMove(board, from, to);
        if (!validation.is_valid) {
            return { false, validation.reason };
        }

        arbiter.startMotion(moverCell.getContent(), from, to);
        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "runtime_error" };
    }
}

MoveResult GameEngine::requestJump(const Position& at) {
    try {
        if (gameState.isGameOver()) {
            return { false, "game_over" };
        }

        Board& board = gameState.getBoard();
        if (!board.isWithinBounds(at)) {
            return { false, "outside_board" };
        }

        const Cell& cell = board.getCell(at);
        const std::string& piece = cell.getContent();
        if (cell.isEmpty()) {
            return { false, "empty_source" };
        }
        if (arbiter.hasActiveMotion(piece[0])) {
            return { false, "move_in_flight" };
        }

        arbiter.startJump(piece, at);
        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "runtime_error" };
    }
}

void GameEngine::wait(int ms) {
    if (ms < 0) {
        throw std::invalid_argument("wait milliseconds must be non-negative");
    }

    const std::vector<ArrivalEvent> arrivals = arbiter.advanceTime(ms, gameState.getBoard());
    for (const ArrivalEvent& arrival : arrivals) {
        if (isKing(arrival.capturedPiece)) {
            gameState.setGameOver(true);
        }
    }
}

bool GameEngine::isGameOver() const {
    return gameState.isGameOver();
}

int GameEngine::rowCount() const {
    return gameState.rowCount();
}

int GameEngine::columnCount() const {
    return gameState.columnCount();
}

GameSnapshot GameEngine::snapshot() const {
    return gameState.createSnapshot();
}
