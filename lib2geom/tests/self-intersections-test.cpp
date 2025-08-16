/** @file
 * @brief Unit tests for PathVector::intersectSelf()
 */
/*
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright 2022 Authors
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

#include <gtest/gtest.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>

using namespace Geom;

#define PV(d) (parse_svg_path(d))
#define PTH(d) (PV(d)[0])

class PVSelfIntersections : public testing::Test
{
protected:
    PathVector const _rectangle, _bowtie, _bowtie_curved, _bowtie_node, _openpath,
        _open_closed_nonintersecting, _open_closed_intersecting, _tangential, _degenerate_segments,
        _degenerate_closing, _degenerate_multiple;

    PVSelfIntersections()
        // A simple rectangle.
        : _rectangle{PV("M 0,0 L 5,0 5,8 0,8 Z")}
        // A polyline path with a self-intersection @(2,1).
        , _bowtie{PV("M 0,0 L 4,2 V 0 L 0,2 Z")}
        // A curved bow-tie path with a self-intersection @(10,5) between cubic Béziers.
        , _bowtie_curved{PV("M 0,0 V 10 C 10,10 10,0 20,0 V 10 C 10,10 10,0 0,0 Z")}
        // As above, but twice as large and the self-intersection @(20,10) happens at a node.
        , _bowtie_node{PV("M 0,0 V 20 C 0,20 10,20 20,10 25,5 30,0 40,0 V 20 "
                          "C 30,20 25,15 20,10 10,0 0,0 0,0 Z")}
        // An open path with no self-intersections ◠―◡
        , _openpath{PV("M 0,0 A 10,10 0,0,1 20,0 L 40,0 Q 50,10 60,0")}
        // A line and a square with no intersections | □
        , _open_closed_nonintersecting{PV("M 0,0 V 20 M 10,0 V 20 H 30 V 0 Z")}
        // A line slicing through a square; two self-intersections ⎅
        , _open_closed_intersecting{PV("M 10,0 V 40 M 0,10 V 30 H 20 V 10 Z")}
        // A circle whose diameter precisely coincides with the top side of a rectangle.
        , _tangential{PV("M 0,0 A 10,10 0,0,1 20,0 A 10,10, 0,0,1 0,0 Z M 0,0 H 20 V 30 H 0 Z")}
        // A rectangle containing degenerate segments.
        , _degenerate_segments{PV("M 0,0 H 5 V 4 L 5,4 V 8 H 5 L 5,8 H 0 Z")}
        // A rectangle with a degenerate closing segment.
        , _degenerate_closing{PV("M 0,0 H 5 V 8 H 0 L 0,0 Z")}
        // Multiple consecutive degenerate segments, with a degenerate closing segment in the middle.
        , _degenerate_multiple{PV("M 0,0 L 0,0 V 0 H 0 L 5,0 V 8 H 0 L 0,0 V 0 H 0 Z")}
    {
    }
};

/* Ensure that no spurious intersections are returned. */
TEST_F(PVSelfIntersections, NoSpurious)
{
    auto empty = PathVector();
    EXPECT_EQ(empty.intersectSelf().size(), 0u);

    auto r = _rectangle.intersectSelf();
    EXPECT_EQ(r.size(), 0u);

    auto o = _openpath.intersectSelf();
    EXPECT_EQ(o.size(), 0u);

    auto n = _open_closed_nonintersecting.intersectSelf();
    EXPECT_EQ(n.size(), 0u);

    auto d = _degenerate_segments.intersectSelf();
    EXPECT_EQ(d.size(), 0u);

    auto dc = _degenerate_closing.intersectSelf();
    EXPECT_EQ(dc.size(), 0u);

    auto dm = _degenerate_multiple.intersectSelf();
    EXPECT_EQ(dm.size(), 0u);

    auto cusp_node = PTH("M 1 3 C 12 8 42 101 86 133 C 78 168 136 83 80 64");
    EXPECT_EQ(cusp_node.intersectSelf().size(), 0u);
}

/* Test figure-eight shaped paths */
TEST_F(PVSelfIntersections, Bowties)
{
    // Simple triangular bowtie: intersection between straight lines
    auto triangular = _bowtie.intersectSelf();
    EXPECT_EQ(triangular.size(), 1u);
    ASSERT_GT(triangular.size(), 0u); // To ensure access to [0]
    EXPECT_TRUE(are_near(triangular[0].point(), Point(2, 1)));

    // Curved bowtie: intersection between cubic Bézier curves
    auto curved_intersections = _bowtie_curved.intersectSelf();
    EXPECT_EQ(curved_intersections.size(), 1u);
    ASSERT_GT(curved_intersections.size(), 0u);
    EXPECT_TRUE(are_near(curved_intersections[0].point(), Point(10, 5)));

    // Curved bowtie but the intersection point is a node on both paths
    auto node_case_intersections = _bowtie_node.intersectSelf();
    EXPECT_EQ(node_case_intersections.size(), 1u);
    ASSERT_GT(node_case_intersections.size(), 0u);
    EXPECT_TRUE(are_near(node_case_intersections[0].point(), Point(20, 10)));
}

/* Test intersecting an open path with a closed one */
TEST_F(PVSelfIntersections, OpenClosed)
{
    // Square cut by a vertical line
    auto open_closed = _open_closed_intersecting.intersectSelf();
    auto const P1 = Point(10, 10);
    auto const P2 = Point(10, 30);

    ASSERT_EQ(open_closed.size(), 2u); // Prevent crash on out-of-bounds access
    // This test doesn't care about how the intersections are ordered.
    bool points_as_expected = (are_near(open_closed[0].point(), P1) && are_near(open_closed[1].point(), P2))
            || (are_near(open_closed[0].point(), P2) && are_near(open_closed[1].point(), P1));
    EXPECT_TRUE(points_as_expected);
}

/* Test some nasty, tangential crossings: a circle with a rectangle built on its diameter. */
TEST_F(PVSelfIntersections, Tangential)
{
    auto circle_x_rect = _tangential.intersectSelf();
    auto const P1 = Point(0, 0);
    auto const P2 = Point(20, 0);

    ASSERT_EQ(circle_x_rect.size(), 2u); // Prevent crash on out-of-bounds access
    // This test doesn't care how the intersections are ordered.
    bool points_as_expected = (are_near(circle_x_rect[0].point(), P1) && are_near(circle_x_rect[1].point(), P2))
            || (are_near(circle_x_rect[0].point(), P2) && are_near(circle_x_rect[1].point(), P1));
    EXPECT_TRUE(points_as_expected);
}

/* Regression test for issue https://gitlab.com/inkscape/lib2geom/-/issues/33 */
TEST_F(PVSelfIntersections, Regression33)
{
    // Test case provided by Pascal Bies in the issue description.
    auto const line = LineSegment(Point(486, 597), Point(313, 285));
    Point const c{580.1377046525328, 325.5830744834947};
    Point const d{289.35338528516013, 450.62476639303753};
    auto const curve = CubicBezier(c, c, d, d);

    EXPECT_EQ(curve.intersect(line).size(), 1);
}

/* Regression test for issue https://gitlab.com/inkscape/lib2geom/-/issues/46 */
TEST_F(PVSelfIntersections, NumericalInstability)
{
    // Test examples provided by M.B. Fraga in the issue report.
    auto missing_intersection = PTH("M 138 237 C 293 207 129 12 167 106 Q 205 200 309 198 z");
    auto missing_xings = missing_intersection.intersectSelf();
    EXPECT_EQ(missing_xings.size(), 2);

    auto duplicate_intersection = PTH("M 60 280 C 60 213 236 227 158 178 S 174 306 127 310 Q 80 314 60 280 z");
    auto const only_expected = Point(130.9693916417836, 224.587385497877);
    auto duplicate_xings = duplicate_intersection.intersectSelf();
    ASSERT_EQ(duplicate_xings.size(), 1);
    EXPECT_TRUE(are_near(duplicate_xings[0].point(), only_expected));
}

/* Check various numerically challenging paths consisting of 2 cubic Béziers. */
TEST_F(PVSelfIntersections, NumericallyChallenging)
{
    auto two_kinks = PTH("M 85 88 C 4 425 19 6 72 426 C 128 6 122 456 68 96");
    EXPECT_EQ(two_kinks.intersectSelf().size(), 3);

    auto omega = PTH("M 47 132 C 179 343 0 78 106 74 C 187 74 0 358 174 106");
    EXPECT_EQ(omega.intersectSelf().size(), 0);

    auto spider = PTH("M 47 132 C 203 339 0 78 106 74 C 187 74 0 358 174 106");
    EXPECT_EQ(spider.intersectSelf().size(), 4);

    auto egret = PTH("M 38 340 C 183 141 16 76 255 311 C 10 79 116 228 261 398");
    EXPECT_EQ(egret.intersectSelf().size(), 0);
}

/* Test a regression from 88040ea2aeab8ccec2b0e96c7bda2fc7d500d5ec */
TEST_F(PVSelfIntersections, BigonFiltering)
{
    auto const lens = PTH("M 0,0 C 2,1 3,1 5,0 A 2.5,1 0 1 0 0,0 Z");
    auto const xings = lens.intersectSelf();
    // This is a simple closed path, so we expect that no self-intersections are reported.
    EXPECT_EQ(xings.size(), 0);
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
