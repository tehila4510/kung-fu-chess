#include "doctest.h"
#include "server/MatchQueue.h"

#include <chrono>

using Clock = std::chrono::steady_clock;
using Ms = std::chrono::milliseconds;

namespace {

Clock::time_point at(int ms) {
    return Clock::time_point(Ms(ms));
}

}  // namespace

TEST_CASE("MatchQueue empty and single waiter") {
    MatchQueue queue;

    SUBCASE("tryMatch on empty queue returns no match") {
        CHECK_FALSE(queue.tryMatch(1).has_value());
        CHECK(queue.size() == 0);
    }

    SUBCASE("single waiter cannot self-match") {
        REQUIRE(queue.enqueue(1, 1500, at(0)));
        CHECK_FALSE(queue.tryMatch(1).has_value());
        CHECK(queue.contains(1));
        CHECK(queue.size() == 1);
    }
}

TEST_CASE("MatchQueue ELO window boundary") {
    MatchQueue queue;

    SUBCASE("exactly 100 apart matches") {
        REQUIRE(queue.enqueue(1, 1500, at(0)));
        REQUIRE(queue.enqueue(2, 1600, at(10)));
        const auto matched = queue.tryMatch(2);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 1);
        CHECK(matched->black == 2);
        CHECK(queue.size() == 0);
    }

    SUBCASE("exactly 101 apart does not match") {
        REQUIRE(queue.enqueue(1, 1500, at(0)));
        REQUIRE(queue.enqueue(2, 1601, at(10)));
        CHECK_FALSE(queue.tryMatch(2).has_value());
        CHECK(queue.contains(1));
        CHECK(queue.contains(2));
        CHECK(queue.size() == 2);
    }

    SUBCASE("exactly 100 apart matches on the low side") {
        REQUIRE(queue.enqueue(1, 1500, at(0)));
        REQUIRE(queue.enqueue(2, 1400, at(10)));
        const auto matched = queue.tryMatch(2);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 1);
        CHECK(matched->black == 2);
    }

    SUBCASE("exactly 101 apart does not match on the low side") {
        REQUIRE(queue.enqueue(1, 1500, at(0)));
        REQUIRE(queue.enqueue(2, 1399, at(10)));
        CHECK_FALSE(queue.tryMatch(2).has_value());
        CHECK(queue.size() == 2);
    }
}

TEST_CASE("MatchQueue earliest queuedAt among multiple candidates") {
    MatchQueue queue;

    // Three waiters in range; earliest queuedAt wins (not map iteration order).
    REQUIRE(queue.enqueue(10, 1520, at(30)));  // later
    REQUIRE(queue.enqueue(11, 1480, at(5)));   // earliest
    REQUIRE(queue.enqueue(12, 1550, at(20)));  // middle

    REQUIRE(queue.enqueue(99, 1500, at(100)));
    const auto matched = queue.tryMatch(99);
    REQUIRE(matched.has_value());
    CHECK(matched->white == 11);
    CHECK(matched->black == 99);
    CHECK_FALSE(queue.contains(11));
    CHECK_FALSE(queue.contains(99));
    CHECK(queue.contains(10));
    CHECK(queue.contains(12));
    CHECK(queue.size() == 2);
}

TEST_CASE("MatchQueue same-rating bucket is FIFO by queuedAt") {
    MatchQueue queue;

    REQUIRE(queue.enqueue(1, 1500, at(0)));
    REQUIRE(queue.enqueue(2, 1500, at(10)));
    REQUIRE(queue.enqueue(3, 1500, at(20)));
    CHECK(queue.bucketSize(1500) == 3);

    REQUIRE(queue.enqueue(9, 1500, at(100)));
    const auto first = queue.tryMatch(9);
    REQUIRE(first.has_value());
    CHECK(first->white == 1);
    CHECK(first->black == 9);
    CHECK(queue.bucketSize(1500) == 2);
    CHECK(queue.contains(2));
    CHECK(queue.contains(3));
    CHECK_FALSE(queue.contains(1));
    CHECK_FALSE(queue.contains(9));

    REQUIRE(queue.enqueue(8, 1500, at(200)));
    const auto second = queue.tryMatch(8);
    REQUIRE(second.has_value());
    CHECK(second->white == 2);
    CHECK(second->black == 8);
    CHECK(queue.contains(3));
    CHECK(queue.size() == 1);
}

TEST_CASE("MatchQueue removal clears both players and empty buckets") {
    MatchQueue queue;

    // Leftovers outside the joiner's ±100 window so they are not candidates.
    REQUIRE(queue.enqueue(1, 1200, at(0)));
    REQUIRE(queue.enqueue(2, 1250, at(5)));
    REQUIRE(queue.enqueue(3, 1500, at(10)));

    REQUIRE(queue.enqueue(4, 1500, at(50)));
    const auto matched = queue.tryMatch(4);
    REQUIRE(matched.has_value());
    CHECK(matched->white == 3);
    CHECK(matched->black == 4);

    CHECK_FALSE(queue.contains(3));
    CHECK_FALSE(queue.contains(4));
    CHECK(queue.bucketSize(1500) == 0);
    CHECK(queue.contains(1));
    CHECK(queue.contains(2));
    CHECK(queue.bucketSize(1200) == 1);
    CHECK(queue.bucketSize(1250) == 1);
    CHECK(queue.bucketCount() == 2);
    CHECK(queue.size() == 2);
}

TEST_CASE("MatchQueue timeout expiry boundary") {
    MatchQueue queue;
    const auto t0 = at(0);

    REQUIRE(queue.enqueue(1, 1500, t0));
    REQUIRE(queue.enqueue(2, 1600, t0));

    SUBCASE("age exactly 60000 ms is expired") {
        const auto expired = queue.expire(t0 + Ms(MatchQueue::kQueueTimeoutMs));
        CHECK(expired.size() == 2);
        CHECK(queue.size() == 0);
        CHECK(queue.bucketCount() == 0);
    }

    SUBCASE("age just under 60000 ms is not expired") {
        const auto expired =
            queue.expire(t0 + Ms(MatchQueue::kQueueTimeoutMs - 1));
        CHECK(expired.empty());
        CHECK(queue.contains(1));
        CHECK(queue.contains(2));
        CHECK(queue.size() == 2);
    }

    SUBCASE("only the timed-out waiter is removed") {
        queue.remove(1);
        queue.remove(2);
        REQUIRE(queue.enqueue(1, 1500, t0));
        REQUIRE(queue.enqueue(2, 1600, t0 + Ms(1000)));
        const auto expired =
            queue.expire(t0 + Ms(MatchQueue::kQueueTimeoutMs));
        REQUIRE(expired.size() == 1);
        CHECK(expired[0] == 1);
        CHECK_FALSE(queue.contains(1));
        CHECK(queue.contains(2));
    }
}

TEST_CASE("MatchQueue re-enqueue after expiry or match") {
    MatchQueue queue;
    const auto t0 = at(0);

    SUBCASE("after expiry") {
        REQUIRE(queue.enqueue(1, 1500, t0));
        const auto expired =
            queue.expire(t0 + Ms(MatchQueue::kQueueTimeoutMs));
        REQUIRE(expired.size() == 1);
        CHECK_FALSE(queue.contains(1));

        REQUIRE(queue.enqueue(1, 1500, t0 + Ms(70000)));
        CHECK(queue.contains(1));
        CHECK(queue.size() == 1);
        CHECK_FALSE(queue.tryMatch(1).has_value());
    }

    SUBCASE("after match") {
        REQUIRE(queue.enqueue(1, 1500, t0));
        REQUIRE(queue.enqueue(2, 1500, t0 + Ms(10)));
        REQUIRE(queue.tryMatch(2).has_value());
        CHECK(queue.size() == 0);

        REQUIRE(queue.enqueue(1, 1510, t0 + Ms(100)));
        REQUIRE(queue.enqueue(2, 1520, t0 + Ms(110)));
        const auto matched = queue.tryMatch(2);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 1);
        CHECK(matched->black == 2);
    }
}

TEST_CASE("MatchQueue low rating window uses signed math") {
    MatchQueue queue;

    REQUIRE(queue.enqueue(1, 0, at(0)));
    REQUIRE(queue.enqueue(2, 50, at(10)));

    // rating 50 searches [ -50, 150 ]; opponent at 0 is in range.
    const auto matched = queue.tryMatch(2);
    REQUIRE(matched.has_value());
    CHECK(matched->white == 1);
    CHECK(matched->black == 2);

    REQUIRE(queue.enqueue(3, 50, at(0)));
    REQUIRE(queue.enqueue(4, 151, at(10)));  // 101 apart from 50
    CHECK_FALSE(queue.tryMatch(4).has_value());
    CHECK(queue.size() == 2);
}

TEST_CASE("MatchQueue color assignment is earlier White, joiner Black") {
    MatchQueue queue;

    SUBCASE("different ratings") {
        REQUIRE(queue.enqueue(7, 1400, at(0)));
        REQUIRE(queue.enqueue(8, 1450, at(50)));
        const auto matched = queue.tryMatch(8);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 7);
        CHECK(matched->black == 8);
    }

    SUBCASE("same rating") {
        REQUIRE(queue.enqueue(3, 1500, at(5)));
        REQUIRE(queue.enqueue(4, 1500, at(15)));
        const auto matched = queue.tryMatch(4);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 3);
        CHECK(matched->black == 4);
    }

    SUBCASE("joiner is always Black even if rating is lower") {
        REQUIRE(queue.enqueue(1, 1600, at(0)));
        REQUIRE(queue.enqueue(2, 1500, at(20)));
        const auto matched = queue.tryMatch(2);
        REQUIRE(matched.has_value());
        CHECK(matched->white == 1);
        CHECK(matched->black == 2);
    }
}

TEST_CASE("MatchQueue enqueue rejects duplicate id") {
    MatchQueue queue;
    REQUIRE(queue.enqueue(1, 1500, at(0)));
    CHECK_FALSE(queue.enqueue(1, 1600, at(10)));
    CHECK(queue.size() == 1);
    CHECK(queue.bucketSize(1500) == 1);
    CHECK(queue.bucketSize(1600) == 0);
}
