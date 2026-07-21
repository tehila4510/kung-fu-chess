#include "server/MatchQueue.h"

#include <cmath>

bool MatchQueue::enqueue(PlayerId id, int rating, TimePoint queuedAt) {
    if (entries_.find(id) != entries_.end()) {
        return false;
    }
    auto& bucket = buckets_[rating];
    bucket.push_back(id);
    Waiter waiter;
    waiter.rating = rating;
    waiter.queuedAt = queuedAt;
    waiter.bucketIt = std::prev(bucket.end());
    entries_.emplace(id, waiter);
    return true;
}

void MatchQueue::remove(PlayerId id) {
    auto entryIt = entries_.find(id);
    if (entryIt == entries_.end()) {
        return;
    }
    const int rating = entryIt->second.rating;
    auto bucketIt = buckets_.find(rating);
    if (bucketIt != buckets_.end()) {
        bucketIt->second.erase(entryIt->second.bucketIt);
        if (bucketIt->second.empty()) {
            buckets_.erase(bucketIt);
        }
    }
    entries_.erase(entryIt);
}

std::optional<MatchQueue::MatchPair> MatchQueue::tryMatch(PlayerId joiner) {
    auto joinerIt = entries_.find(joiner);
    if (joinerIt == entries_.end()) {
        return std::nullopt;
    }

    const int rating = joinerIt->second.rating;
    const int low = rating - kEloWindow;
    const int high = rating + kEloWindow;

    auto begin = buckets_.lower_bound(low);
    auto end = buckets_.upper_bound(high);

    PlayerId best = 0;
    bool found = false;
    TimePoint bestTime{};

    for (auto bucketIt = begin; bucketIt != end; ++bucketIt) {
        for (PlayerId candidate : bucketIt->second) {
            if (candidate == joiner) {
                continue;
            }
            auto candIt = entries_.find(candidate);
            if (candIt == entries_.end()) {
                continue;
            }
            if (std::abs(candIt->second.rating - rating) > kEloWindow) {
                continue;
            }
            if (!found || candIt->second.queuedAt < bestTime) {
                best = candidate;
                bestTime = candIt->second.queuedAt;
                found = true;
            }
        }
    }

    if (!found) {
        return std::nullopt;
    }

    remove(best);
    remove(joiner);
    return MatchPair{best, joiner};
}

std::vector<MatchQueue::PlayerId> MatchQueue::expire(TimePoint now) {
    std::vector<PlayerId> expired;
    for (const auto& entry : entries_) {
        const auto ageMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now - entry.second.queuedAt)
                .count();
        if (ageMs >= kQueueTimeoutMs) {
            expired.push_back(entry.first);
        }
    }
    for (PlayerId id : expired) {
        remove(id);
    }
    return expired;
}

bool MatchQueue::contains(PlayerId id) const {
    return entries_.find(id) != entries_.end();
}

std::size_t MatchQueue::size() const {
    return entries_.size();
}

std::size_t MatchQueue::bucketCount() const {
    return buckets_.size();
}

std::size_t MatchQueue::bucketSize(int rating) const {
    auto it = buckets_.find(rating);
    if (it == buckets_.end()) {
        return 0;
    }
    return it->second.size();
}
