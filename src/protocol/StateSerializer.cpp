#include "protocol/StateSerializer.h"

#include "protocol/Algebraic.h"

#include <nlohmann/json.hpp>

namespace protocol {
namespace {

nlohmann::json eventToJson(const GameEvent& event) {
    nlohmann::json j = {
        { "type", toString(event.type) },
        { "color", std::string(1, event.color) },
        { "from", event.from },
        { "to", event.to },
        { "piece", event.piece },
        { "capturedPiece", event.capturedPiece },
        { "whiteScore", event.whiteScore },
        { "blackScore", event.blackScore },
        { "reason", event.reason }
    };
    return j;
}

std::string restKindToString(RestKind kind) {
    return kind == RestKind::Short ? "short" : "long";
}

}  // namespace

std::string serializeGameStateJson(const GameEngine& engine,
                                   const std::vector<GameEvent>& events,
                                   const std::string& status,
                                   const std::string& reason) {
    const GameSnapshot snap = engine.snapshot();
    const int rows = engine.rowCount();

    nlohmann::json board = nlohmann::json::array();
    for (const auto& row : snap.cells) {
        board.push_back(row);
    }

    nlohmann::json motions = nlohmann::json::array();
    for (const MotionView& motion : engine.activeMotions()) {
        motions.push_back({
            { "piece", motion.piece },
            { "from", positionToSquare(motion.from, rows) },
            { "to", positionToSquare(motion.to, rows) },
            { "progress", motion.progress }
        });
    }

    nlohmann::json rests = nlohmann::json::array();
    for (const RestView& rest : engine.activeRests()) {
        rests.push_back({
            { "piece", rest.piece },
            { "at", positionToSquare(rest.at, rows) },
            { "kind", restKindToString(rest.kind) },
            { "remaining", rest.remaining }
        });
    }

    nlohmann::json eventArr = nlohmann::json::array();
    for (const GameEvent& event : events) {
        eventArr.push_back(eventToJson(event));
    }

    nlohmann::json root = {
        { "type", "state" },
        { "status", status },
        { "reason", reason },
        { "gameOver", snap.gameOver },
        { "board", board },
        { "motions", motions },
        { "rests", rests },
        { "events", eventArr }
    };
    return root.dump();
}

std::string serializeErrorJson(const std::string& reason) {
    nlohmann::json root = {
        { "type", "error" },
        { "reason", reason }
    };
    return root.dump();
}

std::string serializeWelcomeJson(char assignedColor) {
    nlohmann::json root = {
        { "type", "welcome" },
        { "color", std::string(1, assignedColor) },
        { "message", "assigned_seat" }
    };
    return root.dump();
}

std::string serializeAuthRequiredJson() {
    nlohmann::json root = {
        { "type", "auth_required" }
    };
    return root.dump();
}

std::string serializeAuthOkJson(const std::string& username, int rating,
                                const std::string& role) {
    nlohmann::json root = {
        { "type", "auth_ok" },
        { "username", username },
        { "rating", rating },
        { "role", role }
    };
    return root.dump();
}

std::string serializeSearchingJson() {
    nlohmann::json root = {
        { "type", "searching" }
    };
    return root.dump();
}

}  // namespace protocol
