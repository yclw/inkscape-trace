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

#include "trace.h"

#include <2geom/transforms.h>
#include <cassert>


namespace Inkscape::Trace {
TraceResult trace(std::unique_ptr<TracingEngine> engine,
                  RgbMap const &rgbmap) {
  if (!engine) {
    return TraceResult();
  }
  return engine->trace(rgbmap);
}

} // namespace Inkscape::Trace
