#ifndef REAL_TIME_ARBITER_H
#define REAL_TIME_ARBITER_H
#include "model/Board.h"
#include "realtime/Motion.h"
#include "realtime/MotionView.h"
#include <optional>
#include <string>
#include <vector>

struct ArrivalEvent {
    Position at;
    std::string capturedPiece;
};

class RealTimeArbiter {
    static constexpr int kColorCount = 2;
    static constexpr int kJumpDurationMs = 1000;
    long long clockMs = 0;
    long long nextStartSeq = 0;
    std::optional<Motion> active[kColorCount];
public:
    bool hasActiveMotion() const;
    bool hasActiveMotion(char color) const;
    bool hasActiveTravel(char color) const;
    void startMotion(const std::string& piece, const Position& from, const Position& to);
    void startJump(const std::string& piece, const Position& at);
    std::vector<ArrivalEvent> advanceTime(int ms, Board& board);
    std::vector<MotionView> activeMotions() const;
};
#endif