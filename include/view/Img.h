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

    // Creates a blank BGR image filled with the given color.
    Img& create(int width, int height,
                const cv::Scalar& fill = cv::Scalar(30, 30, 30, 255));

    void draw_on(Img& other_img, int x, int y) const;

    void put_text(const std::string& txt, int x, int y, double font_size,
                  const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                  int thickness = 1);

    // Draws centered text; optional outline for readability over busy backgrounds.
    void put_text_centered(const std::string& txt, double font_size,
                           const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                           int thickness = 2);

    // Like put_text_centered, but within a rectangle (e.g. board region only).
    void put_text_centered_in_rect(const std::string& txt, int rect_x, int rect_y,
                                   int rect_w, int rect_h, double font_size,
                                   const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                                   int thickness = 2);

    void draw_solid_disc(int center_x, int center_y, int radius, const cv::Scalar& bgr);

    void draw_ring(int center_x, int center_y, int radius, const cv::Scalar& bgr,
                   int thickness);

    // Blends a filled rectangle onto the image. alpha is in [0, 1].
    void draw_filled_rect(int x, int y, int w, int h, const cv::Scalar& bgr,
                          double alpha);

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
