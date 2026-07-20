#include "view/Renderer.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace view {
namespace {

const cv::Scalar kBlack(20, 20, 20);
const cv::Scalar kWhite(255, 255, 255);
const cv::Scalar kPanelText(220, 220, 220);
const cv::Scalar kLongRestYellow(0, 220, 255);
const cv::Scalar kShortRestBlue(255, 120, 40);
constexpr double kRestOverlayAlpha = 0.45;

void drawOverlay(Img& frame, const CellOverlay& overlay) {
    if (overlay.kind == HighlightKind::LongRest || overlay.kind == HighlightKind::ShortRest) {
        if (overlay.cell_w <= 0 || overlay.cell_h <= 0 || overlay.remaining <= 0.0) {
            return;
        }

        const double remaining = std::clamp(overlay.remaining, 0.0, 1.0);
        const int fill_h = std::max(1, static_cast<int>(overlay.cell_h * remaining));
        const int fill_y = overlay.cell_y + overlay.cell_h - fill_h;
        const cv::Scalar& color = overlay.kind == HighlightKind::LongRest
                                     ? kLongRestYellow
                                     : kShortRestBlue;
        frame.draw_filled_rect(overlay.cell_x, fill_y, overlay.cell_w, fill_h, color,
                               kRestOverlayAlpha);
        return;
    }

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

void drawHistoryColumn(Img& frame, int panel_x, int panel_w, const char* title,
                       const std::vector<std::string>& lines) {
    if (panel_w <= 0) {
        return;
    }
    frame.put_text(title, panel_x + 12, 28, 0.55, kWhite, 1);
    int y = 56;
    const int line_step = 18;
    for (const std::string& line : lines) {
        if (y > frame.height() - 12) {
            break;
        }
        frame.put_text(line, panel_x + 8, y, 0.38, kPanelText, 1);
        y += line_step;
    }
}

}  // namespace

Renderer::Renderer(Img background, std::string window_name)
    : background_(std::move(background)), window_name_(std::move(window_name)) {
    if (!background_.is_loaded()) {
        throw std::runtime_error("Renderer requires a loaded background image.");
    }
}

int Renderer::showFrame(const std::vector<PlacedSprite>& sprites,
                      const std::vector<CellOverlay>& overlays, int wait_ms,
                      const std::string& banner_text,
                      const HistoryHud& history) const {
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

    if (history.panel_width > 0 && history.board_width > 0) {
        drawHistoryColumn(frame, 0, history.panel_width, "WHITE", history.white_lines);
        drawHistoryColumn(frame, history.panel_width + history.board_width,
                          history.panel_width, "BLACK", history.black_lines);
    }

    if (!banner_text.empty()) {
        if (history.panel_width > 0 && history.board_width > 0) {
            frame.draw_filled_rect(history.panel_width, 0, history.board_width,
                                   frame.height(), kBlack, 0.55);
            const double font_size = std::max(1.2, history.board_width / 280.0);
            const int thickness = std::max(2, history.board_width / 200);
            frame.put_text_centered_in_rect(banner_text, history.panel_width, 0,
                                            history.board_width, frame.height(),
                                            font_size, kWhite, thickness);
        } else {
            frame.draw_filled_rect(0, 0, frame.width(), frame.height(), kBlack, 0.55);
            const double font_size = std::max(1.2, frame.width() / 280.0);
            const int thickness = std::max(2, frame.width() / 200);
            frame.put_text_centered(banner_text, font_size, kWhite, thickness);
        }
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
