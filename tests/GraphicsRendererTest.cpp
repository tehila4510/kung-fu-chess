#include "doctest.h"
#include "view/Img.h"
#include "view/Renderer.h"

#include <filesystem>
#include <stdexcept>
#include <vector>

TEST_CASE("Renderer construction and sprite validation") {
    REQUIRE(std::filesystem::exists("assets/board.png"));

    SUBCASE("unloaded background is rejected") {
        view::Img unloaded;
        CHECK_THROWS_AS(view::Renderer(unloaded, "graphics-test"), std::runtime_error);
    }

    SUBCASE("null sprite pointer throws before display") {
        view::Img board;
        board.read("assets/board.png");
        const view::Renderer renderer(board, "graphics-test-null-sprite");

        std::vector<view::PlacedSprite> sprites{{nullptr, 0, 0}};
        CHECK_THROWS_AS(renderer.showFrame(sprites, {}, 1), std::runtime_error);
        renderer.close();
    }

    SUBCASE("unloaded sprite throws before display") {
        view::Img board;
        board.read("assets/board.png");
        const view::Renderer renderer(board, "graphics-test-unloaded-sprite");

        view::Img unloaded;
        std::vector<view::PlacedSprite> sprites{{&unloaded, 0, 0}};
        CHECK_THROWS_AS(renderer.showFrame(sprites, {}, 1), std::runtime_error);
        renderer.close();
    }

    SUBCASE("rest overlays with non-positive geometry are ignored safely") {
        view::Img board;
        board.read("assets/board.png");
        const view::Renderer renderer(board, "graphics-test-rest-skip");

        // Empty sprite list: overlays alone still reach display. Close immediately.
        // Use overlays that must be skipped (no draw) so only the background shows.
        std::vector<view::CellOverlay> overlays = {
            view::CellOverlay{0, 0, 0, 0, 0, 0, 0, 0.5, view::HighlightKind::LongRest},
            view::CellOverlay{0, 0, 0, 10, 10, 20, 20, 0.0, view::HighlightKind::ShortRest},
            view::CellOverlay{5, 5, 0, 0, 0, 0, 0, 0.0, view::HighlightKind::Move},
        };
        CHECK_NOTHROW(renderer.showFrame({}, overlays, 1));
        renderer.close();
    }
}
