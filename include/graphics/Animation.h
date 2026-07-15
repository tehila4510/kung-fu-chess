#ifndef GRAPHICS_ANIMATION_H
#define GRAPHICS_ANIMATION_H

#include "view/Img.h"

#include <cstddef>
#include <vector>

namespace graphics {

class Animation {
public:
    Animation(std::vector<view::Img> frames, double fps, bool loop);

    bool update(double dt_seconds);
    const view::Img& current_frame() const;
    std::size_t current_frame_index() const;
    void reset();

private:
    std::vector<view::Img> frames_;
    double fps_;
    bool loop_;
    double elapsed_seconds_;
    std::size_t frame_index_;
    bool finished_;
};

}  // namespace graphics

#endif
