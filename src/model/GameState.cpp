#include "model/GameState.h"

#include <stdexcept>

void GameState::initialize(Board initialBoard) {
    if (initialBoard.getRowCount() <= 0 || initialBoard.getColCount() <= 0) {
        throw std::invalid_argument("Cannot initialize game state with an empty board");
    }
    board = std::move(initialBoard);
    gameOver = false;
}

Board& GameState::getBoard() {
    return board;
}

const Board& GameState::getBoard() const {
    return board;
}

bool GameState::isGameOver() const {
    return gameOver;
}

void GameState::setGameOver(bool state) {
    gameOver = state;
}

int GameState::rowCount() const {
    return board.getRowCount();
}

int GameState::columnCount() const {
    return board.getColCount();
}

GameSnapshot GameState::createSnapshot() const {
    GameSnapshot snap;
    snap.gameOver = gameOver;

    const int rows = board.getRowCount();
    const int cols = board.getColCount();
    snap.cells.reserve(static_cast<size_t>(rows));
    for (int r = 0; r < rows; ++r) {
        std::vector<std::string> row;
        row.reserve(static_cast<size_t>(cols));
        for (int c = 0; c < cols; ++c) {
            row.push_back(board.getCell(Position{ r, c }).getContent());
        }
        snap.cells.push_back(std::move(row));
    }
    return snap;
}
