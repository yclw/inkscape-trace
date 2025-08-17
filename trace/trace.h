#ifndef INKSCAPE_TRACE_H
#define INKSCAPE_TRACE_H

#include <string>
#include <utility>
#include <vector>
#include "trace/imagemap/imagemap.h"

namespace Inkscape {

namespace Trace {

enum class TraceType {
  BRIGHTNESS,       // 亮度
  BRIGHTNESS_MULTI, // 亮度多
  CANNY,            // 边缘检测
  QUANT,            // 量化
  QUANT_COLOR,      // 量化颜色
  QUANT_MONO,       // 量化单色

  // Used in tracedialog.cpp
  AUTOTRACE_SINGLE,    // 自动单（未使用）
  AUTOTRACE_MULTI,     // 自动多（未使用）
  AUTOTRACE_CENTERLINE // 自动中线（未使用）
};

// 追踪结果项 - 直接存储 SVG 路径数据，无需复杂转换
struct TraceResultItem {
  TraceResultItem(std::string style_, std::string pathData_)
      : style(std::move(style_)), pathData(std::move(pathData_)) {}

  std::string style;    // CSS 样式，如 "fill:#000000"
  std::string pathData; // SVG 路径数据，如 "M10,10 L20,20 C30,30 40,40 50,50 Z"
};

// 追踪结果
class TraceResult {
public:
  std::vector<TraceResultItem> items;
  TraceResult() = default;
  TraceResult(std::initializer_list<TraceResultItem> items) : items(items) {}

  std::string toSvg(int width, int height) const;
  
  std::string toSvgPaths() const;
  
  void saveToSvg(const std::string& filename, int width, int height) const;
};

/**
 * A generic interface for plugging different autotracers into Inkscape.
 * 一个通用的接口，用于将不同的自动追踪器插入到 Inkscape 中。
 */
// 追踪引擎
class TracingEngine {
  public:
    TracingEngine() = default;
    virtual ~TracingEngine() = default;
  
    /**
     * This is the working method of this interface, and all implementing classes.
     * Take an RgbMap, trace it, and return TraceResult containing style attributes 
     * and SVG path data strings that are directly compatible with the d="" attribute 
     * of SVG <path> elements. No geometric conversion is needed.
     *
     * This function will be called off-main-thread, so is required to be
     * thread-safe. The lack of const however indicates that it is not required to
     * be re-entrant.
     */
    virtual TraceResult trace(RgbMap const &rgbmap) = 0;
  
    /**
     * Generate a quick preview without any actual tracing. Like trace(), this
     * must be thread-safe.
     */
    virtual RgbMap preview(RgbMap const &rgbmap) = 0;
  

  };

/**
 * Synchronous trace operation that directly returns the result.
 * Takes an RgbMap as input along with the engine.
 */
TraceResult trace(std::unique_ptr<Inkscape::Trace::TracingEngine> engine,
                  RgbMap const &rgbmap);

} // namespace Trace
} // namespace Inkscape

#endif // INKSCAPE_TRACE_H
