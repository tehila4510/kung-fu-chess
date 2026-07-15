#ifndef GRAPHICS_FILE_CONFIG_SOURCE_H
#define GRAPHICS_FILE_CONFIG_SOURCE_H

#include "graphics/IConfigSource.h"

namespace graphics {

class FileConfigSource : public IConfigSource {
public:
    GraphicsConfig load(const std::string& state_dir) const override;
};

}  // namespace graphics

#endif
