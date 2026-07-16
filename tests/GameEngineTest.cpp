#include "doctest.h"
#include "engine/GameEngine.h"
#include "io/BoardParser.h"

#include <string>
#include <vector>

namespace {

GameEngine loadEngine(const std::vector<std::string>& rows) {
    std::string text;
    for (const std::string& row : rows) {
        text += row;
        text += '\n';
    }

    BoardParseResult parsed = BoardParser().parseText(text);
    REQUIRE(parsed.board.has_value());

    GameEngine engine;
    engine.setup(std::move(*parsed.board));
    return engine;
}

}

TEST_CASE("GameEngine orchestrates moves, timing and game over") {
    SUBCASE("snapshot reflects the loaded board") {
        GameEngine engine = loadEngine({ "wR . . bK" });
        const GameSnapshot snap = engine.snapshot();
        CHECK(snap.cells[0][0] == "wR");
        CHECK(snap.cells[0][3] == "bK");
        CHECK_FALSE(snap.gameOver);
    }

    SUBCASE("a legal move is accepted") {
        GameEngine engine = loadEngine({ "wR . . ." });
        const MoveOutcome result = engine.requestMove({ 0, 0 }, { 0, 3 });
        CHECK(result.is_accepted);
    }

    SUBCASE("the moving player is disabled while their piece is in flight") {
        GameEngine engine = loadEngine({
            "wR . . .",
            ". . . .",
            "bR . . ." });

        CHECK(engine.requestMove({ 0, 0 }, { 0, 3 }).is_accepted);
        const MoveOutcome second = engine.requestMove({ 0, 0 }, { 0, 1 });
        CHECK_FALSE(second.is_accepted);
        CHECK(second.reason == "move_in_flight");
    }

    SUBCASE("opposite colors may travel while the other player is in flight") {
        GameEngine engine = loadEngine({
            "wR . . .",
            ". . . .",
            "bR . . ." });

        CHECK(engine.requestMove({ 0, 0 }, { 0, 3 }).is_accepted);
        const MoveOutcome second = engine.requestMove({ 2, 0 }, { 2, 3 });
        CHECK(second.is_accepted);
    }

    SUBCASE("capturing the king ends the game") {
        GameEngine engine = loadEngine({ "wR . . bK" });
        CHECK(engine.requestMove({ 0, 0 }, { 0, 3 }).is_accepted);

        engine.wait(3000);
        CHECK(engine.isGameOver());
        CHECK(engine.snapshot().gameOver);

        const MoveOutcome afterOver = engine.requestMove({ 0, 3 }, { 1, 3 });
        CHECK_FALSE(afterOver.is_accepted);
        CHECK(afterOver.reason == "game_over");
    }
}
