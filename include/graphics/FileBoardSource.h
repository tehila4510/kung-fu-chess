#ifndef GRAPHICS_FILE_BOARD_SOURCE_H
#define GRAPHICS_FILE_BOARD_SOURCE_H

#include "graphics/IBoardSource.h"

namespace graphics {

class FileBoardSource : public IBoardSource {
public:
    BoardLayout load(const std::string& path) const override;
};

}  // namespace graphics

#endif
