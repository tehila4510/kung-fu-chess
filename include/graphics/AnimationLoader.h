#ifndef GRAPHICS_ANIMATION_LOADER_H
#define GRAPHICS_ANIMATION_LOADER_H

#include "view/Img.h"

#include <string>
#include <utility>
#include <vector>

namespace graphics {

class AnimationLoader {
public:
    // Loads and sorts the PNG sprite frames of a state dir, scaled to cell_size.
    static std::vector<view::Img> loadFrames(const std::string& state_dir,
                                       std::pair<int, int> cell_size);
};

}  // namespace graphics

#endif
