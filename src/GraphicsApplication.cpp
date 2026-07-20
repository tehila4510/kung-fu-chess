#include "GraphicsApplication.h"

#include "bus/GameEvent.h"
#include "model/Board.h"
#include "protocol/Algebraic.h"
#include "realtime/MotionView.h"
#include "realtime/RestView.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <map>
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
                               int cell_w, int cell_h, int origin_x) {
    const int x =
        origin_x + static_cast<int>(col * cell_w) + (cell_w - sprite.width()) / 2;
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

const std::string& restState(RestKind kind) {
    static const std::string short_rest = graphics::kShortRestState;
    static const std::string long_rest = graphics::kLongRestState;
    return kind == RestKind::Short ? short_rest : long_rest;
}

std::map<Position, const RestView*> indexRests(const std::vector<RestView>& rests) {
    std::map<Position, const RestView*> by_cell;
    for (const RestView& rest : rests) {
        by_cell[rest.at] = &rest;
    }
    return by_cell;
}

view::Img makeWindowBackground(view::Img board, int panel_w) {
    if (!board.is_loaded()) {
        throw std::runtime_error("Board background image must be loaded.");
    }
    if (panel_w <= 0) {
        throw std::invalid_argument("History panel width must be positive.");
    }
    view::Img canvas;
    canvas.create(board.width() + 2 * panel_w, board.height());
    board.draw_on(canvas, panel_w, 0);
    return canvas;
}

std::vector<std::string> formatHistoryLines(
    const std::vector<MoveHistoryEntry>& entries) {
    std::vector<std::string> lines;
    lines.reserve(entries.size());
    for (const MoveHistoryEntry& entry : entries) {
        lines.push_back(MoveHistorySubscriber::formatEntry(entry));
    }
    return lines;
}

}  // namespace

const std::string GraphicsApplication::kWindowName = "Kung Fu Chess";
const std::string GraphicsApplication::kSoundsDir = "assets/sounds";

GraphicsApplication::GraphicsApplication(graphics::BoardLayout layout,
                                         view::Img background)
    : layout_(std::move(layout)),
      cell_w_(requirePositiveCellExtent(background.width(), layout_.cols, "column")),
      cell_h_(requirePositiveCellExtent(background.height(), layout_.rows, "row")),
      cell_size_{cell_w_, cell_h_},
      board_w_(background.width()),
      panel_w_(kPanelWidth),
      engine_(),
      controller_(engine_),
      mapper_(cell_w_, cell_h_, layout_.cols, layout_.rows, kPanelWidth, 0),
      cache_(),
      visuals_(),
      renderer_(makeWindowBackground(std::move(background), kPanelWidth), kWindowName),
      sound_(kSoundsDir, std::cout),
      history_() {
    bus_.subscribe(&sound_);
    bus_.subscribe(&history_);
    engine_.setup(Board(toEngineGrid(layout_.cells)));
    GameEvent started;
    started.type = GameEventType::GameStarted;
    publish(started);
}

GraphicsApplication::~GraphicsApplication() {
    bus_.unsubscribe(&history_);
    bus_.unsubscribe(&sound_);
}

void GraphicsApplication::publish(const GameEvent& event) {
    GameEvent stamped = event;
    stamped.timeMs = engine_.elapsedMs();
    bus_.publish(stamped);
}

std::string GraphicsApplication::pieceAt(const Position& at) const {
    const GameSnapshot snap = engine_.snapshot();
    if (at.row < 0 || at.col < 0 ||
        at.row >= static_cast<int>(snap.cells.size()) ||
        at.col >= static_cast<int>(snap.cells[static_cast<size_t>(at.row)].size())) {
        return ".";
    }
    return snap.cells[static_cast<size_t>(at.row)][static_cast<size_t>(at.col)];
}

void GraphicsApplication::publishArrivals(const GameSnapshot& before,
                                          const GameSnapshot& after) {
    for (const ArrivalEvent& arrival : engine_.lastArrivals()) {
        if (arrival.piece.size() < 2) {
            continue;
        }
        if (arrival.capturedPiece != "." && !arrival.capturedPiece.empty()) {
            GameEvent captured;
            captured.type = GameEventType::PieceCaptured;
            captured.color = (arrival.piece[0] == 'w') ? 'W' : 'B';
            captured.piece = arrival.piece;
            captured.capturedPiece = arrival.capturedPiece;
            captured.to = protocol::positionToSquare(arrival.at, engine_.rowCount());
            publish(captured);
        }
        if (arrival.promoted) {
            GameEvent promoted;
            promoted.type = GameEventType::PiecePromoted;
            promoted.color = (arrival.piece[0] == 'w') ? 'W' : 'B';
            promoted.piece = arrival.piece;
            promoted.reason = "pawn_to_queen";
            promoted.to = protocol::positionToSquare(arrival.at, engine_.rowCount());
            publish(promoted);
        }
    }

    if (!before.gameOver && after.gameOver) {
        GameEvent ended;
        ended.type = GameEventType::GameEnded;
        ended.reason = "king_captured";
        publish(ended);
    }
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
            const GameSnapshot before = engine_.snapshot();
            engine_.wait(dt_ms);
            publishArrivals(before, engine_.snapshot());
        }

        const std::vector<view::PlacedSprite> sprites = buildSprites(dt_seconds);
        const GameSnapshot snapshot = engine_.snapshot();
        const std::vector<MotionView> motions = engine_.activeMotions();
        const std::vector<RestView> rests = engine_.activeRests();
        std::vector<view::CellOverlay> overlays = buildRestOverlays(rests);
        const std::vector<view::CellOverlay> legal =
            buildLegalMoveOverlays(snapshot, motions);
        overlays.insert(overlays.end(), legal.begin(), legal.end());
        const std::string banner = snapshot.gameOver ? "GAME OVER" : "";
        view::HistoryHud history_hud;
        history_hud.panel_width = panel_w_;
        history_hud.board_width = board_w_;
        history_hud.white_lines = formatHistoryLines(history_.whiteEntries());
        history_hud.black_lines = formatHistoryLines(history_.blackEntries());
        const int key =
            renderer_.showFrame(sprites, overlays, kFrameWaitMs, banner, history_hud);
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
            const bool hadSelection = controller_.hasActiveSelection();
            controller_.clearSelection();
            if (hadSelection) {
                GameEvent cleared;
                cleared.type = GameEventType::SelectionCleared;
                publish(cleared);
            }
        }
        std::cout << "Click off board at (" << click->x << ", " << click->y << ")\n";
        return;
    }

    if (click->is_double) {
        const std::string piece = pieceAt(*cell);
        const MoveOutcome result = controller_.jump(*cell);
        if (result.is_accepted) {
            std::cout << "Jump accepted at (" << cell->row << ", " << cell->col << ")\n";
            const std::string square =
                protocol::positionToSquare(*cell, engine_.rowCount());
            GameEvent jumpEvent;
            jumpEvent.type = GameEventType::JumpMade;
            jumpEvent.color = (piece.size() == 2 && piece[0] == 'w') ? 'W' : 'B';
            jumpEvent.piece = piece;
            jumpEvent.from = square;
            jumpEvent.to = square;
            publish(jumpEvent);
        } else {
            std::cout << "Jump rejected: " << result.reason << '\n';
        }
        return;
    }

    const bool hadSelection = controller_.hasActiveSelection();
    const Position from = hadSelection ? controller_.selectedCell() : Position{};
    const std::string mover = hadSelection ? pieceAt(from) : std::string(".");
    const ClickResult result = controller_.click(*cell);
    switch (result.outcome) {
        case ClickOutcome::Selected: {
            std::cout << "Selected piece at (" << cell->row << ", " << cell->col
                      << ")\n";
            const std::string piece = pieceAt(*cell);
            GameEvent selected;
            selected.type = GameEventType::PieceSelected;
            selected.color = (piece.size() == 2 && piece[0] == 'w') ? 'W' : 'B';
            selected.piece = piece;
            publish(selected);
            break;
        }
        case ClickOutcome::Cleared: {
            std::cout << "Selection cleared\n";
            GameEvent cleared;
            cleared.type = GameEventType::SelectionCleared;
            publish(cleared);
            break;
        }
        case ClickOutcome::MoveRequested:
            if (result.moveResult.is_accepted) {
                std::cout << "Move accepted to (" << cell->row << ", " << cell->col
                          << ")\n";
                GameEvent moveEvent;
                moveEvent.type = GameEventType::MoveMade;
                moveEvent.color = (mover.size() == 2 && mover[0] == 'w') ? 'W' : 'B';
                moveEvent.piece = mover;
                moveEvent.from =
                    protocol::positionToSquare(from, engine_.rowCount());
                moveEvent.to =
                    protocol::positionToSquare(*cell, engine_.rowCount());
                publish(moveEvent);
            } else {
                std::cout << "Move rejected: " << result.moveResult.reason << '\n';
                GameEvent cleared;
                cleared.type = GameEventType::SelectionCleared;
                publish(cleared);
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
    const std::vector<RestView> rests = engine_.activeRests();
    const std::map<Position, const RestView*> rests_by_cell = indexRests(rests);

    std::set<Position> motion_sources;
    for (const MotionView& motion : motions) {
        motion_sources.insert(motion.from);
    }

    pruneStaleVisuals(snapshot, motions);

    std::set<Position> live_rest_cells;
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
            const RestView* rest = nullptr;
            const auto rest_it = rests_by_cell.find(cell);
            if (rest_it != rests_by_cell.end()) {
                rest = rest_it->second;
            }

            if (rest != nullptr) {
                live_rest_cells.insert(cell);
                graphics::Animation& anim = visual.animationFor(restState(rest->kind));
                const auto prev = rest_anim_keys_.find(cell);
                if (prev == rest_anim_keys_.end() || prev->second != rest->kind) {
                    anim.reset();
                    rest_anim_keys_[cell] = rest->kind;
                }
                anim.update(dt_seconds);
                sprites.push_back(
                    placeSprite(anim.current_frame(), row, col, cell_w_, cell_h_,
                                panel_w_));
            } else {
                rest_anim_keys_.erase(cell);
                graphics::Animation& idle =
                    visual.animationFor(graphics::kIdleState);
                idle.update(dt_seconds);
                sprites.push_back(
                    placeSprite(idle.current_frame(), row, col, cell_w_, cell_h_,
                                panel_w_));
            }
        }
    }

    for (auto it = rest_anim_keys_.begin(); it != rest_anim_keys_.end();) {
        if (live_rest_cells.count(it->first) == 0) {
            it = rest_anim_keys_.erase(it);
        } else {
            ++it;
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
            placeSprite(anim.current_frame(), row, col, cell_w_, cell_h_, panel_w_));
    }

    return sprites;
}

std::vector<view::CellOverlay> GraphicsApplication::buildRestOverlays(
    const std::vector<RestView>& rests) const {
    std::vector<view::CellOverlay> overlays;
    overlays.reserve(rests.size());

    for (const RestView& rest : rests) {
        view::CellOverlay overlay;
        overlay.kind = rest.kind == RestKind::Short ? view::HighlightKind::ShortRest
                                                    : view::HighlightKind::LongRest;
        overlay.cell_x = panel_w_ + rest.at.col * cell_w_;
        overlay.cell_y = rest.at.row * cell_h_;
        overlay.cell_w = cell_w_;
        overlay.cell_h = cell_h_;
        overlay.remaining = rest.remaining;
        overlays.push_back(overlay);
    }

    return overlays;
}

std::vector<view::CellOverlay> GraphicsApplication::buildLegalMoveOverlays(
    const GameSnapshot& snapshot, const std::vector<MotionView>& motions) const {
    std::vector<view::CellOverlay> overlays;
    if (!controller_.hasActiveSelection()) {
        return overlays;
    }

    const Position selected = controller_.selectedCell();
    const std::string selected_token = [&snapshot, &selected]() -> std::string {
        if (selected.row < 0 || selected.row >= static_cast<int>(snapshot.cells.size())) {
            return ".";
        }
        const std::vector<std::string>& row =
            snapshot.cells[static_cast<size_t>(selected.row)];
        if (selected.col < 0 || selected.col >= static_cast<int>(row.size())) {
            return ".";
        }
        return row[static_cast<size_t>(selected.col)];
    }();

    if (selected_token.empty() || selected_token == ".") {
        return overlays;
    }

    const char mover_color = selected_token[0];
    const std::set<Position> legal_moves = engine_.legalMovesFrom(selected);
    const int cell_min = std::min(cell_w_, cell_h_);
    const int move_dot_radius = std::max(3, cell_min * 8 / 100);
    const int capture_radius = std::max(4, cell_min * 11 / 100);

    for (const Position& dest : legal_moves) {
        bool is_capture = false;
        if (dest.row >= 0 && dest.row < static_cast<int>(snapshot.cells.size())) {
            const std::vector<std::string>& row =
                snapshot.cells[static_cast<size_t>(dest.row)];
            if (dest.col >= 0 && dest.col < static_cast<int>(row.size())) {
                const std::string& token = row[static_cast<size_t>(dest.col)];
                is_capture = !token.empty() && token != "." && token[0] != mover_color;
            }
        }

        if (!is_capture) {
            for (const MotionView& motion : motions) {
                if (motion.from == motion.to && motion.from == dest
                    && motion.piece.size() == 2 && motion.piece[0] != mover_color) {
                    is_capture = true;
                    break;
                }
            }
        }

        const std::pair<int, int> center = mapper_.cellCenterPixel(dest);
        view::CellOverlay overlay;
        overlay.center_x = center.first;
        overlay.center_y = center.second;
        overlay.radius = is_capture ? capture_radius : move_dot_radius;
        overlay.kind = is_capture ? view::HighlightKind::Capture
                                  : view::HighlightKind::Move;
        overlays.push_back(overlay);
    }

    return overlays;
}
