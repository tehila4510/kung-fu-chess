#include "graphics/FileConfigSource.h"

#include <fstream>
#include <sstream>

namespace graphics {
namespace {

constexpr double kDefaultFps = 6.0;
constexpr bool kDefaultLoop = true;

}  // namespace

GraphicsConfig FileConfigSource::load(const std::string& state_dir) const {
    GraphicsConfig config;
    const std::string config_path = state_dir + "/config.json";

    std::ifstream file(config_path);
    if (!file) {
        return config;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    const std::string json = buffer.str();

    const std::string fps_key = "\"frames_per_sec\"";
    const std::size_t fps_pos = json.find(fps_key);
    if (fps_pos != std::string::npos) {
        const std::size_t colon_pos = json.find(':', fps_pos);
        if (colon_pos != std::string::npos) {
            try {
                config.fps = std::stod(json.substr(colon_pos + 1));
            } catch (const std::exception&) {
                config.fps = kDefaultFps;
            }
        }
    }

    const std::string loop_key = "\"is_loop\"";
    const std::size_t loop_pos = json.find(loop_key);
    if (loop_pos != std::string::npos) {
        const std::size_t colon_pos = json.find(':', loop_pos);
        if (colon_pos != std::string::npos) {
            const std::size_t value_pos = json.find_first_not_of(" \t\n\r", colon_pos + 1);
            if (value_pos != std::string::npos) {
                config.loop = json.compare(value_pos, 4, "true") == 0;
            }
        }
    }

    if (config.fps <= 0.0) {
        config.fps = kDefaultFps;
    }

    return config;
}

}  // namespace graphics
