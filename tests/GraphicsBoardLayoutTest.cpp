#include "doctest.h"
#include "graphics/BoardLayout.h"
#include "graphics/BoardLayoutLoader.h"
#include "graphics/FileBoardSource.h"
#include "graphics/IBoardSource.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

std::filesystem::path makeTempDir(const std::string& name) {
    const std::filesystem::path dir =
        std::filesystem::temp_directory_path() / ("kfc_gfx_" + name);
    std::filesystem::create_directories(dir);
    return dir;
}

std::filesystem::path writeFile(const std::filesystem::path& dir,
                                const std::string& filename,
                                const std::string& contents) {
    const std::filesystem::path path = dir / filename;
    std::ofstream out(path, std::ios::binary);
    out << contents;
    return path;
}

class StubBoardSource : public graphics::IBoardSource {
public:
    explicit StubBoardSource(graphics::BoardLayout layout)
        : layout_(std::move(layout)) {}

    graphics::BoardLayout load(const std::string& /*path*/) const override {
        return layout_;
    }

private:
    graphics::BoardLayout layout_;
};

}  // namespace

TEST_CASE("FileBoardSource parses CSV board layouts and edge cases") {
    const std::filesystem::path dir = makeTempDir("board_layout");
    const graphics::FileBoardSource source;

    SUBCASE("parses a rectangular board with trimmed tokens") {
        const auto path = writeFile(dir, "ok.csv", " KW , PB \n  , .\n");
        const graphics::BoardLayout layout = source.load(path.string());
        CHECK(layout.rows == 2);
        CHECK(layout.cols == 2);
        CHECK(layout.cells[0][0] == "KW");
        CHECK(layout.cells[0][1] == "PB");
        CHECK(layout.cells[1][0].empty());
        CHECK(layout.cells[1][1] == ".");
    }

    SUBCASE("a trailing comma adds an empty final column") {
        const auto path = writeFile(dir, "trail.csv", ",,,,,,,\n");
        const graphics::BoardLayout layout = source.load(path.string());
        CHECK(layout.rows == 1);
        CHECK(layout.cols == 8);
        for (const std::string& cell : layout.cells[0]) {
            CHECK(cell.empty());
        }
    }

    SUBCASE("CRLF line endings are tolerated") {
        const auto path = writeFile(dir, "crlf.csv", "KW,PB\r\n.,.\r\n");
        const graphics::BoardLayout layout = source.load(path.string());
        CHECK(layout.rows == 2);
        CHECK(layout.cols == 2);
        CHECK(layout.cells[0][0] == "KW");
        CHECK(layout.cells[1][1] == ".");
    }

    SUBCASE("blank lines are skipped") {
        const auto path = writeFile(dir, "blank.csv", "KW,PB\n\n.,.\n");
        const graphics::BoardLayout layout = source.load(path.string());
        CHECK(layout.rows == 2);
        CHECK(layout.cols == 2);
    }

    SUBCASE("missing file throws") {
        CHECK_THROWS_AS(source.load((dir / "missing.csv").string()),
                        std::runtime_error);
    }

    SUBCASE("empty file throws") {
        const auto path = writeFile(dir, "empty.csv", "");
        CHECK_THROWS_AS(source.load(path.string()), std::runtime_error);
    }

    SUBCASE("file with only blank lines throws") {
        const auto path = writeFile(dir, "blanks_only.csv", "\n\n\r\n");
        CHECK_THROWS_AS(source.load(path.string()), std::runtime_error);
    }

    SUBCASE("column count mismatch throws") {
        const auto path = writeFile(dir, "bad_cols.csv", "KW,PB,QW\n.,.\n");
        CHECK_THROWS_AS(source.load(path.string()), std::runtime_error);
    }

    SUBCASE("BoardLayout::loadFromCsv delegates to FileBoardSource") {
        const auto path = writeFile(dir, "facade.csv", "KW\n");
        const graphics::BoardLayout layout =
            graphics::BoardLayout::loadFromCsv(path.string());
        CHECK(layout.rows == 1);
        CHECK(layout.cols == 1);
        CHECK(layout.cells[0][0] == "KW");
    }
}

TEST_CASE("BoardLayoutLoader forwards to the injected IBoardSource") {
    graphics::BoardLayout stub_layout;
    stub_layout.cells = {{"KW", "PB"}, {".", "."}};
    stub_layout.rows = 2;
    stub_layout.cols = 2;

    StubBoardSource stub(stub_layout);
    const graphics::BoardLayoutLoader loader(stub);
    const graphics::BoardLayout layout = loader.load("ignored.csv");

    CHECK(layout.rows == 2);
    CHECK(layout.cols == 2);
    CHECK(layout.cells[0][0] == "KW");
    CHECK(layout.cells[1][1] == ".");
}
