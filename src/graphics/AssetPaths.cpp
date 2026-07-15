#include "graphics/AssetPaths.h"

namespace graphics {
namespace {

constexpr const char* kAssetsRoot = "assets";

}  // namespace

std::string AssetPaths::piecesRoot() {
    return std::string(kAssetsRoot) + "/pieces/";
}

std::string AssetPaths::stateDir(const std::string& piece_code, const std::string& state) {
    return piecesRoot() + piece_code + "/states/" + state;
}

std::string AssetPaths::boardCsv() {
    return piecesRoot() + "board.csv";
}

std::string AssetPaths::boardImage() {
    return std::string(kAssetsRoot) + "/board.png";
}

}  // namespace graphics
