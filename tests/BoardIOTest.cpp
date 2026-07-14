#include "doctest.h"
#include "io/BoardParser.h"
#include "io/BoardPrinter.h"

#include <sstream>
#include <string>
#include <vector>

namespace {

std::string layout(const std::vector<std::string>& rows) {
    std::string text;
    for (const std::string& row : rows) {
        text += row;
        text += '\n';
    }
    return text;
}

std::string render(const Board& board) {
    std::ostringstream out;
    BoardPrinter().print(board, out);
    return out.str();
}

}

TEST_CASE("BoardParser derives dimensions dynamically, without hardcoded 8x8") {
    BoardParser parser;

    SUBCASE("4 columns x 3 rows") {
        const auto result = parser.parseText(layout({
            "wR . . bK",
            ".  . . . ",
            "wP wP wP wP" }));
        REQUIRE(result.board.has_value());
        CHECK(result.status == BoardParseStatus::Ok);
        CHECK(result.board->getRowCount() == 3);
        CHECK(result.board->getColCount() == 4);
        CHECK(result.board->getCell(Position{ 0, 0 }).getContent() == "wR");
        CHECK(result.board->getCell(Position{ 0, 3 }).getContent() == "bK");
        CHECK(result.board->getCell(Position{ 2, 2 }).getContent() == "wP");
    }

    SUBCASE("5 x 5") {
        const auto result = parser.parseText(layout({
            "bR bN bB bQ bK",
            ". . . . .",
            ". . . . .",
            ". . . . .",
            "wR wN wB wQ wK" }));
        REQUIRE(result.board.has_value());
        CHECK(result.board->getRowCount() == 5);
        CHECK(result.board->getColCount() == 5);
        CHECK(result.board->getCell(Position{ 0, 3 }).getContent() == "bQ");
        CHECK(result.board->getCell(Position{ 4, 4 }).getContent() == "wK");
    }

    SUBCASE("full 8 x 8") {
        const auto result = parser.parseText(layout({
            "bR bN bB bQ bK bB bN bR",
            "bP bP bP bP bP bP bP bP",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            ". . . . . . . .",
            "wP wP wP wP wP wP wP wP",
            "wR wN wB wQ wK wB wN wR" }));
        REQUIRE(result.board.has_value());
        CHECK(result.board->getRowCount() == 8);
        CHECK(result.board->getColCount() == 8);
        CHECK(result.board->getCell(Position{ 7, 4 }).getContent() == "wK");
    }
}

TEST_CASE("BoardParser reports structured errors instead of crashing") {
    BoardParser parser;

    SUBCASE("unknown token") {
        const auto result = parser.parseText("wR xx . .\n");
        CHECK_FALSE(result.board.has_value());
        CHECK(result.status == BoardParseStatus::UnknownToken);
        CHECK(std::string(toReasonCode(result.status)) == "ERROR UNKNOWN_TOKEN");
    }

    SUBCASE("mismatched row widths") {
        const auto result = parser.parseText(layout({
            "wR . . .",
            "wP wP wP" }));
        CHECK_FALSE(result.board.has_value());
        CHECK(result.status == BoardParseStatus::RowWidthMismatch);
        CHECK(std::string(toReasonCode(result.status)) == "ERROR ROW_WIDTH_MISMATCH");
    }

    SUBCASE("empty input") {
        const auto result = parser.parseText("\n   \n");
        CHECK_FALSE(result.board.has_value());
        CHECK(result.status == BoardParseStatus::EmptyBoard);
    }
}

TEST_CASE("BoardParser honours Board:/Commands: markers and advances the cursor") {
    BoardParser parser;
    const std::vector<std::string> lines = {
        "Board:",
        "wR . . bK",
        ". . . .",
        "Commands:",
        "click 0 0" };

    size_t index = 0;
    const auto result = parser.parseLines(lines, index);
    REQUIRE(result.board.has_value());
    CHECK(result.board->getRowCount() == 2);
    CHECK(result.board->getColCount() == 4);
    CHECK(lines[index] == "click 0 0");
}

TEST_CASE("BoardPrinter renders dynamic geometries and round-trips through the parser") {
    BoardParser parser;

    SUBCASE("output matches the normalized layout for a 4x3 board") {
        const std::string expected =
            "wR . . bK\n"
            ". . . .\n"
            "wP wP wP wP\n";
        const auto parsed = parser.parseText(expected);
        REQUIRE(parsed.board.has_value());
        CHECK(render(*parsed.board) == expected);
    }

    SUBCASE("parse -> print -> parse is stable for a 5x5 board") {
        const std::string original =
            "bR bN bB bQ bK\n"
            ". . . . .\n"
            "wR wN wB wQ wK\n"
            ". . . . .\n"
            "wP wP wP wP wP\n";
        const auto first = parser.parseText(original);
        REQUIRE(first.board.has_value());
        const std::string printed = render(*first.board);
        const auto second = parser.parseText(printed);
        REQUIRE(second.board.has_value());
        CHECK(render(*second.board) == printed);
        CHECK(printed == original);
    }
}
