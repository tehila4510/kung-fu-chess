#include "doctest.h"
#include "rules/RuleEngine.h"
#include "io/BoardParser.h"

#include <string>
#include <vector>

namespace {

Board loadBoard(const std::vector<std::string>& rows) {
    std::string text;
    for (const std::string& row : rows) {
        text += row;
        text += '\n';
    }

    BoardParseResult parsed = BoardParser().parseText(text);
    REQUIRE(parsed.board.has_value());
    return std::move(*parsed.board);
}

bool hasReason(const MoveValidation& v, MoveResult expected) {
    return v.reason == expected;
}

}

TEST_CASE("RuleEngine.validateMove reports reasons") {
    RuleEngine engine;

    SUBCASE("out of bounds") {
        Board board = loadBoard({ ". wK . .", ". . . ." });
        CHECK(toString(engine.validateMove(board, { 0, 1 }, { 5, 5 }).reason) == "outside_board");
        CHECK(toString(engine.validateMove(board, { 5, 5 }, { 0, 1 }).reason) == "outside_board");
    }

    SUBCASE("empty source") {
        Board board = loadBoard({ ". . . ." });
        MoveValidation v = engine.validateMove(board, { 0, 0 }, { 0, 1 });
        CHECK_FALSE(v.is_valid);
        CHECK(hasReason(v, MoveResult::EmptySource));
    }

    SUBCASE("friendly destination") {
        Board board = loadBoard({ "wR wP . .", ". . . ." });
        MoveValidation v = engine.validateMove(board, { 0, 0 }, { 0, 1 });
        CHECK_FALSE(v.is_valid);
        CHECK(hasReason(v, MoveResult::FriendlyDestination));
    }

    SUBCASE("illegal piece move") {
        Board board = loadBoard({
            "wR . . .",
            ". . . .",
            ". . . .",
            ". . . ." });
        MoveValidation v = engine.validateMove(board, { 0, 0 }, { 1, 1 });
        CHECK_FALSE(v.is_valid);
        CHECK(hasReason(v, MoveResult::IllegalPieceMove));
    }

    SUBCASE("legal move accepted") {
        Board board = loadBoard({
            "wR . . .",
            ". . . .",
            ". . . .",
            ". . . ." });
        MoveValidation v = engine.validateMove(board, { 0, 0 }, { 0, 3 });
        CHECK(v.is_valid);
        CHECK(hasReason(v, MoveResult::Ok));
    }
}
