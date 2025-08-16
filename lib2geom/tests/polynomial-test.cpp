/** @file
 * @brief Unit tests for Polynomial and related functions.
 * Uses the Google Testing Framework
 *//*
 * Authors:
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2015-2019 Authors
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
#include <array>
#include <iostream>
#include <glib.h>

#include <2geom/polynomial.h>

using namespace Geom;

TEST(PolynomialTest, SolveQuadratic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2);
        Coord c = a * x1 * x2;

        std::vector<Coord> result = solve_quadratic(a, b, c);

        EXPECT_EQ(result.size(), 2u);
        if (x1 < x2) {
            EXPECT_FLOAT_EQ(result[0], x1);
            EXPECT_FLOAT_EQ(result[1], x2);
        } else {
            EXPECT_FLOAT_EQ(result[0], x2);
            EXPECT_FLOAT_EQ(result[1], x1);
        }
    }
}

TEST(PolynomialTest, SolvePathologicalQuadratic) {
    std::vector<Coord> r;

    r = solve_quadratic(1, -1e9, 1);
    ASSERT_EQ(r.size(), 2u);
    EXPECT_FLOAT_EQ(r[0], 1e-9);
    EXPECT_FLOAT_EQ(r[1], 1e9);

    r = solve_quadratic(1, -4, 3.999999);
    ASSERT_EQ(r.size(), 2u);
    EXPECT_FLOAT_EQ(r[0], 1.999);
    EXPECT_FLOAT_EQ(r[1], 2.001);

    r = solve_quadratic(1, 0, -4);
    ASSERT_EQ(r.size(), 2u);
    EXPECT_FLOAT_EQ(r[0], -2);
    EXPECT_FLOAT_EQ(r[1], 2);

    r = solve_quadratic(1, 0, -16);
    ASSERT_EQ(r.size(), 2u);
    EXPECT_FLOAT_EQ(r[0], -4);
    EXPECT_FLOAT_EQ(r[1], 4);

    r = solve_quadratic(1, 0, -100);
    ASSERT_EQ(r.size(), 2u);
    EXPECT_FLOAT_EQ(r[0], -10);
    EXPECT_FLOAT_EQ(r[1], 10);
}

TEST(PolynomialTest, SolveCubic) {
    for (unsigned i = 0; i < 1000; ++i) {
        Coord x1 = g_random_double_range(-100, 100);
        Coord x2 = g_random_double_range(-100, 100);
        Coord x3 = g_random_double_range(-100, 100);

        Coord a = g_random_double_range(-10, 10);
        Coord b = -a * (x1 + x2 + x3);
        Coord c = a * (x1*x2 + x2*x3 + x1*x3);
        Coord d = -a * x1 * x2 * x3;

        std::vector<Coord> result = solve_cubic(a, b, c, d);
        std::vector<Coord> x(3); x[0] = x1; x[1] = x2; x[2] = x3;
        std::sort(x.begin(), x.end());

        ASSERT_EQ(result.size(), 3u);
        EXPECT_FLOAT_EQ(result[0], x[0]);
        EXPECT_FLOAT_EQ(result[1], x[1]);
        EXPECT_FLOAT_EQ(result[2], x[2]);
    }

    // corner cases
    // (x^2 + 7)(x - 2)
    std::vector<Coord> r1 = solve_cubic(1, -2, 7, -14);
    EXPECT_EQ(r1.size(), 1u);
    EXPECT_FLOAT_EQ(r1[0], 2);

    // (x + 1)^2 (x-2)
    std::vector<Coord> r2 = solve_cubic(1, 0, -3, -2);
    ASSERT_EQ(r2.size(), 3u);
    EXPECT_FLOAT_EQ(r2[0], -1);
    EXPECT_FLOAT_EQ(r2[1], -1);
    EXPECT_FLOAT_EQ(r2[2], 2);
}

/// Check the correctness of the degree-4 equation solver on quartics with 4 roots.
TEST(PolynomialTest, SolveQuartic_4_roots)
{
    g_random_set_seed(0xB737A380);
    double const eps = 4e-6;
    std::array<Coord, 4> roots;

    for (size_t _ = 0; _ < 10'000; ++_) {
        // Generate random but sorted roots
        for (Coord &root : roots) {
            root = g_random_double_range(-100, 100);
        }
        std::sort(roots.begin(), roots.end());

        // Generate random leading coefficient
        Coord a = 0;
        while (a == 0) {
            a = g_random_double_range(-100, 100);
        }

        // Generate symmetric basis polynomials in roots
        Coord const sym1 = roots[0] + roots[1] + roots[2] + roots[3];
        Coord const sym2 = roots[0] * roots[1] +
                           roots[0] * roots[2] +
                           roots[0] * roots[3] +
                           roots[1] * roots[2] +
                           roots[1] * roots[3] +
                           roots[2] * roots[3];
        Coord const sym3 = roots[0] * roots[1] * roots[2] +
                           roots[0] * roots[1] * roots[3] +
                           roots[0] * roots[2] * roots[3] +
                           roots[1] * roots[2] * roots[3];
        Coord const sym4 = roots[0] * roots[1] * roots[2] * roots[3];

        // Try to recover roots from the coefficients of the polynomial
        auto const recovered = solve_quartic(a, -a * sym1, a * sym2, -a * sym3, a * sym4);
        ASSERT_EQ(recovered.size(), 4);
        for (size_t i = 0; i < 4; ++i) {
            EXPECT_TRUE(are_near(recovered[i], roots[i], eps));
        }
    }
}

/// Check the evaluations of a random degree 4 polynomial at the roots found by the solver.
TEST(PolynomialTest, SolveQuartic_Evaluate)
{
    g_random_set_seed(0xB737A380);
    double const eps = 3e-6;
    for (size_t _ = 0; _ < 10'000; ++_) {
        Poly quartic;
        for (size_t i = 0; i < 4; ++i) {
            quartic.push_back(g_random_double_range(-100, 100));
        }
        quartic.push_back(1);

        auto const roots = solve_quartic(quartic[4], quartic[3], quartic[2], quartic[1], quartic[0]);
        for (Coord root : roots) {
            EXPECT_TRUE(are_near(quartic.eval(root), 0, eps));
        }
    }
}

/// Return the coefficients of a random irreducible quadratic polynomial.
static std::array<Coord, 3> get_random_irreducible_quadratic()
{
    double a = 0;
    while (std::abs(a) < 1e-3) {
        a = g_random_double_range(-10, 10);
    }
    double b = g_random_double_range(-10, 10);
    double c = g_random_double_range(1, 100);
    int sign = 2 * g_random_boolean() - 1;
    return {
        sign * sqr(a),
        sign * 2 * a * b,
        sign * (sqr(b) + c)
    };
}

/// Check the correctness of the degree-4 equation solver on quartics with 2 roots.
TEST(PolynomialTest, SolveQuartic_2_roots)
{
    g_random_set_seed(0xB737A380);
    double const eps = 1e-9;

    std::array<Coord, 2> roots;
    for (size_t _ = 0; _ < 10'000; ++_) {
        // Generate random but sorted roots
        for (Coord &root : roots) {
            root = g_random_double_range(-100, 100);
        }
        std::sort(roots.begin(), roots.end());

        // Generate symmetric polynomials
        Coord const sym1 = roots[0] + roots[1];
        Coord const sym2 = roots[0] * roots[1];
        auto const comp = get_random_irreducible_quadratic();

        // Try to recover roots from the coefficients of the polynomial
        auto const recovered = solve_quartic(comp[0],
                                             comp[1] - comp[0] * sym1,
                                             comp[2] + sym2 * comp[0] - comp[1] * sym1,
                                             comp[1] * sym2 - comp[2] * sym1,
                                             comp[2] * sym2);
        ASSERT_EQ(recovered.size(), 2);
        for (size_t i = 0; i < 2; ++i) {
            ASSERT_TRUE(are_near(recovered[i], roots[i], eps));
        }
    }
}

/// Check the correctness of the degree-4 equation solver in the presence of double roots.
TEST(PolynomialTest, SolveQuartic_DoubleRoots)
{
    g_random_set_seed(123456789);
    double const eps = 4e-5;
    std::array<Coord, 4> roots;

    for (size_t _ = 0; _ < 1000; ++_) {
        // Generate random sorted roots, including a double root
        for (size_t i = 0; i < 3; ++i) {
            roots[i] = g_random_double_range(-100, 100);
        }
        roots[3] = roots[g_random_int_range(0, 3)];

        // Generate random leading coefficient
        Coord a = 0;
        while (a == 0) {
            a = g_random_double_range(-100, 100);
        }

        // Generate symmetric basis polynomials in roots
        Coord const sym1 = roots[0] + roots[1] + roots[2] + roots[3];
        Coord const sym2 = roots[0] * roots[1] +
                           roots[0] * roots[2] +
                           roots[0] * roots[3] +
                           roots[1] * roots[2] +
                           roots[1] * roots[3] +
                           roots[2] * roots[3];
        Coord const sym3 = roots[0] * roots[1] * roots[2] +
                           roots[0] * roots[1] * roots[3] +
                           roots[0] * roots[2] * roots[3] +
                           roots[1] * roots[2] * roots[3];
        Coord const sym4 = roots[0] * roots[1] * roots[2] * roots[3];

        // Try to recover roots from the coefficients of the polynomial
        auto const recovered = solve_quartic(a, -a * sym1, a * sym2, -a * sym3, a * sym4);
        for (Coord found : recovered) {
            double best_relative_error = infinity();
            for (Coord root : roots) {
                best_relative_error = std::min(best_relative_error, std::abs((found - root) / root));
            }
            EXPECT_LE(best_relative_error, eps);
        }
    }
}

/// Check the correctness of the degree-4 equation solver on quartics without any roots.
TEST(PolynomialTest, SolveQuartic_0_roots)
{
    g_random_set_seed(0xA380B737);
    for (size_t _ = 0; _ < 10'000; ++_) {
        // Create two irreducible quadratics
        auto const &[a1, b1, c1] = get_random_irreducible_quadratic();
        auto const &[a2, b2, c2] = get_random_irreducible_quadratic();

        // Try to recover roots from the product of those two quadratics
        auto const recovered = solve_quartic(a1 * a2,
                                             a1 * b2 + b1 * a2,
                                             a1 * c2 + b1 * b2 + c1 * a2,
                                             b1 * c2 + c1 * b2,
                                             c1 * c2);
        EXPECT_TRUE(recovered.empty());
    }
}

/// Check the correctness of the degree-4 equation solver in degenerate cases.
TEST(PolynomialTest, SolveQuartic_degenerate)
{
    g_random_set_seed(0xB737A380);
    for (size_t _ = 0; _ < 10'000; ++_) {
        auto const b = g_random_double_range(-100, 100);
        auto const c = g_random_double_range(-100, 100);
        auto const d = g_random_double_range(-100, 100);
        auto const e = g_random_double_range(-100, 100);

        // Check the leading coefficient being 0
        auto const degen1 = solve_quartic(0, b, c, d, e);
        auto const as_cubic = solve_cubic(b, c, d, e);
        EXPECT_EQ(degen1, as_cubic);

        // Check first 2 coefficients being zero
        auto const degen2 = solve_quartic(0, 0, c, d, e);
        auto const as_quadratic = solve_quadratic(c, d, e);
        EXPECT_EQ(degen2, as_quadratic);

        double a = 0;
        while (std::abs(a) < 1e-3) {
            a = g_random_double_range(-100, 100);
        }

        // Check the case of a cubic polynomial multiplied by x
        auto const degen3 = solve_quartic(a, b, c, d, 0);
        auto cubic_and_zero = solve_cubic(a, b, c, d);
        cubic_and_zero.push_back(0);
        std::sort(cubic_and_zero.begin(), cubic_and_zero.end());
        EXPECT_EQ(degen3, cubic_and_zero);

        // Check the case of a quadratic polynomial multiplied by x^2
        auto const degen4 = solve_quartic(a, b, c, 0, 0);
        auto quad_and_zero = solve_quadratic(a, b, c);
        quad_and_zero.push_back(0);
        quad_and_zero.push_back(0);
        std::sort(quad_and_zero.begin(), quad_and_zero.end());
        EXPECT_EQ(degen4, quad_and_zero);
    }
}