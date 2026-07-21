#ifndef MATCH_QUEUE_H
#define MATCH_QUEUE_H

#include <chrono>
#include <cstddef>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

// Rating-bucketed matchmaking queue. STL-only; no websocketpp types.
class MatchQueue {
public:
    using PlayerId = int;
    using TimePoint = std::chrono::steady_clock::time_point;

    static constexpr int kEloWindow = 100;
    static constexpr int kQueueTimeoutMs = 60000;

    struct MatchPair {
        PlayerId white;  // earlier queuedAt
        PlayerId black;  // joiner
    };

    bool enqueue(PlayerId id, int rating, TimePoint queuedAt);
    void remove(PlayerId id);
    std::optional<MatchPair> tryMatch(PlayerId joiner);
    std::vector<PlayerId> expire(TimePoint now);

    bool contains(PlayerId id) const;
    std::size_t size() const;
    std::size_t bucketCount() const;
    std::size_t bucketSize(int rating) const;

private:
    struct Waiter {
        int rating = 0;
        TimePoint queuedAt{};
        std::list<PlayerId>::iterator bucketIt{};
    };

    std::unordered_map<PlayerId, Waiter> entries_;
    std::map<int, std::list<PlayerId>> buckets_;
};

#endif
