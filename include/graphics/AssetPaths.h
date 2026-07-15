#ifndef GRAPHICS_ASSET_PATHS_H
#define GRAPHICS_ASSET_PATHS_H

#include <string>

namespace graphics {

class AssetPaths {
public:
    static std::string piecesRoot();
    static std::string stateDir(const std::string& piece_code, const std::string& state);
    static std::string boardCsv();
    static std::string boardImage();
};

}  // namespace graphics

#endif
