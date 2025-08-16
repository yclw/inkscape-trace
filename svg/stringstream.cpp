// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include "svg/stringstream.h"
#include "svg/strip-trailing-zeros.h"
// #include "preferences.h" // 移除 preferences 依赖
#include <2geom/point.h>

Inkscape::SVGOStringStream::SVGOStringStream()
{
    /* These two are probably unnecessary now that we provide our own operator<< for float and
     * double. */
    ostr.imbue(std::locale::classic());
    ostr.setf(std::ios::showpoint);

    /* 使用默认精度值替代 preferences 依赖 */
    ostr.precision(8); // 默认精度为8
}

Inkscape::SVGOStringStream &
Inkscape::SVGOStringStream::operator<<(double d)
{
    auto &os = *this;

    /* Try as integer first. */
    {
        int const n = int(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s.flags(os.setf(std::ios::showpoint));
    s.precision(os.precision());
    s << d;
    os << strip_trailing_zeros(s.str());
    return os;
}

Inkscape::SVGOStringStream &
Inkscape::SVGOStringStream::operator<<(Geom::Point const & p)
{
    auto &os = *this;
    os << p[0] << ',' << p[1];
    return os;
}

Inkscape::SVGIStringStream::SVGIStringStream():std::istringstream()
{
    this->imbue(std::locale::classic());
    this->setf(std::ios::showpoint);

    /* 使用默认精度值替代 preferences 依赖 */
    this->precision(8); // 默认精度为8
}

Inkscape::SVGIStringStream::SVGIStringStream(const std::string& str):std::istringstream(str)
{
    this->imbue(std::locale::classic());
    this->setf(std::ios::showpoint);

    /* 使用默认精度值替代 preferences 依赖 */
    this->precision(8); // 默认精度为8
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
