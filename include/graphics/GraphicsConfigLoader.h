#ifndef GRAPHICS_CONFIG_LOADER_H
#define GRAPHICS_CONFIG_LOADER_H

#include "graphics/IConfigSource.h"

namespace graphics {

class GraphicsConfigLoader {
public:
    static GraphicsConfig load(const std::string& state_dir);
};

}  // namespace graphics

#endif
