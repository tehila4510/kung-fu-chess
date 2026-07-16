#ifndef GRAPHICS_APPLICATION_H
#define GRAPHICS_APPLICATION_H

#include "engine/GameEngine.h"
#include "graphics/AnimationCache.h"
#include "graphics/BoardLayout.h"
#include "graphics/PieceVisual.h"
#include "input/BoardMapper.h"
#include "input/Controller.h"
#include "model/Position.h"
#include "view/Img.h"
#include "view/Renderer.h"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class GraphicsApplication {
public:
    GraphicsApplication(graphics::BoardLayout layout, view::Img background);

    // Runs until ESC/Q or the window is closed. Returns process exit code.
    int run();

private:
    void handleMouseClick(const std::optional<view::MouseClick>& click);
    std::vector<view::PlacedSprite> buildSprites(double dt_seconds);
    void pruneStaleVisuals(const GameSnapshot& snapshot,
                           const std::vector<MotionView>& motions);
    graphics::PieceVisual& ensureVisual(const Position& cell, const std::string& token);

    static constexpr int kFrameWaitMs = 15;
    static constexpr int kMaxFrameStepMs = 100;
    static const std::string kWindowName;

    graphics::BoardLayout layout_;
    int cell_w_;
    int cell_h_;
    std::pair<int, int> cell_size_;

    GameEngine engine_;
    Controller controller_;
    BoardMapper mapper_;
    graphics::AnimationCache cache_;
    std::map<Position, graphics::PieceVisual> visuals_;
    view::Renderer renderer_;
};

#endif
