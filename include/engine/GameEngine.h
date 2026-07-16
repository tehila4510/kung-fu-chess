#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H
#include "model/Board.h"
#include "model/GameState.h"
#include "realtime/MotionView.h"
#include "rules/RuleEngine.h"
#include "realtime/RealTimeArbiter.h"

#include <vector>

struct MoveOutcome { bool is_accepted; std::string reason; };

class GameEngine {
    GameState gameState;
    RuleEngine ruleEngine;
    RealTimeArbiter arbiter;
public:
    void setup(Board board);
    MoveOutcome requestMove(const Position& from, const Position& to);
    MoveOutcome requestJump(const Position& at);
    void wait(int ms);
    GameSnapshot snapshot() const;
    std::vector<MotionView> activeMotions() const;
    bool isGameOver() const;
    int rowCount() const;
    int columnCount() const;
};
#endif
