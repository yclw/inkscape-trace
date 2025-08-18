// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * A generic interface for plugging different
 *  autotracers into Inkscape.
 *
 * Authors:
 *   Bob Jamison <rjamison@earthlink.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   PBS <pbs3141@gmail.com>
 *
 * Copyright (C) 2004-2022 Authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "trace/trace.h"


#include <cassert>
#include <fstream>
#include <sstream>
#include <locale>

namespace Inkscape::Trace {
TraceResult trace(std::unique_ptr<TracingEngine> engine,
                  RgbMap const &rgbmap) {
  if (!engine) {
    return TraceResult();
  }
  return engine->trace(rgbmap);
}

// TraceResult

std::string TraceResult::toSvg(int width, int height) const {
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

std::string TraceResult::toSvgPaths() const {
  std::ostringstream paths;
  paths.imbue(std::locale::classic());
  
  for (int i = items.size() - 1; i >= 0; --i) {
    if (!items[i].pathData.empty()) {
      paths << "  <path d=\"" << items[i].pathData << "\" style=\"" << items[i].style << "\" />" << std::endl;
    }
  }
  
  return paths.str();
}

void TraceResult::saveToSvg(const std::string& filename, int width, int height) const {
  std::ofstream file(filename);
  if (file.is_open()) {
    file << toSvg(width, height);
    file.close();
  }
}

} // namespace Inkscape::Trace
