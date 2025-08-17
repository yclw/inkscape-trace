// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * Inkscape::SVG::PathString - SVG路径字符串构建器
 *//*
 * Authors: see git history
 * 
 * Copyright 2007 MenTaLguY <mental@rydia.net>
 * Copyright 2008 Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 * Copyright 2013 Tavmjong Bah <tavmjong@free.fr>
 * Copyright (C) 2014 Authors
 * 
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#ifndef SEEN_INKSCAPE_SVG_PATH_STRING_H
#define SEEN_INKSCAPE_SVG_PATH_STRING_H

#include <2geom/point.h>
#include <string>

namespace Inkscape {

namespace SVG {

// 相对坐标与绝对坐标的枚举
enum PATHSTRING_FORMAT {
  PATHSTRING_ABSOLUTE, // 仅使用绝对坐标
  PATHSTRING_RELATIVE, // 仅使用相对坐标
  PATHSTRING_OPTIMIZE, // 优化路径字符串长度
  PATHSTRING_FORMAT_SIZE
};

/**
 * SVG路径字符串构建器类
 * 用于构建SVG路径数据字符串，支持绝对坐标、相对坐标和优化模式
 */
class PathString {
public:
  // 默认构造函数
  PathString();
  
  // 带参数的构造函数，手动覆盖Inkscape的偏好设置
  PathString(PATHSTRING_FORMAT format, int precision, int minexp, bool force);

  // 默认拷贝构造函数
  // 默认赋值操作符

  /**
   * 获取完整的路径字符串
   * @return 包含公共前缀和尾部内容的完整路径字符串
   */
  std::string const &string() {
    std::string const &t = tail();
    final.reserve(commonbase.size() + t.size());
    final = commonbase;
    final += tail();
    return final;
  }

  /**
   * 类型转换操作符，将PathString转换为std::string
   */
  operator std::string const &() { return string(); }

  /**
   * 获取C风格字符串指针
   * @return 路径字符串的C风格字符串指针
   */
  char const *c_str() { return string().c_str(); }

  /**
   * 移动到指定坐标点（使用坐标分量）
   * @param x X坐标
   * @param y Y坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &moveTo(Geom::Coord x, Geom::Coord y) {
    return moveTo(Geom::Point(x, y));
  }

  /**
   * 移动到指定点
   * @param p 目标点
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &moveTo(Geom::Point p) {
    _appendOp('M', 'm');
    _appendPoint(p, true);

    _initial_point = _current_point;
    return *this;
  }

  /**
   * 绘制直线到指定坐标点（使用坐标分量）
   * @param x X坐标
   * @param y Y坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &lineTo(Geom::Coord x, Geom::Coord y) {
    return lineTo(Geom::Point(x, y));
  }

  /**
   * 绘制直线到指定点
   * @param p 目标点
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &lineTo(Geom::Point p) {
    _appendOp('L', 'l');
    _appendPoint(p, true);
    return *this;
  }

  /**
   * 绘制水平直线到指定X坐标
   * @param x 目标X坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &horizontalLineTo(Geom::Coord x) {
    _appendOp('H', 'h');
    _appendX(x, true);
    return *this;
  }

  /**
   * 绘制垂直直线到指定Y坐标
   * @param y 目标Y坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &verticalLineTo(Geom::Coord y) {
    _appendOp('V', 'v');
    _appendY(y, true);
    return *this;
  }

  /**
   * 绘制二次贝塞尔曲线（使用坐标分量）
   * @param cx 控制点X坐标
   * @param cy 控制点Y坐标
   * @param x 终点X坐标
   * @param y 终点Y坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &quadTo(Geom::Coord cx, Geom::Coord cy, Geom::Coord x,
                     Geom::Coord y) {
    return quadTo(Geom::Point(cx, cy), Geom::Point(x, y));
  }

  /**
   * 绘制二次贝塞尔曲线
   * @param c 控制点
   * @param p 终点
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &quadTo(Geom::Point c, Geom::Point p) {
    _appendOp('Q', 'q');
    _appendPoint(c, false);
    _appendPoint(p, true);
    return *this;
  }

  /**
   * 绘制三次贝塞尔曲线（使用坐标分量）
   * @param x0 第一个控制点X坐标
   * @param y0 第一个控制点Y坐标
   * @param x1 第二个控制点X坐标
   * @param y1 第二个控制点Y坐标
   * @param x 终点X坐标
   * @param y 终点Y坐标
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &curveTo(Geom::Coord x0, Geom::Coord y0, Geom::Coord x1,
                      Geom::Coord y1, Geom::Coord x, Geom::Coord y) {
    return curveTo(Geom::Point(x0, y0), Geom::Point(x1, y1), Geom::Point(x, y));
  }

  /**
   * 绘制三次贝塞尔曲线
   * @param c0 第一个控制点
   * @param c1 第二个控制点
   * @param p 终点
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &curveTo(Geom::Point c0, Geom::Point c1, Geom::Point p) {
    _appendOp('C', 'c');
    _appendPoint(c0, false);
    _appendPoint(c1, false);
    _appendPoint(p, true);
    return *this;
  }

  /**
   * 绘制椭圆弧
   * @param rx X轴半径
   * @param ry Y轴半径
   * @param rot 旋转角度（度）
   * @param large_arc 是否为大弧
   * @param sweep 是否为顺时针方向
   * @param p 终点
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &arcTo(Geom::Coord rx, Geom::Coord ry, Geom::Coord rot,
                    bool large_arc, bool sweep, Geom::Point p) {
    _appendOp('A', 'a');
    _appendValue(Geom::Point(rx, ry));
    _appendValue(rot);
    _appendFlag(large_arc);
    _appendFlag(sweep);
    _appendPoint(p, true);
    return *this;
  }

  /**
   * 闭合路径
   * @return 当前PathString对象的引用，支持链式调用
   */
  PathString &closePath() {

    _abs_state.appendOp('Z');
    _rel_state.appendOp('z');

    _current_point = _initial_point;
    return *this;
  }

private:
  /**
   * 添加操作符到绝对和相对状态
   * @param abs_op 绝对坐标操作符
   * @param rel_op 相对坐标操作符
   */
  void _appendOp(char abs_op, char rel_op);

  /**
   * 添加布尔标志值
   * @param flag 布尔值
   */
  void _appendFlag(bool flag) {
    _abs_state.append(flag);
    _rel_state.append(flag);
  }

  /**
   * 添加坐标值
   * @param v 坐标值
   */
  void _appendValue(Geom::Coord v) {
    _abs_state.append(v);
    _rel_state.append(v);
  }

  /**
   * 添加点值
   * @param p 点
   */
  void _appendValue(Geom::Point p) {
    _abs_state.append(p);
    _rel_state.append(p);
  }

  /**
   * 添加X坐标值
   * @param x X坐标
   * @param sc 是否更新当前点
   */
  void _appendX(Geom::Coord x, bool sc) {
    double rx;
    _abs_state.append(x, rx);
    _rel_state.appendRelative(rx, _current_point[Geom::X]);
    if (sc)
      _current_point[Geom::X] = rx;
  }

  /**
   * 添加Y坐标值
   * @param y Y坐标
   * @param sc 是否更新当前点
   */
  void _appendY(Geom::Coord y, bool sc) {
    double ry;
    _abs_state.append(y, ry);
    _rel_state.appendRelative(ry, _current_point[Geom::Y]);
    if (sc)
      _current_point[Geom::Y] = ry;
  }

  /**
   * 添加点值
   * @param p 点
   * @param sc 是否更新当前点
   */
  void _appendPoint(Geom::Point p, bool sc) {
    Geom::Point rp;
    _abs_state.append(p, rp);
    _rel_state.appendRelative(rp, _current_point);
    if (sc)
      _current_point = rp;
  }

  /**
   * 内部状态类，用于管理绝对和相对坐标的字符串构建
   */
  struct State {
    /**
     * 构造函数
     * @param precision 数值精度
     * @param minexp 最小指数
     */
    State(int precision = 8, int minexp = -8) {
      _precision = precision;
      _minexp = minexp;
    }

    /**
     * 添加操作符
     * @param op 操作符
     */
    void appendOp(char op) {
      if (prevop != 0)
        str += ' ';
      str += op;
      prevop = (op == 'M' ? 'L' : op == 'm' ? 'l' : op);
    }

    /**
     * 添加布尔标志
     * @param flag 布尔值
     */
    void append(bool flag) {
      str += ' ';
      str += (flag ? '1' : '0');
    }

    // 添加坐标值的方法声明
    void append(Geom::Coord v);
    void append(Geom::Point v);
    void append(Geom::Coord v, Geom::Coord &rv);
    void append(Geom::Point p, Geom::Point &rp);
    void appendRelative(Geom::Coord v, Geom::Coord r);
    void appendRelative(Geom::Point p, Geom::Point r);

    /**
     * 比较操作符，用于优化模式下的状态选择
     * @param s 要比较的状态
     * @return 当前状态是否小于等于给定状态
     */
    bool operator<=(const State &s) const {
      if (str.size() < s.str.size())
        return true;
      if (str.size() > s.str.size())
        return false;
      if (switches < s.switches)
        return true;
      if (switches > s.switches)
        return false;
      return true;
    }

    // 注意：将其改为Glib::ustring可能会导致path-string.cpp中的问题
    // 因为它假设size()返回字符串的字节大小（而Glib::ustring::resize非常慢）
    std::string str;        // 存储路径字符串
    unsigned int switches = 0; // 状态切换次数
    char prevop = 0;        // 前一个操作符
    int _minexp;            // 最小指数
    int _precision;         // 数值精度

  private:
    // 私有方法声明
    void appendNumber(double v, int precision, int minexp);
    void appendNumber(double v, double &rv);
    void appendRelativeCoord(Geom::Coord v, Geom::Coord r);
  } _abs_state, _rel_state; // 绝对和相对坐标的状态对象

  Geom::Point _initial_point;  // 路径的起始点
  Geom::Point _current_point;  // 当前点位置

  // 如果两个状态有公共前缀，则存储在这里
  // 分离公共前缀可以防止在状态之间重复复制导致二次时间复杂度
  // （以字符/操作符数量为基准）
  std::string commonbase;      // 公共前缀
  std::string final;           // 最终结果字符串
  
  /**
   * 获取尾部字符串（根据格式选择绝对或相对状态）
   * @return 当前格式下的尾部字符串
   */
  std::string const &tail() const {
    return ((_format == PATHSTRING_ABSOLUTE) ||
                    (_format == PATHSTRING_OPTIMIZE && _abs_state <= _rel_state)
                ? _abs_state.str
                : _rel_state.str);
  }

  PATHSTRING_FORMAT _format;           // 路径字符串格式
  bool _force_repeat_commands;         // 是否强制重复命令
};

} // namespace SVG

} // namespace Inkscape

#endif
