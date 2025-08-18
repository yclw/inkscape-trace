#ifndef POTRACE_H
#define POTRACE_H
#include <optional>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>
#include <potracelib.h>

#include "core/core.h"
#include "trace/trace.h"
#include "pbitmap.h"

using potrace_param_t = struct potrace_param_s;
using potrace_path_t = struct potrace_path_s;

namespace Inkscape {
namespace Trace {
namespace Potrace {

// Potrace 追踪引擎
class PotraceTracingEngine final : public TracingEngine {
public:
  PotraceTracingEngine();

  PotraceTracingEngine(TraceType traceType, // 追踪类型
                       bool invert,                          // 是否反转
                       int quantizationNrColors,             // 量化颜色数
                       double brightnessThreshold,           // 亮度阈值
                       double brightnessFloor,               // 亮度地板
                       double cannyHighThreshold,            // 边缘检测阈值
                       int multiScanNrColors,                // 多扫描颜色数
                       bool multiScanStack,                  // 多扫描堆叠
                       bool multiScanSmooth,                 // 多扫描平滑
                       bool multiScanRemoveBackground        // 多扫描移除背景
  );

  ~PotraceTracingEngine() override;

  // 追踪
  TraceResult trace(RgbMap const &rgbmap) override;
  // 预览
  RgbMap preview(RgbMap const &rgbmap) override;
  // 追踪灰度图
  TraceResult traceGrayMap(GrayMap const &grayMap);
  // 设置优化曲线
  void setOptiCurve(int);
  // 设置优化容差
  void setOptTolerance(double);
  // 设置最大透明度
  void setAlphaMax(double);
  // 设置斑点大小
  void setTurdSize(int);

private:
  // Potrace 参数
  potrace_param_t *potraceParams;

  // 追踪类型
  TraceType traceType = TraceType::BRIGHTNESS;
  // 是否反转
  bool invert = false;
  // 量化颜色数
  int quantizationNrColors = 8;

  // 亮度阈值
  double brightnessThreshold = 0.45;
  // 亮度地板
  double brightnessFloor = 0.0;

  // 边缘检测阈值
  double cannyHighThreshold = 0.65;

  // 多扫描颜色数
  int multiScanNrColors = 8;
  // 多扫描堆叠
  bool multiScanStack = true;
  // 多扫描平滑
  bool multiScanSmooth = false;
  // 多扫描移除背景
  bool multiScanRemoveBackground = false;

  // 初始化
  void common_init();

  // 量化
  TraceResult traceQuant(RgbMap const &rgbmap);
  // 亮度多
  TraceResult traceBrightnessMulti(RgbMap const &rgbmap);
  // 单
  TraceResult traceSingle(RgbMap const &rgbmap);

  // 过滤索引
  IndexedMap filterIndexed(RgbMap const &rgbmap) const;
  // 过滤
  std::optional<GrayMap> filter(RgbMap const &rgbmap) const;
  // 灰度图直接转 SVG 路径字符串
  std::string grayMapToSvg(GrayMap const &gm);
  // 直接写 SVG 路径字符串
  void writePathsToSvg(potrace_path_t *paths, std::ostringstream &out) const;
};

} // namespace Potrace
} // namespace Trace
} // namespace Inkscape



#endif // POTRACE_H
