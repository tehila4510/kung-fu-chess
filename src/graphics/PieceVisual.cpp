#include "graphics/PieceVisual.h"

#include <stdexcept>
#include <utility>

namespace graphics {

namespace {

Animation makeAnimation(const AnimationSpec& spec) {
    return Animation(spec.frames, spec.fps, spec.loop);
}

const Animation& requireIdle(const std::map<std::string, Animation>& animations) {
    const auto it = animations.find(kIdleState);
    if (it == animations.end()) {
        throw std::runtime_error("PieceVisual is missing the required idle animation");
    }
    return it->second;
}

Animation& requireIdle(std::map<std::string, Animation>& animations) {
    const auto it = animations.find(kIdleState);
    if (it == animations.end()) {
        throw std::runtime_error("PieceVisual is missing the required idle animation");
    }
    return it->second;
}

}  // namespace

Animation& PieceVisual::animationFor(const std::string& state) {
    const auto it = animations.find(state);
    if (it != animations.end()) {
        return it->second;
    }
    return requireIdle(animations);
}

const Animation& PieceVisual::animationFor(const std::string& state) const {
    const auto it = animations.find(state);
    if (it != animations.end()) {
        return it->second;
    }
    return requireIdle(animations);
}

const std::vector<std::string>& defaultPieceStates() {
    static const std::vector<std::string> states = {
        kIdleState, kMoveState, kJumpState, kShortRestState, kLongRestState,
    };
    return states;
}

PieceVisual makePieceVisual(AnimationCache& cache, const std::string& token,
                            std::pair<int, int> cell_size,
                            const std::vector<std::string>& states) {
    PieceVisual visual;
    visual.token = token;

    const AnimationSpec& idle_spec = cache.get(token, kIdleState, cell_size);
    visual.animations.emplace(kIdleState, makeAnimation(idle_spec));

    for (const std::string& state : states) {
        if (state == kIdleState || visual.animations.count(state) != 0) {
            continue;
        }

        try {
            const AnimationSpec& spec = cache.get(token, state, cell_size);
            visual.animations.emplace(state, makeAnimation(spec));
        } catch (const std::exception&) {
            // Missing asset state: playback falls back to idle via animationFor.
        }
    }

    return visual;
}

}  // namespace graphics
