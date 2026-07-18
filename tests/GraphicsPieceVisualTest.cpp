#include "doctest.h"
#include "graphics/AnimationCache.h"
#include "graphics/IConfigSource.h"
#include "graphics/IFrameSource.h"
#include "graphics/PieceVisual.h"
#include "view/Img.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

class SelectiveFrameSource : public graphics::IFrameSource {
public:
    std::vector<view::Img> loadFrames(const std::string& state_dir,
                                      std::pair<int, int> /*cell_size*/) const override {
        if (state_dir.find("/missing/") != std::string::npos ||
            state_dir.find("\\missing\\") != std::string::npos ||
            state_dir.find("/jump") != std::string::npos) {
            throw std::runtime_error("missing state assets");
        }
        return std::vector<view::Img>(1);
    }
};

class DefaultConfigSource : public graphics::IConfigSource {
public:
    graphics::GraphicsConfig load(const std::string& /*state_dir*/) const override {
        return graphics::GraphicsConfig{6.0, true};
    }
};

}  // namespace

TEST_CASE("defaultPieceStates lists every playable visual state") {
    const std::vector<std::string>& states = graphics::defaultPieceStates();
    REQUIRE(states.size() == 5);
    CHECK(states[0] == graphics::kIdleState);
    CHECK(states[1] == graphics::kMoveState);
    CHECK(states[2] == graphics::kJumpState);
    CHECK(states[3] == graphics::kShortRestState);
    CHECK(states[4] == graphics::kLongRestState);
}

TEST_CASE("makePieceVisual loads idle and skips missing optional states") {
    SelectiveFrameSource frames;
    DefaultConfigSource configs;
    graphics::AnimationCache cache(frames, configs);

    const graphics::PieceVisual visual = graphics::makePieceVisual(
        cache, "KW", {32, 32}, graphics::defaultPieceStates());

    CHECK(visual.token == "KW");
    CHECK(visual.animations.count(graphics::kIdleState) == 1);
    CHECK(visual.animations.count(graphics::kMoveState) == 1);
    CHECK(visual.animations.count(graphics::kJumpState) == 0);
    CHECK(visual.animations.count(graphics::kShortRestState) == 1);
    CHECK(visual.animations.count(graphics::kLongRestState) == 1);

    // Missing jump falls back to idle.
    CHECK(&visual.animationFor(graphics::kJumpState) ==
          &visual.animationFor(graphics::kIdleState));
    CHECK(&visual.animationFor(graphics::kMoveState) !=
          &visual.animationFor(graphics::kIdleState));
}

TEST_CASE("animationFor throws when idle is absent") {
    graphics::PieceVisual visual;
    visual.token = "KW";
    CHECK_THROWS_AS(visual.animationFor(graphics::kIdleState), std::runtime_error);
    CHECK_THROWS_AS(visual.animationFor(graphics::kMoveState), std::runtime_error);
}

TEST_CASE("makePieceVisual propagates a missing idle state") {
    class IdleMissingFrameSource : public graphics::IFrameSource {
    public:
        std::vector<view::Img> loadFrames(const std::string& /*state_dir*/,
                                          std::pair<int, int> /*cell_size*/) const override {
            throw std::runtime_error("idle missing");
        }
    };

    IdleMissingFrameSource frames;
    DefaultConfigSource configs;
    graphics::AnimationCache cache(frames, configs);

    CHECK_THROWS_AS(
        graphics::makePieceVisual(cache, "PB", {16, 16}, {graphics::kIdleState}),
        std::runtime_error);
}
