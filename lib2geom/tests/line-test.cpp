/** @file
 * @brief Unit tests for Line and related functions
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2015 Authors
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

#include "testing.h"
#include <iostream>
#include <glib.h>

#include <2geom/line.h>
#include <2geom/affine.h>

using namespace Geom;

TEST(LineTest, VectorAndVersor) {
    Line a(Point(10, 10), Point(-10, 20));
    Line b(Point(10, 10), Point(15, 15));
    EXPECT_EQ(a.vector(), Point(-20, 10));
    EXPECT_EQ(b.vector(), Point(5, 5));
    EXPECT_EQ(a.versor(), a.vector().normalized());
    EXPECT_EQ(b.versor(), b.vector().normalized());
}

TEST(LineTest, AngleBisector) {
    Point o(0,0), a(1,1), b(3,0), c(-4, 0);
    Point d(0.5231, 0.75223);

    // normal
    Line ab1 = make_angle_bisector_line(a + d, o + d, b + d);
    Line ab2 = make_angle_bisector_line(a - d, o - d, b - d);
    EXPECT_FLOAT_EQ(ab1.angle(), Angle::from_degrees(22.5));
    EXPECT_FLOAT_EQ(ab2.angle(), Angle::from_degrees(22.5));

    // half angle
    Line bc1 = make_angle_bisector_line(b + d, o + d, c + d);
    Line bc2 = make_angle_bisector_line(b - d, o - d, c - d);
    EXPECT_FLOAT_EQ(bc1.angle(), Angle::from_degrees(90));
    EXPECT_FLOAT_EQ(bc2.angle(), Angle::from_degrees(90));

    // zero angle
    Line aa1 = make_angle_bisector_line(a + d, o + d, a + d);
    Line aa2 = make_angle_bisector_line(a - d, o - d, a - d);
    EXPECT_FLOAT_EQ(aa1.angle(), Angle::from_degrees(45));
    EXPECT_FLOAT_EQ(aa2.angle(), Angle::from_degrees(45));
}

TEST(LineTest, Equality) {
    Line a(Point(0,0), Point(2,2));
    Line b(Point(2,2), Point(5,5));

    EXPECT_EQ(a, a);
    EXPECT_EQ(b, b);
    EXPECT_EQ(a, b);
}

TEST(LineTest, Reflection) {
    Line a(Point(10, 0), Point(15,5));
    Point pa(10,5), ra(15,0);

    Line b(Point(1,-2), Point(2,0));
    Point pb(5,1), rb(1,3);
    Affine reflecta = a.reflection(), reflectb = b.reflection();

    Point testra = pa * reflecta;
    Point testrb = pb * reflectb;

    constexpr Coord eps{1e-12};
    EXPECT_near(testra[X], ra[X], eps);
    EXPECT_near(testra[Y], ra[Y], eps);
    EXPECT_near(testrb[X], rb[X], eps);
    EXPECT_near(testrb[Y], rb[Y], eps);
}

TEST(LineTest, RotationToZero) {
    Line a(Point(-5,23), Point(15,27));
    Affine mx = a.rotationToZero(X);
    Affine my = a.rotationToZero(Y);

    for (unsigned i = 0; i <= 12; ++i) {
        double t = -1 + 0.25 * i;
        Point p = a.pointAt(t);
        Point rx = p * mx;
        Point ry = p * my;
        //std::cout << rx[X] << " " << ry[Y] << std::endl;
        // unfortunately this is precise only to about 1e-14
        EXPECT_NEAR(rx[X], 0, 1e-14);
        EXPECT_NEAR(ry[Y], 0, 1e-14);
    }
}

TEST(LineTest, Coefficients) {
    std::vector<Line> lines;
    lines.emplace_back(Point(1e3,1e3), Point(1,1));
    //the case below will never work without normalizing the line
    //lines.emplace_back(Point(1e5,1e5), Point(1e-15,0));
    lines.emplace_back(Point(1e5,1e5), Point(1e5,-1e5));
    lines.emplace_back(Point(-3,10), Point(3,10));
    lines.emplace_back(Point(250,333), Point(-72,121));

    for (auto & line : lines) {
        Coord a, b, c, A, B, C;
        line.coefficients(a, b, c);
        /*std::cout << format_coord_nice(a) << " "
                  << format_coord_nice(b) << " "
                  << format_coord_nice(c) << std::endl;*/
        Line k(a, b, c);
        //std::cout << k.initialPoint() << " " << k.finalPoint() << std::endl;
        k.coefficients(A, B, C);
        /*std::cout << format_coord_nice(A) << " "
                  << format_coord_nice(B) << " "
                  << format_coord_nice(C) << std::endl;*/
        EXPECT_DOUBLE_EQ(a, A);
        EXPECT_DOUBLE_EQ(b, B);
        EXPECT_DOUBLE_EQ(c, C);

        for (unsigned j = 0; j <= 10; ++j) {
            double t = j / 10.;
            Point p = line.pointAt(t);
            /*std::cout << t << " " << p << " "
                      << A*p[X] + B*p[Y] + C << " "
                      << A*(p[X]-1) + B*(p[Y]+1) + C << std::endl;*/
            EXPECT_near(A*p[X] + B*p[Y] + C, 0., 2e-11);
            EXPECT_not_near(A*(p[X]-1) + B*(p[Y]+1) + C, 0., 1e-6);
        }
    }
}

TEST(LineTest, Intersection) {
    Line a(Point(0,3), Point(1,2));
    Line b(Point(0,-3), Point(1,-2));
    LineSegment lsa(Point(0,3), Point(1,2));
    LineSegment lsb(Point(0,-3), Point(1,-2));
    LineSegment lsc(Point(3,1), Point(3, -1));

    std::vector<ShapeIntersection> r1, r2, r3;

    r1 = a.intersect(b);
    ASSERT_EQ(r1.size(), 1u);
    EXPECT_EQ(r1[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, b, r1, 1e-15);

    r2 = a.intersect(lsc);
    ASSERT_EQ(r2.size(), 1u);
    EXPECT_EQ(r2[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, lsc, r2, 1e-15);

    r3 = b.intersect(lsc);
    ASSERT_EQ(r3.size(), 1u);
    EXPECT_EQ(r3[0].point(), Point(3,0));
    EXPECT_intersections_valid(a, lsc, r3, 1e-15);

    EXPECT_TRUE(lsa.intersect(lsb).empty());
    EXPECT_TRUE(lsa.intersect(lsc).empty());
    EXPECT_TRUE(lsb.intersect(lsc).empty());
    EXPECT_TRUE(a.intersect(lsb).empty());
    EXPECT_TRUE(b.intersect(lsa).empty());
}

#define RAND10 g_random_double_range(-10.0, 10.0)

/** Ensure that intersections are reported at endpoints of
 *  identical (overlapping) segments (reversed or not).
 */
TEST(LineTest, CoincidingIntersect)
{
    auto const eps = 1e-14;
    auto const check_endpoint_intersections = [=](LineSegment const &s1, LineSegment const &s2) {
        auto xings = s1.intersect(s2, eps);
        ASSERT_EQ(xings.size(), 2);
        EXPECT_TRUE(are_near(xings[0], s1.initialPoint()));
        EXPECT_TRUE(are_near(xings[1], s1.finalPoint()));
        EXPECT_intersections_valid(s1, s2, xings, eps);
    };

    // This fails on 6f7dfdc0317362bf294fed54ad06d14ac14ad809
    check_endpoint_intersections(LineSegment(Point{1, 0}, Point{0, 0}),
                                 LineSegment(Point{0, 0}, Point{1, 0}));

    g_random_set_seed(0x13370AFA);
    for (size_t _ = 0; _ < 10'000; _++) {
        auto const a = Point(RAND10, RAND10);
        auto const b = Point(RAND10, RAND10);
        auto const ab = LineSegment(a, b);
        auto const ba = LineSegment(b, a);
        check_endpoint_intersections(ab, ab);
        check_endpoint_intersections(ba, ba);
        check_endpoint_intersections(ab, ba);
    }
}

/** Test whether at least one intersection is detected
 * when segments have an overlapping portion (due to numerics,
 * parallel segments may not be exactly detected as parallel).
*/
TEST(LineTest, OverlappingIntersect)
{
    g_random_set_seed(0xCAFECAFE);
    // Suppose the segments are [A, B] and [C, D].
    // Case 1: A=C, B halfway between A and D
    for (size_t _ = 0; _ < 10'000; _++)
    {
        auto const a = Point(RAND10, RAND10);
        auto const d = Point(RAND10, RAND10);
        auto const b = middle_point(a, d);
        auto const ab = LineSegment(a, b);
        auto const cd = LineSegment(a, d);
        auto xings = ab.intersect(cd);
        ASSERT_FALSE(xings.empty());
        EXPECT_TRUE(are_near(xings[0], ab.initialPoint()));
        EXPECT_intersections_valid(ab, cd, xings, 1e-12);
    }

    // Case 2: AB wholly contained inside CD
    for (size_t _ = 0; _ < 10'000; _++)
    {
        auto const c = Point(RAND10, RAND10);
        auto const d = Point(RAND10, RAND10);
        auto const a = lerp(0.25, c, d);
        auto const b = lerp(0.75, c, d);
        auto const ab = LineSegment(a, b);
        auto const cd = LineSegment(a, d);
        auto xings = ab.intersect(cd);
        ASSERT_FALSE(xings.empty());
        EXPECT_TRUE(are_near(xings[0], ab.initialPoint()));
        EXPECT_intersections_valid(ab, cd, xings, 1e-12);
    }
}

/** Ensure that intersections are reported when the segments are separated
 * from one another by less than the passed epsilon.
 */
TEST(LineTest, AlmostIntersect)
{
    for (double eps : {1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-3, 1.0}) {
        auto const vertical   = LineSegment(Point(0.0, 0.5 * eps),   Point(0.0, 1.0));
        auto const horizontal = LineSegment(Point(0.5 * eps, 0.0),   Point(1.0, 0.0));
        auto const butt       = LineSegment(Point(0.0, -0.49 * eps),  Point(0.0, -1.0));
        auto const too_far    = LineSegment(Point(0.0, -0.51 * eps), Point(0.0, -1.0));
        auto xings = vertical.intersect(horizontal, eps);
        ASSERT_FALSE(xings.empty());
        EXPECT_intersections_valid(vertical, horizontal, xings, eps);
        xings = vertical.intersect(butt, eps);
        ASSERT_FALSE(xings.empty());
        EXPECT_intersections_valid(vertical, butt, xings, eps);
        xings = vertical.intersect(too_far, eps);
        ASSERT_TRUE(xings.empty());
    }
}

/** Ensure that overlaps are found as precisely as possible even when epsilon is large. */
TEST(LineTest, FuzzyOverlap)
{
    auto const ab = LineSegment(Point(0, 0), Point(0, 20));
    auto const cd = LineSegment(Point(0, 10), Point(0, 30));
    auto xings = ab.intersect(cd, 4); // extra large eps
    ASSERT_EQ(xings.size(), 2);
    EXPECT_DOUBLE_EQ(xings[0].point()[1], 10);
    EXPECT_DOUBLE_EQ(xings[0].first, 0.5);
    EXPECT_DOUBLE_EQ(xings[0].second, 0.0);
    EXPECT_DOUBLE_EQ(xings[1].point()[1], 20);
    EXPECT_DOUBLE_EQ(xings[1].first, 1.0);
    EXPECT_DOUBLE_EQ(xings[1].second, 0.5);
}

/** Ensure that adjacent collinear segments are still detected as intersecting at
 * their exact common endpoint even when epsilon is large. */
TEST(LineTest, FuzzyEndToEnd)
{
    auto const ab = LineSegment(Point(0, 0), Point(0, 10));
    auto const cd = LineSegment(Point(0, 10), Point(0, 30));
    auto xings = ab.intersect(cd, 4); // extra large eps
    ASSERT_EQ(xings.size(), 1);
    EXPECT_DOUBLE_EQ(xings[0].point()[1], 10);
    EXPECT_DOUBLE_EQ(xings[0].first, 1.0);
    EXPECT_DOUBLE_EQ(xings[0].second, 0.0);
}

/** Ensure that a single intersection is found when the end-to-end
 * juncture contains a gap smaller than epsilon.
 */
TEST(LineTest, AlmostTouch)
{
    auto const ab = LineSegment(Point(0, 0), Point(0, 99));
    auto const cd = LineSegment(Point(0, 101), Point(0, 200));
    auto xings = ab.intersect(cd, 12);          // extra large eps
    ASSERT_EQ(xings.size(), 1);
    auto &x = xings[0];
    EXPECT_DOUBLE_EQ(x.point()[X], 0);
    EXPECT_DOUBLE_EQ(x.point()[Y], 100);
    EXPECT_DOUBLE_EQ(x.first, 1.0);
    EXPECT_DOUBLE_EQ(x.second, 0.0);
}

/** Ensure that a T-arrangement of segments has a single intersection
 * detected if and only if the gap between the vertical part and the
 * horizontal part is less than epsilon.
 */
TEST(LineTest, TBone)
{
    auto const horizontal = LineSegment(Point(-1, 1), Point(1, 1));
    g_random_set_seed(0x01234567);

    for (int exponent = -2; exponent > -13; exponent--) {
        double const eps = std::pow(10, exponent);
        for (size_t _ = 0; _ < 10'000; _++) {
            auto const distance = g_random_double_range(0.0, 2.0 * eps);
            size_t const expected_crossing_count = (size_t)(distance <= eps);
            auto const xings = horizontal.intersect(LineSegment(Point(0, 0), Point(0, 1.0 - distance)), eps);
            ASSERT_EQ(xings.size(), expected_crossing_count);
            if (expected_crossing_count) {
                auto const &x = xings[0];
                EXPECT_DOUBLE_EQ(x.point()[X], 0.0);
                EXPECT_DOUBLE_EQ(x.point()[Y], 1.0 - (0.5 * distance));
                EXPECT_DOUBLE_EQ(x.first,  0.5);
                EXPECT_DOUBLE_EQ(x.second, 1.0);
            }
        }
    }
}

/** Ensure that the normal pushoff is detected as intersecting the original
 * segment if and only if the push off distance is less than epsilon.
 */
TEST(LineTest, PushOff)
{
    auto const seg = LineSegment(Point(0, 0), Point(5, 3));
    auto const normal = (seg.finalPoint() - seg.initialPoint()).cw().normalized();
    g_random_set_seed(0xB787A350);

    for (int exponent = 1; exponent > -13; exponent--) {
        double const eps = std::pow(10, exponent);
        for (size_t _ = 0; _ < 10'000; _++) {
            auto const pushoff_distance = g_random_double_range(0.0, 2.0 * eps);
            std::unique_ptr<LineSegment> pushed_off{
                dynamic_cast<LineSegment *>(
                    seg.transformed(
                        Geom::Translate(pushoff_distance * normal)
                                   )
                                           )       };
            auto const xings = seg.intersect(*pushed_off, eps);
            bool const too_far = pushoff_distance > eps;
            EXPECT_EQ(xings.empty(), too_far);
            for (auto const &x : xings) {
                EXPECT_TRUE(are_near(x.first,  0.0, eps) or are_near(x.first,  1.0, eps));
                EXPECT_TRUE(are_near(x.second, 0.0, eps) or are_near(x.second, 1.0, eps));
            }
        }
    }
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
