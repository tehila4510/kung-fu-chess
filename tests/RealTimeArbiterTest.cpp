#include "doctest.h"
#include "realtime/RealTimeArbiter.h"
#include "model/Board.h"
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

}

TEST_CASE("RealTimeArbiter times and resolves motion") {
    SUBCASE("no motion is inactive") {
        Board board = loadBoard({ "wR . . ." });
        RealTimeArbiter arb;
        CHECK_FALSE(arb.hasActiveMotion());
        CHECK(arb.advanceTime(1000, board).empty());
    }

    SUBCASE("motion stays in flight until arrival") {
        Board board = loadBoard({ "wR . . ." });
        RealTimeArbiter arb;
        arb.startMotion("wR", { 0, 0 }, { 0, 3 });
        CHECK(arb.hasActiveMotion());

        CHECK(arb.advanceTime(2999, board).empty());
        CHECK(arb.hasActiveMotion());
        CHECK(board.getCell({ 0, 0 }).getContent() == "wR");
    }

    SUBCASE("arrival moves the piece and clears the motion") {
        Board board = loadBoard({ "wR . . ." });
        RealTimeArbiter arb;
        arb.startMotion("wR", { 0, 0 }, { 0, 3 });

        auto arrivals = arb.advanceTime(3000, board);
        REQUIRE(arrivals.size() == 1);
        CHECK(arrivals[0].at == Position{ 0, 3 });
        CHECK(arrivals[0].capturedPiece == ".");
        CHECK(board.getCell({ 0, 3 }).getContent() == "wR");
        CHECK(board.getCell({ 0, 0 }).getContent() == ".");
        CHECK_FALSE(arb.hasActiveMotion());
    }

    SUBCASE("capture reports the taken piece") {
        Board board = loadBoard({ "wR . . bK" });
        RealTimeArbiter arb;
        arb.startMotion("wR", { 0, 0 }, { 0, 3 });

        auto arrivals = arb.advanceTime(3000, board);
        REQUIRE(arrivals.size() == 1);
        CHECK(arrivals[0].capturedPiece == "bK");
        CHECK(board.getCell({ 0, 3 }).getContent() == "wR");
    }

    SUBCASE("pawn promotes on the far rank") {
        Board board = loadBoard({
            ". . . .",
            "wP . . ." });
        RealTimeArbiter arb;
        arb.startMotion("wP", { 1, 0 }, { 0, 0 });

        auto arrivals = arb.advanceTime(1000, board);
        REQUIRE(arrivals.size() == 1);
        CHECK(board.getCell({ 0, 0 }).getContent() == "wQ");
    }

    SUBCASE("both colors move simultaneously and resolve together") {
        Board board = loadBoard({
            "wR . . .",
            ". . . bR" });
        RealTimeArbiter arb;
        arb.startMotion("wR", { 0, 0 }, { 0, 2 });
        arb.startMotion("bR", { 1, 3 }, { 1, 1 });

        CHECK(arb.hasActiveMotion('w'));
        CHECK(arb.hasActiveMotion('b'));

        auto arrivals = arb.advanceTime(2000, board);
        CHECK(arrivals.size() == 2);
        CHECK(board.getCell({ 0, 2 }).getContent() == "wR");
        CHECK(board.getCell({ 1, 1 }).getContent() == "bR");
        CHECK_FALSE(arb.hasActiveMotion());
    }

    SUBCASE("one color in flight leaves the other free to move") {
        Board board = loadBoard({ "wR . . ." });
        RealTimeArbiter arb;
        arb.startMotion("wR", { 0, 0 }, { 0, 3 });

        CHECK(arb.hasActiveMotion('w'));
        CHECK_FALSE(arb.hasActiveMotion('b'));
    }
}
