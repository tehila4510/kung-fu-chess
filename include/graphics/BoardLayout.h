#ifndef GRAPHICS_BOARD_LAYOUT_H
#define GRAPHICS_BOARD_LAYOUT_H

#include <string>
#include <vector>

namespace graphics {

struct BoardLayout {
    std::vector<std::vector<std::string>> cells;
    int rows = 0;
    int cols = 0;

    static BoardLayout loadFromCsv(const std::string& path);
};

}  // namespace graphics

#endif
