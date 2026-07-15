#include "doctest.h"
#include "engine/GameEngine.h"
#include "input/Controller.h"
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

TEST_CASE("Controller drives selection from cell positions") {
    GameEngine engine = loadEngine({
        "wR . . .",
        ". . . .",
        "bR . . ." });
    Controller controller(engine);

    SUBCASE("first click on a piece selects it") {
        const ClickResult r = controller.click(Position{ 0, 0 });
        CHECK(r.outcome == ClickOutcome::Selected);
        CHECK(controller.hasActiveSelection());
        CHECK(controller.selectedCell() == Position{ 0, 0 });
    }

    SUBCASE("second click requests a move to the destination") {
        controller.click(Position{ 0, 0 });
        const ClickResult r = controller.click(Position{ 0, 3 });
        CHECK(r.outcome == ClickOutcome::MoveRequested);
        CHECK(r.moveResult.is_accepted);
        CHECK_FALSE(controller.hasActiveSelection());
    }

    SUBCASE("clicking an empty cell with no selection is ignored") {
        const ClickResult r = controller.click(Position{ 0, 1 });
        CHECK(r.outcome == ClickOutcome::Ignored);
        CHECK_FALSE(controller.hasActiveSelection());
    }

    SUBCASE("clicking the selected cell again cancels the selection") {
        controller.click(Position{ 0, 0 });
        const ClickResult r = controller.click(Position{ 0, 0 });
        CHECK(r.outcome == ClickOutcome::Cleared);
        CHECK_FALSE(controller.hasActiveSelection());
    }

    SUBCASE("the bottom-left piece is selectable") {
        const ClickResult r = controller.click(Position{ 2, 0 });
        CHECK(r.outcome == ClickOutcome::Selected);
        CHECK(controller.selectedCell() == Position{ 2, 0 });
    }
}
