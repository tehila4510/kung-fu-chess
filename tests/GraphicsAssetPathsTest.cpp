#include "doctest.h"
#include "graphics/AssetPaths.h"

TEST_CASE("AssetPaths builds the expected relative asset locations") {
    CHECK(graphics::AssetPaths::piecesRoot() == "assets/pieces/");
    CHECK(graphics::AssetPaths::boardCsv() == "assets/pieces/board.csv");
    CHECK(graphics::AssetPaths::boardImage() == "assets/board.png");
    CHECK(graphics::AssetPaths::stateDir("KW", "idle") ==
          "assets/pieces/KW/states/idle");
    CHECK(graphics::AssetPaths::stateDir("PB", "long_rest") ==
          "assets/pieces/PB/states/long_rest");
}
