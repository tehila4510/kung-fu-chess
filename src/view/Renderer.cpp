#include "view/Renderer.h"

#include <stdexcept>
#include <utility>

namespace view {

Renderer::Renderer(Img background, std::string window_name)
    : background_(std::move(background)), window_name_(std::move(window_name)) {
    if (!background_.is_loaded()) {
        throw std::runtime_error("Renderer requires a loaded background image.");
    }
}

int Renderer::showFrame(const std::vector<PlacedSprite>& sprites, int wait_ms) const {
    Img frame = background_.clone();

    for (const PlacedSprite& sprite : sprites) {
        if (sprite.image == nullptr || !sprite.image->is_loaded()) {
            throw std::runtime_error("Renderer received an unloaded sprite.");
        }
        sprite.image->draw_on(frame, sprite.x, sprite.y);
    }

    return frame.display(window_name_, wait_ms);
}

bool Renderer::isOpen() const {
    return Img::isWindowOpen(window_name_);
}

void Renderer::close() const {
    Img::destroyWindows();
}

}  // namespace view
