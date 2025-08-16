/** @file
 * @brief Unit tests for Parallelogram
 *
 * Includes all tests from RotatedRect to demonstrate that it is a generalized
 * version of the rotated rectangle.
 */
/*
 * Authors:
 *   Thomas Holder
 *   Sergei Izmailov
 *
 * SPDX-License-Identifier: LGPL-2.1 or MPL-1.1
 */

#include <2geom/coord.h>
#include <2geom/parallelogram.h>
#include <2geom/transforms.h>

#include <gtest/gtest.h>

using namespace Geom;

// Analogous to RotatedRect::from_rect_rotate
static Parallelogram parallelogram_from_rect_rotate(Rect const &rect, Rotate const &rotate, Point const &point)
{
    Affine affine = Translate(-point) * rotate * Translate(point);
    return Parallelogram(rect) * affine;
}
static Parallelogram parallelogram_from_rect_rotate(Rect const &rect, Rotate const &rotate)
{
    return parallelogram_from_rect_rotate(rect, rotate, rect.midpoint());
}

TEST(ParallelogramTest, midpoint)
{
    Rect r(-0.5, -0.5, 5.5, 5.5);
    auto center = Point(2.5, 2.5);

    EXPECT_EQ(r.midpoint(), center);
    for (double angle : { 0, 1, 25, 45, 90, 135 }) {
        auto rotated_rect = parallelogram_from_rect_rotate(r, Rotate::from_degrees(angle), Point(0, 0));
        auto rotated_center = center * Rotate(angle / 180.0 * M_PI);
        EXPECT_TRUE(Geom::are_near(rotated_rect.midpoint(), rotated_center, 1e-6)) << "Angle = " << angle << " deg";
    }
}

TEST(ParallelogramTest, containsPoint1)
{
    Rect r(0, 0, 1, 1);
    auto rotated_rect = r;
    EXPECT_TRUE(rotated_rect.contains(Point(0, 0)));
    EXPECT_TRUE(rotated_rect.contains(Point(1, 1)));
    EXPECT_TRUE(rotated_rect.contains(Point(0.5, 0.5)));
    EXPECT_FALSE(rotated_rect.contains(Point(1.1, 0.5)));
    EXPECT_FALSE(rotated_rect.contains(Point(0.5, 1.1)));
}

TEST(ParallelogramTest, containsPoint2)
{
    Rect r(0, 0, 1, 1);
    auto rotated_rect = parallelogram_from_rect_rotate(r, Rotate::from_degrees(45), Point(0, 0));
    EXPECT_TRUE(rotated_rect.contains(Point(0, 0)));
    EXPECT_TRUE(rotated_rect.contains(Point(0, 1.2)));
    EXPECT_TRUE(rotated_rect.contains(Point(0.5, 0.9)));
    EXPECT_FALSE(rotated_rect.contains(Point(1, 1)));
    EXPECT_FALSE(rotated_rect.contains(Point(0.1, 0)));
}

TEST(ParallelogramTest, intersects_aligned)
{
    Rect r(0, 0, 1, 1);
    auto rotated_rect = r;
    // point within rect
    EXPECT_TRUE(rotated_rect.intersects(Rect(-1, -1, 2, 2)));
    EXPECT_TRUE(rotated_rect.intersects(Rect(0.1, 0.1, 0.2, 0.2)));
    EXPECT_TRUE(rotated_rect.intersects(Rect(-0.1, -0.1, 0.1, 0.1)));
    EXPECT_FALSE(rotated_rect.intersects(Rect(-0.2, -0.2, -0.1, -0.1)));
    EXPECT_FALSE(rotated_rect.intersects(Rect(1.1, 1.1, 1.2, 1.2)));
    // edge intersection
    EXPECT_TRUE(rotated_rect.intersects(Rect(0.5, -0.1, 0.6, 1.2)));
    EXPECT_TRUE(rotated_rect.intersects(Rect(-0.1, 0.5, 1.2, 0.6)));
}

TEST(ParallelogramTest, bounds)
{
    auto r = Rect::from_xywh(1.260, 0.547, 8.523, 11.932);
    auto rrect = parallelogram_from_rect_rotate(r, Rotate::from_degrees(15.59));
    auto bbox = rrect.bounds();
    auto expected_bbox = Rect::from_xywh(-0.186, -0.378, 11.415, 13.783);
    for (int i = 0; i < 4; i++) {
        EXPECT_TRUE(Geom::are_near(bbox.corner(i), expected_bbox.corner(i), 1e-3));
    }
}

TEST(ParallelogramTest, isSheared)
{
    Parallelogram p(Rect(2, 4, 7, 8));
    EXPECT_FALSE(p.isSheared());
    p *= Rotate(M_PI / 4.0); // 45°
    EXPECT_FALSE(p.isSheared());
    p *= HShear(2);
    EXPECT_TRUE(p.isSheared());
}

TEST(ParallelogramTest, area)
{
    Rect r(2, 4, 7, 8);
    Parallelogram p(r);
    EXPECT_DOUBLE_EQ(p.area(), r.area());
    p *= Rotate(M_PI / 4.0); // 45°
    EXPECT_DOUBLE_EQ(p.area(), r.area());
    p *= HShear(2);
    EXPECT_DOUBLE_EQ(p.area(), r.area());
    p *= Scale(2);
    EXPECT_DOUBLE_EQ(p.area(), r.area() * 4);
}

class ParallelogramTest
    : public testing::TestWithParam<std::tuple<Rect /*rect*/, double /*degrees*/, bool /*intersects*/>> {

    void SetUp() override { target = Rect::from_xywh(0, 0, 11, 13); }

  public:
    Rect target;
};

TEST_P(ParallelogramTest, intersects)
{
    Rect rect;
    double degrees;
    bool intersects;
    std::tie(rect, degrees, intersects) = GetParam();
    EXPECT_EQ(parallelogram_from_rect_rotate(rect, Rotate::from_degrees(degrees)).intersects(target), intersects)
        << "ERROR: rect {" << rect << "} rotated by {" << degrees << "} degrees " << (!intersects ? "" : "NOT ")
        << "intersects with {" << target << "} but MUST " << (intersects ? "" : "NOT");
}

// clang-format off
INSTANTIATE_TEST_CASE_P(intesect_non_aligned, ParallelogramTest,
    testing::Values(
      std::make_tuple(Rect::from_xywh(10.456, -4.479, 7, 5), 0, true),
      std::make_tuple(Rect::from_xywh(10.456, -4.479, 7, 5), 15, false),
      std::make_tuple(Rect::from_xywh(9.929, 12.313, 7, 5), 93.2, false),
      std::make_tuple(Rect::from_xywh(9.929, 12.313, 7, 5), 91.37, true),
      std::make_tuple(Rect::from_xywh(-1, 4, 13, 3), 0, true),
      std::make_tuple(Rect::from_xywh(4, -2, 3, 16), 0, true),
      std::make_tuple(Rect::from_xywh(-5.113, -3.283, 5.000, 7.000), 11.81, false),
      std::make_tuple(Rect::from_xywh(-5.113, -3.283, 5.000, 7.000), 13.35, true),
      std::make_tuple(Rect::from_xywh(1.260, 0.547, 8.523, 11.932), 15.59, true),
      std::make_tuple(Rect::from_xywh(5.328, 0.404, 11, 2), 28.16, true),
      std::make_tuple(Rect::from_xywh(4.853, 10.691, 11, 2), -30.4, true),
      std::make_tuple(Rect::from_xywh(-4.429, 10.752, 11, 2), 29.7, true),
      std::make_tuple(Rect::from_xywh(-4.538, 0.314, 11, 2), -34.19, true),
      std::make_tuple(Rect::from_xywh(8.398, -3.790, 2, 11), -34, true),
      std::make_tuple(Rect::from_xywh(8.614, 6.163, 2, 11), 30.38, true),
      std::make_tuple(Rect::from_xywh(0.492, 6.904, 2, 11), -37.29, true),
      std::make_tuple(Rect::from_xywh(0.202, -3.148, 2, 11), 31.12, true)));

// clang-format on

// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
