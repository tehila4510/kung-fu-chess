#include "graphics/AnimationLoader.h"

#include "graphics/FileFrameSource.h"

namespace graphics {

std::vector<view::Img> AnimationLoader::loadFrames(const std::string& state_dir,
                                                   std::pair<int, int> cell_size) {
    static const FileFrameSource source;
    return source.loadFrames(state_dir, cell_size);
}

}  // namespace graphics
