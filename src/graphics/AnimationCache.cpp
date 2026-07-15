#include "graphics/AnimationCache.h"

#include "graphics/AssetPaths.h"

#include <functional>
#include <utility>

namespace graphics {

namespace {

size_t hashCombine(size_t seed, size_t value) noexcept {
    return seed ^ (value + 0x9e3779b9U + (seed << 6) + (seed >> 2));
}

}  // namespace

AnimationCache::AnimationCache()
    : frame_source_(owned_frame_source_),
      config_source_(owned_config_source_) {}

AnimationCache::AnimationCache(IFrameSource& frame_source, IConfigSource& config_source)
    : frame_source_(frame_source),
      config_source_(config_source) {}

bool AnimationCacheKey::operator==(const AnimationCacheKey& other) const {
    return piece_code == other.piece_code && state == other.state &&
           width == other.width && height == other.height;
}

size_t AnimationCacheKeyHash::operator()(const AnimationCacheKey& key) const noexcept {
    size_t seed = std::hash<std::string>{}(key.piece_code);
    seed = hashCombine(seed, std::hash<std::string>{}(key.state));
    seed = hashCombine(seed, std::hash<int>{}(key.width));
    seed = hashCombine(seed, std::hash<int>{}(key.height));
    return seed;
}

std::string AnimationCache::makeStateDir(const std::string& piece_code,
                                         const std::string& state) {
    return AssetPaths::stateDir(piece_code, state);
}

const AnimationSpec& AnimationCache::get(const std::string& piece_code,
                                         const std::string& state,
                                         std::pair<int, int> cell_size) {
    const AnimationCacheKey key{piece_code, state, cell_size.first, cell_size.second};

    const auto cached = cache_.find(key);
    if (cached != cache_.end()) {
        return cached->second;
    }

    const std::string state_dir = makeStateDir(piece_code, state);
    const GraphicsConfig config = config_source_.load(state_dir);

    AnimationSpec spec;
    spec.frames = frame_source_.loadFrames(state_dir, cell_size);
    spec.fps = config.fps;
    spec.loop = config.loop;

    const auto inserted = cache_.emplace(key, std::move(spec));
    return inserted.first->second;
}

}  // namespace graphics
