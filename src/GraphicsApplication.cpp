#include "GraphicsApplication.h"

#include "model/Board.h"
#include "realtime/MotionView.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>

namespace {

int requirePositiveCellExtent(int image_extent, int grid_count, const char* axis) {
    if (grid_count <= 0) {
        throw std::runtime_error(std::string("Board layout has invalid ") + axis +
                                 " count.");
    }
    const int cell = image_extent / grid_count;
    if (cell <= 0) {
        throw std::runtime_error("Board image is too small for the layout grid.");
    }
    return cell;
}



// Asset CSV uses "KB"/"RW" (kind + color). The engine uses "bK"/"wR" (color + kind).
std::string assetTokenToEngine(const std::string& asset_token) {
    if (asset_token.empty() || asset_token == ".") {
        return ".";
    }
    if (asset_token.size() != 2) {
        return asset_token;
    }

    const char kind = asset_token[0];
    const char color_letter = asset_token[1];
    if (!std::isalpha(static_cast<unsigned char>(kind)) ||
        !std::isalpha(static_cast<unsigned char>(color_letter))) {
        return asset_token;
    }

    const char color = (color_letter == 'W' || color_letter == 'w') ? 'w' : 'b';
    return std::string(1, color) + std::string(1, kind);
}

std::string engineTokenToAsset(const std::string& engine_token) {
    if (engine_token.empty() || engine_token == ".") {
        return "";
    }
    if (engine_token.size() != 2) {
        return engine_token;
    }

    const char color = engine_token[0];
    const char kind = engine_token[1];
    const char color_letter = (color == 'w') ? 'W' : 'B';
    return std::string(1, kind) + std::string(1, color_letter);
}

std::vector<std::vector<std::string>> toEngineGrid(
    const std::vector<std::vector<std::string>>& asset_grid) {
    std::vector<std::vector<std::string>> engine_grid;
    engine_grid.reserve(asset_grid.size());
    for (const std::vector<std::string>& row : asset_grid) {
        std::vector<std::string> engine_row;
        engine_row.reserve(row.size());
        for (const std::string& cell : row) {
            engine_row.push_back(assetTokenToEngine(cell));
        }
        engine_grid.push_back(std::move(engine_row));
    }
    return engine_grid;
}

view::PlacedSprite placeSprite(const view::Img& sprite, double row, double col,
                               int cell_w, int cell_h) {
    const int x = static_cast<int>(col * cell_w) + (cell_w - sprite.width()) / 2;
    const int y = static_cast<int>(row * cell_h) + (cell_h - sprite.height()) / 2;
    return view::PlacedSprite{&sprite, x, y};
}

bool isExitKey(int key) {
    return key == 27 || key == 'q' || key == 'Q';
}

const std::string& motionState(const MotionView& motion) {
    static const std::string jump = graphics::kJumpState;
    static const std::string move = graphics::kMoveState;
    return (motion.from == motion.to) ? jump : move;
}

}  // namespace

const std::string GraphicsApplication::kWindowName = "Kung Fu Chess";

GraphicsApplication::GraphicsApplication(graphics::BoardLayout layout,
                                         view::Img background)
    : layout_(std::move(layout)),
      cell_w_(requirePositiveCellExtent(background.width(), layout_.cols, "column")),
      cell_h_(requirePositiveCellExtent(background.height(), layout_.rows, "row")),
      cell_size_{cell_w_, cell_h_},
      engine_(),
      controller_(engine_),
      mapper_(cell_w_, cell_h_, layout_.cols, layout_.rows),
      cache_(),
      visuals_(),
      renderer_(std::move(background), kWindowName) {
    engine_.setup(Board(toEngineGrid(layout_.cells)));
}

int GraphicsApplication::run() {
    std::cout << "Click a piece to select it, then click a destination to move.\n";
    std::cout << "Double-click a piece to jump in place.\n";
    std::cout << "Press ESC or Q (or close the window) to exit.\n";

    auto previous_time = std::chrono::steady_clock::now();

    while (true) {
        const auto now_time = std::chrono::steady_clock::now();
        const double dt_seconds =
            std::chrono::duration<double>(now_time - previous_time).count();
        previous_time = now_time;

        const int dt_ms =
            static_cast<int>(std::min(dt_seconds * 1000.0,
                                      static_cast<double>(kMaxFrameStepMs)));
        if (dt_ms > 0) {
            engine_.wait(dt_ms);
        }

        const std::vector<view::PlacedSprite> sprites = buildSprites(dt_seconds);
        const int key = renderer_.showFrame(sprites, kFrameWaitMs);
        handleMouseClick(view::Img::pollMouseClick(kWindowName));
        if (isExitKey(key) || !renderer_.isOpen()) {
            break;
        }
    }

    renderer_.close();
    std::cout << "Board window closed.\n";
    return 0;
}

void GraphicsApplication::handleMouseClick(
    const std::optional<view::MouseClick>& click) {
    if (!click.has_value()) {
        return;
    }

    const std::optional<Position> cell = mapper_.pixelToCell(click->x, click->y);
    if (!cell) {
        if (!click->is_double) {
            controller_.clearSelection();
        }
        std::cout << "Click off board at (" << click->x << ", " << click->y << ")\n";
        return;
    }

    if (click->is_double) {
        const MoveOutcome result = controller_.jump(*cell);
        if (result.is_accepted) {
            std::cout << "Jump accepted at (" << cell->row << ", " << cell->col << ")\n";
        } else {
            std::cout << "Jump rejected: " << result.reason << '\n';
        }
        return;
    }

    const ClickResult result = controller_.click(*cell);
    switch (result.outcome) {
        case ClickOutcome::Selected:
            std::cout << "Selected piece at (" << cell->row << ", " << cell->col
                      << ")\n";
            break;
        case ClickOutcome::Cleared:
            std::cout << "Selection cleared\n";
            break;
        case ClickOutcome::MoveRequested:
            if (result.moveResult.is_accepted) {
                std::cout << "Move accepted to (" << cell->row << ", " << cell->col
                          << ")\n";
            } else {
                std::cout << "Move rejected: " << result.moveResult.reason << '\n';
            }
            break;
        case ClickOutcome::Ignored:
            std::cout << "Click ignored at (" << cell->row << ", " << cell->col
                      << ")\n";
            break;
    }
}

void GraphicsApplication::pruneStaleVisuals(
    const GameSnapshot& snapshot, const std::vector<MotionView>& motions) {
    std::set<Position> motion_sources;
    for (const MotionView& motion : motions) {
        motion_sources.insert(motion.from);
    }

    for (auto it = visuals_.begin(); it != visuals_.end();) {
        const Position& cell = it->first;
        if (motion_sources.count(cell) != 0) {
            ++it;
            continue;
        }

        if (cell.row < 0 || cell.row >= static_cast<int>(snapshot.cells.size())) {
            it = visuals_.erase(it);
            continue;
        }

        const std::vector<std::string>& row =
            snapshot.cells[static_cast<size_t>(cell.row)];
        if (cell.col < 0 || cell.col >= static_cast<int>(row.size()) ||
            engineTokenToAsset(row[static_cast<size_t>(cell.col)]) !=
                it->second.token) {
            it = visuals_.erase(it);
            continue;
        }

        ++it;
    }
}

graphics::PieceVisual& GraphicsApplication::ensureVisual(const Position& cell,
                                                         const std::string& token) {
    const auto it = visuals_.find(cell);
    if (it != visuals_.end() && it->second.token == token) {
        return it->second;
    }

    return visuals_
        .insert_or_assign(cell, graphics::makePieceVisual(
                                    cache_, token, cell_size_,
                                    graphics::defaultPieceStates()))
        .first->second;
}

std::vector<view::PlacedSprite> GraphicsApplication::buildSprites(
    double dt_seconds) {
    const GameSnapshot snapshot = engine_.snapshot();
    const std::vector<MotionView> motions = engine_.activeMotions();

    std::set<Position> motion_sources;
    for (const MotionView& motion : motions) {
        motion_sources.insert(motion.from);
    }

    pruneStaleVisuals(snapshot, motions);

    std::vector<view::PlacedSprite> sprites;

    for (int row = 0; row < static_cast<int>(snapshot.cells.size()); ++row) {
        const std::vector<std::string>& cells =
            snapshot.cells[static_cast<size_t>(row)];
        for (int col = 0; col < static_cast<int>(cells.size()); ++col) {
            const std::string& token = cells[static_cast<size_t>(col)];
            if (token.empty() || token == ".") {
                continue;
            }

            const std::string asset_code = engineTokenToAsset(token);
            if (asset_code.empty()) {
                continue;
            }

            const Position cell{row, col};
            if (motion_sources.count(cell) != 0) {
                continue;
            }

            graphics::PieceVisual& visual = ensureVisual(cell, asset_code);
            graphics::Animation& idle =
                visual.animationFor(graphics::kIdleState);
            idle.update(dt_seconds);
            sprites.push_back(
                placeSprite(idle.current_frame(), row, col, cell_w_, cell_h_));
        }
    }

    for (const MotionView& motion : motions) {
        const std::string asset_code = engineTokenToAsset(motion.piece);
        graphics::PieceVisual& visual = ensureVisual(motion.from, asset_code);
        graphics::Animation& anim = visual.animationFor(motionState(motion));
        anim.update(dt_seconds);

        const double row =
            motion.from.row + (motion.to.row - motion.from.row) * motion.progress;
        const double col =
            motion.from.col + (motion.to.col - motion.from.col) * motion.progress;
        sprites.push_back(
            placeSprite(anim.current_frame(), row, col, cell_w_, cell_h_));
    }

    return sprites;
}
