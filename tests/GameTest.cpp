#include "doctest.h"
#include "Game.h"

#include <string>
#include <vector>

static Game makeGame(const std::vector<std::string>& rows) {
    std::vector<std::string> lines = {"Board:"};
    lines.insert(lines.end(), rows.begin(), rows.end());
    lines.push_back("Commands:");

    Game game;
    size_t index = 0;
    game.setup(lines, index);
    return game;
}

TEST_CASE("Game.setupSuccess") {
    Game game = makeGame({". wK . .", ". . . ."});
    CHECK_EQ("wK", game.getBoard().getCell({0, 1}));
}

TEST_CASE("Game.setupFailure") {
    std::vector<std::string> lines = {
        "Board:",
        ". bad . .",
        "Commands:"
    };
    Game game;
    size_t index = 0;
    CHECK_FALSE(game.setup(lines, index));
}

TEST_CASE("Game.selectPiece") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handlePlayerClick({0, 1}, 'w');
    CHECK(game.isPieceSelected('w'));
    CHECK_EQ(0, game.getSelectedPosition('w').row);
    CHECK_EQ(1, game.getSelectedPosition('w').col);
}

TEST_CASE("Game.clickEmptyWithoutSelection") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handlePlayerClick({0, 0}, 'w');
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.clickOutOfBounds") {
    Game game = makeGame({". wK . ."});
    game.handlePlayerClick({0, 1}, 'w');
    CHECK(game.isPieceSelected('w'));
    game.handlePlayerClick({99, 99}, 'w');
    CHECK_EQ("wK", game.getBoard().getCell({0, 1}));
}

TEST_CASE("Game.legalKingMove") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handlePlayerClick({0, 1}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(1000);
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wK", game.getBoard().getCell({0, 2}));
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.illegalKingMoveIgnored") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handlePlayerClick({0, 1}, 'w');
    game.handlePlayerClick({0, 3}, 'w');
    CHECK_EQ("wK", game.getBoard().getCell({0, 1}));
    CHECK_EQ(".", game.getBoard().getCell({0, 3}));
    CHECK(game.isPieceSelected('w'));
}

TEST_CASE("Game.rookDiagonalIgnored") {
    Game game = makeGame({"wR . . .", ". . . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({1, 1}, 'w');
    CHECK_EQ("wR", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({1, 1}));
}

TEST_CASE("Game.reselectFriendlyPiece") {
    Game game = makeGame({"wR wN . .", ". . . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 1}, 'w');
    CHECK(game.isPieceSelected('w'));
    CHECK_EQ(0, game.getSelectedPosition('w').row);
    CHECK_EQ(1, game.getSelectedPosition('w').col);
}

TEST_CASE("Game.captureEnemyPiece") {
    Game game = makeGame({"wR . bQ .", ". . . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(2000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.cannotSelectEnemyPiece") {
    Game game = makeGame({"bK . . .", ". . . ."});
    game.handlePlayerClick({0, 0}, 'w');
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.independentSelections") {
    Game game = makeGame({". wK . .", "bK . . ."});
    game.handlePlayerClick({0, 1}, 'w');
    game.handlePlayerClick({1, 0}, 'b');
    CHECK(game.isPieceSelected('w'));
    CHECK(game.isPieceSelected('b'));
    CHECK_EQ(1, game.getSelectedPosition('w').col);
    CHECK_EQ(0, game.getSelectedPosition('b').col);
}

TEST_CASE("Game.blackCanSelectWhileWhiteHoldsSelection") {
    Game game = makeGame({". wK . .", "bK . . ."});
    game.handlePlayerClick({0, 1}, 'w');
    game.handlePlayerClick({1, 0}, 'b');
    CHECK(game.isPieceSelected('w'));
    CHECK(game.isPieceSelected('b'));
    CHECK_EQ("wK", game.getBoard().getCell({0, 1}));
    CHECK_EQ("bK", game.getBoard().getCell({1, 0}));
}

TEST_CASE("Game.invalidPlayerColorIgnored") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handlePlayerClick({0, 1}, 'x');
    CHECK_FALSE(game.isPieceSelected('w'));
    CHECK_FALSE(game.isPieceSelected('x'));
    CHECK_EQ(-1, game.getSelectedPosition('x').row);
}

TEST_CASE("Game.wait") {
    Game game = makeGame({". wK . ."});
    game.handleWait(500);
    game.handleWait(250);
    CHECK_EQ(750, game.getGameClockMs());
}

TEST_CASE("Game.handleClickSelectsOwnPiece") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handleClick(150, 50);
    CHECK(game.isPieceSelected('w'));
    CHECK_EQ(0, game.getSelectedPosition('w').row);
    CHECK_EQ(1, game.getSelectedPosition('w').col);
}

TEST_CASE("Game.handleClickMoveToEmpty") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handleClick(150, 50);
    game.handleClick(250, 50);
    game.handleWait(1000);
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wK", game.getBoard().getCell({0, 2}));
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.handleClickEmptyWithoutSelectionIgnored") {
    Game game = makeGame({". wK . .", ". . . ."});
    game.handleClick(50, 50);
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.handleClickCaptureUsesSelectionColor") {
    Game game = makeGame({"wR . bR"});
    game.handleClick(50, 50);
    game.handleClick(250, 50);
    game.handleWait(2000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.knightJumpsOverBlockers") {
    Game game = makeGame({"wN wP .", "wP . .", ". . ."});
    game.handleClick(50, 50);
    game.handleClick(150, 250);
    game.handleWait(2000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ("wP", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wP", game.getBoard().getCell({1, 0}));
    CHECK_EQ("wN", game.getBoard().getCell({2, 1}));
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.rookCapturesEnemyAtDestination") {
    Game game = makeGame({"wR . bR"});
    game.handleClick(50, 50);
    game.handleClick(250, 50);
    game.handleWait(2000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.oneCellMoveBeforeArrivalBoardUnchanged") {
    Game game = makeGame({"wR . ."});
    game.handleClick(50, 50);
    game.handleClick(150, 50);
    game.handleWait(500);
    CHECK_EQ("wR", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
}

TEST_CASE("Game.twoCellMoveBeforeAndAfterArrival") {
    Game game = makeGame({"wR . ."});
    game.handleClick(50, 50);
    game.handleClick(250, 50);
    game.handleWait(1000);
    CHECK_EQ("wR", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({0, 2}));
    game.handleWait(1000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.clicksDuringMoveInFlightAreIgnored") {
    Game game = makeGame({"wR . ."});
    game.handleClick(50, 50);
    game.handleClick(250, 50);
    game.handleWait(1000);
    game.handleClick(50, 50);
    game.handleClick(150, 50);
    game.handleWait(1000);
    CHECK_EQ(".", game.getBoard().getCell({0, 0}));
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.kingCaptureEndsGame") {
    Game game = makeGame({"wR . bK"});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    CHECK_FALSE(game.getIsGameOver());
    game.handleWait(2000);
    CHECK(game.getIsGameOver());
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.kingNotCapturedUntilMoveArrives") {
    Game game = makeGame({"wR . bK"});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(1000);
    CHECK_FALSE(game.getIsGameOver());
    CHECK_EQ("bK", game.getBoard().getCell({0, 2}));
    game.handleWait(1000);
    CHECK(game.getIsGameOver());
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}

TEST_CASE("Game.noActionsAfterKingCaptured") {
    Game game = makeGame({"wR . bK", "bR . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(2000);
    CHECK(game.getIsGameOver());

    game.handlePlayerClick({1, 0}, 'b');
    CHECK_FALSE(game.isPieceSelected('b'));
    game.handlePlayerClick({1, 2}, 'b');
    CHECK_EQ("bR", game.getBoard().getCell({1, 0}));
    CHECK_EQ(".", game.getBoard().getCell({1, 2}));

    game.handlePlayerClick({0, 2}, 'w');
    game.handlePlayerClick({0, 1}, 'w');
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
    CHECK_EQ(".", game.getBoard().getCell({0, 1}));
}

TEST_CASE("Game.handleClickBlockedAfterGameOver") {
    Game game = makeGame({"wR . bK", "bR . ."});
    game.handleClick(50, 50);
    game.handleClick(250, 50);
    game.handleWait(2000);
    CHECK(game.getIsGameOver());

    game.handleClick(50, 150);
    game.handleClick(250, 150);
    CHECK_EQ("bR", game.getBoard().getCell({1, 0}));
    CHECK_EQ(".", game.getBoard().getCell({1, 2}));
}

TEST_CASE("Game.capturingNonKingDoesNotEndGame") {
    Game game = makeGame({"wR . bQ", "bK . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(2000);
    CHECK_FALSE(game.getIsGameOver());
    CHECK_EQ("bK", game.getBoard().getCell({1, 0}));

    game.handlePlayerClick({1, 0}, 'b');
    CHECK(game.isPieceSelected('b'));
}

TEST_CASE("Game.blackCapturingWhiteKingEndsGame") {
    Game game = makeGame({"wK . bR"});
    game.handlePlayerClick({0, 2}, 'b');
    game.handlePlayerClick({0, 0}, 'b');
    game.handleWait(2000);
    CHECK(game.getIsGameOver());
    CHECK_EQ("bR", game.getBoard().getCell({0, 0}));

    game.handlePlayerClick({0, 0}, 'w');
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.selectionsClearedWhenKingCaptured") {
    Game game = makeGame({"wR . bK", "bR . ."});
    game.handlePlayerClick({1, 0}, 'b');
    CHECK(game.isPieceSelected('b'));
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handleWait(2000);
    CHECK(game.getIsGameOver());
    CHECK_FALSE(game.isPieceSelected('b'));
    CHECK_FALSE(game.isPieceSelected('w'));
}

TEST_CASE("Game.clicksDuringKingCaptureMoveStillIgnoredUntilArrival") {
    Game game = makeGame({"wR . bK", "bR . ."});
    game.handlePlayerClick({0, 0}, 'w');
    game.handlePlayerClick({0, 2}, 'w');
    game.handlePlayerClick({1, 0}, 'b');
    game.handlePlayerClick({1, 2}, 'b');
    game.handleWait(1000);
    CHECK_FALSE(game.getIsGameOver());
    CHECK_EQ("bK", game.getBoard().getCell({0, 2}));
    CHECK_EQ("bR", game.getBoard().getCell({1, 0}));
    game.handleWait(1000);
    CHECK(game.getIsGameOver());
    CHECK_EQ("wR", game.getBoard().getCell({0, 2}));
}
