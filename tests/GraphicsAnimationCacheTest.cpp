#include "doctest.h"
#include "graphics/AnimationCache.h"
#include "graphics/IConfigSource.h"
#include "graphics/IFrameSource.h"
#include "view/Img.h"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

class StubFrameSource : public graphics::IFrameSource {
public:
    std::vector<view::Img> loadFrames(const std::string& state_dir,
                                      std::pair<int, int> cell_size) const override {
        ++load_count;
        last_state_dir = state_dir;
        last_cell_size = cell_size;
        if (should_throw) {
            throw std::runtime_error("stub frame load failure");
        }

        std::vector<view::Img> frames;
        frames.resize(frame_count);
        return frames;
    }

    mutable int load_count = 0;
    mutable std::string last_state_dir;
    mutable std::pair<int, int> last_cell_size{0, 0};
    int frame_count = 2;
    bool should_throw = false;
};

class StubConfigSource : public graphics::IConfigSource {
public:
    graphics::GraphicsConfig load(const std::string& state_dir) const override {
        ++load_count;
        last_state_dir = state_dir;
        const auto it = by_dir.find(state_dir);
        if (it != by_dir.end()) {
            return it->second;
        }
        return default_config;
    }

    mutable int load_count = 0;
    mutable std::string last_state_dir;
    graphics::GraphicsConfig default_config{9.0, false};
    std::unordered_map<std::string, graphics::GraphicsConfig> by_dir;
};

}  // namespace

TEST_CASE("AnimationCacheKey equality and hash distinguish size and state") {
    const graphics::AnimationCacheKey a{"KW", "idle", 64, 64};
    const graphics::AnimationCacheKey b{"KW", "idle", 64, 64};
    const graphics::AnimationCacheKey c{"KW", "move", 64, 64};
    const graphics::AnimationCacheKey d{"KW", "idle", 32, 64};
    const graphics::AnimationCacheKeyHash hash;

    CHECK(a == b);
    CHECK_FALSE(a == c);
    CHECK_FALSE(a == d);
    CHECK(hash(a) == hash(b));
    CHECK(hash(a) != hash(c));
    CHECK(hash(a) != hash(d));
}

TEST_CASE("AnimationCache loads once per key and reuses cached specs") {
    StubFrameSource frames;
    StubConfigSource configs;
    configs.default_config = {12.0, false};
    graphics::AnimationCache cache(frames, configs);

    const graphics::AnimationSpec& first = cache.get("KW", "idle", {80, 80});
    CHECK(frames.load_count == 1);
    CHECK(configs.load_count == 1);
    CHECK(first.frames.size() == 2);
    CHECK(first.fps == doctest::Approx(12.0));
    CHECK(first.loop == false);
    CHECK(frames.last_state_dir == "assets/pieces/KW/states/idle");
    CHECK(frames.last_cell_size == std::make_pair(80, 80));

    const graphics::AnimationSpec& second = cache.get("KW", "idle", {80, 80});
    CHECK(frames.load_count == 1);
    CHECK(configs.load_count == 1);
    CHECK(&first == &second);

    (void)cache.get("KW", "move", {80, 80});
    CHECK(frames.load_count == 2);
    CHECK(frames.last_state_dir == "assets/pieces/KW/states/move");

    (void)cache.get("KW", "idle", {40, 80});
    CHECK(frames.load_count == 3);
}

TEST_CASE("AnimationCache propagates frame-source failures") {
    StubFrameSource frames;
    frames.should_throw = true;
    StubConfigSource configs;
    graphics::AnimationCache cache(frames, configs);

    CHECK_THROWS_AS(cache.get("PB", "idle", {16, 16}), std::runtime_error);
    CHECK(frames.load_count == 1);
}
