/** @file
 * @brief Unit tests for PlanarGraph class template
 */
/*
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright 2022 the Authors
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
#include <iostream>

#include <2geom/point.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>

#include "planar-graph.h"
#include "testing.h"

using namespace Geom;

#define PV(d) (parse_svg_path(d))
#define PTH(d) (std::move(PV(d)[0]))
#define REV(d) ((PV(d)[0]).reversed())

/** An edge label for the purpose of tests. */
struct TestLabel
{
    unsigned reversal_count = 0, merge_count = 0, detachment_count = 0;
    void onReverse() { reversal_count++; }
    void onMergeWith(TestLabel const &) { merge_count++; }
    void onDetach() { detachment_count++; }
};

using TestGraph = PlanarGraph<TestLabel>;

static std::vector<TestLabel> extract_labels(TestGraph const &graph)
{
    // Find labels of edges remaining in the graph.
    std::vector<TestLabel> result;
    for (auto &e : graph.getEdges()) {
        if (!e.detached) {
            result.push_back(e.label);
        }
    }
    return result;
}

class PlanarGraphTest : public ::testing::Test
{
};

/** Test edge insertion and vertex clumping to within the tolerance. */
TEST(PlanarGraphTest, EdgeInsertion)
{
    double const precision = 1e-3;
    auto graph = TestGraph(precision);
    graph.insertEdge(PTH("M 0, 0 L 1, 0"));
    graph.insertEdge(PTH("M 0, 1 L 1, 1"));      // } Endpoints near
    graph.insertEdge(PTH("M 1, 0 L 1, 1.0009")); // } but not exact.

    auto vertices = graph.getVertices();

    // Test vertex clumping within the given precision
    EXPECT_EQ(vertices.size(), 4);
    EXPECT_EQ(graph.numEdges(), 3);

    // Test lexicographic vertex position sorting by X and then Y
    EXPECT_EQ(vertices.front().point(), Point(0, 0));
    auto after = std::next(vertices.begin());
    EXPECT_EQ(after->point(), Point(0, 1));
    ++after;
    EXPECT_EQ(after->point(), Point(1, 0));
    EXPECT_TRUE(are_near(vertices.back().point(), Point(1, 1), precision));

    EXPECT_FALSE(graph.isRegularized());
}

/** Test PlanarGraph<T>::insertDetached(). */
TEST(PlanarGraphTest, InsertDetached)
{
    TestGraph graph;
    auto detached = graph.insertDetached(PTH("M 0,0 A 1,1 0,0,1 2,0 V -2 H 0 Z"));

    auto const &edges = graph.getEdges();
    EXPECT_EQ(edges.size(), 1);
    EXPECT_TRUE(edges.at(detached).detached);
    EXPECT_TRUE(edges.at(detached).inserted_as_detached);

    EXPECT_EQ(graph.numVertices(), 0);
    EXPECT_EQ(graph.numEdges(false), 0);
    EXPECT_TRUE(graph.isRegularized());
}

/** Test signed area calculation. */
TEST(PlanarGraphTest, ClosedPathArea)
{
    // Square with counter-clockwise oriented boundary, when imagining that the y-axis
    // points up – expect the area to be +1.
    auto square_positive = PTH("M 0,0 H 1 V 1 H 0 Z");
    EXPECT_DOUBLE_EQ(TestGraph::closedPathArea(square_positive), 1.0);

    // Expect negative area for a negatively oriented path.
    auto triangle_negative = PTH("M 0,0 V 1 L 1,1 Z");
    EXPECT_DOUBLE_EQ(TestGraph::closedPathArea(triangle_negative), -0.5);
}

/** Test the detection of direction of deviation of initially tangent paths. */
TEST(PlanarGraphTest, Deviation)
{
    auto vertical_up = PTH("M 0,0 V 1");
    auto arc_right1 = PTH("M 0,0 A 1,1 0,1,0 2,0");
    auto arc_left1 = PTH("M 0,0 A 1,1 0,1,1 -2,0");
    auto arc_right2 = PTH("M 0,0 A 2,2 0,1,0, 4,0");
    auto arc_left2 = PTH("M 0,0 A 2,2 0,1,1 -4,0");
    // A very "flat" Bézier curve deviating to the right but slower than the large arc
    auto bezier_right = PTH("M 0,0 C 0,50 1,20 2,10");

    EXPECT_TRUE(TestGraph::deviatesLeft(arc_left1, arc_left2));
    EXPECT_TRUE(TestGraph::deviatesLeft(arc_left2, vertical_up));
    EXPECT_TRUE(TestGraph::deviatesLeft(vertical_up, arc_right2));
    EXPECT_TRUE(TestGraph::deviatesLeft(vertical_up, bezier_right));
    EXPECT_TRUE(TestGraph::deviatesLeft(bezier_right, arc_right2));
    EXPECT_TRUE(TestGraph::deviatesLeft(arc_right2, arc_right1));
    EXPECT_TRUE(TestGraph::deviatesLeft(arc_left1, arc_right1));
    EXPECT_TRUE(TestGraph::deviatesLeft(arc_left2, arc_right1));

    EXPECT_FALSE(TestGraph::deviatesLeft(arc_right1, vertical_up));
    EXPECT_FALSE(TestGraph::deviatesLeft(arc_right1, arc_right2));
    EXPECT_FALSE(TestGraph::deviatesLeft(vertical_up, arc_left2));
    EXPECT_FALSE(TestGraph::deviatesLeft(arc_left2, arc_left1));
    EXPECT_FALSE(TestGraph::deviatesLeft(arc_right1, arc_left1));
    EXPECT_FALSE(TestGraph::deviatesLeft(arc_right1, arc_left2));
}

/** Test sorting of incidences at a vertex by the outgoing heading. */
TEST(PlanarGraphTest, BasicAzimuthalSort)
{
    TestGraph graph;

    // Imagine the Y-axis pointing up (as in mathematics)!
    bool const clockwise = true;
    unsigned const num_rays = 9;
    unsigned edges[num_rays];

    // Insert the edges randomly but store them in what we know to be the
    // clockwise order of outgoing azimuths from the vertex at the origin.
    edges[7] = graph.insertEdge(PTH("M -0.2, -1 L 0, 0"));
    edges[1] = graph.insertEdge(PTH("M -1, 0.2 L 0, 0"));
    edges[4] = graph.insertEdge(PTH("M 0, 0 L 1, 0.2"));
    edges[6] = graph.insertEdge(PTH("M 0.1, -1 L 0, 0"));
    edges[2] = graph.insertEdge(PTH("M 0, 0 L -0.3, 1"));
    edges[0] = graph.insertEdge(PTH("M -1, 0 H 0"));
    edges[5] = graph.insertEdge(PTH("M 0, 0 L 1, -0.2"));
    edges[3] = graph.insertEdge(PTH("M 0.2, 1 L 0, 0"));
    edges[8] = graph.insertEdge(PTH("M -1, -0.1 L 0, 0"));

    // We expect the incidence to edges[0] to be the last one
    // in the sort order so it should appear first when going clockwise.
    auto [origin, incidence] = graph.getIncidence(edges[0], TestGraph::Incidence::END);
    ASSERT_TRUE(origin);
    ASSERT_TRUE(incidence);

    // Expect ±pi as the azimuth
    EXPECT_DOUBLE_EQ(std::abs(incidence->azimuth), M_PI);

    // Test sort order
    for (unsigned i = 0; i < num_rays; i++) {
        EXPECT_EQ(incidence->index, edges[i]);
        incidence = (TestGraph::Incidence *)&graph.nextIncidence(*origin, *incidence, clockwise);
    }
}

/** Test retrieval of a path inserted as an edge in both orientations. */
TEST(PlanarGraphTest, PathRetrieval)
{
    TestGraph graph;

    Path const path = PTH("M 0,0 L 1,1 C 2,2 4,2 5,1");
    Path const htap = path.reversed();

    auto edge = graph.insertEdge(path);

    ASSERT_EQ(graph.numEdges(), 1);

    auto [start_point, start_incidence] = graph.getIncidence(edge, TestGraph::Incidence::START);
    ASSERT_TRUE(start_point);
    ASSERT_TRUE(start_incidence);
    EXPECT_EQ(graph.getOutgoingPath(start_incidence), path);
    EXPECT_EQ(graph.getIncomingPath(start_incidence), htap);

    auto [end_point, end_incidence] = graph.getIncidence(edge, TestGraph::Incidence::END);
    ASSERT_TRUE(end_point);
    ASSERT_TRUE(end_incidence);
    EXPECT_EQ(graph.getIncomingPath(end_incidence), path);
    EXPECT_EQ(graph.getOutgoingPath(end_incidence), htap);
}

/** Make sure the edge labels are correctly stored. */
TEST(PlanarGraphTest, LabelRetrieval)
{
    TestGraph graph;
    TestLabel label;

    label.reversal_count = 420;
    label.merge_count = 69;
    label.detachment_count = 111;

    auto edge = graph.insertEdge(PTH("M 0,0 L 1,1"), std::move(label));

    auto retrieved = graph.getEdge(edge).label;
    EXPECT_EQ(retrieved.reversal_count, 420);
    EXPECT_EQ(retrieved.merge_count, 69);
    EXPECT_EQ(retrieved.detachment_count, 111);
}

/** Regularization of duplicate edges. */
TEST(PlanarGraphTest, MergeDuplicate)
{
    char const *const d =      "M 2,     3 H 0 C 1,4 1,5 0,6 H 10      L 8, 0";
    char const *const near_d = "M 2.0009,3 H 0 C 1,4 1,5 0,6 H 10.0009 L 8, 0.0005";

    // Test removal of perfect overlap:
    TestGraph graph;
    graph.insertEdge(PTH(d));
    graph.insertEdge(PTH(d)); // exact duplicate
    graph.regularize();

    EXPECT_TRUE(graph.isRegularized());

    auto remaining = extract_labels(graph);

    // Expect there to be only 1 edge after regularization.
    ASSERT_EQ(remaining.size(), 1);

    EXPECT_EQ(remaining[0].merge_count, 1); // expect one merge,
    EXPECT_EQ(remaining[0].reversal_count, 0); // no reversals,
    EXPECT_EQ(remaining[0].detachment_count, 0); // no detachments.

    // Test removal of imperfect overlaps within numerical precision
    TestGraph fuzzy{1e-3};
    fuzzy.insertEdge(PTH(d));
    fuzzy.insertEdge(PTH(near_d));
    fuzzy.regularize();

    EXPECT_TRUE(fuzzy.isRegularized());

    auto fuzmaining = extract_labels(fuzzy);
    ASSERT_EQ(fuzmaining.size(), 1);

    EXPECT_EQ(fuzmaining[0].merge_count, 1); // expect one merge,
    EXPECT_EQ(fuzmaining[0].reversal_count, 0); // no reversals,
    EXPECT_EQ(fuzmaining[0].detachment_count, 0); // no detachments.

    // Test overlap of edges with oppositie orientations.
    TestGraph twoway;
    twoway.insertEdge(PTH(d));
    twoway.insertEdge(REV(d));
    twoway.regularize();

    EXPECT_TRUE(twoway.isRegularized());

    auto left = extract_labels(twoway);
    ASSERT_EQ(left.size(), 1);

    EXPECT_EQ(left[0].merge_count, 1); // expect one merge,
    EXPECT_TRUE(left[0].reversal_count == 0 || left[0].reversal_count == 1); // 0 or 1 reversals
    EXPECT_EQ(left[0].detachment_count, 0); // no detachments.
}

/** Regularization of a shorter edge overlapping a longer one. */
TEST(PlanarGraphTest, MergePartial)
{
    TestGraph graph;
    auto longer = graph.insertEdge(PTH("M 0, 0 L 10, 10"));
    auto shorter = graph.insertEdge(PTH("M 0, 0 L 6, 6"));

    EXPECT_EQ(graph.numVertices(), 3);

    graph.regularize();

    EXPECT_EQ(graph.numVertices(), 3);
    EXPECT_TRUE(graph.isRegularized());

    auto labels = extract_labels(graph);
    ASSERT_EQ(labels.size(), 2);

    EXPECT_EQ(labels[longer].merge_count, 0);
    EXPECT_EQ(labels[longer].reversal_count, 0);
    EXPECT_EQ(labels[longer].detachment_count, 0);

    EXPECT_EQ(labels[shorter].merge_count, 1);
    EXPECT_EQ(labels[shorter].reversal_count, 0);
    EXPECT_EQ(labels[shorter].detachment_count, 0);

    // Now the same thing but with edges of opposite orientations.
    TestGraph graphopp;
    longer = graphopp.insertEdge(PTH("M 0, 0 L 10, 0"));
    shorter = graphopp.insertEdge(PTH("M 10, 0 L 5, 0"));

    EXPECT_EQ(graphopp.numVertices(), 3);

    graphopp.regularize();

    EXPECT_EQ(graphopp.numVertices(), 3);
    EXPECT_TRUE(graphopp.isRegularized());

    labels = extract_labels(graphopp);
    ASSERT_EQ(labels.size(), 2);

    EXPECT_EQ(labels[longer].merge_count, 0);
    EXPECT_EQ(labels[longer].reversal_count, 0);
    EXPECT_EQ(labels[longer].detachment_count, 0);

    EXPECT_EQ(labels[shorter].merge_count, 1);
    EXPECT_EQ(labels[shorter].reversal_count, 0);
    EXPECT_EQ(labels[shorter].detachment_count, 0);
}

/** Regularization of a Y-split. */
TEST(PlanarGraphTest, MergeY)
{
    TestGraph graph;
    auto left = graph.insertEdge(PTH("M 1 0 V 1 L 0, 2"));
    auto right = graph.insertEdge(PTH("M 1,0 V 1 L 2, 2"));

    EXPECT_EQ(graph.numVertices(), 3);
    graph.regularize();
    EXPECT_EQ(graph.numVertices(), 4);

    auto edges = graph.getEdges();
    EXPECT_EQ(edges.size(), 3);

    EXPECT_TRUE(are_near(edges[right].start->point(), Point(1, 1)));
}

/** Test reversal of a wrongly oriented teardrop */
TEST(PlanarGraphTest, Teardrop)
{
    TestGraph graph;
    auto loop = graph.insertEdge(PTH("M 1,0 A 1,1, 0,0,1 0,1 L 2,2 V 1 H 1 V 0"));
    // Insert a few unrelated edges
    auto before = graph.insertEdge(PTH("M 1,0 H 10"));
    auto after = graph.insertEdge(PTH("M 1,0 H -10"));

    EXPECT_EQ(graph.numVertices(), 3);

    graph.regularize();

    EXPECT_EQ(graph.numVertices(), 3);
    auto [start_vertex, start_incidence] = graph.getIncidence(loop, TestGraph::Incidence::START);
    auto [end_vertex, end_incidence] = graph.getIncidence(loop, TestGraph::Incidence::END);

    EXPECT_EQ(start_vertex, end_vertex);
    ASSERT_NE(start_vertex, nullptr);

    // Check that the incidences have been swapped
    EXPECT_EQ(start_vertex->cyclicNextIncidence(end_incidence), start_incidence);
    EXPECT_EQ(start_vertex->cyclicPrevIncidence(start_incidence), end_incidence);
    auto [b, before_incidence] = graph.getIncidence(before, TestGraph::Incidence::START);
    EXPECT_EQ(start_vertex->cyclicNextIncidence(before_incidence), end_incidence);
    auto [a, after_incidence] = graph.getIncidence(after, TestGraph::Incidence::START);
    EXPECT_EQ(start_vertex->cyclicPrevIncidence(after_incidence), start_incidence);
}

/** Test the regularization of a lasso-shaped path. */
TEST(PlanarGraphTest, ReglueLasso)
{
    TestGraph graph;
    // Insert a lasso-shaped path (a teardrop with initial self-overlap).
    auto original_lasso = graph.insertEdge(PTH("M 0,0 V 1 C 0,2 1,3 1,4 "
                                               "A 1,1 0,1,1 -1,4 C -1,3 0,2 0,1 V 0"));
    EXPECT_EQ(graph.numVertices(), 1);

    graph.regularize();
    EXPECT_EQ(graph.numVertices(), 2);
    EXPECT_EQ(graph.numEdges(false), 2);
    EXPECT_TRUE(graph.getEdge(original_lasso).detached);

    auto const &edges = graph.getEdges();
    // Find the edge from origin and ensure it got glued.
    auto from_origin = std::find_if(edges.begin(), edges.end(), [](auto const &edge) -> bool {
        return !edge.detached && (edge.start->point() == Point(0, 0) ||
                                    edge.end->point() == Point(0, 0));
    });
    ASSERT_NE(from_origin, edges.end());
    ASSERT_EQ(from_origin->label.merge_count, 1);
}

/** Test the removal of a collapsed loop. */
TEST(PlanarGraphTest, RemoveCollapsed)
{
    TestGraph graph;
    // Insert a collapsed loop
    auto collapsed = graph.insertEdge(PTH("M 0,0 L 1,1 L 0,0"));
    ASSERT_EQ(graph.numEdges(), 1);
    graph.regularize();
    ASSERT_EQ(graph.numEdges(false), 0);
    ASSERT_TRUE(graph.getEdge(collapsed).detached);

    TestGraph fuzzy(1e-3);
    // Insert a nearly collapsed loop
    auto nearly = fuzzy.insertEdge(PTH("M 0,0 H 2 V 0.001 L 1,0 H 0"));
    ASSERT_EQ(fuzzy.numEdges(), 1);
    fuzzy.regularize();
    ASSERT_EQ(fuzzy.numEdges(false), 0);
    ASSERT_TRUE(fuzzy.getEdge(nearly).detached);
}

/** Test regularization of straddling runs. */
TEST(PlanarGraphTest, RemoveWisp)
{
    TestGraph graph;
    // Insert a horizontal segment at the origin towards positive X:
    graph.insertEdge(PTH("M 0 0 H 1"));
    // Insert a path with a collapsed Bézier curve towards negative X:
    graph.insertEdge(PTH("M 0 0 C -1 0 -1 0 0 0"));
    graph.regularize();

    // Ensure that the folded Bézier is removed (and no segfault occurs).
    EXPECT_EQ(graph.numEdges(false), 1);
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
