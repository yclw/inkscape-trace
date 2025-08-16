/**
 * \file
 * \brief  Simple closed interval class
 *//*
 * Copyright 2007 Michael Sloan <mgsloan@gmail.com>
 *
 * Original Rect/Range code by:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Nathan Hurst <njh@mail.csse.monash.edu.au>
 *   bulia byak <buliabyak@users.sf.net>
 *   MenTaLguY <mental@rydia.net>
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
 * in the file COPYING-LGPL-2.1; if not, output to the Free Software
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
 *
 */
#ifndef LIB2GEOM_SEEN_INTERVAL_H
#define LIB2GEOM_SEEN_INTERVAL_H

#include <boost/none.hpp>
#include <boost/operators.hpp>
#include <2geom/coord.h>
#include <2geom/math-utils.h>
#include <2geom/generic-interval.h>
#include <2geom/int-interval.h>

namespace Geom {

/**
 * @brief Range of real numbers that is never empty.
 *
 * Intervals are closed ranges \f$[a, b]\f$, which means they include their endpoints.
 * To use them as open ranges, you can use the interiorContains() methods.
 *
 * @ingroup Primitives
 */
class Interval
    : public GenericInterval<Coord>
{
    using Base = GenericInterval<Coord>;
public:
    /// @name Create intervals.
    /// @{
    /** @brief Create an interval that contains only zero. */
    constexpr Interval() = default;
    /** @brief Create an interval that contains a single point. */
    explicit constexpr Interval(Coord u) : Base(u) {}
    /** @brief Create an interval that contains all points between @c u and @c v. */
    constexpr Interval(Coord u, Coord v) : Base(u, v) {}
    /** @brief Convert from integer interval */
    constexpr Interval(IntInterval const &i) : Base(i.min(), i.max()) {}
    constexpr Interval(Base const &b) : Base(b) {}

    /** @brief Create an interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to Coord. The given range
     * must not be empty. For potentially empty ranges, see OptInterval.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end). */
    template <typename InputIterator>
    static Interval from_range(InputIterator start, InputIterator end) {
        return Base::from_range(start, end);
    }
    /** @brief Create an interval from a C-style array of values it should contain. */
    static Interval from_array(Coord const *c, unsigned n) {
        return Base::from_array(c, n);
    }
    /// @}

    /// @name Inspect contained values.
    /// @{
    /// Check whether both endpoints are finite.
    bool isFinite() const {
        return std::isfinite(min()) && std::isfinite(max());
    }
    /** @brief Map the interval [0,1] onto this one.
     * This method simply performs 1D linear interpolation between endpoints. */
    constexpr Coord valueAt(Coord t) const {
        return lerp(t, min(), max());
    }
    /** @brief Compute a time value that maps to the given value.
     * The supplied value does not need to be in the interval for this method to work. */
    constexpr Coord timeAt(Coord v) const {
        return (v - min()) / extent();
    }
    /// Find closest time in [0,1] that maps to the given value. */
    constexpr Coord nearestTime(Coord v) const {
        if (v <= min()) return 0;
        if (v >= max()) return 1;
        return timeAt(v);
    }
    /// @}

    /// @name Test coordinates and other intervals for inclusion.
    /// @{
    /** @brief Check whether the interior of the interval includes this number.
     * Interior means all numbers in the interval except its ends. */
    constexpr bool interiorContains(Coord val) const { return min() < val && val < max(); }
    /** @brief Check whether the interior of the interval includes the given interval.
     * Interior means all numbers in the interval except its ends. */
    constexpr bool interiorContains(Interval const &val) const { return min() < val.min() && val.max() < max(); }
    /// Check whether the number is contained in the union of the interior and the lower boundary.
    constexpr bool lowerContains(Coord val) const { return min() <= val && val < max(); }
    /// Check whether the given interval is contained in the union of the interior and the lower boundary.
    constexpr bool lowerContains(Interval const &val) const { return min() <= val.min() && val.max() < max(); }
    /// Check whether the number is contained in the union of the interior and the upper boundary.
    constexpr bool upperContains(Coord val) { return min() < val && val <= max(); }
    /// Check whether the given interval is contained in the union of the interior and the upper boundary.
    constexpr bool upperContains(Interval const &val) const { return min() < val.min() && val.max() <= max(); }
    /** @brief Check whether the interiors of the intervals have any common elements.
     * A single point in common is not considered an intersection. */
    constexpr bool interiorIntersects(Interval const &val) const {
        return std::max(min(), val.min()) < std::min(max(), val.max());
    }
    /// @}

    /// @name Operators
    /// @{
    // IMPL: ScalableConcept
    /** @brief Scale an interval */
    constexpr Interval &operator*=(Coord s) {
        using std::swap;
        _b[0] *= s;
        _b[1] *= s;
        if (s < 0) swap(_b[0], _b[1]);
        return *this;
    }
    /** @brief Scale an interval by the inverse of the specified value */
    constexpr Interval &operator/=(Coord s) {
        using std::swap;
        _b[0] /= s;
        _b[1] /= s;
        if (s < 0) swap(_b[0], _b[1]);
        return *this;
    }
    /** @brief Multiply two intervals.
     * Product is defined as the set of points that can be obtained by multiplying
     * any value from the second operand by any value from the first operand:
     * \f$S = \{x \in A, y \in B: x * y\}\f$ */
    constexpr Interval &operator*=(Interval const &o) {
        // TODO implement properly
        Coord mn = min(), mx = max();
        expandTo(mn * o.min());
        expandTo(mn * o.max());
        expandTo(mx * o.min());
        expandTo(mx * o.max());
        return *this;
    }
    constexpr bool operator==(IntInterval const &ii) const {
        return min() == Coord(ii.min()) && max() == Coord(ii.max());
    }
    constexpr bool operator==(Interval const &other) const {
        return Base::operator==(other);
    }
    /// @}

    /// @name Rounding to integer values
    /// @{
    /** @brief Return the smallest integer interval which contains this one. */
    IntInterval roundOutwards() const {
        return IntInterval(floor(min()), ceil(max()));
    }
    /** @brief Return the largest integer interval which is contained in this one. */
    OptIntInterval roundInwards() const {
        IntCoord u = ceil(min()), v = floor(max());
        if (u > v) return {};
        return IntInterval(u, v);
    }
    /// @}
};

/**
 * @brief Range of real numbers that can be empty.
 * @ingroup Primitives
 */
class OptInterval
    : public GenericOptInterval<Coord>
{
    using Base = GenericOptInterval<Coord>;
public:
    using Base::Base;
    using Base::operator==;
    using Base::operator!=;

    constexpr OptInterval(Base const &b) : Base(b) {}

    /** @brief Promote from IntInterval. */
    constexpr OptInterval(IntInterval const &i) : Base(Interval(i)) {}
    /** @brief Promote from OptIntInterval. */
    constexpr OptInterval(OptIntInterval const &i) {
        if (i) *this = Interval(*i);
    }
};

// functions required for Python bindings
inline Interval unify(Interval const &a, Interval const &b) {
    return a | b;
}
inline OptInterval intersect(Interval const &a, Interval const &b) {
    return a & b;
}

} // namespace Geom

// Structured binding support
template <> struct std::tuple_size<Geom::Interval> : std::integral_constant<size_t, 2> {};
template <size_t I> struct std::tuple_element<I, Geom::Interval> { using type = Geom::Coord; };

// Hash support
template <> struct std::hash<Geom::Interval> : std::hash<Geom::GenericInterval<Geom::Coord>> {};

#endif // LIB2GEOM_SEEN_INTERVAL_H

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
