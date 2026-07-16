#include "view/Renderer.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace view {
namespace {

const cv::Scalar kBlack(20, 20, 20);
const cv::Scalar kWhite(255, 255, 255);

void drawOverlay(Img& frame, const CellOverlay& overlay) {
    if (overlay.radius <= 0) {
        return;
    }

    if (overlay.kind == HighlightKind::Capture) {
        frame.draw_solid_disc(overlay.center_x, overlay.center_y, overlay.radius, kBlack);
        const int ring_thickness = std::max(2, overlay.radius / 5);
        frame.draw_ring(overlay.center_x, overlay.center_y, overlay.radius, kWhite,
                        ring_thickness);
        return;
    }

    frame.draw_solid_disc(overlay.center_x, overlay.center_y, overlay.radius, kBlack);
}

}  // namespace

Renderer::Renderer(Img background, std::string window_name)
    : background_(std::move(background)), window_name_(std::move(window_name)) {
    if (!background_.is_loaded()) {
        throw std::runtime_error("Renderer requires a loaded background image.");
    }
}

int Renderer::showFrame(const std::vector<PlacedSprite>& sprites,
                      const std::vector<CellOverlay>& overlays, int wait_ms) const {
    Img frame = background_.clone();

    for (const CellOverlay& overlay : overlays) {
        drawOverlay(frame, overlay);
    }

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
