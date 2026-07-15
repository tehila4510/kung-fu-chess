#ifndef GRAPHICS_IBOARD_SOURCE_H
#define GRAPHICS_IBOARD_SOURCE_H

#include "graphics/BoardLayout.h"

#include <string>

namespace graphics {

class IBoardSource {
public:
    virtual ~IBoardSource() = default;

    virtual BoardLayout load(const std::string& path) const = 0;
};

}  // namespace graphics

#endif
