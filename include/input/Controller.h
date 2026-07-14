#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "engine/GameEngine.h"
#include "input/BoardMapper.h"
#include "model/Position.h"

enum class ClickOutcome {
    Selected,
    Cleared,
    MoveRequested,
    Ignored
};

struct ClickResult {
    ClickOutcome outcome = ClickOutcome::Ignored;
    MoveOutcome moveResult{ false, "" };
};


class Controller {
    GameEngine& engine;
    BoardMapper mapper;
    Position selection;
    bool hasSelection = false;
public:

    explicit Controller(GameEngine& engine, int cellSize = 100);
    Controller(GameEngine& engine, BoardMapper mapper);
    ClickResult click(int x, int y);
    MoveOutcome jump(int x, int y);
    bool hasActiveSelection() const;
    Position selectedCell() const;
    void clearSelection();
};
#endif
