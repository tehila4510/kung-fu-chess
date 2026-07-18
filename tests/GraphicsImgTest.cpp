#include "doctest.h"
#include "view/Img.h"

#include <filesystem>
#include <stdexcept>
#include <string>

TEST_CASE("Img::read validates path and file existence") {
    view::Img img;

    SUBCASE("empty path throws") {
        CHECK_THROWS_AS(img.read(""), std::invalid_argument);
        CHECK_FALSE(img.is_loaded());
    }

    SUBCASE("missing file throws") {
        CHECK_THROWS_AS(img.read("assets/this_file_does_not_exist.png"),
                        std::runtime_error);
        CHECK_FALSE(img.is_loaded());
    }
}

TEST_CASE("Img loads board assets and supports draw edge cases") {
    REQUIRE(std::filesystem::exists("assets/board.png"));

    view::Img board;
    board.read("assets/board.png");
    REQUIRE(board.is_loaded());
    CHECK(board.width() > 0);
    CHECK(board.height() > 0);

    SUBCASE("clone preserves dimensions") {
        const view::Img copy = board.clone();
        CHECK(copy.is_loaded());
        CHECK(copy.width() == board.width());
        CHECK(copy.height() == board.height());
    }

    SUBCASE("draw_on rejects unloaded source or target") {
        view::Img unloaded;
        CHECK_THROWS_AS(unloaded.draw_on(board, 0, 0), std::runtime_error);
        CHECK_THROWS_AS(board.draw_on(unloaded, 0, 0), std::runtime_error);
    }

    SUBCASE("draw_on rejects sprites that do not fit") {
        view::Img sprite;
        REQUIRE(std::filesystem::exists("assets/pieces/KW/states/idle/sprites/1.png"));
        sprite.read("assets/pieces/KW/states/idle/sprites/1.png");
        REQUIRE(sprite.is_loaded());
        CHECK_THROWS_AS(sprite.draw_on(board, board.width() - 1, 0),
                        std::runtime_error);
        CHECK_THROWS_AS(sprite.draw_on(board, -1, 0), std::runtime_error);
    }

    SUBCASE("keep_aspect scaling fits inside the requested cell") {
        view::Img sprite;
        sprite.read("assets/pieces/KW/states/idle/sprites/1.png", {40, 40}, true);
        REQUIRE(sprite.is_loaded());
        CHECK(sprite.width() <= 40);
        CHECK(sprite.height() <= 40);
        CHECK(sprite.width() > 0);
        CHECK(sprite.height() > 0);
    }

    SUBCASE("drawing helpers reject unloaded images and bad radii") {
        view::Img unloaded;
        CHECK_THROWS_AS(unloaded.put_text("x", 1, 1, 1.0), std::runtime_error);
        CHECK_THROWS_AS(board.draw_solid_disc(1, 1, 0, {0, 0, 0}),
                        std::invalid_argument);
        CHECK_THROWS_AS(board.draw_ring(1, 1, 2, {0, 0, 0}, 0),
                        std::invalid_argument);
    }

    SUBCASE("draw_filled_rect with non-positive size is a no-op") {
        board.draw_filled_rect(0, 0, 0, 10, {0, 0, 255}, 0.5);
        board.draw_filled_rect(0, 0, 10, -1, {0, 0, 255}, 0.5);
        CHECK(board.is_loaded());
    }
}
