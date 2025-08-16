/**
 * @file
 * @brief Interval arithmetic based intersections
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
#ifndef LIB2GEOM_SEEN_GEOMETRIC_INTERSECTION_H
#define LIB2GEOM_SEEN_GEOMETRIC_INTERSECTION_H

#include <2geom/arithmetic-interval.h>
#include <2geom/bezier-curve.h>
#include <2geom/coord.h>
#include <vector>

namespace Geom {

/**
 * @brief Represents intersections between geometric objects.
 *
 * This structure is used to represent intersections. In the case of a single point intersection,
 * a single `GeometricIntersection` object is returned with type `GeometricIntersection::Type::POINT`.
 *
 * In the case of an overlap, two `GeometricIntersection` objects are returned:
 * - The first represents the start of the overlap interval and has type
 * `GeometricIntersection::Type::OVERLAP_START_POINT`.
 * - The second represents the end of the overlap interval and has type
 * `GeometricIntersection::Type::OVERLAP_END_POINT`.
 */
struct GeometricIntersection {
    enum class Type { POINT, OVERLAP_END_POINT };

    Type type;

    /// Represents time value on the first curve
    Coord t;

    /// Represents time value on the second curve
    Coord s;
};

std::vector<GeometricIntersection> intersect(LineSegment const &seg1, LineSegment const &seg2,
                                             double epsilon = EPSILON);


} // namespace Geom

#endif // LIB2GEOM_SEEN_GEOMETRIC_INTERSECTION_H