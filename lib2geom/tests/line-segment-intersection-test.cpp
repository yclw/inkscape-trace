/**
 * @file
 * @brief Unit tests for LineSegment intersection
 * Uses the Google Testing Framework
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

#include <2geom/arithmetic-interval.h>
#include <2geom/arithmetic-interval-utils.h>
#include <2geom/bezier-curve.h>
#include <2geom/geometric-intersection.h>
#include <glib.h>
#include <gtest/gtest.h>

namespace Geom {

bool isValidIntersection(const LineSegment &seg1, const LineSegment &seg2,
                         const std::vector<GeometricIntersection> &result, double epsilon)
{
    for (const auto &inter : result) {
        if (inter.t < 0.0 || inter.t > 1.0 || inter.s < 0.0 || inter.s > 1.0) {
            return false;
        }

        const auto p1 = seg1.pointAt(inter.t);
        const auto p2 = seg2.pointAt(inter.s);

        if (L2(p1 - p2) > epsilon) {
            return false;
        }
    }
    return true;
}

// non-parallel intersection

TEST(LineSegmentIntersectionTest, PointIntersection)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(5, 10), Point(5, -10));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 0.5);
    EXPECT_EQ(result[0].s, 0.5);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, IntersectionNearMiss)
{
    const Geom::LineSegment seg1(Geom::Point(0, 0), Geom::Point(10, 0));
    const Geom::LineSegment seg2(Geom::Point(5, 10), Geom::Point(5, EPSILON * 0.5));

    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 0.5);
    EXPECT_EQ(result[0].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, IntersectionNearMissReverse)
{
    const Geom::LineSegment seg1(Geom::Point(0, 0), Geom::Point(10, 0));
    const Geom::LineSegment seg2(Geom::Point(5, EPSILON * 0.5), Geom::Point(5, 10));

    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 0.5);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CommonEndPoint)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(10, 0), Point(20, 5));

    const auto result = intersect(seg1, seg2, EPSILON);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, AlmostCommonEndPoint)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(10 + EPSILON, 0), Point(20, 5));

    const auto result = intersect(seg1, seg2, EPSILON);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, EndPointsGreaterThanEpsilon)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(10 + EPSILON * 1.5, 0), Point(20, 5));

    const auto result = intersect(seg1, seg2, EPSILON);
    ASSERT_EQ(result.size(), 0);
}

// parallel segments

TEST(LineSegmentIntersectionTest, VerySmallOverlap)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(10 - EPSILON, 0), Point(20, 0));

    const auto result = intersect(seg1, seg2, EPSILON);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, ParallelOutsideEpsilon)
{
    const LineSegment seg1(Point(0, 0), Point(1, 1));
    const LineSegment seg2(Point(1, 0), Point(2, 1));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, ParallelWithinEpsilon)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(0, EPSILON * 0.5), Point(5.0, EPSILON * 0.5));

    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 0.5);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, ParallelBarelyOutsideEpsilon)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(0, EPSILON * 1.5), Point(5.0, EPSILON * 1.5));

    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, ParallelWithinEpsilonReversed)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(5.0, EPSILON * 0.5), Point(0.0, EPSILON * 0.5));

    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 1.0);
    EXPECT_EQ(result[1].t, 0.5);
    EXPECT_EQ(result[1].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearMicroOffset)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(2, EPSILON * 0.999), Point(8, EPSILON * 0.999));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.2);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 0.8);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearJustOutsideMicroOffset)
{
    const LineSegment seg1(Point(0, 0), Point(10, 0));
    const LineSegment seg2(Point(2, EPSILON * 1.001), Point(8, EPSILON * 1.001));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, CollinearTotalOverlap)
{
    const LineSegment seg1(Point(0, 0), Point(4, 0));
    const LineSegment seg2(Point(1, 0), Point(3, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.25);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 0.75);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearLeftOverlap)
{
    const LineSegment seg1(Point(2, 0), Point(4, 0));
    const LineSegment seg2(Point(1, 0), Point(3, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.5);
    EXPECT_EQ(result[1].t, 0.5);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearLeftOverlapReversed)
{
    const LineSegment seg1(Point(4, 0), Point(2, 0));
    const LineSegment seg2(Point(3, 0), Point(1, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.5);
    EXPECT_EQ(result[1].t, 0.5);
    EXPECT_EQ(result[1].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearRightOverlap)
{
    const LineSegment seg1(Point(0, 0), Point(2, 0));
    const LineSegment seg2(Point(1, 0), Point(3, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.5);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 0.5);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, CollinearNoOverlap)
{
    const LineSegment seg1(Point(0, 0), Point(2, 0));
    const LineSegment seg2(Point(3, 0), Point(5, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

// Degenerate segments

TEST(LineSegmentIntersectionTest, DegenerateSegment)
{
    const LineSegment seg1(Point(1, 1), Point(1, 1));
    const LineSegment seg2(Point(0, 0), Point(2, 2));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.5);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 0.5);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, DegenerateSegmentNoIntersection)
{
    const LineSegment seg1(Point(3, 3), Point(3, 3));
    const LineSegment seg2(Point(0, 0), Point(2, 2));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, DegenerateSegmentsIntersection)
{
    const LineSegment seg1(Point(3, 3), Point(3, 3));
    const LineSegment seg2(Point(3, 3), Point(3, 3));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, DegenerateSegmentsNearIntersection)
{
    const LineSegment seg1(Point(EPSILON, EPSILON), Point(EPSILON, EPSILON));
    const LineSegment seg2(Point(EPSILON + EPSILON, EPSILON), Point(EPSILON + EPSILON, EPSILON));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, NearDegenerateIntersection)
{
    const LineSegment seg1(Point(0, EPSILON * 0.5), Point(0, EPSILON * 1.0));
    const LineSegment seg2(Point(0, 0), Point(10, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, DegenerateMidPointCheck)
{
    const LineSegment seg1(Point(EPSILON * 0.5, 0), Point(EPSILON * 1.5, 0));
    const LineSegment seg2(Point(0, 0), Point(1, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_DOUBLE_EQ(result[0].s, EPSILON);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_DOUBLE_EQ(result[1].s, EPSILON);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, DegenerateNearMissIntersection)
{
    const LineSegment seg1(Point(EPSILON * 0.5, EPSILON * 1.01), Point(EPSILON * 1.5, EPSILON * 1.01));
    const LineSegment seg2(Point(0, 0), Point(1, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, AlmostDegenerateSegment)
{
    const LineSegment seg1(Point(0, 0), Point(EPSILON * 1.01, 0));
    const LineSegment seg2(Point(0, 0), Point(1, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_DOUBLE_EQ(result[0].t, 0.0);
    EXPECT_DOUBLE_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_DOUBLE_EQ(result[1].s, EPSILON * 1.01);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

// Random case

TEST(LineSegmentIntersectionTest, IdenticalSegments)
{
    const LineSegment seg1(Point(0, 0), Point(2, 0));
    const LineSegment seg2(Point(0, 0), Point(2, 0));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
    EXPECT_EQ(result[0].t, 0.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_EQ(result[1].t, 1.0);
    EXPECT_EQ(result[1].s, 1.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, ExtremeLargeCoordinates)
{
    const LineSegment seg1(Point(0, 0), Point(2 * 1e50, 2 * 1e50));
    const LineSegment seg2(Point(2 * 1e50, 2 * 1e50), Point(4 * 1e50, 4 * 1e50));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

TEST(LineSegmentIntersectionTest, ExtremeSlopeNoIntersection)
{
    const LineSegment seg1(Point(0, 0), Point(1e-10, 1e10));
    const LineSegment seg2(Point(1e-10 + EPSILON * 1.5, 0), Point(1e-10 + EPSILON * 1.5, 1e10));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 0);
}

TEST(LineSegmentIntersectionTest, NegativeLargeCoordinates)
{
    const LineSegment seg1(Point(0, 0), Point(-2 * 1e50, -2 * 1e50));
    const LineSegment seg2(Point(-2 * 1e50, -2 * 1e50), Point(-4 * 1e50, -4 * 1e50));
    const auto result = intersect(seg1, seg2, EPSILON);

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
    EXPECT_EQ(result[0].t, 1.0);
    EXPECT_EQ(result[0].s, 0.0);
    EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
}

// Randomized test cases

TEST(LineSegmentIntersectionTest, RandomNormalRange)
{
    int num_intersections = 0;
    int num_total = 0;
    g_random_set_seed(0xB747A380);
    for (num_intersections; num_intersections < 1000;) {
        const Point p1(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));
        const Point p2(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));
        const Point p3(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));
        const Point p4(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));

        const LineSegment seg1(p1, p2);
        const LineSegment seg2(p3, p4);
        const auto result = intersect(seg1, seg2, EPSILON);

        EXPECT_LE(result.size(), 2);
        if(result.size() > 0)
        {
            num_intersections++;
            EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
        }
    }
}

TEST(LineSegmentIntersectionTest, RandomNearDegenerate)
{
    g_random_set_seed(0xB747A380);
    for (int i = 0; i < 1000; ++i) {
        const Point p1(g_random_double_range(-10.0, 10.0), g_random_double_range(-10.0, 10.0));
        const Point p2 = p1 + Point(g_random_double_range(-EPSILON, EPSILON), g_random_double_range(-EPSILON, EPSILON));
        const Point p3(g_random_double_range(-10.0, 10.0), g_random_double_range(-10.0, 10.0));
        const Point p4(g_random_double_range(-10.0, 10.0), g_random_double_range(-10.0, 10.0));

        const LineSegment seg1(p1, p2);
        const LineSegment seg2(p3, p4);
        const auto result = intersect(seg1, seg2, EPSILON);

        EXPECT_LE(result.size(), 2);
        EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
    }
}

TEST(LineSegmentIntersectionTest, RandomCollinearOrParallel)
{
    int num_intersections = 0;
    g_random_set_seed(0xB747A380);
    for (num_intersections; num_intersections < 1000;) {
        const double x = g_random_double_range(-1000.0, 1000.0);
        const double y = g_random_double_range(-1000.0, 1000.0);
        const double len1 = g_random_double_range(0.0, 1000.0);
        const double len2 = g_random_double_range(0.0, 1000.0);
        const double offset = g_random_double_range(-2 * EPSILON, 2 * EPSILON);

        const LineSegment seg1(Point(x, y), Point(x + len1, y));
        const LineSegment seg2(Point(x, y + offset), Point(x + len2, y + offset));
        const auto result = intersect(seg1, seg2, EPSILON);

        if (std::abs(offset) <= EPSILON) {
            ASSERT_EQ(result.size(), 2);
            EXPECT_EQ(result[0].type, GeometricIntersection::Type::OVERLAP_END_POINT);
            EXPECT_EQ(result[1].type, GeometricIntersection::Type::OVERLAP_END_POINT);
            EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
            num_intersections++;
        } else {
            ASSERT_EQ(result.size(), 0);
        }
    }
}

TEST(LineSegmentIntersectionTest, RandomCommonEndpoint)
{
    g_random_set_seed(0xB747A380);
    for (int i = 0; i < 1000; ++i) {
        const Point common(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));
        const Point offset(g_random_double_range(-EPSILON * 0.7, EPSILON * 0.7),
                           g_random_double_range(-EPSILON * 0.7, EPSILON * 0.7));

        const LineSegment seg1(common,
                               Point(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0)));
        const LineSegment seg2(common + offset,
                               Point(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0)));
        const auto result = intersect(seg1, seg2, EPSILON);

        ASSERT_EQ(result.size(), 1);
        EXPECT_EQ(result[0].type, GeometricIntersection::Type::POINT);
        EXPECT_NEAR(result[0].t, 0.0, EPSILON);
        EXPECT_NEAR(result[0].s, 0.0, EPSILON);
        EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
    }
}

TEST(LineSegmentIntersectionTest, RandomVerySmallOverlap)
{
    g_random_set_seed(0xB747A380);
    for (int i = 0; i < 1000; ++i) {
        const double start = g_random_double_range(-1000.0, 1000.0);
        const double len1 = g_random_double_range(0.1, 100.0);
        const double epsilon_overlap = EPSILON * g_random_double_range(-1.0, 1.0);

        const Point p1(start, 0.0);
        const Point p2(start + len1, 0.0);

        const Point q1(start + len1 - epsilon_overlap, 0.0);
        const Point q2(q1.x() + g_random_double_range(0.1, 100.0), 0.0);

        const LineSegment seg1(p1, p2);
        const LineSegment seg2(q1, q2);
        const auto result = intersect(seg1, seg2, EPSILON);

        ASSERT_EQ(result.size(), 1);
        EXPECT_TRUE(isValidIntersection(seg1, seg2, result, EPSILON));
    }
}

} // namespace Geom