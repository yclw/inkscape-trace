/**
 * @file
 * @brief Interval arithmetic based intersections
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

#include <2geom/arithmetic-interval-utils.h>
#include <2geom/arithmetic-interval.h>
#include <2geom/bezier-curve.h>
#include <2geom/geometric-intersection.h>
#include <cmath>

namespace Geom {
namespace {

double clamp_to_bound(double t) { return std::clamp(t, 0.0, 1.0); }

double get_nearest_endpoint(double t) { return (t <= 0.5) ? 0.0 : 1.0; }

GeometricIntersection create_intersection(Coord t, Coord s, GeometricIntersection::Type type)
{
    return GeometricIntersection{ .type = type, .t = clamp_to_bound(t), .s = clamp_to_bound(s) };
}

GeometricIntersection make_point_intersection(double t, double s)
{
    return create_intersection(t, s, GeometricIntersection::Type::POINT);
}

void add_overlap(std::vector<GeometricIntersection> &result, double t1, double t2, double s1, double s2)
{
    result.push_back(create_intersection(t1, s1, GeometricIntersection::Type::OVERLAP_END_POINT));
    result.push_back(create_intersection(t2, s2, GeometricIntersection::Type::OVERLAP_END_POINT));
}

void add_degen_overlap(std::vector<GeometricIntersection> &result, double t1, bool seg1_is_degen)
{
    if (seg1_is_degen) {
        add_overlap(result, 0.0, 1.0, t1, t1);
    } else {
        add_overlap(result, t1, t1, 0.0, 1.0);
    }
}

ArithmeticInterval project(ArithmeticInterval dx, ArithmeticInterval dy, ArithmeticInterval vx, ArithmeticInterval vy,
                           ArithmeticInterval denom)
{
    return (vx * dx + vy * dy) / denom;
}

ArithmeticInterval distance_between_points(Point const &a, Point const &b)
{
    return l2(ArithmeticInterval(a[X]) - ArithmeticInterval(b[X]), ArithmeticInterval(a[Y]) - ArithmeticInterval(b[Y]));
}

/**
 * @brief Handle intersection between two collinear segments.
 *
 * This function detects whether two collinear segments:
 * - Overlap partially or completely
 * - Are close enough to be considered intersecting at a single point
 *
 * @param result Vector to store intersections.
 * @param seg1_t1 Param for seg2.start projected onto seg1.
 * @param seg1_t2 Param for seg2.end projected onto seg1.
 * @param seg2_t1 Param for seg1.start projected onto seg2.
 * @param seg2_t2 Param for seg1.end projected onto seg2.
 * @param seg1_dx Direction of segment 1 in x.
 * @param seg2_dx Direction of segment 2 in x.
 * @param seg1 First line segment.
 * @param seg2 Second line segment.
 * @param epsilon Tolerance threshold.
 */
void handle_collinear_segments(std::vector<GeometricIntersection> &result, ArithmeticInterval seg1_t1,
                               ArithmeticInterval seg1_t2, ArithmeticInterval seg2_t1, ArithmeticInterval seg2_t2,
                               ArithmeticInterval seg1_dx, ArithmeticInterval seg2_dx, LineSegment const &seg1,
                               LineSegment const &seg2, double epsilon)
{
    auto const t_start = max(min(seg1_t1, seg1_t2), 0.0);
    auto const t_end = min(max(seg1_t1, seg1_t2), 1.0);
    auto const s_start = max(min(seg2_t1, seg2_t2), 0.0);
    auto const s_end = min(max(seg2_t1, seg2_t2), 1.0);

    auto const endpoint_dist = distance(seg1.pointAt(t_start.middle()), seg1.pointAt(t_end.middle()));

    if (endpoint_dist <= epsilon) {
        double const endpoint_t = get_nearest_endpoint(t_start.middle());
        double const endpoint_s = get_nearest_endpoint(s_start.middle());
        // Get the end points nearest to the overlap and check the distance between those endpoints
        result.push_back(make_point_intersection(endpoint_t, endpoint_s));

    } else if (t_end > t_start && s_end > s_start) {
        // Partial or total overlap
        double s1 = s_start.middle(), s2 = s_end.middle();
        double t1 = t_start.middle(), t2 = t_end.middle();

        // If a segment is in a negative direction, reverse their overlap time values
        if (seg2_dx.middle() < 0.0)
            std::swap(s1, s2);
        if (seg1_dx.middle() < 0.0)
            std::swap(t1, t2);

        add_overlap(result, t1, t2, s1, s2);
    }
}

/**
 * @brief Handle intersection between a degenerate and a normal line segment.
 *
 * This function checks if a degenerate segment (treated as a point or near-zero length segment)
 * intersects a normal segment by projecting its endpoints and checking distance within bounds.
 *
 * @param result Vector to store intersections.
 * @param degenerate_seg The degenerate segment (point-like).
 * @param normal_seg The non-degenerate segment.
 * @param non_degen_dx Direction of the normal segment in x.
 * @param non_degen_dy Direction of the normal segment in y.
 * @param non_degen_sqr_length Squared length of the normal segment.
 * @param epsilon Tolerance threshold.
 * @param bounds Valid [0,1] parameter range for projection.
 * @param seg1_is_degenerate True if the first segment is degenerate.
 */
void handle_single_degenerate_segment(std::vector<GeometricIntersection> &result, const LineSegment &degenerate_seg,
                                      const LineSegment &normal_seg, ArithmeticInterval non_degen_dx,
                                      ArithmeticInterval non_degen_dy, ArithmeticInterval non_degen_sqr_length,
                                      double epsilon, ArithmeticInterval bounds, bool seg1_is_degenerate)
{
    auto const degen_x1 = ArithmeticInterval(degenerate_seg.initialPoint()[X]);
    auto const degen_y1 = ArithmeticInterval(degenerate_seg.initialPoint()[Y]);
    auto const degen_x2 = ArithmeticInterval(degenerate_seg.finalPoint()[X]);
    auto const degen_y2 = ArithmeticInterval(degenerate_seg.finalPoint()[Y]);
    auto const non_degen_x1 = ArithmeticInterval(normal_seg.initialPoint()[X]);
    auto const non_degen_y1 = ArithmeticInterval(normal_seg.initialPoint()[Y]);

    // Project both endpoints of degenerate segment onto non degenerate segment
    auto const t1 =
        project(non_degen_dx, non_degen_dy, degen_x1 - non_degen_x1, degen_y1 - non_degen_y1, non_degen_sqr_length);
    auto const t2 =
        project(non_degen_dx, non_degen_dy, degen_x2 - non_degen_x1, degen_y2 - non_degen_y1, non_degen_sqr_length);

    ArithmeticInterval dist1, dist2;
    bool t1_is_close = false, t2_is_close = false;

    if (t1.intersects(bounds)) {
        dist1 = distance_between_points(Point(degen_x1.middle(), degen_y1.middle()), normal_seg.pointAt(t1.middle()));
        t1_is_close = is_small(dist1, epsilon);
    }

    if (t2.intersects(bounds)) {
        dist2 = distance_between_points(Point(degen_x2.middle(), degen_y2.middle()), normal_seg.pointAt(t2.middle()));
        t2_is_close = is_small(dist2, epsilon);
    }

    // Handle intersection cases
    if (t1_is_close && t2_is_close) {
        // Both endpoints are close - add midpoint
        auto const t_mid = (t1 + t2) / 2.0;
        add_degen_overlap(result, t_mid.middle(), seg1_is_degenerate);
    } else if (t1_is_close) {
        // Only first endpoint is close
        add_degen_overlap(result, t1.middle(), seg1_is_degenerate);
    } else if (t2_is_close) {
        // Only second endpoint is close
        add_degen_overlap(result, t2.middle(), seg1_is_degenerate);
    }
}


} // namespace

std::vector<GeometricIntersection> intersect(LineSegment const &seg1, LineSegment const &seg2, double epsilon)
{
    std::vector<GeometricIntersection> result;

    ArithmeticInterval const bounds(0.0, 1.0);
    ArithmeticInterval const p1x(seg1.initialPoint()[X]);
    ArithmeticInterval const p1y(seg1.initialPoint()[Y]);
    ArithmeticInterval const p2x(seg1.finalPoint()[X]);
    ArithmeticInterval const p2y(seg1.finalPoint()[Y]);

    ArithmeticInterval const q1x(seg2.initialPoint()[X]);
    ArithmeticInterval const q1y(seg2.initialPoint()[Y]);
    ArithmeticInterval const q2x(seg2.finalPoint()[X]);
    ArithmeticInterval const q2y(seg2.finalPoint()[Y]);

    auto const seg1_dx = p2x - p1x;
    auto const seg1_dy = p2y - p1y;
    auto const seg2_dx = q2x - q1x;
    auto const seg2_dy = q2y - q1y;

    auto const length_seg1 = l2(seg1_dx, seg1_dy);
    auto const length_seg2 = l2(seg2_dx, seg2_dy);

    auto const length_sqr_seg1 = square(seg1_dx) + square(seg1_dy);
    auto const length_sqr_seg2 = square(seg2_dx) + square(seg2_dy);

    if (is_small(length_seg1, epsilon) || is_small(length_seg2, epsilon)) {

        if (is_small(length_seg1, epsilon) && is_small(length_seg2, epsilon)) {
            // Both segments are degenerate
            auto const dist = distance(seg1.initialPoint(), seg2.initialPoint());

            if (dist <= epsilon) {
                // total overlap if their distance is less than epsilon
                add_overlap(result, 0.0, 1.0, 0.0, 1.0);
            }
            return result;
        }
        if (is_small(length_seg1, epsilon)) {
            // only seg1 is degenerate
            handle_single_degenerate_segment(result, seg1, seg2, seg2_dx, seg2_dy, length_sqr_seg2, epsilon, bounds,
                                             true);
            return result;
        } else if (is_small(length_seg2, epsilon)) {
            // only seg2 is degenerate
            handle_single_degenerate_segment(result, seg2, seg1, seg1_dx, seg1_dy, length_sqr_seg1, epsilon, bounds,
                                             false);
            return result;
        }
    }

    auto const cross = seg1_dy * seg2_dx - seg1_dx * seg2_dy;

    if (cross.contains(0.0)) {
        // Segments are parallel
        auto const cross_v = (q1x - p1x) * seg1_dy - (q1y - p1y) * seg1_dx;
        if (cross_v.contains(0.0)) {
            // Segments are collinear
            auto const t_seg1_q1 = (q1x - p1x) / seg1_dx;
            auto const t_seg1_q2 = (q2x - p1x) / seg1_dx;
            auto const t_seg2_p1 = (p1x - q1x) / seg2_dx;
            auto const t_seg2_p2 = (p2x - q1x) / seg2_dx;

            handle_collinear_segments(result, t_seg1_q1, t_seg1_q2, t_seg2_p1, t_seg2_p2, seg1_dx, seg2_dx, seg1, seg2,
                                      epsilon);


        } else {
            // Segments are parallel. Check if their distance is less than epsilon
            auto const dist = abs(cross_v) / length_seg1;

            if (is_small(dist, epsilon)) {
                // project endpoints of seg2 onto seg1
                auto const t_seg1_q1 = project(seg1_dx, seg1_dy, q1x - p1x, q1y - p1y, length_sqr_seg1);
                auto const t_seg1_q2 = project(seg1_dx, seg1_dy, q2x - p1x, q2y - p1y, length_sqr_seg1);

                // project endpoints of seg1 onto seg2
                auto const t_seg2_p1 = project(seg2_dx, seg2_dy, p1x - q1x, p1y - q1y, length_sqr_seg2);
                auto const t_seg2_p2 = project(seg2_dx, seg2_dy, p2x - q1x, p2y - q1y, length_sqr_seg2);

                handle_collinear_segments(result, t_seg1_q1, t_seg1_q2, t_seg2_p1, t_seg2_p2, seg1_dx, seg2_dx, seg1,
                                          seg2, epsilon);
            }
        }
    } else {
        // non parallel intersections
        auto const t = ((q1y - p1y) * seg2_dx - (q1x - p1x) * seg2_dy) / cross;
        auto const s = ((q1y - p1y) * seg1_dx - (q1x - p1x) * seg1_dy) / cross;

        if (t.intersects(bounds) && s.intersects(bounds)) {
            result.push_back(make_point_intersection(t.middle(), s.middle()));
        } else {
            // For cases where one segment ALMOST intersects another segment somewhere in the middle
            double const t_val = clamp_to_bound(t.middle());
            double const s_val = clamp_to_bound(s.middle());

            auto const dist = distance(seg1.pointAt(t_val), seg2.pointAt(s_val));

            if (dist <= epsilon) {
                result.push_back(make_point_intersection(t_val, s_val));
            } else {
                // Check the distance between the endpoints nearest to the intersections
                double const endpoint_t = get_nearest_endpoint(t_val);
                double const endpoint_s = get_nearest_endpoint(s_val);

                auto const dist = distance(seg1.pointAt(endpoint_t), seg2.pointAt(endpoint_s));

                if (dist <= epsilon) {
                    result.push_back(make_point_intersection(endpoint_t, endpoint_s));
                }
            }
        }
    }

    return result;
}

} // namespace Geom
