// SPDX-License-Identifier: GPL-2.0-or-later
#include "svg-writer.h"
#include "svg.h"
#include <fstream>
#include <iostream>
#include <locale>

namespace Inkscape {
namespace SVG {

void generateVectorSvg(const Inkscape::Trace::TraceResult &traceResult,
                      const std::string &filename, int width, int height) {
  std::ofstream file(filename);
  file.imbue(std::locale::classic());

  file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  file << "<svg width=\"" << width << "\" height=\"" << height << "\" ";
  file << "viewBox=\"0 0 " << width << " " << height << "\" ";
  file << "xmlns=\"http://www.w3.org/2000/svg\" ";
  file << "preserveAspectRatio=\"xMidYMid meet\">" << std::endl;

  for (auto it = traceResult.rbegin(); it != traceResult.rend(); ++it) {
    if (it->path.empty())
      continue;

    std::string pathData = sp_svg_write_path(it->path);
    file << "  <path d=\"" << pathData << "\" style=\"" << it->style << "\" />"
         << std::endl;
  }

  file << "</svg>" << std::endl;
  file.close();
}

} // namespace SVG
} // namespace Inkscape