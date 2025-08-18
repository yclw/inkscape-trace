#include "core/svg/svg.h"
#include <sstream>
#include <locale>
#include <fstream>

namespace Svg {

std::string Svg::toSvg() const {
  std::ostringstream svg;
  svg.imbue(std::locale::classic());

  svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  svg << "<svg width=\"" << width << "\" height=\"" << height << "\" ";
  svg << "viewBox=\"0 0 " << width << " " << height << "\" ";
  svg << "xmlns=\"http://www.w3.org/2000/svg\" ";
  svg << "preserveAspectRatio=\"xMidYMid meet\">" << std::endl;

  svg << toSvgPaths();

  svg << "</svg>" << std::endl;
  return svg.str();
}

std::string Svg::toSvgPaths() const {
  std::ostringstream paths;
  paths.imbue(std::locale::classic());

  for (int i = items.size() - 1; i >= 0; --i) {
    if (!items[i].pathData.empty()) {
      paths << "  <path d=\"" << items[i].pathData << "\" style=\""
            << items[i].style << "\" />" << std::endl;
    }
  }

  return paths.str();
}

void Svg::saveToSvg(const std::string &filename) const {
  std::ofstream file(filename);
  if (file.is_open()) {
    file << toSvg();
    file.close();
  }
}

} // namespace Svg