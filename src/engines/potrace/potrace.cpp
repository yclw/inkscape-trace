#include "engines/potrace/potrace.h"
#include "filters/filterset.h"
#include "trace/trace.h"
#include <locale>
#include <sstream>
#include <iomanip>
#include <unordered_set>

namespace {

// 状态删除器
struct potrace_state_deleter {
  void operator()(potrace_state_t *p) { potrace_state_free(p); };
};
using potrace_state_uniqptr =
    std::unique_ptr<potrace_state_t, potrace_state_deleter>;

// 位图删除器
struct potrace_bitmap_deleter {
  void operator()(potrace_bitmap_t *p) { bm_free(p); };
};
using potrace_bitmap_uniqptr =
    std::unique_ptr<potrace_bitmap_t, potrace_bitmap_deleter>;

// 十六进制字符串
std::string twohex(int value) {
  std::ostringstream ss;
  ss.imbue(std::locale::classic());
  ss << std::hex << std::setfill('0') << std::setw(2) << value;
  return ss.str();
}

} // namespace

namespace Potrace {

// 构造函数
PotraceTracingEngine::PotraceTracingEngine() { common_init(); }

// 构造函数
PotraceTracingEngine::PotraceTracingEngine(
    TraceType traceType, bool invert, int quantizationNrColors,
    double brightnessThreshold, double brightnessFloor,
    double cannyHighThreshold, int multiScanNrColors, bool multiScanStack,
    bool multiScanSmooth, bool multiScanRemoveBackground)
    : traceType(traceType), invert(invert),
      quantizationNrColors(quantizationNrColors),
      brightnessThreshold(brightnessThreshold),
      brightnessFloor(brightnessFloor), cannyHighThreshold(cannyHighThreshold),
      multiScanNrColors(multiScanNrColors), multiScanStack(multiScanStack),
      multiScanSmooth(multiScanSmooth),
      multiScanRemoveBackground(multiScanRemoveBackground) {
  common_init();
}

// 初始化
void PotraceTracingEngine::common_init() {
  potraceParams = potrace_param_default();
}

// 析构函数
PotraceTracingEngine::~PotraceTracingEngine() {
  potrace_param_free(potraceParams);
}

// 设置优化曲线
void PotraceTracingEngine::setOptiCurve(int opticurve) {
  potraceParams->opticurve = opticurve;
}

// 设置优化容差
void PotraceTracingEngine::setOptTolerance(double opttolerance) {
  potraceParams->opttolerance = opttolerance;
}

// 设置最大透明度
void PotraceTracingEngine::setAlphaMax(double alphamax) {
  potraceParams->alphamax = alphamax;
}

// 设置斑点大小
void PotraceTracingEngine::setTurdSize(int turdsize) {
  potraceParams->turdsize = turdsize;
}

/**
 * Recursively descend the potrace_path_t node tree \a paths, writing paths to
 * \a builder. The \a points set is used to prevent redundant paths.
 */
/**
 * 递归遍历 potrace_path_t 节点树 \a paths, 直接写入 SVG 路径字符串.
 * 这比原来的几何转换方式快得多！
 */
void PotraceTracingEngine::writePathsToSvg(potrace_path_t *paths, 
                                          std::ostringstream &out) const {
  std::unordered_set<std::string> processedPaths; // 防止重复路径
  
  for (auto path = paths; path; path = path->sibling) {
    auto const &curve = path->curve;
    if (curve.n == 0) {
      continue;
    }

    // 开始构建这个路径的 SVG 字符串
    std::ostringstream pathStr;
    
    // 移动到起始点
    auto seg = curve.c[curve.n - 1];
    pathStr << "M" << std::fixed << std::setprecision(2) 
            << seg[2].x << "," << seg[2].y;

    // 处理所有曲线段
    for (int i = 0; i < curve.n; i++) {
      auto seg = curve.c[i];
      switch (curve.tag[i]) {
      case POTRACE_CORNER:
        // 直线段：两个 lineTo 命令
        pathStr << "L" << seg[1].x << "," << seg[1].y
                << "L" << seg[2].x << "," << seg[2].y;
        break;
      case POTRACE_CURVETO:
        // 贝塞尔曲线：curveTo 命令
        pathStr << "C" << seg[0].x << "," << seg[0].y << " "
                << seg[1].x << "," << seg[1].y << " "
                << seg[2].x << "," << seg[2].y;
        break;
      default:
        break;
      }
    }
    pathStr << "Z"; // 闭合路径
    
    // 检查是否已处理过这个路径（避免重复）
    std::string pathString = pathStr.str();
    if (processedPaths.find(pathString) == processedPaths.end()) {
      processedPaths.insert(pathString);
      out << pathString;
    }

    // 递归处理子路径
    if (path->childlist) {
      writePathsToSvg(path->childlist, out);
    }
  }
}

// 过滤
std::optional<GrayMap>
PotraceTracingEngine::filter(RgbMap const &rgbmap) const {
  std::optional<GrayMap> map;

  if (traceType == TraceType::QUANT) {
    // Color quantization -- banding

    // rgbMap->writePPM(rgbMap, "rgb.ppm");
    map = quantizeBand(rgbmap, quantizationNrColors);

  } else if (traceType == TraceType::BRIGHTNESS ||
             traceType == TraceType::BRIGHTNESS_MULTI) {
    // Brightness threshold
    auto gm = rgbMapToGrayMap(rgbmap);
    map = GrayMap(gm.width, gm.height);

    double floor = 3.0 * brightnessFloor * 256.0;
    double cutoff = 3.0 * brightnessThreshold * 256.0;
    for (int y = 0; y < gm.height; y++) {
      for (int x = 0; x < gm.width; x++) {
        double brightness = gm.getPixel(x, y);
        bool black = brightness >= floor && brightness < cutoff;
        map->setPixel(x, y, black ? GrayMap::BLACK : GrayMap::WHITE);
      }
    }

    // map->writePPM(map, "brightness.ppm");

  } else if (traceType == TraceType::CANNY) {
    // Canny edge detection
    auto gm = rgbMapToGrayMap(rgbmap);
    map = grayMapCanny(gm, 0.1, cannyHighThreshold);
    // map->writePPM(map, "canny.ppm");
  }

  // Invert the image if necessary.
  if (map && invert) {
    for (int y = 0; y < map->height; y++) {
      for (int x = 0; x < map->width; x++) {
        auto brightness = map->getPixel(x, y);
        brightness = GrayMap::WHITE - brightness;
        map->setPixel(x, y, brightness);
      }
    }
  }

  return map;
}

// 过滤索引
IndexedMap PotraceTracingEngine::filterIndexed(RgbMap const &rgbmap) const {

  auto map = rgbmap;
  if (multiScanSmooth) {
    map = rgbMapGaussian(map);
  }

  auto imap = rgbMapQuantize(map, multiScanNrColors);

  auto tomono = [](RGB c) -> RGB {
    unsigned char s = ((int)c.r + (int)c.g + (int)c.b) / 3;
    return {s, s, s};
  };

  if (traceType == TraceType::QUANT_MONO ||
      traceType == TraceType::BRIGHTNESS_MULTI) {
    // Turn to grays
    for (auto &c : imap.clut) {
      c = tomono(c);
    }
  }

  return imap;
}

// 预览
RgbMap PotraceTracingEngine::preview(RgbMap const &rgbmap) {
  if (traceType == TraceType::QUANT_COLOR ||
      traceType == TraceType::QUANT_MONO ||
      traceType ==
          TraceType::BRIGHTNESS_MULTI) // this is a lie: multipass doesn't use
                                       // filterIndexed, but it's a better
                                       // preview approx than filter()
  {
    auto imap = filterIndexed(rgbmap);
    return indexedMapToRgbMap(imap);

  } else {
    auto gm = filter(rgbmap);
    if (!gm) {
      return RgbMap(0, 0);
    }

    return grayMapToRgbMap(*gm);
  }
}

/**
 * This is the actual wrapper of the call to Potrace.
 * 直接返回 SVG 路径字符串
 */
// 灰度图直接转 SVG 字符串
std::string PotraceTracingEngine::grayMapToSvg(GrayMap const &grayMap) {
  auto potraceBitmap =
      potrace_bitmap_uniqptr(bm_new(grayMap.width, grayMap.height));
  if (!potraceBitmap) {
    return "";
  }

  bm_clear(potraceBitmap.get(), 0);

  // Read the data out of the GrayMap
  for (int y = 0; y < grayMap.height; y++) {
    for (int x = 0; x < grayMap.width; x++) {
      BM_UPUT(potraceBitmap, x, y, grayMap.getPixel(x, y) ? 0 : 1);
    }
  }
  // Trace the bitmap.

  // Progress reporting removed
  auto potraceState =
      potrace_state_uniqptr(potrace_trace(potraceParams, potraceBitmap.get()));

  potraceBitmap.reset();

  // 直接提取 SVG 路径字符串！
  std::ostringstream svgPath;
  writePathsToSvg(potraceState->plist, svgPath);
  return svgPath.str();
}

/**
 * This is called for a single scan.
 */
// 单
TraceResult PotraceTracingEngine::traceSingle(RgbMap const &rgbmap) {
  brightnessFloor = 0.0; // important to set this, since used by filter()

  auto grayMap = filter(rgbmap);
  if (!grayMap) {
    return {};
  }

  auto svgPath = grayMapToSvg(*grayMap);

  TraceResult results;
  results.items.emplace_back("fill:#000000", std::move(svgPath));
  return results;
}

/**
 * This allows routines that already generate GrayMaps to skip image filtering,
 * increasing performance.
 */
// 追踪灰度图
TraceResult PotraceTracingEngine::traceGrayMap(GrayMap const &grayMap) {
  auto svgPath = grayMapToSvg(grayMap);

  TraceResult results;
  results.items.emplace_back("fill:#000000", std::move(svgPath));
  return results;
}

/**
 * Called for multiple-scanning algorithms
 */
// 亮度多
TraceResult PotraceTracingEngine::traceBrightnessMulti(RgbMap const &rgbmap) {
  double constexpr low = 0.2;  // bottom of range
  double constexpr high = 0.9; // top of range
  double const delta = (high - low) / multiScanNrColors;

  brightnessFloor = 0.0; // Set bottom to black

  TraceResult results;

  for (int i = 0; i < multiScanNrColors; i++) {

    brightnessThreshold = low + delta * i;

    auto grayMap = filter(rgbmap);
    if (!grayMap) {
      continue;
    }

    auto svgPath = grayMapToSvg(*grayMap);
    if (svgPath.empty()) {
      continue;
    }

    // get style info
    int grayVal = 256.0 * brightnessThreshold;
    auto style = "fill-opacity:1.0;fill:#" + twohex(grayVal) + twohex(grayVal) +
                 twohex(grayVal);

    // g_message("### GOT '%s' \n", style.c_str());
    results.items.emplace_back(style, std::move(svgPath));

    if (!multiScanStack) {
      brightnessFloor = brightnessThreshold;
    }
  }

  // Remove the bottom-most scan, if requested.
  if (results.items.size() > 1 && multiScanRemoveBackground) {
    results.items.pop_back();
  }

  return results;
}

/**
 * Quantization
 */
// 量化
TraceResult PotraceTracingEngine::traceQuant(RgbMap const &rgbmap) {
  auto imap = filterIndexed(rgbmap);

  // Create and clear a gray map
  auto gm = GrayMap(imap.width, imap.height);
  for (int row = 0; row < gm.height; row++) {
    for (int col = 0; col < gm.width; col++) {
      gm.setPixel(col, row, GrayMap::WHITE);
    }
  }

  TraceResult results;

  for (int colorIndex = 0; colorIndex < imap.nrColors; colorIndex++) {

    // Update the graymap for the current color index
    for (int row = 0; row < imap.height; row++) {
      for (int col = 0; col < imap.width; col++) {
        int index = imap.getPixel(col, row);
        if (index == colorIndex) {
          gm.setPixel(col, row, GrayMap::BLACK);
        } else if (!multiScanStack) {
          gm.setPixel(col, row, GrayMap::WHITE);
        }
      }
    }

    // Now we have a traceable graymap
    auto svgPath = grayMapToSvg(gm);

    if (!svgPath.empty()) {
      // get style info
      auto rgb = imap.clut[colorIndex];
      auto style = "fill:#" + twohex(rgb.r) + twohex(rgb.g) + twohex(rgb.b);
      results.items.emplace_back(style, std::move(svgPath));
    }
  }

  // Remove the bottom-most scan, if requested.
  if (results.items.size() > 1 && multiScanRemoveBackground) {
    results.items.pop_back();
  }

  return results;
}

// 追踪
TraceResult PotraceTracingEngine::trace(RgbMap const &rgbmap) {
  if (traceType == TraceType::QUANT_COLOR ||
      traceType == TraceType::QUANT_MONO) {
    return traceQuant(rgbmap);
  } else if (traceType == TraceType::BRIGHTNESS_MULTI) {
    return traceBrightnessMulti(rgbmap);
  } else {
    return traceSingle(rgbmap);
  }
}

} // namespace Potrace
