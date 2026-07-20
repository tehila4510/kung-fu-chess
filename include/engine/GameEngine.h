#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H
#include "model/Board.h"
#include "model/GameState.h"
#include "realtime/MotionView.h"
#include "realtime/RestView.h"
#include "rules/RuleEngine.h"
#include "realtime/RealTimeArbiter.h"

#include <set>
#include <vector>

struct MoveOutcome { bool is_accepted; std::string reason; };

class GameEngine {
    GameState gameState;
    RuleEngine ruleEngine;
    RealTimeArbiter arbiter;
    std::vector<ArrivalEvent> lastArrivals_;
public:
    void setup(Board board);
    MoveOutcome requestMove(const Position& from, const Position& to);
    MoveOutcome requestJump(const Position& at);
    void wait(int ms);
    GameSnapshot snapshot() const;
    std::vector<MotionView> activeMotions() const;
    std::vector<RestView> activeRests() const;
    const std::vector<ArrivalEvent>& lastArrivals() const;
    bool isResting(const Position& at) const;
    std::set<Position> legalMovesFrom(const Position& from) const;
    bool isGameOver() const;
    long long elapsedMs() const;
    int rowCount() const;
    int columnCount() const;
};
#endif
