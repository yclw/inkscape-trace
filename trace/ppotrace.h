#ifndef PPOTRACE_H
#define PPOTRACE_H
#include <optional>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include <2geom/path-sink.h>
#include <2geom/point.h>
#include <opencv2/opencv.hpp>
#include "imagemap.h"
#include "trace.h"
#include <glibmm/refptr.h>
#include "imagemap-gdk.h"
#include <iomanip>
#include <potracelib.h>
#include "pbitmap.h"
#include "util/string/ustring-format.h"
#include "quantize.h"
#include "filterset.h"

using potrace_param_t = struct potrace_param_s;
using potrace_path_t = struct potrace_path_s;

namespace Inkscape {
namespace Trace {
namespace Potrace {

enum class TraceType
{
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

// Potrace 追踪引擎
class PotraceTracingEngine final : public TracingEngine
{
public:
    PotraceTracingEngine();

    PotraceTracingEngine(TraceType traceType,           // 追踪类型
                         bool invert,                   // 是否反转
                         int quantizationNrColors,      // 量化颜色数
                         double brightnessThreshold,    // 亮度阈值
                         double brightnessFloor,        // 亮度地板
                         double cannyHighThreshold,     // 边缘检测阈值
                         int multiScanNrColors,         // 多扫描颜色数
                         bool multiScanStack,           // 多扫描堆叠
                         bool multiScanSmooth,          // 多扫描平滑
                         bool multiScanRemoveBackground // 多扫描移除背景
    );

    ~PotraceTracingEngine() override;

    // 追踪  
    TraceResult trace(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf, Async::Progress &progress) override;
    // 预览
    Glib::RefPtr<Gdk::Pixbuf> preview(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf) override;
    // 追踪灰度图
    TraceResult traceGrayMap(GrayMap const &grayMap, Async::Progress &progress);
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
    TraceResult traceQuant(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf, Async::Progress &progress);
    // 亮度多  
    TraceResult traceBrightnessMulti(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf, Async::Progress &progress);
    // 单
    TraceResult traceSingle(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf, Async::Progress &progress);

    // 过滤索引
    IndexedMap filterIndexed(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf) const;
    // 过滤
    std::optional<GrayMap> filter(Glib::RefPtr<Gdk::Pixbuf> const &pixbuf) const;
    // 灰度图转路径
    Geom::PathVector grayMapToPath(GrayMap const &gm, Async::Progress &progress);
    // 写路径
    void writePaths(potrace_path_t *paths, Geom::PathBuilder &builder, std::unordered_set<Geom::Point> &points,
                    Async::Progress &progress) const;
};

} // namespace Potrace
} // namespace Trace
} // namespace Inkscape

#endif // PPOTRACE_H
