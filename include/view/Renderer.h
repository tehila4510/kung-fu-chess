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

class Renderer {
public:
    Renderer(Img background, std::string window_name);
    int showFrame(const std::vector<PlacedSprite>& sprites, int wait_ms) const;

    bool isOpen() const;

    void close() const;

private:
    Img background_;
    std::string window_name_;
};

}  // namespace view

#endif
