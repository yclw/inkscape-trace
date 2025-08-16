/** @file
 * @brief Unit tests for Affine.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Nathan Hurst <njh@njhurst.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *   Johan Engelen <j.b.c.engelen@alumnus.utwente.nl>
 *
 * Copyright 2010 Authors
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

#include <2geom/bezier.h>
#include <2geom/polynomial.h>
#include <2geom/basic-intersection.h>
#include <2geom/bezier-curve.h>
#include <vector>
#include <glib.h>

using std::vector, std::min, std::max;
using namespace Geom;

Poly lin_poly(double a, double b) { // ax + b
    Poly p;
    p.push_back(b);
    p.push_back(a);
    return p;
}

bool are_equal(Bezier A, Bezier B) {
    int maxSize = max(A.size(), B.size());
    double t = 0., dt = 1./maxSize;

    for(int i = 0; i <= maxSize; i++) {
        EXPECT_FLOAT_EQ(A.valueAt(t), B.valueAt(t));// return false;
        t += dt;
    }
    return true;
}

class BezierTest : public ::testing::Test {
protected:

    BezierTest()
        : zero(fragments[0])
        , unit(fragments[1])
        , hump(fragments[2])
        , wiggle(fragments[3])
    {
        zero = Bezier(0.0,0.0);
        unit = Bezier(0.0,1.0);
        hump = Bezier(0,1,0);
        wiggle = Bezier(0,1,-2,3);
    }

    Bezier fragments[4];
    Bezier &zero, &unit, &hump, &wiggle;
};

TEST_F(BezierTest, Basics) {

    //std::cout << unit <<std::endl;
    //std::cout << hump <<std::endl;

    EXPECT_TRUE(Bezier(0,0,0,0).isZero());
    EXPECT_TRUE(Bezier(0,1,2,3).isFinite());

    EXPECT_EQ(3u, Bezier(0,2,4,5).order());

    ///cout << " Bezier::Bezier(const Bezier& b);\n";
    //cout << Bezier(wiggle) << " == " << wiggle << endl;

    //cout << "explicit Bezier(unsigned ord);\n";
    //cout << Bezier(10) << endl;

    //cout << "Bezier(Coord c0, Coord c1);\n";
    //cout << Bezier(0.0,1.0) << endl;

    //cout << "Bezier(Coord c0, Coord c1, Coord c2);\n";
    //cout << Bezier(0,1, 2) << endl;

    //cout << "Bezier(Coord c0, Coord c1, Coord c2, Coord c3);\n";
    //cout << Bezier(0,1,2,3) << endl;

    //cout << "unsigned degree();\n";
    EXPECT_EQ(2u, hump.degree());

    //cout << "unsigned size();\n";
    EXPECT_EQ(3u, hump.size());
}

TEST_F(BezierTest, ValueAt) {
    EXPECT_EQ(0.0, wiggle.at0());
    EXPECT_EQ(3.0, wiggle.at1());

    EXPECT_EQ(0.0, wiggle.valueAt(0.5));

    EXPECT_EQ(0.0, wiggle(0.5));

    //cout << "SBasis toSBasis();\n";
    //cout << unit.toSBasis() << endl;
    //cout << hump.toSBasis() << endl;
    //cout << wiggle.toSBasis() << endl;
}

TEST_F(BezierTest, Casteljau) {
    unsigned N = wiggle.order() + 1;
    std::vector<Coord> left(N), right(N);
    std::vector<Coord> left2(N), right2(N);
    double eps = 1e-15;

    for (unsigned i = 0; i < 10000; ++i) {
        double t = g_random_double_range(0, 1);
        double vok = bernstein_value_at(t, &wiggle[0], wiggle.order());
        double v = casteljau_subdivision<double>(t, &wiggle[0], &left[0], &right[0], wiggle.order());
        EXPECT_near(v, vok, eps);
        EXPECT_EQ(left[0], wiggle.at0());
        EXPECT_EQ(left[wiggle.order()], right[0]);
        EXPECT_EQ(right[wiggle.order()], wiggle.at1());

        double vl = casteljau_subdivision<double>(t, &wiggle[0], &left2[0], NULL, wiggle.order());
        double vr = casteljau_subdivision<double>(t, &wiggle[0], NULL, &right2[0], wiggle.order());
        EXPECT_EQ(vl, vok);
        EXPECT_near(vr, vok, eps);
        EXPECT_vector_near(left2, left, eps);
        EXPECT_vector_equal(right2, right);

        double vnone = casteljau_subdivision<double>(t, &wiggle[0], NULL, NULL, wiggle.order());
        EXPECT_near(vnone, vok, 1e-12);
    }
}

TEST_F(BezierTest, Portion) {
    constexpr Coord eps{1e-12};

    for (unsigned i = 0; i < 10000; ++i) {
        double from = g_random_double_range(0, 1);
        double to = g_random_double_range(0, 1);
        for (auto & input : fragments) {
            Bezier result = portion(input, from, to);

            // the endpoints must correspond exactly
            EXPECT_near(result.at0(), input.valueAt(from), eps);
            EXPECT_near(result.at1(), input.valueAt(to), eps);
        }
    }
}

TEST_F(BezierTest, Subdivide) {
    std::vector<std::pair<Bezier, double> > errors;
    for (unsigned i = 0; i < 10000; ++i) {
        double t = g_random_double_range(0, 1e-6);
        for (auto & input : fragments) {
            std::pair<Bezier, Bezier> result = input.subdivide(t);

            // the endpoints must correspond exactly
            // moreover, the subdivision point must be exactly equal to valueAt(t)
            EXPECT_DOUBLE_EQ(result.first.at0(), input.at0());
            EXPECT_DOUBLE_EQ(result.first.at1(), result.second.at0());
            EXPECT_DOUBLE_EQ(result.second.at0(), input.valueAt(t));
            EXPECT_DOUBLE_EQ(result.second.at1(), input.at1());

            // ditto for valueAt
            EXPECT_DOUBLE_EQ(result.first.valueAt(0), input.valueAt(0));
            EXPECT_DOUBLE_EQ(result.first.valueAt(1), result.second.valueAt(0));
            EXPECT_DOUBLE_EQ(result.second.valueAt(0), input.valueAt(t));
            EXPECT_DOUBLE_EQ(result.second.valueAt(1), input.valueAt(1));

            if (result.first.at1() != result.second.at0()) {
                errors.emplace_back(input, t);
            }
        }
    }
    if (!errors.empty()) {
        std::cout << "Found " << errors.size() << " subdivision errors" << std::endl;
        for (unsigned i = 0; i < errors.size(); ++i) {
            std::cout << "Error #" << i << ":\n"
                          << errors[i].first << "\n"
                          << "t: " << format_coord_nice(errors[i].second) << std::endl;
        }
    }
}

TEST_F(BezierTest, Mutation) {
//Coord &operator[](unsigned ix);
//Coord const &operator[](unsigned ix);
//void setCoeff(unsigned ix double val);
    //cout << "bigun\n";
    Bezier bigun(Bezier::Order(30));
    bigun.setCoeff(5,10.0);
    for(unsigned i = 0; i < bigun.size(); i++) {
        EXPECT_EQ((i == 5) ? 10 : 0, bigun[i]);
    }

    bigun[5] = -3;
    for(unsigned i = 0; i < bigun.size(); i++) {
        EXPECT_EQ((i == 5) ? -3 : 0, bigun[i]);
    }
}

TEST_F(BezierTest, MultiDerivative) {
    vector<double> vnd = wiggle.valueAndDerivatives(0.5, 5);
    expect_array((const double[]){0,0,12,72,0,0}, vnd);
}

TEST_F(BezierTest, DegreeElevation) {
    EXPECT_TRUE(are_equal(wiggle, wiggle));
    Bezier Q = wiggle;
    Bezier P = Q.elevate_degree();
    EXPECT_EQ(P.size(), Q.size()+1);
    //EXPECT_EQ(0, P.forward_difference(1)[0]);
    EXPECT_TRUE(are_equal(Q, P));
    Q = wiggle;
    P = Q.elevate_to_degree(10);
    EXPECT_EQ(10u, P.order());
    EXPECT_TRUE(are_equal(Q, P));
    //EXPECT_EQ(0, P.forward_difference(10)[0]);
    /*Q = wiggle.elevate_degree();
    P = Q.reduce_degree();
    EXPECT_EQ(P.size()+1, Q.size());
    EXPECT_TRUE(are_equal(Q, P));*/
}
//std::pair<Bezier, Bezier > subdivide(Coord t);

// Constructs a linear Bezier with root at t
Bezier linear_root(double t) {
    return Bezier(0-t, 1-t);
}

// Constructs a Bezier with roots at the locations in x
Bezier array_roots(vector<double> x) {
    Bezier b(1);
    for(double i : x) {
        b = multiply(b, linear_root(i));
    }
    return b;
}

TEST_F(BezierTest, Deflate) {
    Bezier b = array_roots(vector_from_array((const double[]){0,0.25,0.5}));
    EXPECT_FLOAT_EQ(0, b.at0());
    b = b.deflate();
    EXPECT_FLOAT_EQ(0, b.valueAt(0.25));
    b = b.subdivide(0.25).second;
    EXPECT_FLOAT_EQ(0, b.at0());
    b = b.deflate();
    const double rootposition = (0.5-0.25) / (1-0.25);
    constexpr Coord eps{1e-12};
    EXPECT_near(0.0, b.valueAt(rootposition), eps);
    b = b.subdivide(rootposition).second;
    EXPECT_near(0.0, b.at0(), eps);
}

TEST_F(BezierTest, Roots) {
    expect_array((const double[]){0, 0.5, 0.5}, wiggle.roots());

    /*Bezier bigun(Bezier::Order(30));
    for(unsigned i = 0; i < bigun.size(); i++) {
        bigun.setCoeff(i,rand()-0.5);
    }
    cout << bigun.roots() << endl;*/

    // The results of our rootfinding are at the moment fairly inaccurate.
    double eps = 5e-4;

    vector<vector<double> > tests;
    tests.push_back(vector_from_array((const double[]){0}));
    tests.push_back(vector_from_array((const double[]){1}));
    tests.push_back(vector_from_array((const double[]){0, 0}));
    tests.push_back(vector_from_array((const double[]){0.5}));
    tests.push_back(vector_from_array((const double[]){0.5, 0.5}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.1, 0.1, 0.1}));
    tests.push_back(vector_from_array((const double[]){0.25,0.75}));
    tests.push_back(vector_from_array((const double[]){0.5,0.5}));
    tests.push_back(vector_from_array((const double[]){0, 0.2, 0.6, 0.6, 1}));
    tests.push_back(vector_from_array((const double[]){.1,.2,.3,.4,.5,.6}));
    tests.push_back(vector_from_array((const double[]){0.25,0.25,0.25,0.75,0.75,0.75}));

    for(auto & test : tests) {
        Bezier b = array_roots(test);
        //std::cout << tests[test_i] << ": " << b << std::endl;
        //std::cout << b.roots() << std::endl;
        EXPECT_vector_near(test, b.roots(), eps);
    }
}

TEST_F(BezierTest, BoundsExact) {
    OptInterval unit_bounds = bounds_exact(unit);
    EXPECT_EQ(unit_bounds->min(), 0);
    EXPECT_EQ(unit_bounds->max(), 1);

    OptInterval hump_bounds = bounds_exact(hump);
    EXPECT_EQ(hump_bounds->min(), 0);
    EXPECT_FLOAT_EQ(hump_bounds->max(), hump.valueAt(0.5));

    OptInterval wiggle_bounds = bounds_exact(wiggle);
    EXPECT_EQ(wiggle_bounds->min(), 0);
    EXPECT_EQ(wiggle_bounds->max(), 3);
}

TEST_F(BezierTest, Operators) {
    // Test equality operators
    EXPECT_EQ(zero, zero);
    EXPECT_EQ(hump, hump);
    EXPECT_EQ(wiggle, wiggle);
    EXPECT_EQ(unit, unit);

    EXPECT_NE(zero, hump);
    EXPECT_NE(hump, zero);
    EXPECT_NE(wiggle, hump);
    EXPECT_NE(zero, wiggle);
    EXPECT_NE(wiggle, unit);

    // Recall that hump == Bezier(0,1,0);
    EXPECT_EQ(hump + 3, Bezier(3, 4, 3));
    EXPECT_EQ(hump - 3, Bezier(-3, -2, -3));
    EXPECT_EQ(hump * 3, Bezier(0, 3, 0));
    EXPECT_EQ(hump / 3, Bezier(0, 1.0/3.0, 0));
    EXPECT_EQ(-hump, Bezier(0, -1, 0));

    Bezier reverse_wiggle = reverse(wiggle);
    EXPECT_EQ(reverse_wiggle.at0(), wiggle.at1());
    EXPECT_EQ(reverse_wiggle.at1(), wiggle.at0());
    EXPECT_TRUE(are_equal(reverse(reverse_wiggle), wiggle));

    //cout << "Bezier portion(const Bezier & a, double from, double to);\n";
    //cout << portion(Bezier(0.0,2.0), 0.5, 1) << endl;

// std::vector<Point> bezier_points(const D2<Bezier > & a) {

    /*cout << "Bezier derivative(const Bezier & a);\n";
    std::cout << derivative(hump) <<std::endl;
    std::cout << integral(hump) <<std::endl;*/

    EXPECT_TRUE(are_equal(derivative(integral(wiggle)), wiggle));
    //std::cout << derivative(integral(hump)) <<std::endl;
    expect_array((const double []){0.5}, derivative(hump).roots());

    EXPECT_TRUE(bounds_fast(hump)->contains(Interval(0,hump.valueAt(0.5))));

    EXPECT_EQ(Interval(0,hump.valueAt(0.5)), *bounds_exact(hump));

    Interval tight_local_bounds(min(hump.valueAt(0.3),hump.valueAt(0.6)),
             hump.valueAt(0.5));
    EXPECT_TRUE(bounds_local(hump, Interval(0.3, 0.6))->contains(tight_local_bounds));

    Bezier Bs[] = {unit, hump, wiggle};
    for(auto B : Bs) {
        Bezier product = multiply(B, B);
        for(int i = 0; i <= 16; i++) {
            double t = i/16.0;
            double b = B.valueAt(t);
            EXPECT_near(b*b, product.valueAt(t), 1e-12);
        }
    }
}

struct XPt {
    XPt(Coord x, Coord y, Coord ta, Coord tb)
        : p(x, y), ta(ta), tb(tb)
    {}
    XPt() {}
    Point p;
    Coord ta, tb;
};

struct XTest {
    D2<Bezier> a;
    D2<Bezier> b;
    std::vector<XPt> s;
};

struct CILess {
    bool operator()(CurveIntersection const &a, CurveIntersection const &b) const {
        if (a.first < b.first) return true;
        if (a.first == b.first && a.second < b.second) return true;
        return false;
    }
};

TEST_F(BezierTest, Intersection) {
    /* Intersection test cases taken from:
     * Dieter Lasser (1988), Calculating the Self-Intersections of Bezier Curves
     * https://archive.org/stream/calculatingselfi00lass
     *
     * The intersection points are not actually calculated to a high precision
     * in the paper. The most relevant tests are whether the curves actually
     * intersect at the returned time values (i.e. whether a(ta) = b(tb))
     * and whether the number of intersections is correct.
     */
    typedef D2<Bezier> D2Bez;
    std::vector<XTest> tests;

    // Example 1
    tests.emplace_back();
    tests.back().a = D2Bez(Bezier(-3.3, -3.3, 0, 3.3, 3.3), Bezier(1.3, -0.7, 2.3, -0.7, 1.3));
    tests.back().b = D2Bez(Bezier(-4.0, -4.0, 0, 4.0, 4.0), Bezier(-0.35, 3.0, -2.6, 3.0, -0.35));
    tests.back().s.resize(4);
    tests.back().s[0] = XPt(-3.12109, 0.76362, 0.09834, 0.20604);
    tests.back().s[1] = XPt(-1.67341, 0.60298, 0.32366, 0.35662);
    tests.back().s[2] = XPt(1.67341, 0.60298, 0.67634, 0.64338);
    tests.back().s[3] = XPt(3.12109, 0.76362, 0.90166, 0.79396);

    // Example 2
    tests.emplace_back();
    tests.back().a = D2Bez(Bezier(0, 0, 3, 3), Bezier(0, 14, -9, 5));
    tests.back().b = D2Bez(Bezier(-1, 13, -10, 4), Bezier(4, 4, 1, 1));
    tests.back().s.resize(9);
    tests.back().s[0] = XPt(0.00809, 1.17249, 0.03029, 0.85430);
    tests.back().s[1] = XPt(0.02596, 1.97778, 0.05471, 0.61825);
    tests.back().s[2] = XPt(0.17250, 3.99191, 0.14570, 0.03029);
    tests.back().s[3] = XPt(0.97778, 3.97404, 0.38175, 0.05471);
    tests.back().s[4] = XPt(1.5, 2.5, 0.5, 0.5);
    tests.back().s[5] = XPt(2.02221, 1.02596, 0.61825, 0.94529);
    tests.back().s[6] = XPt(2.82750, 1.00809, 0.85430, 0.96971);
    tests.back().s[7] = XPt(2.97404, 3.02221, 0.94529, 0.38175);
    tests.back().s[8] = XPt(2.99191, 3.82750, 0.96971, 0.14570);

    // Example 3
    tests.emplace_back();
    tests.back().a = D2Bez(Bezier(-5, -5, -3, 0, 3, 5, 5), Bezier(0, 3.555, -1, 4.17, -1, 3.555, 0));
    tests.back().b = D2Bez(Bezier(-6, -6, -3, 0, 3, 6, 6), Bezier(3, -0.555, 4, -1.17, 4, -0.555, 3));
    tests.back().s.resize(6);
    tests.back().s[0] = XPt(-3.64353, 1.49822, 0.23120, 0.27305);
    tests.back().s[1] = XPt(-2.92393, 1.50086, 0.29330, 0.32148);
    tests.back().s[2] = XPt(-0.77325, 1.49989, 0.44827, 0.45409);
    tests.back().s[3] = XPt(0.77325, 1.49989, 0.55173, 0.54591);
    tests.back().s[4] = XPt(2.92393, 1.50086, 0.70670, 0.67852);
    tests.back().s[5] = XPt(3.64353, 1.49822, 0.76880, 0.72695);

    // Example 4
    tests.emplace_back();
    tests.back().a = D2Bez(Bezier(-4, -10, -2, -2, 2, 2, 10, 4), Bezier(0, 6, 6, 0, 0, 6, 6, 0));
    tests.back().b = D2Bez(Bezier(-8, 0, 8), Bezier(1, 6, 1));
    tests.back().s.resize(4);
    tests.back().s[0] = XPt(-5.69310, 2.23393, 0.06613, 0.14418);
    tests.back().s[1] = XPt(-2.68113, 3.21920, 0.35152, 0.33243);
    tests.back().s[2] = XPt(2.68113, 3.21920, 0.64848, 0.66757);
    tests.back().s[3] = XPt(5.69310, 2.23393, 0.93387, 0.85582);

    //std::cout << std::setprecision(5);

    for (unsigned i = 0; i < tests.size(); ++i) {
        BezierCurve a(tests[i].a), b(tests[i].b);
        std::vector<CurveIntersection> xs;
        xs = a.intersect(b, 1e-8);
        std::sort(xs.begin(), xs.end(), CILess());
        //xs.erase(std::unique(xs.begin(), xs.end(), XEqual()), xs.end());

        std::cout << "\n\n"
                  << "===============================\n"
                  << "=== Intersection Testcase " << i+1 << " ===\n"
                  << "===============================\n" << std::endl;

        EXPECT_EQ(xs.size(), tests[i].s.size());
        //if (xs.size() != tests[i].s.size()) continue;

        for (unsigned j = 0; j < std::min(xs.size(), tests[i].s.size()); ++j) {
            std::cout << xs[j].first << " = " << a.pointAt(xs[j].first) << "   "
                      << xs[j].second << " = " << b.pointAt(xs[j].second) << "\n"
                      << tests[i].s[j].ta << " = " << tests[i].a.valueAt(tests[i].s[j].ta) << "   "
                      << tests[i].s[j].tb << " = " << tests[i].b.valueAt(tests[i].s[j].tb) << std::endl;
        }

        EXPECT_intersections_valid(a, b, xs, 1e-6);
    }

    #if 0
    // these contain second-order intersections
    Coord a5x[] = {-1.5, -1.5, -10, -10, 0, 10, 10, 1.5, 1.5};
    Coord a5y[] = {0, -8, -8, 9, 9, 9, -8, -8, 0};
    Coord b5x[] = {-3, -12, 0, 12, 3};
    Coord b5y[] = {-5, 8, 2.062507, 8, -5};
    Coord p5x[] = {-3.60359, -5.44653, 0, 5.44653, 3.60359};
    Coord p5y[] = {-4.10631, -0.76332, 4.14844, -0.76332, -4.10631};
    Coord p5ta[] = {0.01787, 0.10171, 0.5, 0.89829, 0.98213};
    Coord p5tb[] = {0.12443, 0.28110, 0.5, 0.71890, 0.87557};

    Coord a6x[] = {5, 14, 10, -12, -12, -2};
    Coord a6y[] = {1, 6, -6, -6, 2, 2};
    Coord b6x[] = {0, 2, -10.5, -10.5, 3.5, 3, 8, 6};
    Coord b6y[] = {0, -8, -8, 9, 9, -4.129807, -4.129807, 3};
    Coord p6x[] = {6.29966, 5.87601, 0.04246, -4.67397, -3.57214};
    Coord p6y[] = {1.63288, -0.86192, -2.38219, -2.17973, 1.91463};
    Coord p6ta[] = {0.03184, 0.33990, 0.49353, 0.62148, 0.96618};
    Coord p6tb[] = {0.96977, 0.85797, 0.05087, 0.28232, 0.46102};
    #endif
}

/** Basic test for intersecting a quadratic Bézier with a line segment. */
TEST_F(BezierTest, QuadraticIntersectLineSeg)
{
    double const EPS = 1e-12;
    auto const bow = QuadraticBezier({0, 0}, {1, 1}, {2, 0});
    auto const highhoriz  = LineSegment(Point(0, 0), Point(2, 0));
    auto const midhoriz   = LineSegment(Point(0, 0.25), Point(2, 0.25));
    auto const lowhoriz   = LineSegment(Point(0, 0.5), Point(2, 0.5));
    auto const noninters  = LineSegment(Point(0, 0.5 + EPS), Point(2, 0.5 + EPS));
    auto const noninters2 = LineSegment(Point(1, 0), Point(1, 0.5 - EPS));

    auto const endpoint_intersections = bow.intersect(highhoriz, EPS);
    EXPECT_EQ(endpoint_intersections.size(), 2);
    EXPECT_intersections_valid(bow, highhoriz, endpoint_intersections, EPS);
    for (auto const &ex : endpoint_intersections) {
        EXPECT_DOUBLE_EQ(ex.point()[Y], 0.0);
    }

    auto const mid_intersections = bow.intersect(midhoriz, EPS);
    EXPECT_EQ(mid_intersections.size(), 2);
    EXPECT_intersections_valid(bow, midhoriz, mid_intersections, EPS);
    for (auto const &mx : mid_intersections) {
        EXPECT_DOUBLE_EQ(mx.point()[Y], 0.25);
    }

    auto const tangent_intersection = bow.intersect(lowhoriz, EPS);
    EXPECT_EQ(tangent_intersection.size(), 1);
    EXPECT_intersections_valid(bow, lowhoriz, tangent_intersection, EPS);
    for (auto const &tx : tangent_intersection) {
        EXPECT_DOUBLE_EQ(tx.point()[Y], 0.5);
    }

    auto no_intersections = bow.intersect(noninters, EPS);
    EXPECT_TRUE(no_intersections.empty());

    no_intersections = bow.intersect(noninters2, EPS);
    EXPECT_TRUE(no_intersections.empty());
}

TEST_F(BezierTest, QuadraticIntersectLineRandom)
{
    g_random_set_seed(0xB747A380);
    auto const diagonal = LineSegment(Point(0, 0), Point(1, 1));
    double const EPS = 1e-12;

    for (unsigned i = 0; i < 10'000; i++) {
        auto q = QuadraticBezier({0, 1}, {g_random_double_range(0.0, 1.0), g_random_double_range(0.0, 1.0)}, {1, 0});
        auto xings = q.intersect(diagonal, EPS);
        ASSERT_EQ(xings.size(), 1);
        auto pt = xings[0].point();
        EXPECT_TRUE(are_near(pt[X], pt[Y], EPS));
        EXPECT_intersections_valid(q, diagonal, xings, EPS);
    }
}

/** Basic test for intersecting a cubic Bézier with a line segment. */
TEST_F(BezierTest, CubicIntersectLine)
{
    double const EPS = 1e-12;
    auto const wavelet = CubicBezier({0, 0}, {1, 2}, {0, -2}, {1, 0});

    auto const unit_seg = LineSegment(Point(0, 0), Point(1, 0));
    auto const expect3 = wavelet.intersect(unit_seg, EPS);
    EXPECT_EQ(expect3.size(), 3);
    EXPECT_intersections_valid(wavelet, unit_seg, expect3, EPS);

    auto const half_seg = LineSegment(Point(0, 0), Point(0.5, 0));
    auto const expect2 = wavelet.intersect(half_seg, EPS);
    EXPECT_EQ(expect2.size(), 2);
    EXPECT_intersections_valid(wavelet, half_seg, expect2, EPS);

    auto const less_than_half = LineSegment(Point(0, 0), Point(0.5 - EPS, 0));
    auto const expect1 = wavelet.intersect(less_than_half, EPS);
    EXPECT_EQ(expect1.size(), 1);
    EXPECT_intersections_valid(wavelet, less_than_half, expect1, EPS);

    auto const dollar_stroke = LineSegment(Point(0, 0.5), Point(1, -0.5));
    auto const dollar_xings = wavelet.intersect(dollar_stroke, EPS);
    EXPECT_EQ(dollar_xings.size(), 3);
    EXPECT_intersections_valid(wavelet, dollar_stroke, dollar_xings, EPS);
}

TEST_F(BezierTest, CubicIntersectLineRandom)
{
    g_random_set_seed(0xCAFECAFE);
    auto const diagonal = LineSegment(Point(0, 0), Point(1, 1));
    double const EPS = 1e-8;

    for (unsigned i = 0; i < 10'000; i++) {
        double a1 = g_random_double_range(0.0, 1.0);
        double a2 = g_random_double_range(a1, 1.0);
        double b1 = g_random_double_range(0.0, 1.0);
        double b2 = g_random_double_range(0.0, b1);

        auto c = CubicBezier({0, 1}, {a1, a2}, {b1, b2}, {1, 0});
        auto xings = c.intersect(diagonal, EPS);
        ASSERT_EQ(xings.size(), 1);
        auto pt = xings[0].point();
        EXPECT_TRUE(are_near(pt[X], pt[Y], EPS));
        EXPECT_intersections_valid(c, diagonal, xings, EPS);
    }
}

/** Regression test for issue https://gitlab.com/inkscape/lib2geom/-/issues/47 . */
TEST_F(BezierTest, Balloon)
{
    auto const loop = CubicBezier({0, 0}, {4, -2}, {4, 2}, {0, 0});
    auto const seghoriz = LineSegment(Point(-1, 0), Point(0, 0));

    for (double EPS : {1e-6, 1e-9, 1e-12}) {
        // We expect that 2 intersections are found: one at each end of the loop,
        // both at the coordinates (0, 0).
        auto xings_horiz = loop.intersect(seghoriz, EPS);
        EXPECT_EQ(xings_horiz.size(), 2);
        EXPECT_intersections_valid(loop, seghoriz, xings_horiz, EPS);
    }
}

TEST_F(BezierTest, ExpandToTransformedTest)
{
    auto test_curve = [] (Curve const &c) {
        constexpr int N = 50;
        for (int i = 0; i < N; i++) {
            auto angle = 2 * M_PI * i / N;
            auto transform = Affine(Rotate(angle));
            
            auto copy = std::unique_ptr<Curve>(c.duplicate());
            *copy *= transform;
            auto box1 = copy->boundsExact();
            
            auto pt = c.initialPoint() * transform;
            auto box2 = Rect(pt, pt);
            c.expandToTransformed(box2, transform);
            
            for (auto i : { X, Y }) {
                EXPECT_DOUBLE_EQ(box1[i].min(), box2[i].min());
                EXPECT_DOUBLE_EQ(box1[i].max(), box2[i].max());
            }
        }
    };
    
    test_curve(LineSegment(Point(-1, 0), Point(1, 2)));
    test_curve(QuadraticBezier(Point(-1, 0), Point(1, 1), Point(3, 0)));
    test_curve(CubicBezier(Point(-1, 0), Point(1, 1), Point(2, -2), Point(3, 0)));
}

TEST_F(BezierTest, TimesWithRadiusOfCurvature)
{
    auto test_curve = [](BezierCurve const &curve, double radius, std::vector<Coord> times_expected) {
        auto const times_actual = curve.timesWithRadiusOfCurvature(radius);
        EXPECT_vector_near(times_actual, times_expected, 1e-8);
    };

    test_curve(LineSegment(Point(-1, 0), Point(1, 2)), 1.7, {});
    test_curve(LineSegment(Point(-1, 0), Point(1, 2)), -1.7, {});
    test_curve(QuadraticBezier(Point(-1, 0), Point(0, 1), Point(1, 0)), 1.7, {});
    test_curve(QuadraticBezier(Point(-1, 0), Point(0, 1), Point(1, 0)), -1.7, {0.17426923333331537, 1-0.17426923333331537});
    test_curve(CubicBezier(Point(-1, 0), Point(1, -1), Point(-1, 1), Point(1, 0)), 1.7, {0.122157669319538, 0.473131422069614});
    test_curve(CubicBezier(Point(-1, 0), Point(1, -1), Point(-1, 1), Point(1, 0)), -1.7, {1 - 0.473131422069614, 1 - 0.122157669319538});
    test_curve(CubicBezier(Point(-1, 0), Point(1, -2), Point(-2, -1), Point(1, 0)), 1.7, {});
    test_curve(CubicBezier(Point(-1, 0), Point(1, -2), Point(-2, -1), Point(1, 0)), -1.7, {0.16316709499671345, 0.82043191574008589});
    // degenerate cases, cubic representation of a point / line
    test_curve(CubicBezier(Point(1, 0), Point(1, 0), Point(1, 0), Point(1, 0)), 1.7, {});
    test_curve(CubicBezier(Point(1, 1), Point(2, 2), Point(1, 1), Point(2, 2)), -1.7, {});
    test_curve(CubicBezier(Point(1, 0), Point(1, 0), Point(1, 0), Point(2, 0)), 1.7, {});
}

TEST_F(BezierTest, ForwardDifferenceTest)
{
    auto b = Bezier(3, 4, 2, -5, 7);
    EXPECT_EQ(b.forward_difference(1), Bezier(19, 34, 22, 5));
    EXPECT_EQ(b.forward_difference(2), Bezier(-3, 2, 2));
}

TEST_F(BezierTest, Coincident)
{
    auto const b1 = Geom::CubicBezier({0, 0}, {1, 0}, {2, 0}, {3, 0});
    auto const b2 = Geom::CubicBezier({0, 0}, {1, 1e-9}, {2, 0}, {3, 0});
    auto const b3 = Geom::CubicBezier({0, 0}, {1, 1e-9}, {2, -1e-9}, {3, 0});
    auto const b1r = Geom::CubicBezier({3, 0}, {2, 0}, {1, 0}, {0, 0});

    // Exactly coincident - no intersections.
    EXPECT_EQ(b1.intersect(b1).size(), 0);
    EXPECT_EQ(b1r.intersect(b1).size(), 0);

    // Approximately coincident - should still have intersections.
    EXPECT_EQ(b1.intersect(b2).size(), 2);
    EXPECT_EQ(b1.intersect(b3).size(), 3);
    EXPECT_EQ(b1r.intersect(b2).size(), 2);
    // ASSERT_EQ(b1r.intersect(b3).size(), 3); // Fails, outputs 4.
}

TEST_F(BezierTest, InfiniteRecursion)
{
    auto const b = Geom::Bezier{-0.0030759119071035457, -0.0030759119071035457, 0.32441359420920435, -9.612067618408332};
    EXPECT_EQ(b.roots().size(), 0);
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
