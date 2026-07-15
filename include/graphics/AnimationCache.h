#ifndef GRAPHICS_ANIMATION_CACHE_H
#define GRAPHICS_ANIMATION_CACHE_H

#include "view/Img.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace graphics {

// Immutable animation blueprint: decoded frames plus their playback settings.
// Frames hold reference-counted image data, so copying a spec is cheap.
struct AnimationSpec {
    std::vector<view::Img> frames;
    double fps = 6.0;
    bool loop = true;
};

struct AnimationCacheKey {
    std::string piece_code;
    std::string state;
    int width = 0;
    int height = 0;

    bool operator==(const AnimationCacheKey& other) const;
};

struct AnimationCacheKeyHash {
    size_t operator()(const AnimationCacheKey& key) const noexcept;
};

class AnimationCache {
public:
    // Loads (or returns cached) frames + config for a piece state at a size.
    const AnimationSpec& get(const std::string& piece_code,
                             const std::string& state,
                             std::pair<int, int> cell_size);

private:
    static std::string makeStateDir(const std::string& piece_code,
                                    const std::string& state);

    std::unordered_map<AnimationCacheKey, AnimationSpec, AnimationCacheKeyHash> cache_;
};

}  // namespace graphics

#endif
