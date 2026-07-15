#ifndef GRAPHICS_ICONFIG_SOURCE_H
#define GRAPHICS_ICONFIG_SOURCE_H

#include <string>

namespace graphics {

struct GraphicsConfig {
    double fps = 6.0;
    bool loop = true;
};

class IConfigSource {
public:
    virtual ~IConfigSource() = default;

    virtual GraphicsConfig load(const std::string& state_dir) const = 0;
};

}  // namespace graphics

#endif
