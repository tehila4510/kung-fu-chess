#ifndef GRAPHICS_IFRAME_SOURCE_H
#define GRAPHICS_IFRAME_SOURCE_H

#include "view/Img.h"

#include <string>
#include <utility>
#include <vector>

namespace graphics {

class IFrameSource {
public:
    virtual ~IFrameSource() = default;

    // Loads and sorts the PNG sprite frames of a state dir, scaled to cell_size.
    virtual std::vector<view::Img> loadFrames(const std::string& state_dir,
                                              std::pair<int, int> cell_size) const = 0;
};

}  // namespace graphics

#endif
