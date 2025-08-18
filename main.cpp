#include "engines/engines.h"
#include "trace/trace.h"
#include <cstring>
#include <opencv2/opencv.hpp>

using namespace Potrace;

// 将OpenCV Mat转换为RgbMap
RgbMap matToRgbMap(const cv::Mat &mat) {
  cv::Mat rgb_mat;
  if (mat.channels() == 3) {
    cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
  } else {
    cv::cvtColor(mat, rgb_mat, cv::COLOR_GRAY2RGB);
  }

  int width = rgb_mat.cols;
  int height = rgb_mat.rows;
  int nchannels = rgb_mat.channels();

  auto rgbmap = RgbMap(width, height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      auto pixel = rgb_mat.at<cv::Vec3b>(y, x);
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
      TraceType::QUANT,  // 颜色量化
      false,  // 是否反转
      4,  // 颜色量化数量
      0.45,  // 亮度阈值
      0.0,  // 亮度阈值
      0.55,  // 亮度阈值
      2,  // 多扫描颜色数量
      true,  // 多扫描堆叠
      false,  // 多扫描平滑
      false  // 多扫描移除背景
    );

  auto traceResult = trace(std::move(engine), rgbmap);

  if (traceResult.items.empty()) {
    return 1;
  }

  // 使用新的 TraceResult 内置方法直接保存 SVG！
  traceResult.saveToSvg("output.svg", image.cols, image.rows);

  return 0;
}
