#include "view/Img.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace view {
namespace {

struct WindowInput {
    std::string window_name;
    std::optional<MouseClick> pending_click;
};

std::unordered_map<std::string, WindowInput> g_window_inputs;
std::unordered_set<std::string> g_windows_created;

void onMouse(int event, int x, int y, int /*flags*/, void* userdata) {
    auto* input = static_cast<WindowInput*>(userdata);
    if (input == nullptr) {
        return;
    }

    if (event == cv::EVENT_LBUTTONDBLCLK) {
        input->pending_click = MouseClick{x, y, true};
        return;
    }
    if (event == cv::EVENT_LBUTTONDOWN) {
        input->pending_click = MouseClick{x, y, false};
    }
}

WindowInput& ensureWindowInput(const std::string& window_name) {
    return g_window_inputs.emplace(window_name, WindowInput{window_name, std::nullopt})
        .first->second;
}

void ensureWindowReady(const std::string& window_name) {
    if (g_windows_created.count(window_name) == 0) {
        cv::namedWindow(window_name, cv::WINDOW_AUTOSIZE);
        g_windows_created.insert(window_name);
    }
}

void registerMouseCallback(const std::string& window_name) {
    WindowInput& input = ensureWindowInput(window_name);
    cv::setMouseCallback(window_name, onMouse, &input);
}

}  // namespace

Img::Img() = default;

Img& Img::read(const std::string& path,
               const std::pair<int, int>& size,
               bool keep_aspect,
               int interpolation) {
    if (path.empty()) {
        throw std::invalid_argument("Image path must not be empty");
    }

    img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        throw std::runtime_error("Cannot load image: " + path);
    }

    const int target_w = size.first;
    const int target_h = size.second;
    if (target_w != 0 && target_h != 0) {
        const int w = img.cols;
        const int h = img.rows;
        if (w <= 0 || h <= 0) {
            throw std::runtime_error("Loaded image has invalid dimensions: " + path);
        }
        if (keep_aspect) {
            const double scale = std::min(static_cast<double>(target_w) / w,
                                          static_cast<double>(target_h) / h);
            const int new_w = std::max(1, static_cast<int>(w * scale));
            const int new_h = std::max(1, static_cast<int>(h * scale));
            cv::resize(img, img, cv::Size(new_w, new_h), 0, 0, interpolation);
        } else {
            cv::resize(img, img, cv::Size(target_w, target_h), 0, 0, interpolation);
        }
    }

    return *this;
}

Img& Img::create(int width, int height, const cv::Scalar& fill) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Blank image dimensions must be positive");
    }
    img = cv::Mat(height, width, CV_8UC3, fill);
    return *this;
}

void Img::draw_on(Img& other_img, int x, int y) const {
    if (img.empty() || other_img.img.empty()) {
        throw std::runtime_error("Both images must be loaded before drawing.");
    }

    const cv::Mat& source = img;
    cv::Mat& target = other_img.img;

    const int w = source.cols;
    const int h = source.rows;
    if (x < 0 || y < 0 || x + w > target.cols || y + h > target.rows) {
        throw std::runtime_error("Image does not fit at the specified position.");
    }

    cv::Mat roi = target(cv::Rect(x, y, w, h));

    if (source.channels() != 4) {
        cv::Mat converted = source;
        if (source.channels() != target.channels()) {
            const int code = target.channels() == 4 ? cv::COLOR_BGR2BGRA
                                                     : cv::COLOR_BGRA2BGR;
            cv::cvtColor(source, converted, code);
        }
        converted.copyTo(roi);
        return;
    }

    std::vector<cv::Mat> src_ch;
    cv::split(source, src_ch);
    cv::Mat alpha;
    src_ch[3].convertTo(alpha, CV_32FC1, 1.0 / 255.0);
    const cv::Mat inv_alpha = 1.0 - alpha;

    std::vector<cv::Mat> dst_ch;
    cv::split(roi, dst_ch);
    const int color_channels = std::min<int>(3, static_cast<int>(dst_ch.size()));
    for (int c = 0; c < color_channels; ++c) {
        cv::Mat s, d;
        src_ch[c].convertTo(s, CV_32FC1);
        dst_ch[c].convertTo(d, CV_32FC1);
        cv::Mat blended = s.mul(alpha) + d.mul(inv_alpha);
        blended.convertTo(dst_ch[c], dst_ch[c].type());
    }
    cv::merge(dst_ch, roi);
}

void Img::put_text(const std::string& txt, int x, int y, double font_size,
                   const cv::Scalar& color, int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    cv::putText(img, txt, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_size,
                color, thickness, cv::LINE_AA);
}

void Img::put_text_centered(const std::string& txt, double font_size,
                            const cv::Scalar& color, int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    int baseline = 0;
    const cv::Size size =
        cv::getTextSize(txt, cv::FONT_HERSHEY_SIMPLEX, font_size, thickness, &baseline);
    const int x = (img.cols - size.width) / 2;
    const int y = (img.rows + size.height) / 2;
    cv::putText(img, txt, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_size,
                cv::Scalar(0, 0, 0), thickness + 2, cv::LINE_AA);
    cv::putText(img, txt, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_size, color,
                thickness, cv::LINE_AA);
}

void Img::put_text_centered_in_rect(const std::string& txt, int rect_x, int rect_y,
                                    int rect_w, int rect_h, double font_size,
                                    const cv::Scalar& color, int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    if (rect_w <= 0 || rect_h <= 0) {
        return;
    }
    int baseline = 0;
    const cv::Size size =
        cv::getTextSize(txt, cv::FONT_HERSHEY_SIMPLEX, font_size, thickness, &baseline);
    const int x = rect_x + (rect_w - size.width) / 2;
    const int y = rect_y + (rect_h + size.height) / 2;
    cv::putText(img, txt, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_size,
                cv::Scalar(0, 0, 0), thickness + 2, cv::LINE_AA);
    cv::putText(img, txt, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_size, color,
                thickness, cv::LINE_AA);
}

void Img::draw_solid_disc(int center_x, int center_y, int radius, const cv::Scalar& bgr) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    if (radius <= 0) {
        throw std::invalid_argument("Highlight radius must be positive.");
    }
    cv::circle(img, cv::Point(center_x, center_y), radius, bgr, -1, cv::LINE_AA);
}

void Img::draw_ring(int center_x, int center_y, int radius, const cv::Scalar& bgr,
                   int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    if (radius <= 0 || thickness <= 0) {
        throw std::invalid_argument("Highlight radius and thickness must be positive.");
    }
    cv::circle(img, cv::Point(center_x, center_y), radius, bgr, thickness, cv::LINE_AA);
}

void Img::draw_filled_rect(int x, int y, int w, int h, const cv::Scalar& bgr,
                           double alpha) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    if (w <= 0 || h <= 0) {
        return;
    }

    const double a = std::clamp(alpha, 0.0, 1.0);
    if (a <= 0.0) {
        return;
    }

    const int x0 = std::max(0, x);
    const int y0 = std::max(0, y);
    const int x1 = std::min(img.cols, x + w);
    const int y1 = std::min(img.rows, y + h);
    if (x0 >= x1 || y0 >= y1) {
        return;
    }

    const cv::Rect roi(x0, y0, x1 - x0, y1 - y0);
    cv::Mat region = img(roi);
    cv::Mat tint(region.size(), region.type(), bgr);
    cv::addWeighted(tint, a, region, 1.0 - a, 0.0, region);
}

void Img::show() const {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    cv::imshow("Kung Fu Chess", img);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

Img Img::clone() const {
    Img copy;
    if (!img.empty()) {
        img.copyTo(copy.img);
    }
    return copy;
}

int Img::display(const std::string& window_name, int wait_ms) const {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    ensureWindowReady(window_name);
    cv::imshow(window_name, img);
    registerMouseCallback(window_name);
    return cv::waitKey(wait_ms);
}

void Img::destroyWindows() {
    g_window_inputs.clear();
    g_windows_created.clear();
    cv::destroyAllWindows();
}

bool Img::isWindowOpen(const std::string& window_name) {
    return cv::getWindowProperty(window_name, cv::WND_PROP_VISIBLE) >= 1.0;
}

std::optional<MouseClick> Img::pollMouseClick(const std::string& window_name) {
    const auto it = g_window_inputs.find(window_name);
    if (it == g_window_inputs.end() || !it->second.pending_click.has_value()) {
        return std::nullopt;
    }

    const MouseClick click = *it->second.pending_click;
    it->second.pending_click.reset();
    return click;
}

const cv::Mat& Img::get_mat() const {
    return img;
}

bool Img::is_loaded() const {
    return !img.empty();
}

int Img::width() const {
    return img.cols;
}

int Img::height() const {
    return img.rows;
}

}  // namespace view
