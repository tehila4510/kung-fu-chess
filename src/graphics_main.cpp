// Graphics entry point: renders the initial board with animated pieces.
// No game logic yet — pieces just play their looping "idle" animation.

#include "graphics/Animation.h"
#include "graphics/AnimationCache.h"
#include "graphics/AssetPaths.h"
#include "graphics/BoardLayoutLoader.h"
#include "model/Position.h"
#include "view/Img.h"
#include "view/Renderer.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

const std::string kWindowName = "Kung Fu Chess";
const std::string kIdleState = "idle";
constexpr int kFrameWaitMs = 15;

struct PieceAnimation {
    Position cell;
    graphics::Animation animation;
};

std::vector<PieceAnimation> buildPieceAnimations(const graphics::BoardLayout& layout,
                                                 std::pair<int, int> cell_size,
                                                 graphics::AnimationCache& cache) {
    std::vector<PieceAnimation> pieces;

    for (int row = 0; row < layout.rows; ++row) {
        const std::vector<std::string>& cells = layout.cells[row];
        for (int col = 0; col < static_cast<int>(cells.size()); ++col) {
            const std::string& code = cells[col];
            if (code.empty()) {
                continue;
            }

            const graphics::AnimationSpec& spec =
                cache.get(code, kIdleState, cell_size);
            pieces.push_back(PieceAnimation{
                Position{row, col},
                graphics::Animation(spec.frames, spec.fps, spec.loop)});
        }
    }

    return pieces;
}

// Centre a sprite inside its cell (idle sprites keep aspect, so they are
// usually smaller than the cell).
view::PlacedSprite placeSprite(const view::Img& sprite, const Position& cell,
                         int cell_w, int cell_h) {
    const int x = cell.col * cell_w + (cell_w - sprite.width()) / 2;
    const int y = cell.row * cell_h + (cell_h - sprite.height()) / 2;
    return view::PlacedSprite{&sprite, x, y};
}

bool isExitKey(int key) {
    return key == 27 || key == 'q' || key == 'Q';
}

void updateAnimations(std::vector<PieceAnimation>& pieces, double dt_seconds) {
    for (PieceAnimation& piece : pieces) {
        piece.animation.update(dt_seconds);
    }
}

int renderFrame(view::Renderer& renderer,
                const std::vector<PieceAnimation>& pieces,
                int cell_w, int cell_h) {
    std::vector<view::PlacedSprite> sprites;
    sprites.reserve(pieces.size());
    for (const PieceAnimation& piece : pieces) {
        sprites.push_back(placeSprite(piece.animation.current_frame(),
                                      piece.cell, cell_w, cell_h));
    }
    return renderer.showFrame(sprites, kFrameWaitMs);
}

}  // namespace

int main() {
    try {
        const graphics::BoardLayout layout =
            graphics::BoardLayoutLoader{}.load(graphics::AssetPaths::boardCsv());

        view::Img background;
        background.read(graphics::AssetPaths::boardImage());

        const int cell_w = background.width() / layout.cols;
        const int cell_h = background.height() / layout.rows;
        if (cell_w <= 0 || cell_h <= 0) {
            throw std::runtime_error("Board image is too small for the layout grid.");
        }
        const std::pair<int, int> cell_size{cell_w, cell_h};

        graphics::AnimationCache cache;
        std::vector<PieceAnimation> pieces =
            buildPieceAnimations(layout, cell_size, cache);

        view::Renderer renderer(std::move(background), kWindowName);

        std::cout << "Rendering " << pieces.size() << " animated pieces on a "
                  << layout.rows << "x" << layout.cols << " board.\n";
        std::cout << "Press ESC or Q (or close the window) to exit.\n";

        auto previous_time = std::chrono::steady_clock::now();

        while (true) {
            const auto now_time = std::chrono::steady_clock::now();
            const double dt_seconds =
                std::chrono::duration<double>(now_time - previous_time).count();
            previous_time = now_time;

            updateAnimations(pieces, dt_seconds);
            const int key = renderFrame(renderer, pieces, cell_w, cell_h);
            if (isExitKey(key) || !renderer.isOpen()) {
                break;
            }
        }

        renderer.close();
        std::cout << "Board window closed.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        view::Img::destroyWindows();
        return 1;
    }
}
