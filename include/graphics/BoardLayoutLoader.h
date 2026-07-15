#ifndef GRAPHICS_BOARD_LAYOUT_LOADER_H
#define GRAPHICS_BOARD_LAYOUT_LOADER_H

#include "graphics/FileBoardSource.h"
#include "graphics/IBoardSource.h"

#include <string>

namespace graphics {

class BoardLayoutLoader {
public:
    BoardLayoutLoader();
    explicit BoardLayoutLoader(IBoardSource& source);

    BoardLayout load(const std::string& path) const;

private:
    FileBoardSource owned_source_;
    IBoardSource& source_;
};

}  // namespace graphics

#endif
