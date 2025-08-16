#include <cmath>
#include <vector>
#include <iterator>
#include <iostream>

#include <glib.h>

#include <2geom/bezier.h>
#include <2geom/path.h>
#include <2geom/pathvector.h>
#include <2geom/path-intersection.h>
#include <2geom/svg-path-parser.h>
#include <2geom/svg-path-writer.h>

#include "testing.h"

using namespace std;
using namespace Geom;

Path string_to_path(const char* s) {
    PathVector pv = parse_svg_path(s);
    assert(pv.size() == 1);
    return pv[0];
}

// Path fixture
class PathTest : public ::testing::Test {
protected:
    PathTest() {
        line.append(LineSegment(Point(0,0), Point(1,0)));
        square = string_to_path("M 0,0 1,0 1,1 0,1 z");
        circle = string_to_path("M 0,0 a 4.5,4.5 0 1 1 -9,0 4.5,4.5 0 1 1 9,0 z");
        arcs = string_to_path("M 0,0 a 5,10 45 0 1 10,10 a 5,10 45 0 1 0,0 z");
        diederik = string_to_path("m 262.6037,35.824151 c 0,0 -92.64892,-187.405851 30,-149.999981 104.06976,31.739531 170,109.9999815 170,109.9999815 l -10,-59.9999905 c 0,0 40,79.99999 -40,79.99999 -80,0 -70,-129.999981 -70,-129.999981 l 50,0 C 435.13571,-131.5667 652.76275,126.44872 505.74322,108.05672 358.73876,89.666591 292.6037,-14.175849 292.6037,15.824151 c 0,30 -30,20 -30,20 z");
        cmds = string_to_path("M 0,0 V 100 H 100 Q 100,0 0,0 L 200,0 C 200,100 300,100 300,0 S 200,-100 200,0");

        p_open = string_to_path("M 0,0 L 0,5 5,5 5,0");
        p_closed = p_open;
        p_closed.close(true);
        p_add = string_to_path("M -1,6 L 6,6");
        p_single = string_to_path("M 0,0");
        p_a3Bezier = string_to_path("m 0,0 c 0,0 80,80 100,0"); // asymmetric cubic bezier

        p_open.setStitching(true);
        p_closed.setStitching(true);
    }

    // Objects declared here can be used by all tests in the test case for Foo.
    Path line, square, circle, arcs, diederik, cmds;
    Path p_open, p_closed, p_add, p_single, p_a3Bezier;
};

TEST_F(PathTest, CopyConstruction) {
    Path pa = p_closed;
    Path pc(p_closed);
    EXPECT_EQ(pa, p_closed);
    EXPECT_EQ(pa.closed(), p_closed.closed());
    EXPECT_EQ(pc, p_closed);
    EXPECT_EQ(pc.closed(), p_closed.closed());

    Path poa = cmds;
    Path poc(cmds);
    EXPECT_EQ(poa, cmds);
    EXPECT_EQ(poa.closed(), cmds.closed());
    EXPECT_EQ(poc, cmds);
    EXPECT_EQ(poc.closed(), cmds.closed());

    PathVector pvc(pa);
    EXPECT_EQ(pvc[0], pa);
    PathVector pva((Geom::Path()));
    pva[0] = pa;
    EXPECT_EQ(pva[0], pa);
}

TEST_F(PathTest, PathInterval) {
    PathTime n2_before(1, 0.9995), n2_after(2, 0.0005),
                 n3_before(2, 0.9995), n3_after(3, 0.0005),
                 mid2(2, 0.5), mid3(3, 0.5);

    // ival[x][0] - normal
    // ival[x][1] - reversed
    // ival[x][2] - crosses start
    // ival[x][3] - reversed, crosses start
    PathInterval ival[5][4];

    ival[0][0] = PathInterval(n2_before, n2_after, false, 4);
    ival[0][1] = PathInterval(n2_after, n2_before, false, 4);
    ival[0][2] = PathInterval(n2_before, n2_after, true, 4);
    ival[0][3] = PathInterval(n2_after, n2_before, true, 4);
    ival[1][0] = PathInterval(n2_before, n3_after, false, 4);
    ival[1][1] = PathInterval(n3_after, n2_before, false, 4);
    ival[1][2] = PathInterval(n2_before, n3_after, true, 4);
    ival[1][3] = PathInterval(n3_after, n2_before, true, 4);
    ival[2][0] = PathInterval(n2_before, mid2, false, 4);
    ival[2][1] = PathInterval(mid2, n2_before, false, 4);
    ival[2][2] = PathInterval(n2_before, mid2, true, 4);
    ival[2][3] = PathInterval(mid2, n2_before, true, 4);
    ival[3][0] = PathInterval(mid2, mid3, false, 4);
    ival[3][1] = PathInterval(mid3, mid2, false, 4);
    ival[3][2] = PathInterval(mid2, mid3, true, 4);
    ival[3][3] = PathInterval(mid3, mid2, true, 4);
    ival[4][0] = PathInterval(n2_after, n3_before, false, 4);
    ival[4][1] = PathInterval(n3_before, n2_after, false, 4);
    ival[4][2] = PathInterval(n2_after, n3_before, true, 4);
    ival[4][3] = PathInterval(n3_before, n2_after, true, 4);

    EXPECT_TRUE(ival[0][0].contains(n2_before));
    EXPECT_TRUE(ival[0][0].contains(n2_after));
    EXPECT_TRUE(ival[0][1].contains(n2_before));
    EXPECT_TRUE(ival[0][1].contains(n2_after));

    for (unsigned i = 0; i <= 4; ++i) {
        EXPECT_FALSE(ival[i][0].reverse());
        EXPECT_TRUE(ival[i][1].reverse());
        EXPECT_TRUE(ival[i][2].reverse());
        EXPECT_FALSE(ival[i][3].reverse());
    }

    for (unsigned i = 0; i <= 4; ++i) {
        for (unsigned j = 0; j <= 3; ++j) {
            //std::cout << i << " " << j << " " << ival[i][j] << std::endl;
            EXPECT_TRUE(ival[i][j].contains(ival[i][j].inside(1e-3)));
        }
    }

    PathTime n1(1, 0.0), n1x(0, 1.0),
                 n2(2, 0.0), n2x(1, 1.0),
                 n3(3, 0.0), n3x(2, 1.0);
    PathTime tests[8] = { n1, n1x, n2, n2x, n3, n3x, mid2, mid3 };

    // 0: false for both
    // 1: true for normal, false for cross_start
    // 2: false for normal, true for cross_start
    // 3: true for both

    int const NORMAL = 1, CROSS = 2, BOTH = 3;

    int includes[5][8] = {
        { CROSS,  CROSS,  NORMAL, NORMAL, CROSS,  CROSS,  CROSS,  CROSS  },
        { CROSS,  CROSS,  NORMAL, NORMAL, NORMAL, NORMAL, NORMAL, CROSS  },
        { CROSS,  CROSS,  NORMAL, NORMAL, CROSS,  CROSS,  BOTH,   CROSS  },
        { CROSS,  CROSS,  CROSS,  CROSS,  NORMAL, NORMAL, BOTH,   BOTH   },
        { CROSS,  CROSS,  CROSS,  CROSS,  CROSS,  CROSS,  NORMAL, CROSS  }
    };
    unsigned sizes[5][2] = {
        { 2, 4 },
        { 3, 3 },
        { 2, 4 },
        { 2, 4 },
        { 1, 5 }
    };

    for (unsigned i = 0; i < 5; ++i) {
        for (unsigned j = 0; j < 8; ++j) {
            EXPECT_EQ(ival[i][0].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][1].contains(tests[j]), bool(includes[i][j] & NORMAL));
            EXPECT_EQ(ival[i][2].contains(tests[j]), bool(includes[i][j] & CROSS));
            EXPECT_EQ(ival[i][3].contains(tests[j]), bool(includes[i][j] & CROSS));
        }
        EXPECT_EQ(ival[i][0].curveCount(), sizes[i][0]);
        EXPECT_EQ(ival[i][1].curveCount(), sizes[i][0]);
        EXPECT_EQ(ival[i][2].curveCount(), sizes[i][1]);
        EXPECT_EQ(ival[i][3].curveCount(), sizes[i][1]);
    }
}

TEST_F(PathTest, Continuity) {
    line.checkContinuity();
    square.checkContinuity();
    circle.checkContinuity();
    diederik.checkContinuity();
    cmds.checkContinuity();
}

TEST_F(PathTest, RectConstructor) {
    Rect r(Point(0,0), Point(10,10));
    Path rpath(r);

    EXPECT_EQ(rpath.size(), 4u);
    EXPECT_TRUE(rpath.closed());
    for (unsigned i = 0; i < 4; ++i) {
        EXPECT_TRUE(dynamic_cast<LineSegment const *>(&rpath[i]) != NULL);
        EXPECT_EQ(rpath[i].initialPoint(), r.corner(i));
    }
}

TEST_F(PathTest, Reversed) {
    std::vector<Path> a, r;
    a.push_back(p_open);
    a.push_back(p_closed);
    a.push_back(circle);
    a.push_back(diederik);
    a.push_back(cmds);

    for (auto & i : a) {
        r.push_back(i.reversed());
    }

    for (unsigned i = 0; i < a.size(); ++i) {
        EXPECT_EQ(r[i].size(), a[i].size());
        EXPECT_EQ(r[i].initialPoint(), a[i].finalPoint());
        EXPECT_EQ(r[i].finalPoint(), a[i].initialPoint());
        EXPECT_EQ(r[i].reversed(), a[i]);
        Point p1 = r[i].pointAt(0.75);
        Point p2 = a[i].pointAt(a[i].size() - 0.75);
        EXPECT_FLOAT_EQ(p1[X], p2[X]);
        EXPECT_FLOAT_EQ(p1[Y], p2[Y]);
        EXPECT_EQ(r[i].closed(), a[i].closed());
        a[i].checkContinuity();
    }
}

TEST_F(PathTest, ValueAt) {
    EXPECT_EQ(Point(0,0), line.initialPoint());
    EXPECT_EQ(Point(1,0), line.finalPoint());

    EXPECT_EQ(Point(0.5, 0.0), line.pointAt(0.5));

    EXPECT_EQ(Point(0,0), square.initialPoint());
    EXPECT_EQ(Point(0,0), square.finalPoint());
    EXPECT_EQ(Point(1,0), square.pointAt(1));
    EXPECT_EQ(Point(0.5,1), square.pointAt(2.5));
    EXPECT_EQ(Point(0,0.5), square.pointAt(3.5));
    EXPECT_EQ(Point(0,0), square.pointAt(4));
}

TEST_F(PathTest, NearestPoint) {
    EXPECT_EQ(0, line.nearestTime(Point(0,0)).asFlatTime());
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,0)).asFlatTime());
    EXPECT_EQ(0.5, line.nearestTime(Point(0.5,1)).asFlatTime());
    EXPECT_EQ(1, line.nearestTime(Point(100,0)).asFlatTime());
    EXPECT_EQ(0, line.nearestTime(Point(-100,1000)).asFlatTime());

    EXPECT_EQ(0, square.nearestTime(Point(0,0)).asFlatTime());
    EXPECT_EQ(1, square.nearestTime(Point(1,0)).asFlatTime());
    EXPECT_EQ(3, square.nearestTime(Point(0,1)).asFlatTime());

    //cout << diederik.nearestTime(Point(247.32293,-43.339507)) << endl;

    Point p(511.75,40.85);
    EXPECT_FLOAT_EQ(6.5814033, diederik.nearestTime(p).asFlatTime());
    /*cout << diederik.pointAt(diederik.nearestTime(p)) << endl
         << diederik.pointAt(6.5814033) << endl
         << distance(diederik.pointAt(diederik.nearestTime(p)), p) << "  "
         << distance(diederik.pointAt(6.5814033), p) << endl;*/

}

TEST_F(PathTest, Winding) {
    // test points in special positions
    EXPECT_EQ(line.winding(Point(-1, 0)), 0);
    EXPECT_EQ(line.winding(Point(2, 0)), 0);
    EXPECT_EQ(line.winding(Point(0, 1)), 0);
    EXPECT_EQ(line.winding(Point(0, -1)), 0);
    EXPECT_EQ(line.winding(Point(1, 1)), 0);
    EXPECT_EQ(line.winding(Point(1, -1)), 0);

    EXPECT_EQ(square.winding(Point(0, -1)), 0);
    EXPECT_EQ(square.winding(Point(1, -1)), 0);
    EXPECT_EQ(square.winding(Point(0, 2)), 0);
    EXPECT_EQ(square.winding(Point(1, 2)), 0);
    EXPECT_EQ(square.winding(Point(-1, 0)), 0);
    EXPECT_EQ(square.winding(Point(-1, 1)), 0);
    EXPECT_EQ(square.winding(Point(2, 0)), 0);
    EXPECT_EQ(square.winding(Point(2, 1)), 0);
    EXPECT_EQ(square.winding(Point(0.5, 0.5)), 1);

    EXPECT_EQ(circle.winding(Point(-4.5,0)), 1);
    EXPECT_EQ(circle.winding(Point(-3.5,0)), 1);
    EXPECT_EQ(circle.winding(Point(-4.5,1)), 1);
    EXPECT_EQ(circle.winding(Point(-10,0)), 0);
    EXPECT_EQ(circle.winding(Point(1,0)), 0);

    Path yellipse = string_to_path("M 0,0 A 40 20 90 0 0 0,-80 40 20 90 0 0 0,0 z");
    EXPECT_EQ(yellipse.winding(Point(-1, 0)), 0);
    EXPECT_EQ(yellipse.winding(Point(-1, -80)), 0);
    EXPECT_EQ(yellipse.winding(Point(1, 0)), 0);
    EXPECT_EQ(yellipse.winding(Point(1, -80)), 0);
    EXPECT_EQ(yellipse.winding(Point(0, -40)), -1);
    std::vector<double> r[4];
    r[0] = yellipse[0].roots(0, Y);
    r[1] = yellipse[0].roots(-80, Y);
    r[2] = yellipse[1].roots(0, Y);
    r[3] = yellipse[1].roots(-80, Y);
    for (auto & i : r) {
        for (double j : i) {
            std::cout << format_coord_nice(j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << yellipse[0].unitTangentAt(0) << " "
              << yellipse[0].unitTangentAt(1) << " "
              << yellipse[1].unitTangentAt(0) << " "
              << yellipse[1].unitTangentAt(1) << std::endl;

    Path half_ellipse = string_to_path("M 0,0 A 40 20 90 0 0 0,-80 L -20,-40 z");
    EXPECT_EQ(half_ellipse.winding(Point(-1, 0)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(-1, -80)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(1, 0)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(1, -80)), 0);
    EXPECT_EQ(half_ellipse.winding(Point(0, -40)), -1);

    // extra nasty cases with exact double roots
    Path hump = string_to_path("M 0,0 Q 1,1 2,0 L 2,2 0,2 Z");
    EXPECT_EQ(hump.winding(Point(0.25, 0.5)), 1);
    EXPECT_EQ(hump.winding(Point(1.75, 0.5)), 1);

    Path hump2 = string_to_path("M 0,0 L 2,0 2,2 Q 1,1 0,2 Z");
    EXPECT_EQ(hump2.winding(Point(0.25, 1.5)), 1);
    EXPECT_EQ(hump2.winding(Point(1.75, 1.5)), 1);
}

/// Regression test for issue https://gitlab.com/inkscape/lib2geom/-/issues/58
TEST_F(PathTest, Issue58)
{
    auto const random_point_in = [](Geom::Rect const &box) -> Point {
        Coord const x = g_random_double_range(box[X].min(), box[X].max());
        Coord const y = g_random_double_range(box[Y].min(), box[Y].max());
        return {x, y};
    };

    auto const verify_windings = [](Ellipse const &e, Path const &path, Point const &pt) {
        int const winding = path.winding(pt);
        if (e.contains(pt)) {
            EXPECT_EQ(winding, 1);
        } else {
            EXPECT_EQ(winding, 0);
        }
    };

    // Example elliptical path from issue https://gitlab.com/inkscape/lib2geom/-/issues/58
    char const *const issue_d = "M 495.8157837290847 280.07459226562503"
                                "A 166.63407933993605 132.04407218873035 0 0 1 329.1817043891487 412.11866445435544"
                                "A 166.63407933993605 132.04407218873035 0 0 1 162.54762504921263 280.07459226562503"
                                "A 166.63407933993605 132.04407218873035 0 0 1 329.1817043891487 148.0305200768947"
                                "A 166.63407933993605 132.04407218873035 0 0 1 495.8157837290847 280.07459226562503"
                                "z";
    auto const pv = parse_svg_path(issue_d);
    auto const issue_ellipse = Ellipse(Point(329.1817043891487, 280.07459226562503),
                                       Point(166.63407933993605, 132.04407218873035), 0);

    auto box = issue_ellipse.boundsExact();
    box.expandBy(1.0);

    g_random_set_seed(0xE111BB5E);
    for (size_t _ = 0; _ < 10'000; _++) {
        verify_windings(issue_ellipse, pv[0], random_point_in(box));
    }
}

TEST_F(PathTest, SVGRoundtrip) {
    SVGPathWriter sw;

    Path transformed = diederik * (Rotate(1.23456789) * Scale(1e-8) * Translate(1e-9, 1e-9));

    for (unsigned i = 0; i < 4; ++i) {
        sw.setOptimize(i & 1);
        sw.setUseShorthands(i & 2);

        sw.feed(line);
        //cout << sw.str() << endl;
        Path line_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(line_svg == line);
        sw.clear();

        sw.feed(square);
        //cout << sw.str() << endl;
        Path square_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(square_svg == square);
        sw.clear();

        sw.feed(circle);
        //cout << sw.str() << endl;
        Path circle_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(circle_svg == circle);
        sw.clear();

        sw.feed(arcs);
        //cout << sw.str() << endl;
        Path arcs_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(arcs_svg == arcs);
        sw.clear();

        sw.feed(diederik);
        //cout << sw.str() << endl;
        Path diederik_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(diederik_svg == diederik);
        sw.clear();

        sw.feed(transformed);
        //cout << sw.str() << endl;
        Path transformed_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(transformed_svg == transformed);
        sw.clear();

        sw.feed(cmds);
        //cout << sw.str() << endl;
        Path cmds_svg = string_to_path(sw.str().c_str());
        EXPECT_TRUE(cmds_svg == cmds);
        sw.clear();
    }
}

TEST_F(PathTest, Portion) {
    PathTime a(0, 0.5), b(3, 0.5);
    PathTime c(1, 0.25), d(1, 0.75);

    EXPECT_EQ(square.portion(a, b), string_to_path("M 0.5, 0 L 1,0 1,1 0,1 0,0.5"));
    EXPECT_EQ(square.portion(b, a), string_to_path("M 0,0.5 L 0,1 1,1 1,0 0.5,0"));
    EXPECT_EQ(square.portion(a, b, true), string_to_path("M 0.5,0 L 0,0 0,0.5"));
    EXPECT_EQ(square.portion(b, a, true), string_to_path("M 0,0.5 L 0,0 0.5,0"));
    EXPECT_EQ(square.portion(c, d), string_to_path("M 1,0.25 L 1,0.75"));
    EXPECT_EQ(square.portion(d, c), string_to_path("M 1,0.75 L 1,0.25"));
    EXPECT_EQ(square.portion(c, d, true), string_to_path("M 1,0.25 L 1,0 0,0 0,1 1,1 1,0.75"));
    EXPECT_EQ(square.portion(d, c, true), string_to_path("M 1,0.75 L 1,1 0,1 0,0 1,0 1,0.25"));

    // verify that no matter how an endpoint is specified, the result is the same
    PathTime a1(0, 1.0), a2(1, 0.0);
    PathTime b1(2, 1.0), b2(3, 0.0);
    Path result = string_to_path("M 1,0 L 1,1 0,1");
    EXPECT_EQ(square.portion(a1, b1), result);
    EXPECT_EQ(square.portion(a1, b2), result);
    EXPECT_EQ(square.portion(a2, b1), result);
    EXPECT_EQ(square.portion(a2, b2), result);
}

TEST_F(PathTest, AppendSegment) {
    Path p_open = line, p_closed = line;
    p_open.setStitching(true);
    p_open.append(new LineSegment(Point(10,20), Point(10,25)));
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_closed.setStitching(true);
    p_closed.close(true);
    p_closed.append(new LineSegment(Point(10,20), Point(10,25)));
    EXPECT_EQ(p_closed.size(), 4u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, AppendPath) {
    p_open.append(p_add);
    Path p_expected = string_to_path("M 0,0 L 0,5 5,5 5,0 -1,6 6,6");
    EXPECT_EQ(p_open.size(), 5u);
    EXPECT_EQ(p_open, p_expected);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_expected.close(true);
    p_closed.append(p_add);
    EXPECT_EQ(p_closed.size(), 6u);
    EXPECT_EQ(p_closed, p_expected);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, AppendPortion) {
    // A closed path with two curves:
    Path bigon = string_to_path("M 0,0 Q 1,1 2,0 Q 1,-1 0,0 Z");
    Path target{Point(0, 0)};

    PathTime end_time{1, 1.0}; // End of the closed path
    PathTime mid_time{1, 0.0}; // Middle of the closed path (juncture between the two curves)
    bigon.appendPortionTo(target, end_time, mid_time, true /* do cross start */);

    // We expect that the target path now contains the entire first curve "M 0,0 Q 1,1 2,0",
    // since we started at the end of a closed path and requested to cross its start.
    EXPECT_EQ(target.size(), 1);
    EXPECT_EQ(target, string_to_path("M 0,0 Q 1,1 2,0"));

    // Similar test but with reversal (swapped times)
    Path target_reverse{Point(2, 0)};
    bigon.appendPortionTo(target_reverse, mid_time, end_time, true /* do cross start please */);
    // What do we expect? To cross start going from the midpoint to the endpoint requires
    // not taking the obvious route (bigon[1]) but rather taking bigon[0] in reverse.
    EXPECT_EQ(target_reverse.size(), 1);
    EXPECT_EQ(target_reverse, string_to_path("M 2,0 Q 1,1 0,0"));

    // Similar test but using start time
    PathTime start_time{0, 0.0};
    Path mid_target{Point(2, 0)};
    bigon.appendPortionTo(mid_target, mid_time, start_time, true /* cross start to 0:0 */);
    // We expect to go forward from mid_time and cross over the start to start_time.
    EXPECT_EQ(mid_target.size(), 1);
    EXPECT_EQ(mid_target, string_to_path("M 2,0 Q 1,-1 0,0"));

    // Use start time with reversal
    Path mid_reverse{Point(0, 0)};
    bigon.appendPortionTo(mid_reverse, start_time, mid_time, true /* Cross start, going backwards. */);
    // We expect that we don't go forwards from start_time to mid_time, but rather cross over the starting
    // point and backtrack over bigon[1] to the midpoint.
    EXPECT_EQ(mid_reverse.size(), 1);
    EXPECT_EQ(mid_reverse, string_to_path("M 0,0 Q 1,-1 2,0"));
}

TEST_F(PathTest, ReplaceMiddle) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 2, p_add);
    EXPECT_EQ(p_open.size(), 5u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 2, p_add);
    EXPECT_EQ(p_closed.size(), 6u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceStart) {
    p_open.replace(p_open.begin(), p_open.begin() + 2, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_closed.replace(p_closed.begin(), p_closed.begin() + 2, p_add);
    EXPECT_EQ(p_closed.size(), 5u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceEnd) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 3, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 3, p_add);
    EXPECT_EQ(p_closed.size(), 5u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceClosing) {
    p_open.replace(p_open.begin() + 1, p_open.begin() + 4, p_add);
    EXPECT_EQ(p_open.size(), 3u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    p_closed.replace(p_closed.begin() + 1, p_closed.begin() + 4, p_add);
    EXPECT_EQ(p_closed.size(), 4u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, ReplaceEverything) {
    p_open.replace(p_open.begin(), p_open.end(), p_add);
    EXPECT_EQ(p_open.size(), 1u);
    EXPECT_NO_THROW(p_open.checkContinuity());

    // TODO: in this specific case, it may make sense to set the path to open...
    // Need to investigate what behavior is sensible here
    p_closed.replace(p_closed.begin(), p_closed.end(), p_add);
    EXPECT_EQ(p_closed.size(), 2u);
    EXPECT_NO_THROW(p_closed.checkContinuity());
}

TEST_F(PathTest, EraseLast) {
    p_open.erase_last();
    Path p_expected = string_to_path("M 0,0 L 0,5 5,5");
    EXPECT_EQ(p_open, p_expected);
    EXPECT_NO_THROW(p_open.checkContinuity());
}

TEST_F(PathTest, AreNear) {
    Path nudged_arcs1 = string_to_path("M 0,0 a 5,10 45 0 1 10,10.0000005 a 5,10 45 0 1 0,0 z");
    Path nudged_arcs2 = string_to_path("M 0,0 a 5,10 45 0 1 10,10.00005 a 5,10 45 0 1 0,0 z");
    EXPECT_EQ(are_near(diederik, diederik, 0), true);
    EXPECT_EQ(are_near(cmds, diederik, 1e-6), false);
    EXPECT_EQ(are_near(arcs, nudged_arcs1, 1e-6), true);
    EXPECT_EQ(are_near(arcs, nudged_arcs2, 1e-6), false);
}

TEST_F(PathTest, Roots) {
    Path path;
    path.start(Point(0, 0));
    path.appendNew<Geom::LineSegment>(Point(1, 1));
    path.appendNew<Geom::LineSegment>(Point(2, 0));

    EXPECT_FALSE(path.closed());

    // Trivial case: make sure that path is not closed
    std::vector<PathTime> roots = path.roots(0.5, Geom::X);
    EXPECT_EQ(roots.size(), 1u);
    EXPECT_EQ(path.valueAt(roots[0], Geom::Y), 0.5);

    // Now check that it is closed if we make it so
    path.close(true);
    roots = path.roots(0.5, Geom::X);
    EXPECT_EQ(roots.size(), 2u);
}

TEST_F(PathTest, PartingPoint)
{
    // === Test complete overlaps between identical curves ===
    // Line segment
    auto line = string_to_path("M 0,0 L 3.33, 7.77");
    auto pt = parting_point(line, line);
    EXPECT_TRUE(are_near(pt.point(), line.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // Cubic Bézier
    auto bezier = string_to_path("M 0,0 C 1,1 14,1 15,0");
    pt = parting_point(bezier, bezier);
    EXPECT_TRUE(are_near(pt.point(), bezier.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // Eliptical arc
    auto const arc = string_to_path("M 0,0 A 100,20 0,0,0 200,0");
    pt = parting_point(arc, arc);
    EXPECT_TRUE(are_near(pt.point(), arc.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // === Test complete overlap between degree-elevated and degree-shrunk Béziers ===
    auto artificially_cubic = string_to_path("M 0,0 C 10,10 20,10 30,0");
    auto really_quadratic = string_to_path("M 0,0 Q 15,15 30,0");
    pt = parting_point(artificially_cubic, really_quadratic);
    EXPECT_TRUE(are_near(pt.point(), artificially_cubic.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 1.0));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 1.0));

    // === Test complete overlaps between a curve and its subdivision ===
    // Straight line
    line = string_to_path("M 0,0 L 15,15");
    auto subdivided_line = string_to_path("M 0,0 L 3,3 L 4,4 L 9,9 L 15,15");
    pt = parting_point(line, subdivided_line);
    EXPECT_TRUE(are_near(pt.point(), line.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // Cubic Bézier
    bezier = string_to_path("M 0,0 C 0,40 50,40 50,0");
    auto de_casteljau = string_to_path("M 0,0 C 0,10 3.125,17.5 7.8125,22.5 12.5,27.5 18.75,30 25,30"
                                       " 31.25,30 37.5,27.5 42.1875,22.5 46.875,17.5 50,10 50,0");
    pt = parting_point(bezier, de_casteljau);
    EXPECT_TRUE(are_near(pt.point(), bezier.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // Eliptical arc
    auto subdivided_arc = string_to_path("M 0,0 A 100,20, 0,0,0 100,20 A 100,20 0,0,0 200,0");
    pt = parting_point(arc, subdivided_arc);
    EXPECT_TRUE(are_near(pt.point(), arc.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 1.0));

    // === Test complete overlap between different subdivisions ===
    auto line1 = string_to_path("M 0,0 L 3,3 L 5,5 L 10,10");
    auto line2 = string_to_path("M 0,0 L 2,2 L 4.2,4.2 L 4.5,4.5 L 6,6 L 10,10");
    pt = parting_point(line1, line2);
    EXPECT_TRUE(are_near(pt.point(), line1.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(),  line1.timeRange().max()));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), line2.timeRange().max()));

    // === Test complete overlaps in the presence of degenerate segments ===
    // Straight line
    line = string_to_path("M 0,0 L 15,15");
    subdivided_line = string_to_path("M 0,0 L 3,3 H 3 V 3 L 3,3 L 4,4 H 4 V 4 L 4,4 L 9,9 H 9 L 15,15");
    pt = parting_point(line, subdivided_line);
    EXPECT_TRUE(are_near(pt.point(), line.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 1.0));

    // Eliptical arc
    auto arc_degen = string_to_path("M 0,0 A 100,20, 0,0,0 100,20 H 100 V 20 L 100,20 A 100,20 0,0,0 200,0");
    pt = parting_point(arc, arc_degen);
    EXPECT_TRUE(are_near(pt.point(), arc.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 1.0));

    // === Paths that overlap but one is shorter than the other ===
    // Straight lines
    auto long_line = string_to_path("M 0,0 L 20,10");
    auto short_line = string_to_path("M 0,0 L 4,2");
    pt = parting_point(long_line, short_line);
    EXPECT_TRUE(are_near(pt.point(), short_line.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 0.2));
    EXPECT_TRUE(are_near(pt.second.t, 1.0));

    // Cubic Bézier
    auto const s_shape = string_to_path("M 0,0 C 10, 0 0,10 10,10");
    auto half_s = string_to_path("M 0,0 C 5,0 5,2.5 5,5");
    pt = parting_point(s_shape, half_s);
    EXPECT_TRUE(are_near(pt.first.t, 0.5));
    EXPECT_TRUE(are_near(pt.second.t, 1.0));

    // Elliptical arc
    auto quarter_ellipse = string_to_path("M 0,0 A 100,20, 0,0,0 100,20");
    pt = parting_point(arc, quarter_ellipse);
    EXPECT_TRUE(are_near(pt.point(), quarter_ellipse.finalPoint()));
    EXPECT_TRUE(are_near(pt.first.t, 0.5));
    EXPECT_TRUE(are_near(pt.second.t, 1.0));

    // === Paths that overlap initially but then they split ===
    // Straight lines
    auto boring_line = string_to_path("M 0,0 L 50,10");
    auto line_then_arc = string_to_path("M 0,0 L 5,1 A 1,1 0,0,0 7,1");
    pt = parting_point(boring_line, line_then_arc);
    EXPECT_TRUE(are_near(pt.point(), Point(5, 1)));
    EXPECT_TRUE(are_near(pt.first.t, 0.1));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 1.0));

    // Cubic Bézier
    auto half_s_then_line = string_to_path("M 0,0 C 5,0 5,2.5 5,5 L 10,10");
    pt = parting_point(s_shape, half_s_then_line);
    EXPECT_TRUE(are_near(pt.point(), Point(5, 5)));
    EXPECT_TRUE(are_near(pt.first.t, 0.5));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 1.0));

    // Elliptical arc
    auto quarter_ellipse_then_quadratic = string_to_path("M 0,0 A 100,20, 0,0,0 100,20 Q 120,40 140,60");
    pt = parting_point(arc, quarter_ellipse_then_quadratic);
    EXPECT_TRUE(are_near(pt.point(), Point(100, 20)));
    EXPECT_TRUE(are_near(pt.first.t, 0.5));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 1.0));

    // === Paths that split at a common node ===
    // Polylines
    auto branch_90 = string_to_path("M 0,0 H 3 H 6 V 7");
    auto branch_45 = string_to_path("M 0,0 H 2 H 6 L 7,7");
    pt = parting_point(branch_90, branch_45);
    EXPECT_TRUE(are_near(pt.point(), Point(6, 0)));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 2.0));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 2.0));

    // Arcs
    auto quarter_circle_then_horiz = string_to_path("M 0,0 A 1,1 0,0,0 1,1 H 10");
    auto quarter_circle_then_slant = string_to_path("M 0,0 A 1,1 0,0,0 1,1 L 10, 1.1");
    pt = parting_point(quarter_circle_then_horiz, quarter_circle_then_slant);
    EXPECT_TRUE(are_near(pt.point(), Point(1, 1)));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 1.0));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 1.0));

    // Last common nodes followed by degenerates
    auto degen_horiz = string_to_path("M 0,0 A 1,1 0,0,0 1,1 V 1 H 1 L 1,1 H 10");
    auto degen_slant = string_to_path("M 0,0 A 1,1 0,0,0 1,1 V 1 H 1 L 1,1 L 10, 1.1");
    pt = parting_point(quarter_circle_then_horiz, quarter_circle_then_slant);
    EXPECT_TRUE(are_near(pt.point(), Point(1, 1)));

    // === Paths that split at the starting point ===
    auto vertical = string_to_path("M 0,0 V 1");
    auto quarter = string_to_path("M 0,0 A 1,1 0,0,0, 1,1");
    pt = parting_point(vertical, quarter);
    EXPECT_TRUE(are_near(pt.point(), Point(0, 0)));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 0.0));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 0.0));

    // === Symmetric split (both legs of the same length) ===
    auto left_leg = string_to_path("M 1,0 L 0,10");
    auto right_leg = string_to_path("M 1,0 L 2,10");
    pt = parting_point(left_leg, right_leg);
    EXPECT_TRUE(are_near(pt.point(), Point(1, 0)));
    EXPECT_TRUE(are_near(pt.first.asFlatTime(), 0.0));
    EXPECT_TRUE(are_near(pt.second.asFlatTime(), 0.0));

    // === Different starting points ===
    auto start_at_0_0 = string_to_path("M 0,0 C 1,0 0,1 1,1");
    auto start_at_10_10 = string_to_path("M 10,10 L 50,50");
    pt = parting_point(start_at_0_0, start_at_10_10);
    EXPECT_TRUE(are_near(pt.point(), Point (5,5)));
    EXPECT_DOUBLE_EQ(pt.first.t, -1.0);
    EXPECT_DOUBLE_EQ(pt.second.t, -1.0);
    EXPECT_EQ(pt.first.curve_index, 0);
    EXPECT_EQ(pt.second.curve_index, 0);
}

TEST_F(PathTest, InitialFinalTangents) {
    // Test tangents for an open path
    auto L_shape = string_to_path("M 1,1 H 0 V 0");
    EXPECT_EQ(L_shape.initialUnitTangent(), Point(-1.0, 0.0));
    EXPECT_EQ(L_shape.finalUnitTangent(), Point(0.0, -1.0));

    // Closed path with non-degenerate closing segment
    auto triangle = string_to_path("M 0,0 H 2 L 0,3 Z");
    EXPECT_EQ(triangle.initialUnitTangent(), Point(1.0, 0.0));
    EXPECT_EQ(triangle.finalUnitTangent(), Point(0.0, -1.0));

    // Closed path with a degenerate closing segment
    auto full360 = string_to_path("M 0,0 A 1,1, 0,1,1, 0,2 A 1,1 0,1,1 0,0 Z");
    EXPECT_EQ(full360.initialUnitTangent(), Point(1.0, 0.0));
    EXPECT_EQ(full360.finalUnitTangent(), Point(1.0, 0.0));

    // Test multiple degenerate segments at the start
    auto start_degen = string_to_path("M 0,0 L 0,0 H 0 V 0 Q 1,0 1,1");
    EXPECT_EQ(start_degen.initialUnitTangent(), Point(1.0, 0.0));

    // Test multiple degenerate segments at the end
    auto end_degen = string_to_path("M 0,0 L 1,1 H 1 V 1 L 1,1");
    double comp = 1.0 / sqrt(2.0);
    EXPECT_EQ(end_degen.finalUnitTangent(), Point(comp, comp));

    // Test a long and complicated path with both tangents along the positive x-axis.
    auto complicated = string_to_path("M 0,0 H 0 L 1,0 C 2,1 3,2 1,0 L 1,0 H 1 Q 2,3 0,5 H 2");
    EXPECT_EQ(complicated.initialUnitTangent(), Point(1.0, 0.0));
    EXPECT_EQ(complicated.finalUnitTangent(), Point(1.0, 0.0));
}

TEST_F(PathTest, WithoutDegenerates) {
    // Ensure nothing changes when there are no degenerate segments to remove.
    auto plain_open = string_to_path("M 0,0 Q 5,5 10,10");
    EXPECT_EQ(plain_open, plain_open.withoutDegenerateCurves());

    auto closed_nondegen_closing = string_to_path("M 0,0 L 5,5 H 0 Z");
    EXPECT_EQ(closed_nondegen_closing,closed_nondegen_closing.withoutDegenerateCurves());

    // Ensure that a degenerate closing segment is left alone.
    auto closed_degen_closing = string_to_path("M 0,0 L 2,4 H 0 L 0,0 Z");
    EXPECT_EQ(closed_degen_closing, closed_degen_closing.withoutDegenerateCurves());

    // Ensure that a trivial path is left alone (both open and closed).
    auto trivial_open = string_to_path("M 0,0");
    EXPECT_EQ(trivial_open, trivial_open.withoutDegenerateCurves());

    auto trivial_closed = string_to_path("M 0,0 Z");
    EXPECT_EQ(trivial_closed, trivial_closed.withoutDegenerateCurves());

    // Ensure that initial degenerate segments are removed
    auto degen_start = string_to_path("M 0,0 L 0,0 H 0 V 0 Q 5,5 10,10");
    auto degen_start_cleaned = degen_start.withoutDegenerateCurves();
    EXPECT_EQ(degen_start_cleaned, string_to_path("M 0,0 Q 5,5 10,10"));
    EXPECT_NE(degen_start.size(), degen_start_cleaned.size());

    // Ensure that degenerate segments are removed from the middle
    auto degen_middle = string_to_path("M 0,0 L 1,1 H 1 V 1 L 1,1 Q 6,6 10,10");
    auto degen_middle_cleaned = degen_middle.withoutDegenerateCurves();
    EXPECT_EQ(degen_middle_cleaned, string_to_path("M 0,0 L 1,1 Q 6,6 10,10"));
    EXPECT_NE(degen_middle.size(), degen_middle_cleaned.size());

    // Ensure that degenerate segment are removed from the end of an open path
    auto end_open = string_to_path("M 0,0 L 1,1 H 1 V 1 L 1,1");
    auto end_open_cleaned = end_open.withoutDegenerateCurves();
    EXPECT_EQ(end_open_cleaned, string_to_path("M 0,0 L 1,1"));
    EXPECT_NE(end_open.size(), end_open_cleaned.size());

    // Ensure removal of degenerates just before the closing segment
    auto end_nondegen = string_to_path("M 0,0 L 1,1 L 0,1 H 0 V 1 Z");
    auto end_nondegen_cleaned = end_nondegen.withoutDegenerateCurves();
    EXPECT_EQ(end_nondegen_cleaned, string_to_path("M 0,0 L 1,1 L 0,1 Z"));
    EXPECT_NE(end_nondegen.size(), end_nondegen_cleaned.size());
}

/** Test Path::extrema() */
TEST_F(PathTest, GetExtrema) {

    // Circle of radius 4.5 centered at (-4.5, 0).
    auto extrema_x = circle.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(-9, 0));
    EXPECT_EQ(extrema_x.max_point, Point( 0, 0));
    EXPECT_DOUBLE_EQ(extrema_x.min_time.asFlatTime(), 1.0);
    EXPECT_DOUBLE_EQ(extrema_x.max_time.asFlatTime(), 0.0);
    EXPECT_EQ(extrema_x.glance_direction_at_min, -1.0);
    EXPECT_EQ(extrema_x.glance_direction_at_max, 1.0);

    auto extrema_y = circle.extrema(Y);
    EXPECT_EQ(extrema_y.min_point, Point(-4.5, -4.5));
    EXPECT_EQ(extrema_y.max_point, Point(-4.5,  4.5));
    EXPECT_DOUBLE_EQ(extrema_y.min_time.asFlatTime(), 1.5);
    EXPECT_DOUBLE_EQ(extrema_y.max_time.asFlatTime(), 0.5);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_min,  1.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_max, -1.0);

    // Positively oriented unit square
    extrema_x = square.extrema(X);
    EXPECT_DOUBLE_EQ(extrema_x.min_point[X], 0.0);
    EXPECT_DOUBLE_EQ(extrema_x.max_point[X], 1.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_max,  1.0);

    extrema_y = square.extrema(Y);
    EXPECT_DOUBLE_EQ(extrema_y.min_point[Y], 0.0);
    EXPECT_DOUBLE_EQ(extrema_y.max_point[Y], 1.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_min, 1.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_max, -1.0);

    // Path glancing its min X line while going towards negative Y
    auto down_glance = string_to_path("M 1,18 L 0,0 1,-20");
    extrema_x = down_glance.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);
    EXPECT_DOUBLE_EQ(extrema_x.min_time.asFlatTime(), 1.0);

    // Similar but not at a node
    auto down_glance_smooth = string_to_path("M 1,20 C 0,20 0,-20 1,-20");
    extrema_x = down_glance_smooth.extrema(X);
    EXPECT_TRUE(are_near(extrema_x.min_point[Y], 0.0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);
    EXPECT_DOUBLE_EQ(extrema_x.min_time.asFlatTime(), 0.5);

    // Path coming down to the min X and then retreating horizontally
    auto retreat = string_to_path("M 1,20 L 0,0 H 5 L 4,-20");
    extrema_x = retreat.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_EQ(extrema_x.max_point, Point(5, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_max, -1.0);
    EXPECT_DOUBLE_EQ(extrema_x.min_time.asFlatTime(), 1.0);
    EXPECT_DOUBLE_EQ(extrema_x.max_time.asFlatTime(), 2.0);

    // Perfectly horizontal path
    auto horizontal = string_to_path("M 0,0 H 12");
    extrema_x = horizontal.extrema(X);
    extrema_y = horizontal.extrema(Y);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_EQ(extrema_x.max_point, Point(12, 0));
    EXPECT_DOUBLE_EQ(extrema_y.min_point[Y], 0.0);
    EXPECT_DOUBLE_EQ(extrema_y.max_point[Y], 0.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, 0.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_max, 0.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_min, 1.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_max, 1.0);
    EXPECT_DOUBLE_EQ(extrema_x.min_time.asFlatTime(), 0.0);
    EXPECT_DOUBLE_EQ(extrema_x.max_time.asFlatTime(), 1.0);

    // Perfectly vertical path
    auto vertical = string_to_path("M 0,0 V 42");
    extrema_y = vertical.extrema(Y);
    extrema_x = vertical.extrema(X);
    EXPECT_DOUBLE_EQ(extrema_x.min_point[Y], 0.0);
    EXPECT_DOUBLE_EQ(extrema_x.max_point[Y], 0.0);
    EXPECT_EQ(extrema_y.min_point, Point(0, 0));
    EXPECT_EQ(extrema_y.max_point, Point(0, 42));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, 1.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_max, 1.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_min, 0.0);
    EXPECT_FLOAT_EQ(extrema_y.glance_direction_at_max, 0.0);
    EXPECT_DOUBLE_EQ(extrema_y.min_time.asFlatTime(), 0.0);
    EXPECT_DOUBLE_EQ(extrema_y.max_time.asFlatTime(), 1.0);

    // Detect downward glance at the closing point (degenerate closing segment)
    auto closed = string_to_path("M 0,0 L 1,-2 H 3 V 5 H 1 L 0,0 Z");
    extrema_x = closed.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);

    // Same but with a non-degenerate closing segment
    auto closed_nondegen = string_to_path("M 0,0 L 1,-2 H 3 V 5 H 1 Z");
    extrema_x = closed_nondegen.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);

    // Collapsed Bezier not glancing up nor down
    auto collapsed = string_to_path("M 10, 0 Q -10 0 10, 0");
    extrema_x = collapsed.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_EQ(extrema_x.max_point, Point(10, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, 0.0);
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_max, 0.0);

    // Degenerate segments at min X
    auto degen = string_to_path("M 0.01,20 L 0, 0 H 0 V 0 L 0,0 V 0 L 0.02 -30");
    extrema_x = degen.extrema(X);
    EXPECT_EQ(extrema_x.min_point, Point(0, 0));
    EXPECT_FLOAT_EQ(extrema_x.glance_direction_at_min, -1.0);
}

/** Regression test for issue https://gitlab.com/inkscape/lib2geom/-/issues/50 */
TEST_F(PathTest, PizzaSlice)
{
    auto pv = parse_svg_path("M 0 0 L 0.30901699437494745 0.9510565162951535 "
                             "A 1 1 0 0 1 -0.8090169943749473 0.5877852522924732 z");
    auto &sector = pv[0];
    Path piece;
    EXPECT_NO_THROW(piece = sector.portion(PathTime(0, 0.0), PathTime(2, 0.0), false));
    EXPECT_FALSE(piece.closed());
    EXPECT_TRUE(piece.size() == 2 ||
               (piece.size() == 3 && piece[2].isDegenerate()));
    EXPECT_EQ(piece.finalPoint(), Point(-0.8090169943749473, 0.5877852522924732));

    // Test slicing in the middle of an arc and past its end
    pv = parse_svg_path("M 0,0 H 1 A 1,1 0 0 1 0.3080657835086775,0.9513650577098072 z");
    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(1, 0.5), PathTime(2, 1.0)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.finalPoint(), pv[0].finalPoint());

    // Test slicing from before the start to a point on the arc
    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(0, 0.5), PathTime(1, 0.5)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.initialPoint(), pv[0].pointAt(PathTime(0, 0.5)));
    EXPECT_EQ(piece.finalPoint(), pv[0].pointAt(PathTime(1, 0.5)));

    // Test slicing a part of the arc
    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(1, 0.25), PathTime(1, 0.75)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.size(), 1);

    // Test slicing in reverse
    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(2, 1.0), PathTime(1, 0.5)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.finalPoint(), pv[0].pointAt(PathTime(1, 0.5)));

    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(1, 0.5), PathTime(0, 0.5)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.initialPoint(), pv[0].pointAt(PathTime(1, 0.5)));
    EXPECT_EQ(piece.finalPoint(), pv[0].pointAt(PathTime(0, 0.5)));

    EXPECT_NO_THROW(piece = pv[0].portion(PathTime(1, 0.75), PathTime(1, 0.25)));
    EXPECT_FALSE(piece.closed());
    EXPECT_EQ(piece.size(), 1);
}

TEST_F(PathTest, SetFinal) {
    constexpr auto pt = Point(4, 4);

    auto path = p_single;
    path.setFinal(pt);
    EXPECT_EQ(path.finalPoint(), pt);

    path = line;
    path.setFinal(pt);
    EXPECT_EQ(path.finalPoint(), pt);
    EXPECT_EQ(path.initialPoint(), line.initialPoint());
}

TEST_F(PathTest, SetInitial) {
    constexpr auto pt = Point(4, 4);

    auto path = p_single;
    path.setInitial(pt);
    EXPECT_EQ(path.initialPoint(), pt);

    path = line;
    path.setInitial(pt);
    EXPECT_EQ(path.initialPoint(), pt);
    EXPECT_EQ(path.finalPoint(), line.finalPoint());
}

TEST_F(PathTest, Length) {
    EXPECT_NEAR(p_single.length(), 0, 1e-3);
    EXPECT_NEAR(p_open.length(), 15, 1e-3);
    EXPECT_NEAR(line.length(), 1, 1e-3);
    EXPECT_NEAR(square.length(), 4, 1e-3);
    EXPECT_NEAR(circle.length(), 9 * M_PI, 1e-3);
    EXPECT_NEAR(cmds.length(), 962.32, 1e-2);
}

TEST_F(PathTest, MidTime) {
    PathTime midTime = p_single.midTime();
    EXPECT_TRUE((midTime.curve_index == 0) && (midTime.t == 0.0));
    midTime = p_open.midTime();
    EXPECT_TRUE((midTime.curve_index == 1) && (midTime.t == 0.5));
    midTime = line.midTime();
    EXPECT_TRUE((midTime.curve_index == 0) && (midTime.t == 0.5));
    midTime = square.midTime();
    EXPECT_TRUE((midTime.curve_index == 1) && (midTime.t == 1));
    midTime = circle.midTime();
    EXPECT_TRUE((midTime.curve_index == 0) && (midTime.t == 1));
    midTime = cmds.midTime(1e-3);
    EXPECT_NEAR(cmds.portion(PathTime(0,0), midTime).length(), cmds.portion(midTime, PathTime(cmds.size_default() - 1,1)).length(), 1e-3);
    midTime = p_a3Bezier.midTime(1e-3);
    EXPECT_NEAR(p_a3Bezier.portion(PathTime(0,0), midTime).length(), p_a3Bezier.portion(midTime, PathTime(0,1)).length(), 1e-3);
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
