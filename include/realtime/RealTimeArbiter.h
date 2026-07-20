#ifndef REAL_TIME_ARBITER_H
#define REAL_TIME_ARBITER_H
#include "model/Board.h"
#include "realtime/Motion.h"
#include "realtime/MotionView.h"
#include "realtime/Rest.h"
#include "realtime/RestView.h"
#include <map>
#include <optional>
#include <string>
#include <vector>

struct ArrivalEvent {
    Position at;
    std::string piece;
    std::string capturedPiece;
    bool promoted = false;
};

class RealTimeArbiter {
    static constexpr int kColorCount = 2;
    long long clockMs = 0;
    long long nextStartSeq = 0;
    std::optional<Motion> active[kColorCount];
    std::map<Position, Rest> rests;
public:
    static constexpr int kJumpDurationMs = 5000;
    static constexpr int kShortRestMs = 1000;
    static constexpr int kLongRestMs = 2500;

    long long elapsedMs() const;
    bool hasActiveMotion() const;
    bool hasActiveMotion(char color) const;
    bool hasActiveTravel(char color) const;
    bool isResting(const Position& at) const;
    void startMotion(const std::string& piece, const Position& from, const Position& to);
    void startJump(const std::string& piece, const Position& at);
    std::vector<ArrivalEvent> advanceTime(int ms, Board& board);
    std::vector<MotionView> activeMotions() const;
    std::vector<RestView> activeRests() const;
};
#endif
