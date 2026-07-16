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
    Capture
};

struct CellOverlay {
    int center_x = 0;
    int center_y = 0;
    int radius = 0;
    HighlightKind kind = HighlightKind::Move;
};

class Renderer {
public:
    Renderer(Img background, std::string window_name);
    int showFrame(const std::vector<PlacedSprite>& sprites,
                  const std::vector<CellOverlay>& overlays, int wait_ms) const;

    bool isOpen() const;

    void close() const;

private:
    Img background_;
    std::string window_name_;
};

}  // namespace view

#endif
