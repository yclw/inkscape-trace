#ifndef SVG_H
#define SVG_H

#include <string>
#include <vector>

namespace Svg {

struct SvgItem {
  std::string pathData;
  std::string style;
};

class Svg {
public:
  int width;
  int height;
  std::vector<SvgItem> items;
  Svg() = default;
  Svg(std::initializer_list<SvgItem> items, int width, int height)
      : items(items), width(width), height(height) {}
  std::string toSvg() const;
  std::string toSvgPaths() const;
  void saveToSvg(const std::string &filename) const;
};

} // namespace Svg
#endif