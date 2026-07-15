#include "graphics/FileFrameSource.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace graphics {
namespace {

struct NumericPngName {
    std::filesystem::path path;
    int numeric_order;
    std::string stem;
};

int numericStemValue(const std::string& stem) {
    if (stem.empty()) {
        return -1;
    }

    std::size_t index = 0;
    while (index < stem.size() && std::isdigit(static_cast<unsigned char>(stem[index])) != 0) {
        ++index;
    }

    if (index == 0) {
        return -1;
    }

    try {
        return std::stoi(stem.substr(0, index));
    } catch (const std::exception&) {
        return -1;
    }
}

bool isPngFile(const std::filesystem::path& path) {
    if (!path.has_extension()) {
        return false;
    }

    std::string extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return extension == ".png";
}

bool compareNumericPngNames(const NumericPngName& lhs, const NumericPngName& rhs) {
    if (lhs.numeric_order != rhs.numeric_order) {
        return lhs.numeric_order < rhs.numeric_order;
    }
    return lhs.stem < rhs.stem;
}

std::vector<std::filesystem::path> listSpritePaths(const std::string& state_dir) {
    const std::filesystem::path sprites_dir =
        std::filesystem::path(state_dir) / "sprites";

    if (!std::filesystem::exists(sprites_dir) ||
        !std::filesystem::is_directory(sprites_dir)) {
        throw std::runtime_error("Sprite directory not found: " + sprites_dir.string());
    }

    std::vector<NumericPngName> png_files;
    for (const auto& entry : std::filesystem::directory_iterator(sprites_dir)) {
        if (!entry.is_regular_file() || !isPngFile(entry.path())) {
            continue;
        }

        const std::string stem = entry.path().stem().string();
        png_files.push_back(
            NumericPngName{entry.path(), numericStemValue(stem), stem});
    }

    if (png_files.empty()) {
        throw std::runtime_error("No PNG sprite frames found in: " + sprites_dir.string());
    }

    std::sort(png_files.begin(), png_files.end(), compareNumericPngNames);

    std::vector<std::filesystem::path> paths;
    paths.reserve(png_files.size());
    for (const NumericPngName& png_file : png_files) {
        paths.push_back(png_file.path);
    }
    return paths;
}

}  // namespace

std::vector<view::Img> FileFrameSource::loadFrames(const std::string& state_dir,
                                                   std::pair<int, int> cell_size) const {
    const std::vector<std::filesystem::path> sprite_paths = listSpritePaths(state_dir);

    std::vector<view::Img> frames;
    frames.reserve(sprite_paths.size());

    for (const std::filesystem::path& sprite_path : sprite_paths) {
        view::Img frame;
        frame.read(sprite_path.string(), cell_size, true);
        frames.push_back(std::move(frame));
    }

    return frames;
}

}  // namespace graphics
