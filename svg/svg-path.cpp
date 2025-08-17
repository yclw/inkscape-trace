// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * svg-path.cpp: 核心SVG路径写入功能，用于TraceResult → SVG转换
 * 作者: Inkscape Trace Project
 *
 * Copyright (C) 2000-2008 authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "svg/svg-path.h"
#include "svg/path-string.h"
#include <2geom/curves.h>           // 曲线
#include <2geom/pathvector.h>       // 路径向量
#include <2geom/sbasis-to-bezier.h> // 贝塞尔曲线拟合
#include <string>

namespace Inkscape {
namespace SVG {

/**
 * 核心函数：将曲线写入PathString
 * 这是将几何曲线转换为SVG路径字符串的核心函数
 * 
 * @param str PathString对象，用于构建SVG路径字符串
 * @param c 要转换的曲线指针
 * @param normalize 是否标准化曲线（将二次贝塞尔曲线转换为三次贝塞尔曲线）
 */
static void sp_svg_write_curve(Inkscape::SVG::PathString &str,
                               Geom::Curve const *c, bool normalize = false) {
  // TODO: 这段代码需要被移除并替换为适当的路径接收器

  // 处理直线段
  if (Geom::LineSegment const *line_segment =
          dynamic_cast<Geom::LineSegment const *>(c)) {
    // 如果c是LineSegment，则写入lineTo

    // 不序列化缝合段（StitchSegment是特殊的路径段类型）
    if (!dynamic_cast<Geom::Path::StitchSegment const *>(c)) {
      // 检查是否为垂直线段（X坐标相同）
      if (!normalize && line_segment->initialPoint()[Geom::X] ==
                            line_segment->finalPoint()[Geom::X]) {
        str.verticalLineTo(line_segment->finalPoint()[Geom::Y]);
      } 
      // 检查是否为水平线段（Y坐标相同）
      else if (!normalize && line_segment->initialPoint()[Geom::Y] ==
                                   line_segment->finalPoint()[Geom::Y]) {
        str.horizontalLineTo(line_segment->finalPoint()[Geom::X]);
      } 
      // 普通斜线段
      else {
        str.lineTo((*line_segment)[1][0], (*line_segment)[1][1]);
      }
    }
    
  // 处理二次贝塞尔曲线
  } else if (Geom::QuadraticBezier const *quadratic_bezier =
                 dynamic_cast<Geom::QuadraticBezier const *>(c)) {
    if (normalize) {
      // 将二次贝塞尔曲线转换为三次贝塞尔曲线
      // 使用德卡斯特洛算法计算控制点
      double x1 = 1.0 / 3 * quadratic_bezier->initialPoint()[0] +
                  2.0 / 3 * (*quadratic_bezier)[1][0];
      double x2 = 2.0 / 3 * (*quadratic_bezier)[1][0] +
                  1.0 / 3 * (*quadratic_bezier)[2][0];
      double y1 = 1.0 / 3 * quadratic_bezier->initialPoint()[1] +
                  2.0 / 3 * (*quadratic_bezier)[1][1];
      double y2 = 2.0 / 3 * (*quadratic_bezier)[1][1] +
                  1.0 / 3 * (*quadratic_bezier)[2][1];
      str.curveTo(x1, y1, x2, y2, (*quadratic_bezier)[2][0],
                  (*quadratic_bezier)[2][1]);
    } else {
      // 直接使用二次贝塞尔曲线格式
      str.quadTo((*quadratic_bezier)[1][0], (*quadratic_bezier)[1][1],
                 (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1]);
    }
    
  // 处理三次贝塞尔曲线
  } else if (Geom::CubicBezier const *cubic_bezier =
                 dynamic_cast<Geom::CubicBezier const *>(c)) {
    // 如果c是CubicBezier，则写入curveTo
    // 三次贝塞尔曲线有四个控制点：起点、两个控制点、终点

    str.curveTo((*cubic_bezier)[1][0], (*cubic_bezier)[1][1],
                (*cubic_bezier)[2][0], (*cubic_bezier)[2][1],
                (*cubic_bezier)[3][0], (*cubic_bezier)[3][1]);
                
  // 处理椭圆弧
  } else if (Geom::EllipticalArc const *elliptical_arc =
                 dynamic_cast<Geom::EllipticalArc const *>(c)) {
    // 如果c是EllipticalArc，则写入arcTo
    // 椭圆弧需要半径、旋转角度、大弧标志、扫描标志和终点

    str.arcTo(elliptical_arc->ray(Geom::X), elliptical_arc->ray(Geom::Y),
              Geom::deg_from_rad(elliptical_arc->rotationAngle()),
              elliptical_arc->largeArc(), elliptical_arc->sweep(),
              elliptical_arc->finalPoint());
              
  } else {
    // 这种情况处理SBasis以及所有其他曲线类型
    // SBasis是一种数学表示，需要转换为贝塞尔曲线才能写入SVG
    
    // 将SBasis曲线转换为三次贝塞尔曲线路径，精度为0.1
    Geom::Path sbasis_path =
        Geom::cubicbezierpath_from_sbasis(c->toSBasis(), 0.1);

    // 递归调用，将转换后的新路径转换为SVG格式
    for (const auto &iter : sbasis_path) {
      sp_svg_write_curve(str, &iter, normalize);
    }
  }
}

/**
 * 核心函数：写入单个路径
 * 将几何路径转换为SVG路径字符串
 * 
 * @param str PathString对象，用于构建SVG路径字符串
 * @param p 要转换的路径
 * @param normalize 是否标准化曲线
 */
static void sp_svg_write_path(Inkscape::SVG::PathString &str,
                              Geom::Path const &p, bool normalize = false) {
  // 移动到路径的起始点
  str.moveTo(p.initialPoint()[0], p.initialPoint()[1]);

  // 遍历路径中的所有曲线段（除了最后一个闭合段）
  for (Geom::Path::const_iterator cit = p.begin(); cit != p.end_open(); ++cit) {
    sp_svg_write_curve(str, &(*cit), normalize);
  }

  // 如果路径是闭合的，则添加闭合命令
  if (p.closed()) {
    str.closePath();
  }
}

/**
 * 主要公共函数：将PathVector写入SVG路径字符串
 * 这是将整个路径向量转换为SVG路径字符串的入口函数
 * 
 * @param p 要转换的路径向量
 * @param normalize 是否标准化曲线
 * @return 生成的SVG路径字符串
 */
std::string sp_svg_write_path(Geom::PathVector const &p, bool normalize) {
  Inkscape::SVG::PathString str;
  
  // 如果需要标准化，创建具有特定精度设置的PathString
  if (normalize) {
    str = Inkscape::SVG::PathString(Inkscape::SVG::PATHSTRING_ABSOLUTE, 4, -2,
                                    true);
  }

  // 遍历路径向量中的所有路径
  for (const auto &pit : p) {
    sp_svg_write_path(str, pit, normalize);
  }

  return str;
}

/**
 * 便捷函数：写入单个路径
 * 将单个几何路径转换为SVG路径字符串
 * 
 * @param p 要转换的路径
 * @return 生成的SVG路径字符串
 */
std::string sp_svg_write_path(Geom::Path const &p) {
  Inkscape::SVG::PathString str;
  sp_svg_write_path(str, p);
  return str;
}

} // namespace SVG
} // namespace Inkscape
