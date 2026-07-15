#include "graphics/Animation.h"

#include <stdexcept>
#include <utility>

namespace graphics {

Animation::Animation(std::vector<view::Img> frames, double fps, bool loop)
    : frames_(std::move(frames)),
      fps_(fps),
      loop_(loop),
      elapsed_seconds_(0.0),
      frame_index_(0),
      finished_(false) {
    if (frames_.empty()) {
        throw std::runtime_error("Animation requires at least one frame");
    }
    if (fps_ <= 0.0) {
        throw std::runtime_error("Animation fps must be positive");
    }
}

bool Animation::update(double dt_seconds) {
    if (finished_) {
        return true;
    }

    if (dt_seconds <= 0.0) {
        return false;
    }

    const double frame_duration = 1.0 / fps_;
    elapsed_seconds_ += dt_seconds;

    // O(1): compute whole frames elapsed instead of looping per frame.
    const std::size_t steps =
        static_cast<std::size_t>(elapsed_seconds_ / frame_duration);
    if (steps == 0) {
        return false;
    }
    elapsed_seconds_ -= static_cast<double>(steps) * frame_duration;

    if (loop_) {
        frame_index_ = (frame_index_ + steps) % frames_.size();
        return false;
    }

    const std::size_t frames_to_last = frames_.size() - 1 - frame_index_;
    if (steps <= frames_to_last) {
        frame_index_ += steps;
        return false;
    }

    frame_index_ = frames_.size() - 1;
    finished_ = true;
    elapsed_seconds_ = 0.0;
    return true;
}

const view::Img& Animation::current_frame() const {
    return frames_.at(frame_index_);
}

std::size_t Animation::current_frame_index() const {
    return frame_index_;
}

void Animation::reset() {
    elapsed_seconds_ = 0.0;
    frame_index_ = 0;
    finished_ = false;
}

}  // namespace graphics
