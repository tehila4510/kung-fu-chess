#ifndef VIEW_IMG_H
#define VIEW_IMG_H

#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <utility>

namespace view {

struct MouseClick {
    int x = 0;
    int y = 0;
    bool is_double = false;
};

class Img {
public:
    Img();
    Img& read(const std::string& path,
              const std::pair<int, int>& size = {0, 0},
              bool keep_aspect = false,
              int interpolation = cv::INTER_AREA);

    void draw_on(Img& other_img, int x, int y) const;

    void put_text(const std::string& txt, int x, int y, double font_size,
                  const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                  int thickness = 1);

    void show() const;

    Img clone() const;

    int display(const std::string& window_name = "Kung Fu Chess",
                int wait_ms = 1) const;

    static void destroyWindows();
    static bool isWindowOpen(const std::string& window_name);
    static std::optional<MouseClick> pollMouseClick(const std::string& window_name);

    const cv::Mat& get_mat() const;
    bool is_loaded() const;
    int width() const;
    int height() const;

private:
    cv::Mat img;
};

}  // namespace view

#endif
