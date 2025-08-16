/**
 * @file
 * @brief  Unit tests for the ArithmeticInterval - Wrapper for boost::numeric::interval
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

#include <gtest/gtest.h>
#include <2geom/arithmetic-interval-utils.h>
#include <2geom/arithmetic-interval.h>
#include <cmath>

namespace Geom {

TEST(ArithmeticIntervalTest, DefaultConstructor) {
    ArithmeticInterval interval;
    EXPECT_EQ(interval.min(), 0.0);
    EXPECT_EQ(interval.max(), 0.0);
    EXPECT_TRUE(interval.isSingular());
}

TEST(ArithmeticIntervalTest, SingleValueConstructor) {
    ArithmeticInterval interval(5.0);
    EXPECT_EQ(interval.min(), 5.0);
    EXPECT_EQ(interval.max(), 5.0);
    EXPECT_TRUE(interval.isSingular());
}

TEST(ArithmeticIntervalTest, RangeConstructor) {
    ArithmeticInterval interval(1.0, 5.0);
    EXPECT_EQ(interval.min(), 1.0);
    EXPECT_EQ(interval.max(), 5.0);
    EXPECT_EQ(interval.extent(), 4.0);
    EXPECT_EQ(interval.middle(), 3.0);
}

TEST(ArithmeticIntervalTest, ContainsValue) {
    ArithmeticInterval interval(1.0, 5.0);
    EXPECT_TRUE(interval.contains(3.0));
    EXPECT_FALSE(interval.contains(0.0));
    EXPECT_FALSE(interval.contains(6.0));
}

TEST(ArithmeticIntervalTest, ContainsInterval) {
    ArithmeticInterval a(1.0, 5.0);
    ArithmeticInterval b(2.0, 4.0);
    ArithmeticInterval c(0.0, 6.0);
    EXPECT_TRUE(a.contains(b));
    EXPECT_FALSE(a.contains(c));
}

TEST(ArithmeticIntervalTest, Intersects) {
    ArithmeticInterval a(1.0, 5.0);
    ArithmeticInterval b(4.0, 6.0);
    ArithmeticInterval c(6.0, 8.0);
    EXPECT_TRUE(a.intersects(b));
    EXPECT_FALSE(a.intersects(c));
}

TEST(ArithmeticIntervalTest, Addition) {
    ArithmeticInterval a(1.0, 2.0);
    ArithmeticInterval b(3.0, 4.0);
    ArithmeticInterval result = a + b;
    EXPECT_EQ(result.min(), 4.0);
    EXPECT_EQ(result.max(), 6.0);

    result = 1.0 + b;
    EXPECT_EQ(result.min(), 4.0);
    EXPECT_EQ(result.max(), 5.0);
}

TEST(ArithmeticIntervalTest, Subtraction) {
    ArithmeticInterval a(1.0, 2.0);
    ArithmeticInterval b(3.0, 4.0);
    ArithmeticInterval result = a - b;
    EXPECT_EQ(result.min(), -3.0);
    EXPECT_EQ(result.max(), -1.0);

    result = 1.0 - b;
    EXPECT_EQ(result.min(), -3.0);
    EXPECT_EQ(result.max(), -2.0);
}

TEST(ArithmeticIntervalTest, ScalarMultiplication) {
    ArithmeticInterval a(1.0, 2.0);
    ArithmeticInterval result = a * 2.0;
    EXPECT_EQ(result.min(), 2.0);
    EXPECT_EQ(result.max(), 4.0);
}

TEST(ArithmeticIntervalTest, ScalarDivision) {
    ArithmeticInterval a(2.0, 4.0);
    ArithmeticInterval result = a / 2.0;
    EXPECT_EQ(result.min(), 1.0);
    EXPECT_EQ(result.max(), 2.0);
}

TEST(ArithmeticIntervalTest, ComparisonOperators) {
    ArithmeticInterval a(5.0);
    ArithmeticInterval b(5.0);
    ArithmeticInterval c(5.0, 5.0);
    ArithmeticInterval d(1.0, 2.0);
    ArithmeticInterval e(1.0, 2.0);

    EXPECT_EQ(a, b);
    EXPECT_EQ(a, c);
    EXPECT_EQ(d, e);

    EXPECT_NE(a, d);

    EXPECT_LT(e, a);
    EXPECT_GT(a, e);

    EXPECT_LE(a, b);
    EXPECT_GE(a, b);
    EXPECT_GE(a, d);
    EXPECT_LE(d, a);
}

TEST(ArithmeticIntervalTest, AssignmentOperators) {
    ArithmeticInterval a(1.0, 2.0);
    ArithmeticInterval b(3.0, 4.0);

    a += b;
    EXPECT_EQ(a.min(), 4.0);
    EXPECT_EQ(a.max(), 6.0);

    a = ArithmeticInterval(1.0, 2.0);
    a -= b;
    EXPECT_EQ(a.min(), -3.0);
    EXPECT_EQ(a.max(), -1.0);

    a = ArithmeticInterval(1.0, 2.0);
    a *= 2.0;
    EXPECT_EQ(a.min(), 2.0);
    EXPECT_EQ(a.max(), 4.0);

    a = ArithmeticInterval(1.0, 2.0);
    a /= 2.0;
    EXPECT_EQ(a.min(), 0.5);
    EXPECT_EQ(a.max(), 1.0);
}

TEST(ArithmeticIntervalTest, Intersection) {
    ArithmeticInterval a(1.0, 5.0);
    ArithmeticInterval b(3.0, 7.0);
    ArithmeticInterval result = ArithmeticInterval::intersection(a, b);
    EXPECT_EQ(result.min(), 3.0);
    EXPECT_EQ(result.max(), 5.0);
}

} // namespace Geom
