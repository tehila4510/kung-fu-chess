#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "engine/GameEngine.h"
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
    Position selection;
    bool hasSelection = false;
public:

    explicit Controller(GameEngine& engine);
    ClickResult click(const Position& cell);
    MoveOutcome jump(const Position& cell);
    bool hasActiveSelection() const;
    Position selectedCell() const;
    void clearSelection();
};
#endif
