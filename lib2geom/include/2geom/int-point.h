/**
 *  \file
 *  \brief Cartesian point / 2D vector with integer coordinates
 *//*
 * Copyright 2011 Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#ifndef LIB2GEOM_SEEN_INT_POINT_H
#define LIB2GEOM_SEEN_INT_POINT_H

#include <cassert>
#include <tuple>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <2geom/coord.h>

namespace Geom {

/**
 * @brief Two-dimensional point with integer coordinates.
 *
 * This class is an exact equivalent of Point, except it stores integer coordinates.
 * Integer points are useful in contexts related to rasterized graphics, for example
 * for bounding boxes when rendering SVG.
 *
 * @see Point
 * @ingroup Primitives */
class IntPoint
    : boost::additive< IntPoint
    , boost::totally_ordered< IntPoint
    , boost::multiplicative< IntPoint, IntCoord
    , boost::multiplicative< IntPoint
    >>>> // base class chaining, see documentation for Boost.Operator
{
    IntCoord _pt[2] = { 0, 0 };
public:
    /// @name Create integer points
    /// @{
    /** Construct a point at the origin. */
    constexpr IntPoint() = default;
    /** Construct a point from its coordinates. */
    constexpr IntPoint(IntCoord x, IntCoord y)
        : _pt{ x, y }
    {}
    /// @}

    /// @name Access the coordinates of a point
    /// @{
    IntCoord operator[](unsigned i) const { assert(i < 2); return _pt[i]; }
    IntCoord &operator[](unsigned i) { assert(i < 2); return _pt[i]; }
    constexpr IntCoord operator[](Dim2 d) const { return _pt[d]; }
    constexpr IntCoord &operator[](Dim2 d) { return _pt[d]; }

    constexpr IntCoord x() const noexcept { return _pt[X]; }
    constexpr IntCoord &x() noexcept { return _pt[X]; }
    constexpr IntCoord y() const noexcept { return _pt[Y]; }
    constexpr IntCoord &y() noexcept { return _pt[Y]; }

    // Structured binding support
    template <size_t I> constexpr IntCoord get() const { static_assert(I < 2); return _pt[I]; }
    template <size_t I> constexpr IntCoord &get() { static_assert(I < 2); return _pt[I]; }
    /// @}

    /// @name Vector-like operations
    /// @{
    constexpr IntCoord lengthSq() const { return _pt[X] * _pt[X] + _pt[Y] * _pt[Y]; }
    /** @brief Return a point like this point but rotated -90 degrees.
     * If the y axis grows downwards and the x axis grows to the
     * right, then this is 90 degrees counter-clockwise. */
    constexpr IntPoint ccw() const {
        return IntPoint(_pt[Y], -_pt[X]);
    }
    /** @brief Return a point like this point but rotated +90 degrees.
     * If the y axis grows downwards and the x axis grows to the
     * right, then this is 90 degrees clockwise. */
    constexpr IntPoint cw() const {
        return IntPoint(-_pt[Y], _pt[X]);
    }
    /// @}

    /// @name Vector-like arithmetic operations
    /// @{
    constexpr IntPoint operator-() const {
        return IntPoint(-_pt[X], -_pt[Y]);
    }
    constexpr IntPoint &operator+=(IntPoint const &o) {
        _pt[X] += o._pt[X];
        _pt[Y] += o._pt[Y];
        return *this;
    }
    constexpr IntPoint &operator-=(IntPoint const &o) {
        _pt[X] -= o._pt[X];
        _pt[Y] -= o._pt[Y];
        return *this;
    }
    constexpr IntPoint &operator*=(IntPoint const &o) {
        _pt[X] *= o._pt[X];
        _pt[Y] *= o._pt[Y];
        return *this;
    }
    constexpr IntPoint &operator*=(IntCoord o) {
        _pt[X] *= o;
        _pt[Y] *= o;
        return *this;
    }
    constexpr IntPoint &operator/=(IntPoint const &o) {
        _pt[X] /= o._pt[X];
        _pt[Y] /= o._pt[Y];
        return *this;
    }
    constexpr IntPoint &operator/=(IntCoord o) {
        _pt[X] /= o;
        _pt[Y] /= o;
        return *this;
    }
    /// @}
    
    /// @name Various utilities
    /// @{
    /** @brief Equality operator. */
    constexpr bool operator==(IntPoint const &p) const {
        return _pt[X] == p[X] && _pt[Y] == p[Y];
    }
    /** @brief Lexicographical ordering for points.
     * Y coordinate is regarded as more significant. When sorting according to this
     * ordering, the points will be sorted according to the Y coordinate, and within
     * points with the same Y coordinate according to the X coordinate. */
    constexpr bool operator<(IntPoint const &p) const {
        return _pt[Y] < p[Y] || (_pt[Y] == p[Y] && _pt[X] < p[X]);
    }
    /// @}
    
    /** @brief Lexicographical ordering functor.
     * @param d The more significant dimension */
    template <Dim2 d> struct LexLess;
    /** @brief Lexicographical ordering functor.
     * @param d The more significant dimension */
    template <Dim2 d> struct LexGreater;
    /** @brief Lexicographical ordering functor with runtime dimension. */
    struct LexLessRt {
        LexLessRt(Dim2 d) : dim(d) {}
        inline bool operator()(IntPoint const &a, IntPoint const &b) const;
    private:
        Dim2 dim;
    };
    /** @brief Lexicographical ordering functor with runtime dimension. */
    struct LexGreaterRt {
        LexGreaterRt(Dim2 d) : dim(d) {}
        inline bool operator()(IntPoint const &a, IntPoint const &b) const;
    private:
        Dim2 dim;
    };
};

template<> struct IntPoint::LexLess<X> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[X] < b[X] || (a[X] == b[X] && a[Y] < b[Y]);
    }
};
template<> struct IntPoint::LexLess<Y> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[Y] < b[Y] || (a[Y] == b[Y] && a[X] < b[X]);
    }
};
template<> struct IntPoint::LexGreater<X> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[X] > b[X] || (a[X] == b[X] && a[Y] > b[Y]);
    }
};
template<> struct IntPoint::LexGreater<Y> {
    bool operator()(IntPoint const &a, IntPoint const &b) const {
        return a[Y] > b[Y] || (a[Y] == b[Y] && a[X] > b[X]);
    }
};
inline bool IntPoint::LexLessRt::operator()(IntPoint const &a, IntPoint const &b) const {
    return dim ? IntPoint::LexLess<Y>()(a, b) : IntPoint::LexLess<X>()(a, b);
}
inline bool IntPoint::LexGreaterRt::operator()(IntPoint const &a, IntPoint const &b) const {
    return dim ? IntPoint::LexGreater<Y>()(a, b) : IntPoint::LexGreater<X>()(a, b);
}

} // namespace Geom

// Structured binding support
template <> struct std::tuple_size<Geom::IntPoint> : std::integral_constant<size_t, 2> {};
template <size_t I> struct std::tuple_element<I, Geom::IntPoint> { using type = Geom::IntCoord; };

// Hash support
template <> struct std::hash<Geom::IntPoint>
{
    size_t operator()(Geom::IntPoint const &p) const noexcept {
        size_t hash = 0;
        boost::hash_combine(hash, p.x());
        boost::hash_combine(hash, p.y());
        return hash;
    }
};

#endif // LIB2GEOM_SEEN_INT_POINT_H

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
