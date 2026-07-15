#ifndef GRAPHICS_CONFIG_LOADER_H
#define GRAPHICS_CONFIG_LOADER_H

#include <string>

namespace graphics {

struct GraphicsConfig {
    double fps = 6.0;
    bool loop = true;
};

class GraphicsConfigLoader {
public:
    static GraphicsConfig load(const std::string& state_dir);
};

}  // namespace graphics

#endif
