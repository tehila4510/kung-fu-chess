#include "doctest.h"
#include "graphics/FileConfigSource.h"
#include "graphics/GraphicsConfigLoader.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path makeTempDir(const std::string& name) {
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("kfc_gfx_" + name);
    std::filesystem::create_directories(dir);
    return dir;
}

void writeConfig(const std::filesystem::path& state_dir, const std::string& json) {
    std::filesystem::create_directories(state_dir);
    std::ofstream out(state_dir / "config.json", std::ios::binary);
    out << json;
}

}  // namespace

TEST_CASE("FileConfigSource loads fps/loop with safe defaults") {
    const std::filesystem::path root = makeTempDir("config");
    const graphics::FileConfigSource source;

    SUBCASE("missing config.json keeps defaults") {
        const auto state_dir = root / "missing";
        std::filesystem::create_directories(state_dir);
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(6.0));
        CHECK(config.loop == true);
    }

    SUBCASE("reads nested graphics keys from real-shaped JSON") {
        const auto state_dir = root / "nested";
        writeConfig(state_dir,
                    "{\n"
                    "  \"graphics\": {\n"
                    "    \"frames_per_sec\": 4.5,\n"
                    "    \"is_loop\": true\n"
                    "  }\n"
                    "}\n");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(4.5));
        CHECK(config.loop == true);
    }

    SUBCASE("is_loop false is honoured") {
        const auto state_dir = root / "noloop";
        writeConfig(state_dir, "{\"frames_per_sec\": 8, \"is_loop\": false}");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(8.0));
        CHECK(config.loop == false);
    }

    SUBCASE("non-true is_loop values become false") {
        const auto state_dir = root / "loop_weird";
        writeConfig(state_dir, "{\"is_loop\": TRUE}");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.loop == false);
    }

    SUBCASE("invalid fps text falls back to default") {
        const auto state_dir = root / "bad_fps";
        writeConfig(state_dir, "{\"frames_per_sec\": not_a_number, \"is_loop\": true}");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(6.0));
        CHECK(config.loop == true);
    }

    SUBCASE("non-positive fps falls back to default") {
        const auto state_dir = root / "zero_fps";
        writeConfig(state_dir, "{\"frames_per_sec\": 0, \"is_loop\": false}");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(6.0));
        CHECK(config.loop == false);
    }

    SUBCASE("negative fps falls back to default") {
        const auto state_dir = root / "neg_fps";
        writeConfig(state_dir, "{\"frames_per_sec\": -3}");
        const graphics::GraphicsConfig config = source.load(state_dir.string());
        CHECK(config.fps == doctest::Approx(6.0));
    }

    SUBCASE("GraphicsConfigLoader delegates to FileConfigSource") {
        const auto state_dir = root / "facade";
        writeConfig(state_dir, "{\"frames_per_sec\": 12, \"is_loop\": false}");
        const graphics::GraphicsConfig config =
            graphics::GraphicsConfigLoader::load(state_dir.string());
        CHECK(config.fps == doctest::Approx(12.0));
        CHECK(config.loop == false);
    }
}
