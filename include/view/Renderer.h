#ifndef RENDERER_H
#define RENDERER_H

#include "view/Img.h"

#include <string>
#include <vector>

namespace view {

// The Img pointed to must outlive this PlacedSprite. Do not store PlacedSprite
// beyond the frame it was created for (e.g. rebuild each render tick).
struct PlacedSprite {
    const Img* image = nullptr;
    int x = 0;
    int y = 0;
};

enum class HighlightKind {
    Move,
    Capture,
    LongRest,
    ShortRest
};

struct CellOverlay {
    int center_x = 0;
    int center_y = 0;
    int radius = 0;
    // Used by rest cooldown overlays (cell rectangle + remaining fraction).
    int cell_x = 0;
    int cell_y = 0;
    int cell_w = 0;
    int cell_h = 0;
    double remaining = 0.0;
    HighlightKind kind = HighlightKind::Move;
};

// Optional side-panel move history. panel_width <= 0 disables the HUD.
struct HistoryHud {
    std::vector<std::string> white_lines;
    std::vector<std::string> black_lines;
    int panel_width = 0;
    int board_width = 0;
};

class Renderer {
public:
    Renderer(Img background, std::string window_name);
    int showFrame(const std::vector<PlacedSprite>& sprites,
                  const std::vector<CellOverlay>& overlays, int wait_ms,
                  const std::string& banner_text = "",
                  const HistoryHud& history = HistoryHud{}) const;

    bool isOpen() const;

    void close() const;

private:
    Img background_;
    std::string window_name_;
};

}  // namespace view

#endif
