// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * PathString - SVG路径字符串构建器
 *//*
 * Authors: see git history
 * 
 * Copyright 2008 Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 * Copyright 2013 Tavmjong Bah <tavmjong@free.fr>
 * Copyright (C) 2018 Authors
 * 
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include "svg/path-string.h"
#include "svg/svg.h"
#include <cmath>
#include <iostream>

// 数值精度范围：1<=numericprecision<=16
// double类型只能精确到（略少于）16位数字
// （少于1位数字没有意义）
// 注意：这些常量用于分配足够的空间来保存序列化的数字
static int const minprec = 1;    // 最小精度
static int const maxprec = 16;   // 最大精度

namespace Inkscape {
namespace SVG {

/**
 * 使用默认值构造路径字符串（简化版本，不包含偏好设置）
 */
PathString::PathString() {
  _format = PATHSTRING_ABSOLUTE;  // 默认使用绝对坐标
  _force_repeat_commands = false; // 默认不强制重复命令
  int precision = 8;              // 默认精度为8
  int minexp = -8;                // 默认最小指数为-8
  _abs_state = State(precision, minexp);
  _rel_state = State(precision, minexp);
}

/**
 * 构造路径字符串，手动覆盖Inkscape的偏好设置
 * @param format 路径字符串格式
 * @param precision 数值精度
 * @param minexp 最小指数
 * @param force_repeat 是否强制重复命令
 */
PathString::PathString(PATHSTRING_FORMAT format, int precision, int minexp,
                       bool force_repeat)
    : _abs_state(precision, minexp), _rel_state(precision, minexp),
      _format(format), _force_repeat_commands(force_repeat) {}

// 对于绝对和相对路径...整个路径都保存在"tail"中
// 对于优化路径，在绝对和相对之间切换时，将tail添加到commonbase
/**
 * 添加操作符到绝对和相对状态
 * @param abs_op 绝对坐标操作符
 * @param rel_op 相对坐标操作符
 */
void PathString::_appendOp(char abs_op, char rel_op) {
  // 检查操作符是否重复（如果不强制重复命令）
  bool abs_op_repeated = _abs_state.prevop == abs_op && !_force_repeat_commands;
  bool rel_op_repeated = _rel_state.prevop == rel_op && !_force_repeat_commands;

  // 根据格式处理操作符
  switch (_format) {
  case PATHSTRING_ABSOLUTE:
    // 绝对坐标模式：如果不重复则添加操作符
    if (!abs_op_repeated)
      _abs_state.appendOp(abs_op);
    break;
  case PATHSTRING_RELATIVE:
    // 相对坐标模式：如果不重复则添加操作符
    if (!rel_op_repeated)
      _rel_state.appendOp(rel_op);
    break;
  case PATHSTRING_OPTIMIZE: {
    // 优化模式：计算添加操作符后的字符串大小
    unsigned int const abs_added_size = abs_op_repeated ? 0 : 2;
    unsigned int const rel_added_size = rel_op_repeated ? 0 : 2;
    
    // 如果相对状态更短，则切换到相对状态
    if (_rel_state.str.size() + 2 < _abs_state.str.size() + abs_added_size) {

      // 存储公共前缀
      commonbase += _rel_state.str;
      _rel_state.str.clear();
      // 将相对状态复制到绝对状态
      _abs_state = _rel_state;
      _abs_state.switches++;
      abs_op_repeated = false;
      // 不需要将绝对状态复制到相对状态：
      //   _rel_state.str.size()+2 < _abs_state.str.size()+abs_added_size
      //   _rel_state.str.size()+rel_added_size < _abs_state.str.size()+2
      //   _abs_state.str.size()+2 > _rel_state.str.size()+rel_added_size
    } else if (_abs_state.str.size() + 2 <
               _rel_state.str.size() + rel_added_size) {

      // 如果绝对状态更短，则切换到绝对状态
      // 存储公共前缀
      commonbase += _abs_state.str;
      _abs_state.str.clear();
      // 将绝对状态复制到相对状态
      _rel_state = _abs_state;
      _abs_state.switches++;
      rel_op_repeated = false;
    }
    
    // 添加操作符到两个状态
    if (!abs_op_repeated)
      _abs_state.appendOp(abs_op);
    if (!rel_op_repeated)
      _rel_state.appendOp(rel_op);
  } break;
  default:
    std::cerr << "Better not be here!" << std::endl;
  }
}

/**
 * 添加单个坐标值
 * @param v 坐标值
 */
void PathString::State::append(Geom::Coord v) {
  str += ' ';
  appendNumber(v, _precision, _minexp);
}

/**
 * 添加点坐标
 * @param p 点
 */
void PathString::State::append(Geom::Point p) {
  str += ' ';
  appendNumber(p[Geom::X], _precision, _minexp);
  str += ',';
  appendNumber(p[Geom::Y], _precision, _minexp);
}

/**
 * 添加坐标值并返回四舍五入后的值
 * @param v 输入坐标值
 * @param rv 输出四舍五入后的坐标值
 */
void PathString::State::append(Geom::Coord v, Geom::Coord &rv) {
  str += ' ';
  appendNumber(v, rv);
}

/**
 * 添加点坐标并返回四舍五入后的值
 * @param p 输入点
 * @param rp 输出四舍五入后的点
 */
void PathString::State::append(Geom::Point p, Geom::Point &rp) {
  str += ' ';
  appendNumber(p[Geom::X], rp[Geom::X]);
  str += ',';
  appendNumber(p[Geom::Y], rp[Geom::Y]);
}

// 注意：以下appendRelativeCoord函数在表示差值所需的总位数
// 超过double精度时不会完全精确。虽然这种情况不太可能发生，
// 但如果确实发生了，不精确的值不太可能被选择
// （因为它可能比绝对值长很多）

// 注意：这假设v和r已经被四舍五入（包括如果它们<10^minexp则刷新为零）
/**
 * 添加相对坐标值
 * @param v 当前值
 * @param r 参考值
 */
void PathString::State::appendRelativeCoord(Geom::Coord v, Geom::Coord r) {
  int const minexp = _minexp - _precision + 1;
  // 计算最小（绝对值意义上）数字的最后一个有效数字之后的位置
  int const digitsEnd =
      (int)floor(log10(std::min(fabs(v), fabs(r)))) -
      _precision;
  // 计算四舍五入的差值
  double const roundeddiff = floor((v - r) * pow(10., -digitsEnd - 1) + .5);
  // 计算roundeddiff中的位数
  int const numDigits = (int)floor(log10(fabs(roundeddiff))) +
                        1;
  
  // 根据情况处理不同的数值
  if (r == 0) {
    // 如果参考值为0，直接添加当前值
    appendNumber(v, _precision, minexp);
  } else if (v == 0) {
    // 如果当前值为0，添加负的参考值
    appendNumber(-r, _precision, minexp);
  } else if (numDigits > 0) {
    // 如果有有效位数，添加差值
    appendNumber(v - r, numDigits, minexp);
  } else {
    // 这假设输入数字已经被四舍五入到'precision'位
    str += '0';
  }
}

/**
 * 添加相对点坐标
 * @param p 当前点
 * @param r 参考点
 */
void PathString::State::appendRelative(Geom::Point p, Geom::Point r) {
  str += ' ';
  appendRelativeCoord(p[Geom::X], r[Geom::X]);
  str += ',';
  appendRelativeCoord(p[Geom::Y], r[Geom::Y]);
}

/**
 * 添加相对坐标值
 * @param v 当前值
 * @param r 参考值
 */
void PathString::State::appendRelative(Geom::Coord v, Geom::Coord r) {
  str += ' ';
  appendRelativeCoord(v, r);
}

/**
 * 添加数字到字符串中
 * @param v 数值
 * @param precision 精度
 * @param minexp 最小指数
 */
void PathString::State::appendNumber(double v, int precision, int minexp) {
  str.append(sp_svg_number_write_de(v, precision, minexp));
}

/**
 * 添加数字到字符串中并返回四舍五入后的值
 * @param v 输入数值
 * @param rv 输出四舍五入后的数值
 */
void PathString::State::appendNumber(double v, double &rv) {
  size_t const oldsize = str.size();
  appendNumber(v, _precision, _minexp);
  // 稍微邪恶的做法，我知道（但std::string应该将其数据存储在一个大的内存块中，所以...）
  char *begin_of_num = const_cast<char *>(
      str.data() +
      oldsize);
  sp_svg_number_read_d(begin_of_num, &rv);
}

} // namespace SVG
} // namespace Inkscape
