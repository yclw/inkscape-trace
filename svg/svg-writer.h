// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SVG_WRITER_H
#define SEEN_SVG_WRITER_H

/*
 * SVG file writer utilities
 *
 * Authors:
 *   Inkscape Trace Project
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <string>
#include "trace/trace.h"

namespace Inkscape {
namespace SVG {

/**
 * 生成SVG文件
 * 将TraceResult转换为完整的SVG文件
 * 
 * @param traceResult 追踪结果，包含路径和样式信息
 * @param filename 输出SVG文件名
 * @param width SVG画布宽度
 * @param height SVG画布高度
 */
void generateVectorSvg(const Inkscape::Trace::TraceResult &traceResult,
                      const std::string &filename, int width, int height);

} // namespace SVG
} // namespace Inkscape

#endif // SEEN_SVG_WRITER_H
