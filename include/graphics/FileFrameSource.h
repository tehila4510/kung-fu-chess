#ifndef GRAPHICS_FILE_FRAME_SOURCE_H
#define GRAPHICS_FILE_FRAME_SOURCE_H

#include "graphics/IFrameSource.h"

namespace graphics {

class FileFrameSource : public IFrameSource {
public:
    std::vector<view::Img> loadFrames(const std::string& state_dir,
                                      std::pair<int, int> cell_size) const override;
};

}  // namespace graphics

#endif
