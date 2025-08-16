/** @file
 * @brief Unit tests for EllipticalArc.
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

#include <2geom/elliptical-arc.h>
#include <2geom/exception.h>
#include <2geom/line.h>
#include <glib.h>
#include <optional>

using namespace Geom;

TEST(EllipticalArcTest, PointAt) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, false, true, Point(-40,0));
    EXPECT_near(a.pointAt(0), a.initialPoint(), 1e-14);
    EXPECT_near(a.pointAt(1), a.finalPoint(), 1e-14);
    EXPECT_near(a.pointAt(0.5), Point(-20,10), 1e-14);

    EllipticalArc b(Point(0,0), Point(10,20), 0, false, true, Point(-40,0));
    EXPECT_near(b.pointAt(0), b.initialPoint(), 1e-14);
    EXPECT_near(b.pointAt(1), b.finalPoint(), 1e-14);
    EXPECT_near(b.pointAt(0.5), Point(-20,40), 1e-14);

    EllipticalArc c(Point(200,0), Point(40,20), Angle::from_degrees(90), false, false, Point(200,100));
    EXPECT_near(c.pointAt(0), c.initialPoint(), 1e-13);
    EXPECT_near(c.pointAt(1), c.finalPoint(), 1e-13);
    EXPECT_near(c.pointAt(0.5), Point(175, 50), 1e-13);
}

TEST(EllipticalArc, Transform) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, false, true, Point(-40,0));
    EllipticalArc b(Point(-40,0), Point(10,20), M_PI/2, false, true, Point(0,0));
    EllipticalArc c = a;
    Affine m = Rotate::around(Point(-20,0), M_PI);
    c.transform(m);

    for (unsigned i = 0; i <= 100; ++i) {
        Coord t = i/100.;
        EXPECT_near(c.pointAt(t), b.pointAt(t), 1e-12);
        EXPECT_near(a.pointAt(t)*m, c.pointAt(t), 1e-12);
    }
}

TEST(EllipticalArcTest, Duplicate) {
    EllipticalArc a(Point(0,0), Point(10,20), M_PI/2, true, false, Point(-40,0));
    EllipticalArc *b = static_cast<EllipticalArc*>(a.duplicate());
    EXPECT_EQ(a, *b);
    delete b;
}

TEST(EllipticalArcTest, LineSegmentIntersection) {
    std::vector<CurveIntersection> r1;
    EllipticalArc a3(Point(0,0), Point(5,1.5), 0, true, true, Point(0,2));
    LineSegment ls(Point(0,5), Point(7,-3));
    r1 = a3.intersect(ls);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a3, ls, r1, 1e-10);

    g_random_set_seed(0xB747A380);
    // Test with randomized arcs and segments.
    for (size_t _ = 0; _ < 10'000; _++) {
        auto arc = EllipticalArc({g_random_double_range(1.0, 5.0), 0.0},
                                 {g_random_double_range(6.0, 8.0), g_random_double_range(2.0, 7.0)},
                                  g_random_double_range(-0.5, 0.5), true, g_random_boolean(),
                                 {g_random_double_range(-5.0, -1.0), 0.0});
        Coord x = g_random_double_range(15, 30);
        Coord y = g_random_double_range(10, 20);
        auto seg = LineSegment(Point(-x, y), Point(x, -y));
        auto xings = arc.intersect(seg);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(arc, seg, xings, 1e-12);
    }

    // Test with degenerate arcs
    EllipticalArc x_squash_pos{{3.0, 0.0}, {3.0, 2.0}, 0, true, true, {-3.0, 0.0}};
    EllipticalArc x_squash_neg{{3.0, 0.0}, {3.0, 2.0}, 0, true, false, {-3.0, 0.0}};
    auto const squash_to_x = Scale(1.0, 0.0);
    x_squash_pos *= squash_to_x; // squash to X axis interval [-3, 3].
    x_squash_neg *= squash_to_x;

    for (size_t _ = 0; _ < 10'000; _++) {
        auto seg = LineSegment(Point(g_random_double_range(-3.0, 3.0), g_random_double_range(-3.0, -1.0)),
                               Point(g_random_double_range(-3.0, 3.0), g_random_double_range(1.0, 3.0)));
        auto xings = x_squash_pos.intersect(seg);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(x_squash_pos, seg, xings, 1e-12);

        std::unique_ptr<Curve> rev{x_squash_pos.reverse()};
        xings = rev->intersect(seg);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(*rev, seg, xings, 1e-12);

        xings = x_squash_neg.intersect(seg);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(x_squash_neg, seg, xings, 1e-12);

        rev.reset(x_squash_neg.reverse());
        xings = rev->intersect(seg);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(*rev, seg, xings, 1e-12);
    }

    // Now test with an arc squashed to the Y-axis.
    EllipticalArc y_squash_pos{{0.0, -2.0}, {3.0, 2.0}, 0, true, true, {0.0, 2.0}};
    EllipticalArc y_squash_neg{{0.0, -2.0}, {3.0, 2.0}, 0, true, false, {0.0, 2.0}};
    auto const squash_to_y = Scale(0.0, 1.0);
    y_squash_pos *= squash_to_y; // Y-axis interval [-2, 2].
    y_squash_neg *= squash_to_y;

    for (size_t _ = 0; _ < 10'000; _++) {
        auto seg = LineSegment(Point(g_random_double_range(-3.0, -1.0), g_random_double_range(-2.0, 2.0)),
                               Point(g_random_double_range(1.0, 3.0),   g_random_double_range(-2.0, 2.0)));
        auto xings = y_squash_pos.intersect(seg, 1e-10);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(y_squash_pos, seg, xings, 1e-12);

        std::unique_ptr<Curve> rev{y_squash_pos.reverse()};
        xings = rev->intersect(seg, 1e-12);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(*rev, seg, xings, 1e-12);

        xings = y_squash_neg.intersect(seg, 1e-12);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(y_squash_neg, seg, xings, 1e-12);

        rev.reset(y_squash_neg.reverse());
        xings = rev->intersect(seg, 1e-12);
        EXPECT_EQ(xings.size(), 1u);
        EXPECT_intersections_valid(*rev, seg, xings, 1e-12);
    }

    // Test whether the coincidence between the common endpoints of an
    // arc and a segment is correctly detected as an intersection.
    {
        Point const from{1, 0};
        Point const to{0.30901699437494745, 0.9510565162951535};
        auto arc = EllipticalArc(from, {1, 1}, 0, false, true, to);
        auto seg = LineSegment({0, 0}, to);
        auto xings = arc.intersect(seg);
        ASSERT_EQ(xings.size(), 1);
        EXPECT_TRUE(are_near(xings[0].point(), to, 1e-12));
        EXPECT_TRUE(are_near(xings[0].first, 1.0, 1e-24));
        EXPECT_TRUE(are_near(xings[0].second, 1.0, 1e-24));

        auto seg2 = LineSegment(Point{1, 1}, from);
        xings = arc.intersect(seg2);
        ASSERT_EQ(xings.size(), 1);
        EXPECT_TRUE(are_near(xings[0].point(), from, 1e-12));
        EXPECT_TRUE(are_near(xings[0].first, 0.0, 1e-24));
        EXPECT_TRUE(are_near(xings[0].second, 1.0, 1e-24));
    }
}

TEST(EllipticalArcTest, ArcIntersection) {
    std::vector<CurveIntersection> r1, r2;

    EllipticalArc a1(Point(0,0), Point(6,3), 0.1, false, false, Point(10,0));
    EllipticalArc a2(Point(0,2), Point(6,3), -0.1, false, true, Point(10,2));
    r1 = a1.intersect(a2);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a1, a2, r1, 1e-10);

    EllipticalArc a3(Point(0,0), Point(5,1.5), 0, true, true, Point(0,2));
    EllipticalArc a4(Point(3,5), Point(5,1.5), M_PI/2, true, true, Point(5,0));
    r2 = a3.intersect(a4);
    EXPECT_EQ(r2.size(), 3u);
    EXPECT_intersections_valid(a3, a4, r2, 1e-10);

    // Make sure intersections are found between two identical arcs on the unit circle.
    EllipticalArc const upper(Point(1, 0), Point(1, 1), 0, true, true, Point(-1, 0));
    auto self_intersect = upper.intersect(upper);
    EXPECT_EQ(self_intersect.size(), 2u);

    // Make sure intersections are found between overlapping arcs.
    EllipticalArc const right(Point(0, -1), Point(1, 1), 0, true, true, Point(0, 1));
    auto quartering_overlap_xings = right.intersect(upper);
    EXPECT_EQ(quartering_overlap_xings.size(), 2u);

    // Make sure intersecections are found between an arc and its sub-arc.
    EllipticalArc const middle(upper.pointAtAngle(0.25 * M_PI), Point(1, 1), 0, true, true, upper.pointAtAngle(-0.25 * M_PI));
    EXPECT_EQ(middle.intersect(upper).size(), 2u);

    // Make sure intersections are NOT found between non-overlapping sub-arcs of the same circle.
    EllipticalArc const arc1{Point(1, 0), Point(1, 1), 0, true, true, Point(0, 1)};
    EllipticalArc const arc2{Point(-1, 0), Point(1, 1), 0, true, true, Point(0, -1)};
    EXPECT_EQ(arc1.intersect(arc2).size(), 0u);

    // Overlapping sub-arcs but on an Ellipse with different rays.
    EllipticalArc const eccentric{Point(2, 0), Point(2, 1), 0, true, true, Point(-2, 0)};
    EllipticalArc const subarc{eccentric.pointAtAngle(0.8), Point(2, 1), 0, true, true, eccentric.pointAtAngle(2)};
    EXPECT_EQ(eccentric.intersect(subarc).size(), 2u);

    // Check intersection times for two touching arcs.
    EllipticalArc const lower{Point(-1, 0), Point(1, 1), 0, false, true, Point(0, -1)};
    auto expected_neg_x = upper.intersect(lower);
    ASSERT_EQ(expected_neg_x.size(), 1);
    auto const &left_pt = expected_neg_x[0];
    EXPECT_EQ(left_pt.point(), Point(-1, 0));
    EXPECT_DOUBLE_EQ(left_pt.first, 1.0); // Expect (-1, 0) reached at the end of upper
    EXPECT_DOUBLE_EQ(left_pt.second, 0.0); // Expect (-1, 0) passed at the start of lower
}

TEST(EllipticalArcTest, BezierIntersection) {
    std::vector<CurveIntersection> r1, r2;

    EllipticalArc a3(Point(0,0), Point(1.5,5), M_PI/2, true, true, Point(0,2));
    CubicBezier bez1(Point(0,3), Point(7,3), Point(0,-1), Point(7,-1));
    r1 = a3.intersect(bez1);
    EXPECT_EQ(r1.size(), 2u);
    EXPECT_intersections_valid(a3, bez1, r1, 1e-10);

    EllipticalArc a4(Point(3,5), Point(5,1.5), 3*M_PI/2, true, true, Point(5,5));
    CubicBezier bez2(Point(0,5), Point(10,-4), Point(10,5), Point(0,-4));
    r2 = a4.intersect(bez2);
    EXPECT_EQ(r2.size(), 4u);
    EXPECT_intersections_valid(a4, bez2, r2, 1e-10);
}

TEST(EllipticalArcTest, ExpandToTransformedTest)
{
    auto test_curve = [] (EllipticalArc const &c, bool with_initial_bbox) {
        constexpr int N = 200;
        for (int i = 0; i < N; i++) {
            auto angle = 2 * M_PI * i / N;
            auto transform = Affine(Rotate(angle)) * Scale(0.9, 1.2);

            auto const box0 = with_initial_bbox ? Rect::from_xywh(10 * std::sin(angle * 13), 10 * std::sin(angle * 17), 5.0, 5.0) : OptRect();

            auto copy = std::unique_ptr<Curve>(c.duplicate());
            *copy *= transform;
            auto box1 = copy->boundsExact() | box0;

            auto pt = c.initialPoint() * transform;
            auto box2 = Rect(pt, pt) | box0;
            c.expandToTransformed(box2, transform);

            for (auto i : { X, Y }) {
                EXPECT_NEAR(box1[i].min(), box2[i].min(), 2e-15);
                EXPECT_NEAR(box1[i].max(), box2[i].max(), 2e-15);
            }
        }
    };

    for (auto b : { false, true }) {
        test_curve(EllipticalArc(Point(0, 0), 1.0, 2.0, 0.0, false, false, Point(1, 1)), b);
        test_curve(EllipticalArc(Point(0, 0), 3.0, 2.0, M_PI / 6, false, false, Point(1, 1)), b);
        test_curve(EllipticalArc(Point(0, 0), 1.0, 2.0, M_PI / 5, true, true, Point(1, 1)), b);
        test_curve(EllipticalArc(Point(1, 0), 1.0, 0.0, M_PI / 5, false, false, Point(1, 1)), b);
        test_curve(EllipticalArc(Point(1, 0), 0.0, 0.0, 0.0, false, false, Point(2, 0)), b);
        test_curve(EllipticalArc(Point(1, 0), 0.0, 0.0, 0.0, false, false, Point(1, 0)), b);
    }
}

#ifdef HAVE_GSL
/// Test that an arc can be recovered from its endpoints, end tangent lines and an interior point.
TEST(EllipticalArcTest, ArcFitRoundtrip)
{
    g_random_set_seed(0xB747A380);
    constexpr double relative_precision = 0.001;

    auto const generate_pseudorandom_point = [] {
        return Point(g_random_double_range(-1000.0, 1000.0), g_random_double_range(-1000.0, 1000.0));
    };

    auto const generate_pseudorandom_arc = [&] {
        auto const initial_point = generate_pseudorandom_point();
        auto const final_point = generate_pseudorandom_point();

        // Make sure that both rays are large enough so the arc doesn't degenerate to a chord
        double const dist = distance(initial_point, final_point);
        auto const rays =
            Point(g_random_double_range(2.0 * dist, 40.0 * dist), g_random_double_range(2.0 * dist, 40.0 * dist));
        return EllipticalArc(initial_point, rays, g_random_double_range(-M_PI, M_PI), g_random_boolean(),
                             g_random_boolean(), final_point);
    };

    auto const L_infty = [](Point const &pt) { return std::max(std::abs(pt.x()), std::abs(pt.y())); };

    for (unsigned _ = 0; _ < 10'000; ++_) {
        auto const arc = generate_pseudorandom_arc();

        auto const initial_point = arc.initialPoint();
        auto const final_point = arc.finalPoint();
        double const precision = std::max(L_infty(initial_point), L_infty(final_point)) * relative_precision;

        auto const initial_tangent = arc.pointAndDerivatives(0.0, 1).back();
        auto const initial_line = Line::from_origin_and_vector(initial_point, initial_tangent);

        auto const final_tangent = arc.pointAndDerivatives(1.0, 1).back();
        auto const final_line = Line::from_origin_and_vector(final_point, final_tangent);

        auto const intermediate_point = arc.pointAt(g_random_double_range(0.2, 0.8));

        auto const testArc = EllipticalArc::from_tangents_and_point(initial_line, intermediate_point, final_line);
        EXPECT_TRUE(arc.isNear(testArc, precision));
    }
}

struct ArcFittingTestCase {
    struct {
        Line initial_line;
        Line final_line;
        Point intermediate_point;
    } input;
    std::optional<EllipticalArc> expected; // empty if an exception is expected
};

const ArcFittingTestCase arc_fitting_test_cases[] = {
    ArcFittingTestCase{ // Right semicircle
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}), // Horizontal line at Y = 1
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}), // Horizontal line at Y = -1
            .intermediate_point = {1.0, 0.0}
        },
        .expected = EllipticalArc{{0.0, 1.0}, {1.0, 1.0}, 0.0, true, false, {0.0, -1.0}}
    },
    ArcFittingTestCase{ // Left semicircle
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}),
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}),
            .intermediate_point = {-1.0, 0.0}
        },
        .expected = EllipticalArc{{0.0, 1.0}, {1.0, 1.0}, 0.0, false, false, {0.0, -1.0}}
    },
    ArcFittingTestCase{ // Right horizontal semiellipse
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}),
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}),
            .intermediate_point = {20.0, 0.0}
        },
        .expected = EllipticalArc{{0.0, 1.0}, {20.0, 1.0}, 0.0, true, false, {0.0, -1.0}}
    },
    ArcFittingTestCase{ // Left horizontal semiellipse
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}),
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}),
            .intermediate_point = {-20.0, 0.0}
        },
        .expected = EllipticalArc{{0.0, 1.0}, {20.0, 1.0}, 0.0, false, false, {0.0, -1.0}}
    },
    ArcFittingTestCase{ // 3/4 of a circle
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}), // Horizontal line at Y = 1
            .final_line = Line({-1.0, 0.0}, {-1.0, 1.0}), // Vertical line at X = -1
            .intermediate_point = {1.0, 0.0}
        },
        .expected = EllipticalArc{{0.0, 1.0}, {1.0, 1.0}, 0.0, true, false, {-1.0, 0.0}}
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}),
            .final_line = Line({0.0, -1.0}, {1.0, -1.0}), // parallel horizontal lines
            .intermediate_point = {0.0, 1.1} // outside of the sandwich - no fit possible
        },
        .expected = {}
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 0.0}, {1.0, 0.0}), // Y axis
            .final_line = Line({0.0, 0.0}, {0.0, 1.0}), // X axis
            .intermediate_point = {0.0, 0.0} // origin
        },
        .expected = {} // everything is degenerate
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 0.0}, {0.0, 1.0}), // X axis
            .final_line = Line({0.0, 0.0}, {0.0, 1.0}), // X axis again!
            .intermediate_point = {1.0, 0.0}
        },
        .expected = {}
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}), // Horizontal line at Y = 1
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}), // Horizontal line at Y = -1
            .intermediate_point = {0.0, 1.0} // Initial point!
        },
        .expected = {}
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 1.0}), // Horizontal line at Y = 1
            .final_line = Line({0.0, -1.0}, {-1.0, -1.0}), // Horizontal line at Y = -1
            .intermediate_point = {0.0, -1.0} // Final point!
        },
        .expected = {}
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 0.0}),
            .final_line = Line({0.0, 1.0}, {1.0, 0.0}), // Same line twice
            .intermediate_point = {0.0, 0.0}
        },
        .expected = {} // Solution not unique
    },
    ArcFittingTestCase{
        .input = {
            .initial_line = Line({0.0, 1.0}, {1.0, 0.0}),
            .final_line = Line({0.0, -1.0}, {1.0, 0.0}), // lines intersect at (1, 0)
            .intermediate_point = {1.1, 0.0} // Point is on the wrong side
        },
        .expected = {} // No ellipse can cross its tangents
    },
};

struct EllipticalArcFittingTest : ::testing::TestWithParam<ArcFittingTestCase> {};

TEST_P(EllipticalArcFittingTest, ArcFittingSpecificCase)
{
    constexpr double precision = 1e-9;
    auto const &tc = GetParam();
    if (tc.expected) {
        EXPECT_TRUE(EllipticalArc::from_tangents_and_point(tc.input.initial_line, tc.input.intermediate_point,
                                                           tc.input.final_line)
                        .isNear(*tc.expected, precision));
    } else {
        EXPECT_THROW(EllipticalArc::from_tangents_and_point(tc.input.initial_line, tc.input.intermediate_point,
                                                            tc.input.final_line),
                     RangeError);
    }
}

INSTANTIATE_TEST_SUITE_P(EllipticalArcFittingTests, EllipticalArcFittingTest,
                         ::testing::ValuesIn(arc_fitting_test_cases));
#endif // HAVE_GSL
