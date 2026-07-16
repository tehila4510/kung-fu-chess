#ifndef GRAPHICS_PIECE_VISUAL_H
#define GRAPHICS_PIECE_VISUAL_H

#include "graphics/Animation.h"
#include "graphics/AnimationCache.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace graphics {

static constexpr const char* kIdleState = "idle";
static constexpr const char* kMoveState = "move";
static constexpr const char* kJumpState = "jump";
static constexpr const char* kShortRestState = "short_rest";
static constexpr const char* kLongRestState = "long_rest";

struct PieceVisual {
    std::string token;
    std::map<std::string, Animation> animations;

    Animation& animationFor(const std::string& state);
    const Animation& animationFor(const std::string& state) const;
};

const std::vector<std::string>& defaultPieceStates();

PieceVisual makePieceVisual(AnimationCache& cache, const std::string& token,
                            std::pair<int, int> cell_size,
                            const std::vector<std::string>& states);

}  // namespace graphics

#endif
