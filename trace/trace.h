#ifndef INKSCAPE_TRACE_H
#define INKSCAPE_TRACE_H

#include <string>
#include <utility>
#include <vector>
#include <2geom/pathvector.h>
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

// 追踪结果项
struct TraceResultItem {
  TraceResultItem(std::string style_, Geom::PathVector path_)
      : style(std::move(style_)), path(std::move(path_)) {}

  std::string style;
  Geom::PathVector path;
};

// 追踪结果
using TraceResult = std::vector<TraceResultItem>;

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
     * Take an RgbMap, trace it, and return a style attribute and the path data
     * that is compatible with the d="" attribute of an SVG <path> element.
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
  
    /**
     * Return true if the user should be checked with before tracing because the
     * image is too big.
     */
    virtual bool check_image_size(Geom::IntPoint const &size) const {
      return false;
    }
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
