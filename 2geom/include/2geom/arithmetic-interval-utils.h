/**
 * @file
 * @brief Utility functions for arithmetic-interval.h
 *//*
 * Copyright 2025 Muhammad Rafay Irfan <rafay119muhammad@gmail.com>
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
 *
 */

#ifndef LIB2GEOM_SEEN_ARITHMETIC_INTERVAL_UTILS_H
#define LIB2GEOM_SEEN_ARITHMETIC_INTERVAL_UTILS_H

#include <2geom/arithmetic-interval.h>
#include <boost/numeric/interval.hpp>

namespace Geom {

inline ArithmeticInterval operator+(Coord scalar, const ArithmeticInterval &interval) { return interval + scalar; }

inline ArithmeticInterval operator-(Coord scalar, const ArithmeticInterval &interval)
{
    ArithmeticInterval result(scalar);
    result -= interval;
    return result;
}

inline ArithmeticInterval operator*(Coord scalar, const ArithmeticInterval &interval) { return interval * scalar; }

inline ArithmeticInterval operator/(Coord scalar, const ArithmeticInterval &interval)
{
    ArithmeticInterval result(scalar);
    result /= interval;
    return result;
}

/**
 * @brief Computes the square root of the interval.
 *
 * Returns the interval containing the square roots of all values in the input interval.
 *
 * @param a Input interval.
 * @return ArithmeticInterval Interval containing the square root of values in 'a'.
 */
inline ArithmeticInterval sqrt(const ArithmeticInterval &a)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::sqrt(a.toBoost());
    return result;
}

/**
 * @brief Computes the absolute value of the interval.
 *
 * Returns the interval containing the absolute values of all points in the input interval.
 * For example, abs([-3, 2]) results in [0, 3].
 *
 * @param a Input interval.
 * @return ArithmeticInterval Interval with all values non-negative.
 */
inline ArithmeticInterval abs(const ArithmeticInterval &a)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::abs(a.toBoost());
    return result;
}

/**
 * @brief Computes the pointwise maximum of two intervals.
 *
 * Returns an interval whose lower bound is the maximum of the two lower bounds,
 * and upper bound is the maximum of the two upper bounds.
 *
 * @param a First interval.
 * @param b Second interval.
 * @return ArithmeticInterval Interval representing the pointwise maximum.
 */
inline ArithmeticInterval max(const ArithmeticInterval &a, const ArithmeticInterval &b)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::max(a.toBoost(), b.toBoost());
    return result;
}

/**
 * @brief Computes the pointwise minimum of two intervals.
 *
 * Returns an interval whose lower bound is the minimum of the two lower bounds,
 * and upper bound is the minimum of the two upper bounds.
 *
 * @param a First interval.
 * @param b Second interval.
 * @return ArithmeticInterval Interval representing the pointwise minimum.
 */
inline ArithmeticInterval min(const ArithmeticInterval &a, const ArithmeticInterval &b)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::min(a.toBoost(), b.toBoost());
    return result;
}

/**
 * @brief Computes the pointwise maximum of an interval and a scalar.
 *
 * Returns an interval with each bound being the maximum of the corresponding interval
 * bound and the scalar value.
 *
 * @param a Interval.
 * @param b Scalar value.
 * @return ArithmeticInterval Resulting interval after pointwise max.
 */
inline ArithmeticInterval max(const ArithmeticInterval &a, Coord b)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::max(a.toBoost(), b);
    return result;
}

/**
 * @brief Computes the pointwise minimum of an interval and a scalar.
 *
 * Returns an interval with each bound being the minimum of the corresponding interval
 * bound and the scalar value.
 *
 * @param a Interval.
 * @param b Scalar value.
 * @return ArithmeticInterval Resulting interval after pointwise min.
 */
inline ArithmeticInterval min(const ArithmeticInterval &a, Coord b)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::min(a.toBoost(), b);
    return result;
}

/**
 * @brief Computes the square of the interval.
 *
 * Returns an interval containing the squares of all values in the input interval.
 * For intervals that span zero (e.g. [-2, 3]), the result will start from zero.
 *
 * @param a Input interval.
 * @return ArithmeticInterval Interval representing the square of values in 'a'.
 */
inline ArithmeticInterval square(const ArithmeticInterval &a)
{
    ArithmeticInterval result;
    result._interval = boost::numeric::square(a.toBoost());
    return result;
}

/**
 * @brief Computes the Euclidean (L2) norm of two intervals.
 *
 * Calculates the square root of the sum of squares of two input intervals.
 * Represents the interval form of the Euclidean distance √(dx² + dy²).
 *
 * @param dx Interval representing the x-component.
 * @param dy Interval representing the y-component.
 * @return ArithmeticInterval Interval representing the Euclidean norm.
 */
inline ArithmeticInterval l2(const ArithmeticInterval &dx, const ArithmeticInterval &dy)
{
    return sqrt(square(dx) + square(dy));
}

/**
 * @brief Checks if an interval is approximately zero.
 *
 * Returns true if the interval intersects with [-epsilon, epsilon],
 * i.e., the value is considered small enough to be "zero" within tolerance.
 *
 * @param a Input interval.
 * @param epsilon Tolerence.
 * @return true if 'a' intersects zero within epsilon tolerance, false otherwise.
 */
inline bool is_small(const ArithmeticInterval &a, Coord epsilon = EPSILON)
{
    return a.intersects(ArithmeticInterval(-epsilon, epsilon));
}

} // namespace Geom

#endif // LIB2GEOM_SEEN_ARITHMETIC_INTERVAL_UTILS_H