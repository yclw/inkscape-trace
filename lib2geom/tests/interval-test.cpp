/** @file
 * @brief Unit tests for Interval, OptInterval, IntInterval, OptIntInterval.
 *//*
 * Authors:
 *   Thomas Holder
 *   PBS <pbs3141@gmail.com>
 *
 * Copyright 2021-2023 Authors
 *
 * SPDX-License-Identifier: LGPL-2.1 OR MPL-1.1
 */

#include <cassert>
#include <unordered_map>
#include <gtest/gtest.h>
#include <2geom/interval.h>

#define assert_true(x) assert(x)
#define assert_false(x) assert(!(x))

namespace Geom {

template <typename IntervalType, typename OptIntervalType>
constexpr bool equality_test()
{
    IntervalType a(3, 5), a2(a), a3, b(4, 7);
    OptIntervalType empty, oa = a, oa2;

    assert_true(a[X] == 3);
    assert_true(a[Y] == 5);
    assert_true(a == a);
    assert_false(a != a);
    assert_true(a == a2);
    assert_false(a != a2);
    a3 = a;
    assert_true(a == a3);
    assert_true(empty == empty);
    assert_false(empty != empty);
    assert_false(a == empty);
    assert_true(a != empty);
    assert_false(empty == a);
    assert_true(empty != a);
    assert_false(a == b);
    assert_true(a != b);
    assert_true(a == oa);
    assert_false(a != oa);
    oa2 = oa;
    assert_true(oa2 == oa);

    return true;
}

TEST(IntervalTest, EqualityTest)
{
    constexpr bool results[] = { equality_test<Interval, OptInterval>(),
                                 equality_test<IntInterval, OptIntInterval>() };
}

template <typename IntervalType>
constexpr bool structured_binding_test()
{
    auto a = IntervalType(1, 2);

    // Check unpacking the endpoints works.
    {
        auto [x, y] = a;
        assert_true(a[X] == x);
        assert_true(a[Y] == y);
    }

    // Ensure interval is read-only.
    {
        auto &[x, y] = a;
        assert_true(a[X] == x);
        assert_true(a[Y] == y);
        x = 3;
        y = 4;
        assert_true(a == IntervalType(1, 2));
    }

    return true;
}

TEST(IntervalTest, StructuredBindingTest)
{
    constexpr bool results[] = { structured_binding_test<Interval>(),
                                 structured_binding_test<IntInterval>() };
}

TEST(IntervalTest, Hash)
{
    auto test = [] <typename IntervalType> {
        std::unordered_map<IntervalType, int> map;
        map[IntervalType(1, 1)] = 1;
        map[IntervalType(1, 2)] = 2;
        map[IntervalType(2, 2)] = 3;
        EXPECT_EQ(map[IntervalType(1, 1)], 1);
        EXPECT_EQ(map[IntervalType(1, 2)], 2);
        EXPECT_EQ(map[IntervalType(2, 2)], 3);
    };

    test.template operator()<Interval>();
    test.template operator()<IntInterval>();
}

} // namespace Geom

// vim: filetype=cpp:expandtab:shiftwidth=4:softtabstop=4:fileencoding=utf-8:textwidth=99 :
