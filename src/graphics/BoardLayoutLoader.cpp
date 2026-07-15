#include "graphics/BoardLayoutLoader.h"

namespace graphics {

BoardLayoutLoader::BoardLayoutLoader()
    : source_(owned_source_) {}

BoardLayoutLoader::BoardLayoutLoader(IBoardSource& source)
    : source_(source) {}

BoardLayout BoardLayoutLoader::load(const std::string& path) const {
    return source_.load(path);
}

}  // namespace graphics
