#include "server/GameSession.h"

#include "bus/IGameEventListener.h"
#include "io/BoardParser.h"
#include "protocol/Algebraic.h"

#include <stdexcept>
#include <utility>

namespace {

const char* kStandardBoard = R"(
bR bN bB bK bQ bB bN bR
bP bP bP bP bP bP bP bP
.  .  .  .  .  .  .  .
.  .  .  .  .  .  .  .
.  .  .  .  .  .  .  .
.  .  .  .  .  .  .  .
wP wP wP wP wP wP wP wP
wR wN wB wK wQ wB wN wR
Board:
)";

}  // namespace

class GameSession::EventCollector : public IGameEventListener {
public:
    std::vector<GameEvent> pending;

    void onEvent(const GameEvent& event) override {
        pending.push_back(event);
    }
};

GameSession::GameSession() : collector_(new EventCollector()) {
    bus_.subscribe(collector_);
    startNewGame();
}

GameSession::~GameSession() {
    if (collector_ != nullptr) {
        bus_.unsubscribe(collector_);
        delete collector_;
        collector_ = nullptr;
    }
}

EventBus& GameSession::bus() {
    return bus_;
}

const GameEngine& GameSession::engine() const {
    return engine_;
}

GameEngine& GameSession::engine() {
    return engine_;
}

void GameSession::publish(const GameEvent& event) {
    bus_.publish(event);
}

char GameSession::engineColor(char wireColor) const {
    return (wireColor == 'W') ? 'w' : 'b';
}

std::string GameSession::pieceAt(const Position& at) const {
    const GameSnapshot snap = engine_.snapshot();
    if (at.row < 0 || at.col < 0 ||
        at.row >= static_cast<int>(snap.cells.size()) ||
        at.col >= static_cast<int>(snap.cells[static_cast<size_t>(at.row)].size())) {
        return ".";
    }
    return snap.cells[static_cast<size_t>(at.row)][static_cast<size_t>(at.col)];
}

void GameSession::startNewGame() {
    BoardParseResult parsed = BoardParser().parseText(kStandardBoard);
    if (!parsed.board.has_value()) {
        throw std::runtime_error("Failed to load standard board for server session");
    }
    engine_.setup(std::move(*parsed.board));
    whiteScore_ = 0;
    blackScore_ = 0;
    if (collector_ != nullptr) {
        collector_->pending.clear();
    }
    GameEvent started;
    started.type = GameEventType::GameStarted;
    publish(started);
}

void GameSession::publishArrivals(const GameSnapshot& before,
                                  const GameSnapshot& after) {
    const int rows = static_cast<int>(before.cells.size());
    for (const ArrivalEvent& arrival : engine_.lastArrivals()) {
        if (arrival.piece.size() < 2) {
            continue;
        }
        if (arrival.capturedPiece != "." && !arrival.capturedPiece.empty()) {
            GameEvent captured;
            captured.type = GameEventType::PieceCaptured;
            captured.color = (arrival.piece[0] == 'w') ? 'W' : 'B';
            captured.piece = arrival.piece;
            captured.capturedPiece = arrival.capturedPiece;
            captured.to = protocol::positionToSquare(arrival.at, rows);
            publish(captured);

            const int points = capturePoints(arrival.capturedPiece);
            if (captured.color == 'W') {
                whiteScore_ += points;
            } else {
                blackScore_ += points;
            }
            GameEvent score;
            score.type = GameEventType::ScoreUpdated;
            score.whiteScore = whiteScore_;
            score.blackScore = blackScore_;
            publish(score);
        }

        if (arrival.promoted) {
            GameEvent promoted;
            promoted.type = GameEventType::PiecePromoted;
            promoted.color = (arrival.piece[0] == 'w') ? 'W' : 'B';
            promoted.piece = arrival.piece;
            promoted.to = protocol::positionToSquare(arrival.at, rows);
            promoted.reason = "pawn_to_queen";
            publish(promoted);
        }
    }

    if (!before.gameOver && after.gameOver) {
        GameEvent ended;
        ended.type = GameEventType::GameEnded;
        ended.reason = "king_captured";
        ended.whiteScore = whiteScore_;
        ended.blackScore = blackScore_;
        publish(ended);
    }
}

std::vector<GameEvent> GameSession::drainEvents() {
    if (collector_ == nullptr) {
        return {};
    }
    std::vector<GameEvent> out;
    out.swap(collector_->pending);
    return out;
}

SessionResult GameSession::tick(int ms) {
    SessionResult result;
    result.accepted = true;
    result.reason = "ok";
    if (ms <= 0) {
        result.events = drainEvents();
        return result;
    }

    const GameSnapshot before = engine_.snapshot();
    engine_.wait(ms);
    const GameSnapshot after = engine_.snapshot();
    publishArrivals(before, after);
    result.events = drainEvents();
    return result;
}

SessionResult GameSession::handleCommand(char playerColor, const std::string& line) {
    SessionResult result;
    const int rows = engine_.rowCount();
    const int cols = engine_.columnCount();
    const protocol::ParseCommandResult parsed =
        protocol::parseWireCommand(line, rows, cols);
    if (!parsed.ok) {
        result.reason = parsed.error;
        result.events = drainEvents();
        return result;
    }

    const protocol::WireCommand& cmd = parsed.command;
    if (cmd.kind == protocol::WireCommandKind::State) {
        result.accepted = true;
        result.reason = "ok";
        result.events = drainEvents();
        return result;
    }

    if (cmd.kind == protocol::WireCommandKind::Wait) {
        return tick(cmd.waitMs);
    }

    if (cmd.color != playerColor) {
        result.reason = "wrong_color";
        result.events = drainEvents();
        return result;
    }

    if (cmd.kind == protocol::WireCommandKind::Move) {
        const std::string piece = pieceAt(cmd.from);
        if (piece.size() != 2 || piece[0] != engineColor(playerColor)) {
            result.reason = "not_your_piece";
            result.events = drainEvents();
            return result;
        }
        const MoveOutcome outcome = engine_.requestMove(cmd.from, cmd.to);
        result.accepted = outcome.is_accepted;
        result.reason = outcome.reason;
        if (outcome.is_accepted) {
            GameEvent moveEvent;
            moveEvent.type = GameEventType::MoveMade;
            moveEvent.color = playerColor;
            moveEvent.piece = piece;
            moveEvent.from = protocol::positionToSquare(cmd.from, rows);
            moveEvent.to = protocol::positionToSquare(cmd.to, rows);
            publish(moveEvent);
        }
        result.events = drainEvents();
        return result;
    }

    if (cmd.kind == protocol::WireCommandKind::Jump) {
        const std::string piece = pieceAt(cmd.from);
        if (piece.size() != 2 || piece[0] != engineColor(playerColor)) {
            result.reason = "not_your_piece";
            result.events = drainEvents();
            return result;
        }
        const MoveOutcome outcome = engine_.requestJump(cmd.from);
        result.accepted = outcome.is_accepted;
        result.reason = outcome.reason;
        if (outcome.is_accepted) {
            GameEvent jumpEvent;
            jumpEvent.type = GameEventType::JumpMade;
            jumpEvent.color = playerColor;
            jumpEvent.piece = piece;
            jumpEvent.from = protocol::positionToSquare(cmd.from, rows);
            jumpEvent.to = jumpEvent.from;
            publish(jumpEvent);
        }
        result.events = drainEvents();
        return result;
    }

    result.reason = "unknown_command";
    result.events = drainEvents();
    return result;
}
