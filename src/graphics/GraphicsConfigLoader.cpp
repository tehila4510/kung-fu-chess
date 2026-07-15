#include "graphics/GraphicsConfigLoader.h"

#include "graphics/FileConfigSource.h"

namespace graphics {

GraphicsConfig GraphicsConfigLoader::load(const std::string& state_dir) {
    static const FileConfigSource source;
    return source.load(state_dir);
}

}  // namespace graphics
