/**
 *  @file
 *  @brief Closed interval of generic values
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

#ifndef LIB2GEOM_SEEN_GENERIC_INTERVAL_H
#define LIB2GEOM_SEEN_GENERIC_INTERVAL_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <tuple>
#include <boost/functional/hash.hpp>
#include <2geom/coord.h>

namespace Geom {

template <typename C>
class GenericOptInterval;

/**
 * @brief A range of numbers which is never empty.
 * @ingroup Primitives
 */
template <typename C>
class GenericInterval
    : CoordTraits<C>::IntervalOps
{
    using CInterval = typename CoordTraits<C>::IntervalType;
    using Self = GenericInterval<C>;
protected:
    C _b[2] = { 0, 0 };
public:
    /// @name Create intervals.
    /// @{
    /** @brief Create an interval that contains only zero. */
    constexpr GenericInterval() = default;
    /** @brief Create an interval that contains a single point. */
    explicit constexpr GenericInterval(C u) { _b[0] = _b[1] = u; }
    /** @brief Create an interval that contains all points between @c u and @c v. */
    constexpr GenericInterval(C u, C v) {
        if (u <= v) {
            _b[0] = u; _b[1] = v;
        } else {
            _b[0] = v; _b[1] = u;
        }
    }

    /** @brief Create an interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to C. The given range
     * must not be empty. For potentially empty ranges, see GenericOptInterval.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end). */
    template <typename InputIterator>
    static CInterval from_range(InputIterator start, InputIterator end) {
        assert(start != end);
        CInterval result(*start++);
        for (; start != end; ++start) result.expandTo(*start);
        return result;
    }
    /** @brief Create an interval from a C-style array of values it should contain. */
    static CInterval from_array(C const *c, unsigned n) {
        return from_range(c, c + n);
    }
    /// @}

    /// @name Inspect contained values.
    /// @{
    constexpr C min() const { return _b[0]; }
    constexpr C max() const { return _b[1]; }
    constexpr C extent() const { return max() - min(); }
    constexpr C middle() const { return (max() + min()) / 2; }
    constexpr bool isSingular() const { return min() == max(); }
    C operator[](unsigned i) const { assert(i < 2); return _b[i]; }
    constexpr C operator[](Dim2 d) const { return _b[d]; }
    constexpr C clamp(C val) const {
        return std::clamp(val, min(), max());
    }
    /// Return the closer end of the interval.
    C nearestEnd(C val) const {
        C dmin = std::abs(val - min()), dmax = std::abs(val - max());
        return dmin <= dmax ? min() : max();
    }
    // Structured binding support
    template <size_t I> constexpr C get() const { static_assert(I < 2); return _b[I]; }
    /// @}

    /// @name Test coordinates and other intervals for inclusion.
    /// @{
    /** @brief Check whether the interval includes this number. */
    constexpr bool contains(C val) const {
        return min() <= val && val <= max();
    }
    /** @brief Check whether the interval includes the given interval. */
    constexpr bool contains(CInterval const &val) const {
        return min() <= val.min() && val.max() <= max();
    }
    /** @brief Check whether the intervals have any common elements. */
    constexpr bool intersects(CInterval const &val) const {
        return contains(val.min()) || contains(val.max()) || val.contains(*this);
    }
    /// @}

    /// @name Modify the interval.
    /// @{
    //TODO: NaN handleage for the next two?
    /** @brief Set the lower boundary of the interval.
     * When the given number is larger than the interval's largest element,
     * it will be reduced to the single number @c val. */
    constexpr void setMin(C val) {
        if (val > _b[1]) {
            _b[0] = _b[1] = val;
        } else {
            _b[0] = val;
        }
    }
    /** @brief Set the upper boundary of the interval.
     * When the given number is smaller than the interval's smallest element,
     * it will be reduced to the single number @c val. */
    constexpr void setMax(C val) {
        if (val < _b[0]) {
            _b[1] = _b[0] = val;
        } else {
            _b[1] = val;
        }
    }
    /// Set both ends of the interval simultaneously
    constexpr void setEnds(C a, C b) {
        if (a <= b) {
            _b[0] = a;
            _b[1] = b;
        } else {
            _b[0] = b;
            _b[1] = a;
        }
    }
    /** @brief Extend the interval to include the given number. */
    constexpr void expandTo(C val) {
       if (val < _b[0]) _b[0] = val;
       if (val > _b[1]) _b[1] = val;  // no else, as we want to handle NaN
    }
    /** @brief Expand or shrink the interval in both directions by the given amount.
     * After this method, the interval's length (extent) will be increased by
     * <code>amount * 2</code>. Negative values can be given; they will shrink the interval.
     * Shrinking by a value larger than half the interval's length will create a degenerate
     * interval containing only the midpoint of the original. */
    constexpr void expandBy(C amount) {
        _b[0] -= amount;
        _b[1] += amount;
        if (_b[0] > _b[1]) {
            C halfway = (_b[0] + _b[1]) / 2;
            _b[0] = _b[1] = halfway;
        }
    }
    /** @brief Union the interval with another one.
     * The resulting interval will contain all points of both intervals.
     * It might also contain some points which didn't belong to either - this happens
     * when the intervals did not have any common elements. */
    constexpr void unionWith(CInterval const &a) {
        if (a._b[0] < _b[0]) _b[0] = a._b[0];
        if (a._b[1] > _b[1]) _b[1] = a._b[1];
    }
    /// @}

    /// @name Operators
    /// @{
    //IMPL: OffsetableConcept
    //TODO: rename output_type to something else in the concept
    using output_type = C;
    /** @brief Offset the interval by a specified amount */
    constexpr Self &operator+=(C amnt) {
        _b[0] += amnt; _b[1] += amnt;
        return *this;
    }
    /** @brief Offset the interval by the negation of the specified amount */
    constexpr Self &operator-=(C amnt) {
        _b[0] -= amnt; _b[1] -= amnt;
        return *this;
    }
    
    /** @brief Return an interval mirrored about 0 */
    constexpr Self operator-() const { return { -_b[1], -_b[0] }; }
    // IMPL: AddableConcept
    /** @brief Add two intervals.
     * Sum is defined as the set of points that can be obtained by adding any two values
     * from both operands: \f$S = \{x \in A, y \in B: x + y\}\f$ */
    constexpr Self &operator+=(CInterval const &o) {
        _b[0] += o._b[0];
        _b[1] += o._b[1];
        return *this;
    }
    /** @brief Subtract two intervals.
     * Difference is defined as the set of points that can be obtained by subtracting
     * any value from the second operand from any value from the first operand:
     * \f$S = \{x \in A, y \in B: x - y\}\f$ */
    constexpr Self &operator-=(CInterval const &o) {
        // equal to *this += -o
        _b[0] -= o._b[1];
        _b[1] -= o._b[0];
        return *this;
    }
    /** @brief Union two intervals.
     * Note that the intersection-and-assignment operator is not defined,
     * because the result of an intersection can be empty, while Interval cannot. */
    constexpr Self &operator|=(CInterval const &o) {
        unionWith(o);
        return *this;
    }
    /** @brief Test for interval equality. */
    constexpr bool operator==(CInterval const &other) const {
        return min() == other.min() && max() == other.max();
    }
    /// @}
};

/** @brief Union two intervals
 * @relates GenericInterval */
template <typename C>
inline GenericInterval<C> unify(GenericInterval<C> const &a, GenericInterval<C> const &b) {
    return a | b;
}

/**
 * @brief A range of numbers that can be empty.
 * @ingroup Primitives
 */
template <typename C>
class GenericOptInterval
    : public std::optional<typename CoordTraits<C>::IntervalType>
    , boost::orable< GenericOptInterval<C>
    , boost::andable< GenericOptInterval<C>
      >>
{
    using CInterval = typename CoordTraits<C>::IntervalType;
    using OptCInterval = typename CoordTraits<C>::OptIntervalType;
    using Base = std::optional<CInterval>;
public:
    /// @name Create optionally empty intervals.
    /// @{
    /** @brief Create an empty interval. */
    constexpr GenericOptInterval() = default;
    /** @brief Wrap an existing interval. */
    constexpr GenericOptInterval(GenericInterval<C> const &a) : Base(CInterval(a)) {}
    /** @brief Create an interval containing a single point. */
    constexpr GenericOptInterval(C u) : Base(CInterval(u)) {}
    /** @brief Create an interval containing a range of numbers. */
    constexpr GenericOptInterval(C u, C v) : Base(CInterval(u, v)) {}

    /** @brief Create a possibly empty interval containing a range of values.
     * The resulting interval will contain all values from the given range.
     * The return type of iterators must be convertible to C. The given range
     * may be empty.
     * @param start Beginning of the range
     * @param end   End of the range
     * @return Interval that contains all values from [start, end), or nothing if the range
     *         is empty. */
    template <typename InputIterator>
    static GenericOptInterval<C> from_range(InputIterator start, InputIterator end) {
        if (start == end) {
            return {};
        }
        return CInterval::from_range(start, end);
    }
    /// @}

    /** @brief Check whether this interval is empty. */
    constexpr bool empty() const { return !*this; }

    /** @brief Union with another interval, gracefully handling empty ones. */
    constexpr void unionWith(GenericOptInterval<C> const &a) {
        if (a) {
            if (*this) { // check that we are not empty
                (*this)->unionWith(*a);
            } else {
                *this = *a;
            }
        }
    }
    constexpr void intersectWith(GenericOptInterval<C> const &o) {
        if (o && *this) {
            C u = std::max((*this)->min(), o->min());
            C v = std::min((*this)->max(), o->max());
            if (u <= v) {
                *this = CInterval(u, v);
                return;
            }
        }
        *this = {};
    }
    constexpr GenericOptInterval<C> &operator|=(OptCInterval const &o) {
        unionWith(o);
        return *this;
    }
    constexpr GenericOptInterval<C> &operator&=(OptCInterval const &o) {
        intersectWith(o);
        return *this;
    }

    // The equality operators inherited from std::optional don't work with derived types, because
    // the template overload ignores that the derived type is also an optional. It would result in
    // `GenericInterval() != GenericInterval()` being true.
    template <typename U, typename = std::enable_if_t<std::is_base_of_v<Base, U>>>
    constexpr bool operator==(U const &other) const {
        return static_cast<Base const &>(*this) == static_cast<Base const &>(other);
    }
    template <typename U, typename = std::enable_if_t<std::is_base_of_v<Base, U>>>
    constexpr bool operator!=(U const &other) const {
        return static_cast<Base const &>(*this) != static_cast<Base const &>(other);
    }
};

/** @brief Intersect two intervals and return a possibly empty range of numbers
 * @relates GenericOptInterval */
template <typename C>
inline GenericOptInterval<C> intersect(GenericInterval<C> const &a, GenericInterval<C> const &b) {
    return GenericOptInterval<C>(a) & GenericOptInterval<C>(b);
}
/** @brief Intersect two intervals and return a possibly empty range of numbers
 * @relates GenericOptInterval */
template <typename C>
inline GenericOptInterval<C> operator&(GenericInterval<C> const &a, GenericInterval<C> const &b) {
    return GenericOptInterval<C>(a) & GenericOptInterval<C>(b);
}

template <typename C>
inline std::ostream &operator<<(std::ostream &out,
                                GenericInterval<C> const &I) {
    return out << "Interval(" << I.min() << ", " << I.max() << ")";
}

template <typename C>
inline std::ostream &operator<<(std::ostream &out,
                                GenericOptInterval<C> const &I) {
    return I ? (out << *I) : (out << "Interval (empty)");
}

} // namespace Geom

// Structured binding support
template <typename C> struct std::tuple_size<Geom::GenericInterval<C>> : std::integral_constant<size_t, 2> {};
template <size_t I, typename C> struct std::tuple_element<I, Geom::GenericInterval<C>> { using type = C; };

// Hash support
template <typename C> struct std::hash<Geom::GenericInterval<C>>
{
    size_t operator()(Geom::GenericInterval<C> const &a) const noexcept {
        size_t hash = 0;
        boost::hash_combine(hash, a.min());
        boost::hash_combine(hash, a.max());
        return hash;
    }
};

#endif // LIB2GEOM_SEEN_GENERIC_INTERVAL_H

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
