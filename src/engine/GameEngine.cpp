#include "engine/GameEngine.h"

#include <stdexcept>
#include <vector>

static bool isKing(const std::string& piece) {
    return piece.size() == 2 && piece[1] == 'K';
}

static std::vector<AirborneOccupant> airborneForValidation(const RealTimeArbiter& arbiter) {
    std::vector<AirborneOccupant> occupants;
    for (const MotionView& motion : arbiter.activeMotions()) {
        if (motion.from == motion.to) {
            occupants.push_back(AirborneOccupant{ motion.from, motion.piece });
        }
    }
    return occupants;
}

void GameEngine::setup(Board initialBoard) {
    gameState.initialize(std::move(initialBoard));
}

MoveOutcome GameEngine::requestMove(const Position& from, const Position& to) {
    try {
        if (gameState.isGameOver()) {
            return { false, "game_over" };
        }

        Board& board = gameState.getBoard();

        const MoveValidation validation =
            ruleEngine.validateMove(board, from, to, airborneForValidation(arbiter));
        if (!validation.is_valid) {
            return { false, toString(validation.reason) };
        }

        const Cell& moverCell = board.getCell(from);
        const std::string& mover = moverCell.getContent();
        if (arbiter.isResting(from)) {
            return { false, "piece_resting" };
        }
        if (mover.size() == 2) {
            const char color = mover[0];
            if (arbiter.hasActiveMotion(color)) {
                return { false, "move_in_flight" };
            }
        }

        arbiter.startMotion(moverCell.getContent(), from, to);
        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "runtime_error" };
    }
}

MoveOutcome GameEngine::requestJump(const Position& at) {
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
        if (arbiter.isResting(at)) {
            return { false, "piece_resting" };
        }
        if (arbiter.hasActiveMotion(piece[0])) {
            return { false, "move_in_flight" };
        }

        arbiter.startJump(piece, at);
        board.setCell(at, ".");
        return { true, "ok" };
    } catch (const std::exception&) {
        return { false, "runtime_error" };
    }
}

void GameEngine::wait(int ms) {
    if (ms < 0) {
        throw std::invalid_argument("wait milliseconds must be non-negative");
    }

    lastArrivals_ = arbiter.advanceTime(ms, gameState.getBoard());
    for (const ArrivalEvent& arrival : lastArrivals_) {
        if (isKing(arrival.capturedPiece)) {
            gameState.setGameOver(true);
        }
    }
}

const std::vector<ArrivalEvent>& GameEngine::lastArrivals() const {
    return lastArrivals_;
}

bool GameEngine::isGameOver() const {
    return gameState.isGameOver();
}

long long GameEngine::elapsedMs() const {
    return arbiter.elapsedMs();
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

std::vector<MotionView> GameEngine::activeMotions() const {
    return arbiter.activeMotions();
}

std::vector<RestView> GameEngine::activeRests() const {
    return arbiter.activeRests();
}

bool GameEngine::isResting(const Position& at) const {
    return arbiter.isResting(at);
}

std::set<Position> GameEngine::legalMovesFrom(const Position& from) const {
    try {
        if (arbiter.isResting(from)) {
            return {};
        }
        const Board& board = gameState.getBoard();
        return ruleEngine.legalMoves(board, from, airborneForValidation(arbiter));
    } catch (const std::exception&) {
        return {};
    }
}
