// SPDX-License-Identifier: GPL-2.0-or-later
#include <2geom/convex-hull.h>
#include <2geom/transforms.h>
#include <gtest/gtest.h>
#include <glib.h>

namespace Geom {

// Get the axis-aligned bouding box of a set of points, transforming by affine first.
auto aligned_bbox(std::vector<Point> const &pts, Affine const &affine = identity())
{
    OptRect rect;
    for (auto &pt : pts) {
        rect.expandTo(pt * affine);
    }
    return rect;
}

// Get an approximation to the minimum bouding box area.
double approx_min(std::vector<Point> const &pts)
{
    int constexpr N = 100;

    double min = std::numeric_limits<double>::max();

    for (int i = 0; i < N; i++) {
        auto t = (double)i / N * M_PI * 0.5;
        min = std::min(min, aligned_bbox(pts, Rotate(t)).area());
    }

    return min;
}

// Get a random collection of points.
auto randpts()
{
    std::vector<Point> pts;

    int count = 5 + (g_random_int() % 10);
    for (int i = 0; i < count; i++) {
        pts.emplace_back(g_random_double(), g_random_double());
    }

    return pts;
}

TEST(MinBBoxTest, Empty)
{
    auto const hull = ConvexHull();
    auto [rotation, optrect] = hull.minAreaRotation();
    EXPECT_FALSE(optrect);
}

TEST(MinBBoxTest, SinglePoint)
{
    auto const hull = ConvexHull(Point(0, 0));
    auto [rotation, optrect] = hull.minAreaRotation();
    EXPECT_EQ(optrect, Rect::from_xywh(0, 0, 0, 0));
}

TEST(MinBBoxTest, Randomised)
{
    g_random_set_seed(0xdeadbeef);

    for (int i = 0; i < 100; i++) {
        auto const pts = randpts();
        auto const hull = ConvexHull(pts);

        auto [rotation, optrect] = hull.minAreaRotation();

        // The point set is never empty, so the returned bounds should not be empty.
        ASSERT_TRUE(optrect);

        // Ensure that the returned bounds is indeed the bounds at the returned rotation.
        auto rect2 = aligned_bbox(pts, rotation);
        for (int i = 0; i < 2; i++) {
            ASSERT_NEAR(optrect->min()[i], rect2->min()[i], 1e-5);
            ASSERT_NEAR(optrect->max()[i], rect2->max()[i], 1e-5);
        }

        // Ensure that no other rotations give tighter bounds than the returned rotation.
        ASSERT_LE(optrect->area(), approx_min(pts));
    }
}

} // namespace Geom
