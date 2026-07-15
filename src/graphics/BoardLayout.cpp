#include "graphics/BoardLayout.h"

#include "graphics/FileBoardSource.h"

namespace graphics {

BoardLayout BoardLayout::loadFromCsv(const std::string& path) {
    static const FileBoardSource source;
    return source.load(path);
}

}  // namespace graphics
