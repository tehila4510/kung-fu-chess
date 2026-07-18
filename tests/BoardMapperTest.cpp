#include "doctest.h"
#include "input/BoardMapper.h"

#include <optional>
#include <stdexcept>

TEST_CASE("BoardMapper maps pixels to cells and rejects off-board clicks") {
    BoardMapper mapper(100, 8, 8);

    SUBCASE("dimensions are reported back to the caller") {
        CHECK(mapper.getCellSize() == 100);
        CHECK(mapper.getWidthCells() == 8);
        CHECK(mapper.getHeightCells() == 8);
    }

    SUBCASE("a pixel inside the first cell maps to (0,0)") {
        const std::optional<Position> cell = mapper.pixelToCell(50, 50);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 0);
        CHECK(cell->col == 0);
    }

    SUBCASE("column comes from x and row from y") {
        const std::optional<Position> cell = mapper.pixelToCell(250, 150);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 1);
        CHECK(cell->col == 2);
    }

    SUBCASE("negative coordinates fall outside the board") {
        CHECK_FALSE(mapper.pixelToCell(-1, 50).has_value());
        CHECK_FALSE(mapper.pixelToCell(50, -1).has_value());
    }

    SUBCASE("coordinates beyond the last cell fall outside the board") {
        CHECK_FALSE(mapper.pixelToCell(800, 50).has_value());
        CHECK_FALSE(mapper.pixelToCell(50, 800).has_value());
    }

    SUBCASE("the last valid pixel maps to the last cell") {
        const std::optional<Position> cell = mapper.pixelToCell(799, 799);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 7);
        CHECK(cell->col == 7);
    }
}

TEST_CASE("BoardMapper supports rectangular boards with independent bounds") {
    BoardMapper mapper(100, 4, 3);

    SUBCASE("dimensions reflect a rectangular board") {
        CHECK(mapper.getWidthCells() == 4);
        CHECK(mapper.getHeightCells() == 3);
    }

    SUBCASE("the last valid pixel maps to the bottom-right cell") {
        const std::optional<Position> cell = mapper.pixelToCell(399, 299);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 2);
        CHECK(cell->col == 3);
    }

    SUBCASE("a column just past the width is rejected") {
        CHECK_FALSE(mapper.pixelToCell(400, 50).has_value());
    }

    SUBCASE("a row just past the height is rejected") {
        CHECK_FALSE(mapper.pixelToCell(50, 300).has_value());
    }

    SUBCASE("width and height are validated independently") {
        CHECK_FALSE(mapper.pixelToCell(50, 350).has_value());
        const std::optional<Position> cell = mapper.pixelToCell(350, 50);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 0);
        CHECK(cell->col == 3);
    }
}

TEST_CASE("BoardMapper honours a non-default cell size") {
    BoardMapper mapper(50, 6, 5);

    SUBCASE("cells scale with the configured size") {
        const std::optional<Position> cell = mapper.pixelToCell(120, 60);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 1);
        CHECK(cell->col == 2);
    }

    SUBCASE("the far edge respects the smaller cell size") {
        CHECK_FALSE(mapper.pixelToCell(300, 50).has_value());
        CHECK_FALSE(mapper.pixelToCell(50, 250).has_value());
        const std::optional<Position> cell = mapper.pixelToCell(299, 249);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 4);
        CHECK(cell->col == 5);
    }
}

TEST_CASE("BoardMapper supports rectangular cells used by the graphics board") {
    BoardMapper mapper(100, 50, 8, 8);

    SUBCASE("cell width and height are independent") {
        CHECK(mapper.getCellWidth() == 100);
        CHECK(mapper.getCellHeight() == 50);
        CHECK(mapper.getCellSize() == 100);
    }

    SUBCASE("pixel mapping uses each axis cell size") {
        const std::optional<Position> cell = mapper.pixelToCell(250, 75);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 1);
        CHECK(cell->col == 2);
    }

    SUBCASE("cellCenterPixel returns the geometric center") {
        const auto center = mapper.cellCenterPixel(Position{1, 2});
        CHECK(center.first == 250);
        CHECK(center.second == 75);
    }

    SUBCASE("boundary pixels still map into the last cell") {
        const std::optional<Position> cell = mapper.pixelToCell(799, 399);
        REQUIRE(cell.has_value());
        CHECK(cell->row == 7);
        CHECK(cell->col == 7);
    }
}

TEST_CASE("BoardMapper rejects non-positive construction dimensions") {
    CHECK_THROWS_AS(BoardMapper(0, 8, 8), std::invalid_argument);
    CHECK_THROWS_AS(BoardMapper(-1, 8, 8), std::invalid_argument);
    CHECK_THROWS_AS(BoardMapper(10, 0, 8), std::invalid_argument);
    CHECK_THROWS_AS(BoardMapper(10, 8, 0), std::invalid_argument);
    CHECK_THROWS_AS(BoardMapper(10, 0, 8, 8), std::invalid_argument);
    CHECK_THROWS_AS(BoardMapper(10, 10, 0, 8), std::invalid_argument);
}
