#include "svg/svg.h"
#include "trace/engine/potrace/potrace.h"
#include "trace/imagemap/imagemap.h"
#include "trace/trace.h" // 使用友好的trace.cpp接口
#include <cstring>       // for memcpy
#include <opencv2/opencv.hpp>

using namespace Inkscape::Trace::Potrace;
using namespace Inkscape::Trace;

// 将OpenCV Mat转换为RgbMap
Inkscape::Trace::RgbMap matToRgbMap(const cv::Mat &mat) {
  cv::Mat rgb_mat;
  if (mat.channels() == 3) {
    cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
  } else {
    cv::cvtColor(mat, rgb_mat, cv::COLOR_GRAY2RGB);
  }

  int width = rgb_mat.cols;
  int height = rgb_mat.rows;
  int nchannels = rgb_mat.channels(); // 应该是3

  auto rgbmap = Inkscape::Trace::RgbMap(width, height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      auto pixel = rgb_mat.at<cv::Vec3b>(y, x);

      // 严格按照原始逻辑：假设没有alpha通道 (nchannels == 3)
      int alpha = 255;
      int white = 255 - alpha; // = 0，因为alpha=255
      unsigned char r = (int)pixel[0] * alpha / 256 + white;
      unsigned char g = (int)pixel[1] * alpha / 256 + white;
      unsigned char b = (int)pixel[2] * alpha / 256 + white;

      rgbmap.setPixel(x, y, {r, g, b});
    }
  }

  return rgbmap;
}



int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }

  std::string imageFile = argv[1];

  // 加载图像
  cv::Mat image = cv::imread(imageFile);
  if (image.empty()) {
    return 1;
  }

  // 转换为RgbMap
  auto rgbmap = matToRgbMap(image);

  auto engine = std::make_unique<PotraceTracingEngine>(
      Inkscape::Trace::TraceType::QUANT_COLOR, false, 2, 0.35, 0.0, 0.55, 2,
      true, false, false);

  auto traceResult = Inkscape::Trace::trace(std::move(engine), rgbmap);

  if (traceResult.empty()) {
    return 1;
  }

  Inkscape::SVG::generateVectorSvg(traceResult, "output.svg", image.cols, image.rows);

  return 0;
}
