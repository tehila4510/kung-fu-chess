#include "doctest.h"
#include "graphics/Animation.h"
#include "view/Img.h"

#include <stdexcept>
#include <vector>

namespace {

std::vector<view::Img> makeFrames(std::size_t count) {
    std::vector<view::Img> frames;
    frames.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        frames.emplace_back();
    }
    return frames;
}

}  // namespace

TEST_CASE("Animation rejects invalid construction") {
    SUBCASE("empty frames throw") {
        CHECK_THROWS_AS(graphics::Animation(std::vector<view::Img>{}, 6.0, true),
                        std::runtime_error);
    }

    SUBCASE("non-positive fps throws") {
        CHECK_THROWS_AS(graphics::Animation(makeFrames(1), 0.0, true),
                        std::runtime_error);
        CHECK_THROWS_AS(graphics::Animation(makeFrames(1), -1.0, false),
                        std::runtime_error);
    }
}

TEST_CASE("Animation advances frames for looping playback") {
    graphics::Animation anim(makeFrames(3), 10.0, true);

    SUBCASE("starts on the first frame") {
        CHECK(anim.current_frame_index() == 0);
        CHECK_FALSE(anim.update(0.0));
        CHECK_FALSE(anim.update(-1.0));
        CHECK(anim.current_frame_index() == 0);
    }

    SUBCASE("one frame duration advances by one index") {
        CHECK_FALSE(anim.update(0.1));
        CHECK(anim.current_frame_index() == 1);
    }

    SUBCASE("large dt wraps around while looping") {
        CHECK_FALSE(anim.update(0.35));  // 3.5 frames at 10 fps
        CHECK(anim.current_frame_index() == 0);  // (0 + 3) % 3
    }

    SUBCASE("reset returns to the first frame") {
        anim.update(0.25);
        CHECK(anim.current_frame_index() == 2);
        anim.reset();
        CHECK(anim.current_frame_index() == 0);
        CHECK_FALSE(anim.update(0.05));
        CHECK(anim.current_frame_index() == 0);
    }
}

TEST_CASE("Animation finishes once for non-looping playback") {
    graphics::Animation anim(makeFrames(3), 10.0, false);

    SUBCASE("stops on the last frame and reports finished") {
        CHECK(anim.update(0.35));  // 3 frames at 10 fps overshoots the last
        CHECK(anim.current_frame_index() == 2);
        CHECK(anim.update(1.0));  // already finished
        CHECK(anim.current_frame_index() == 2);
    }

    SUBCASE("exact steps to the last frame do not finish yet") {
        graphics::Animation exact(makeFrames(3), 10.0, false);
        CHECK_FALSE(exact.update(0.2));  // two frames: 0 -> 2
        CHECK(exact.current_frame_index() == 2);
        CHECK(exact.update(0.1));
        CHECK(exact.current_frame_index() == 2);
    }

    SUBCASE("reset clears the finished flag") {
        CHECK(anim.update(1.0));
        anim.reset();
        CHECK_FALSE(anim.update(0.05));
        CHECK(anim.current_frame_index() == 0);
    }
}
