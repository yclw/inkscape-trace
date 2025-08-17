// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SVG_PATH_H
#define SEEN_SVG_PATH_H

/*
 * SVG path writing functionality for TraceResult → SVG conversion
 * Authors:
 *   Inkscape Trace Project
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <string>
#include <2geom/forward.h>

namespace Inkscape {
namespace SVG {

// Main public function for writing PathVector to SVG path string
std::string sp_svg_write_path(Geom::PathVector const &p, bool normalize = false);

// Convenience function for writing single Path
std::string sp_svg_write_path(Geom::Path const &p);

} // namespace SVG
} // namespace Inkscape

#endif // SEEN_SVG_PATH_H
