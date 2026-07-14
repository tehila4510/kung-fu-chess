#include "input/Controller.h"

Controller::Controller(GameEngine& engine, int cellSize)
    : engine(engine),
      mapper(cellSize, engine.columnCount(), engine.rowCount()) {}

Controller::Controller(GameEngine& engine, BoardMapper mapper)
    : engine(engine), mapper(mapper) {}

bool Controller::hasActiveSelection() const {
    return hasSelection;
}

Position Controller::selectedCell() const {
    return selection;
}

void Controller::clearSelection() {
    hasSelection = false;
    selection = Position{};
}

static std::string cellToken(const GameSnapshot& snap, const Position& cell) {
    if (cell.row < 0 || cell.row >= static_cast<int>(snap.cells.size())) {
        return ".";
    }
    const std::vector<std::string>& row = snap.cells[static_cast<size_t>(cell.row)];
    if (cell.col < 0 || cell.col >= static_cast<int>(row.size())) {
        return ".";
    }
    return row[static_cast<size_t>(cell.col)];
}

static bool cellHasPiece(const GameSnapshot& snap, const Position& cell) {
    return cellToken(snap, cell) != ".";
}

static bool sameColor(const GameSnapshot& snap, const Position& a, const Position& b) {
    const std::string tokenA = cellToken(snap, a);
    const std::string tokenB = cellToken(snap, b);
    return tokenA != "." && tokenB != "." && tokenA[0] == tokenB[0];
}

ClickResult Controller::click(int x, int y) {
    try {
        const std::optional<Position> mapped = mapper.pixelToCell(x, y);

        if (!mapped) {
            clearSelection();
            return { ClickOutcome::Cleared, { false, "" } };
        }
        const Position cell = *mapped;
        const GameSnapshot snap = engine.snapshot();

        if (!hasSelection) {
            if (!cellHasPiece(snap, cell)) {
                return { ClickOutcome::Ignored, { false, "" } };
            }
            selection = cell;
            hasSelection = true;
            return { ClickOutcome::Selected, { false, "" } };
        }

        if (cell == selection) {
            clearSelection();
            return { ClickOutcome::Cleared, { false, "" } };
        }

        if (cellHasPiece(snap, cell) && sameColor(snap, cell, selection)) {
            selection = cell;
            return { ClickOutcome::Selected, { false, "" } };
        }

        const Position from = selection;
        clearSelection();
        const MoveResult result = engine.requestMove(from, cell);
        return { ClickOutcome::MoveRequested, result };
    } catch (const std::exception&) {
        clearSelection();
        return { ClickOutcome::Cleared, { false, "runtime_error" } };
    }
}

MoveResult Controller::jump(int x, int y) {
    try {
        const std::optional<Position> mapped = mapper.pixelToCell(x, y);
        if (!mapped) {
            return { false, "outside_board" };
        }
        return engine.requestJump(*mapped);
    } catch (const std::exception&) {
        return { false, "runtime_error" };
    }
}
