#ifndef GRAPHICS_APPLICATION_H
#define GRAPHICS_APPLICATION_H

#include "bus/EventBus.h"
#include "bus/MoveHistorySubscriber.h"
#include "bus/SoundSubscriber.h"
#include "engine/GameEngine.h"
#include "graphics/AnimationCache.h"
#include "graphics/BoardLayout.h"
#include "graphics/PieceVisual.h"
#include "input/BoardMapper.h"
#include "input/Controller.h"
#include "model/Position.h"
#include "realtime/RestView.h"
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
    ~GraphicsApplication();

    GraphicsApplication(const GraphicsApplication&) = delete;
    GraphicsApplication& operator=(const GraphicsApplication&) = delete;

    // Runs until ESC/Q or the window is closed. Returns process exit code.
    int run();

private:
    void publish(const GameEvent& event);
    void publishArrivals(const GameSnapshot& before, const GameSnapshot& after);
    void handleMouseClick(const std::optional<view::MouseClick>& click);
    std::vector<view::PlacedSprite> buildSprites(double dt_seconds);
    std::vector<view::CellOverlay> buildLegalMoveOverlays(
        const GameSnapshot& snapshot, const std::vector<MotionView>& motions) const;
    std::vector<view::CellOverlay> buildRestOverlays(
        const std::vector<RestView>& rests) const;
    void pruneStaleVisuals(const GameSnapshot& snapshot,
                           const std::vector<MotionView>& motions);
    graphics::PieceVisual& ensureVisual(const Position& cell, const std::string& token);
    std::string pieceAt(const Position& at) const;

    static constexpr int kFrameWaitMs = 15;
    static constexpr int kMaxFrameStepMs = 100;
    static constexpr int kPanelWidth = 220;
    static const std::string kWindowName;
    static const std::string kSoundsDir;

    graphics::BoardLayout layout_;
    int cell_w_;
    int cell_h_;
    std::pair<int, int> cell_size_;
    int board_w_;
    int panel_w_;

    GameEngine engine_;
    Controller controller_;
    BoardMapper mapper_;
    graphics::AnimationCache cache_;
    std::map<Position, graphics::PieceVisual> visuals_;
    // Tracks which rest kind was last shown so animation resets on landing.
    std::map<Position, RestKind> rest_anim_keys_;
    view::Renderer renderer_;

    EventBus bus_;
    SoundSubscriber sound_;
    MoveHistorySubscriber history_;
};

#endif
