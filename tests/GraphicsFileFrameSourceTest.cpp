#include "doctest.h"
#include "graphics/FileFrameSource.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

std::filesystem::path makeTempDir(const std::string& name) {
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("kfc_gfx_" + name);
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

}  // namespace

TEST_CASE("FileFrameSource validates sprite directories and loads real assets") {
    const graphics::FileFrameSource source;

    SUBCASE("missing sprites directory throws") {
        const auto dir = makeTempDir("no_sprites");
        CHECK_THROWS_AS(source.loadFrames(dir.string(), {16, 16}), std::runtime_error);
    }

    SUBCASE("empty sprites directory throws") {
        const auto dir = makeTempDir("empty_sprites");
        std::filesystem::create_directories(dir / "sprites");
        CHECK_THROWS_AS(source.loadFrames(dir.string(), {16, 16}), std::runtime_error);
    }

    SUBCASE("non-png files alone are not enough") {
        const auto dir = makeTempDir("txt_only");
        const auto sprites = dir / "sprites";
        std::filesystem::create_directories(sprites);
        std::ofstream(sprites / "1.txt") << "not a png";
        CHECK_THROWS_AS(source.loadFrames(dir.string(), {16, 16}), std::runtime_error);
    }

    SUBCASE("loads and sorts numeric KW idle frames from assets") {
        REQUIRE(std::filesystem::exists("assets/pieces/KW/states/idle/sprites"));
        const std::vector<view::Img> frames =
            source.loadFrames("assets/pieces/KW/states/idle", {32, 32});
        CHECK(frames.size() >= 1);
        for (const view::Img& frame : frames) {
            CHECK(frame.is_loaded());
            CHECK(frame.width() <= 32);
            CHECK(frame.height() <= 32);
        }
    }
}
