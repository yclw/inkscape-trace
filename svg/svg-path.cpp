// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * svg-path.cpp: Parse SVG path element data into bezier path.
 * Authors:
 *   Johan Engelen
 *   (old nartbpath code that has been deleted: Raph Levien <raph@artofcode.com>)
 *   (old nartbpath code that has been deleted: Lauris Kaplinski <lauris@ximian.com>)
 *
 * Copyright (C) 2000 Eazel, Inc.
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2008 Johan Engelen
 *
 * Copyright (C) 2000-2008 authors
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cstring>
#include <string>
#include <glib.h> // g_assert()

#include <2geom/pathvector.h>
#include <2geom/curves.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/path-sink.h>
#include <2geom/svg-path-parser.h>

#include "svg/svg.h"
#include "svg/path-string.h"

/*
 * Parses the path in str. When an error is found in the pathstring, this method
 * returns a truncated path up to where the error was found in the pathstring.
 * Returns an empty PathVector when str==NULL
 */
Geom::PathVector sp_svg_read_pathv(char const * str)
{
    Geom::PathVector pathv;
    if (!str)
        return pathv;  // return empty pathvector when str == NULL

    Geom::PathBuilder builder(pathv);
    Geom::SVGPathParser parser(builder);
    parser.setZSnapThreshold(Geom::EPSILON);

    try {
        parser.parse(str);
    }
    catch (Geom::SVGPathParseError &e) {
        builder.flush();
        // This warning is extremely annoying when testing
        g_warning(
            "Malformed SVG path, truncated path up to where error was found.\n Input path=\"%s\"\n Parsed path=\"%s\"",
            str, sp_svg_write_path(pathv).c_str());
    }

    return pathv;
}

static void sp_svg_write_curve(Inkscape::SVG::PathString & str, Geom::Curve const * c, bool normalize = false) {
    // TODO: this code needs to removed and replaced by appropriate path sink
    if(Geom::LineSegment const *line_segment = dynamic_cast<Geom::LineSegment const  *>(c)) {
        // don't serialize stitch segments
        if (!dynamic_cast<Geom::Path::StitchSegment const *>(c)) {
            if (!normalize && line_segment->initialPoint()[Geom::X] == line_segment->finalPoint()[Geom::X]) {
                str.verticalLineTo( line_segment->finalPoint()[Geom::Y] );
            } else if (!normalize && line_segment->initialPoint()[Geom::Y] == line_segment->finalPoint()[Geom::Y]) {
                str.horizontalLineTo( line_segment->finalPoint()[Geom::X] );
            } else {
                str.lineTo( (*line_segment)[1][0], (*line_segment)[1][1] );
            }
        }
    }
    else if(Geom::QuadraticBezier const *quadratic_bezier = dynamic_cast<Geom::QuadraticBezier const  *>(c)) {
        if (normalize) {
            // Convert to bezier curve
            double x1 = 1.0 / 3 * quadratic_bezier->initialPoint()[0] + 2.0 / 3 * (*quadratic_bezier)[1][0];
            double x2 = 2.0 / 3 * (*quadratic_bezier)[1][0] + 1.0 / 3 * (*quadratic_bezier)[2][0];
            double y1 = 1.0 / 3 * quadratic_bezier->initialPoint()[1] + 2.0 / 3 * (*quadratic_bezier)[1][1];
            double y2 = 2.0 / 3 * (*quadratic_bezier)[1][1] + 1.0 / 3 * (*quadratic_bezier)[2][1];
            str.curveTo(x1, y1, x2, y2, (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1]);
        } else {
            str.quadTo( (*quadratic_bezier)[1][0], (*quadratic_bezier)[1][1],
                        (*quadratic_bezier)[2][0], (*quadratic_bezier)[2][1] );
        }
    }
    else if(Geom::CubicBezier const *cubic_bezier = dynamic_cast<Geom::CubicBezier const  *>(c)) {
        str.curveTo( (*cubic_bezier)[1][0], (*cubic_bezier)[1][1],
                     (*cubic_bezier)[2][0], (*cubic_bezier)[2][1],
                     (*cubic_bezier)[3][0], (*cubic_bezier)[3][1] );
    }
    else if(Geom::EllipticalArc const *elliptical_arc = dynamic_cast<Geom::EllipticalArc const *>(c)) {
        str.arcTo( elliptical_arc->ray(Geom::X), elliptical_arc->ray(Geom::Y),
                   Geom::deg_from_rad(elliptical_arc->rotationAngle()),
                   elliptical_arc->largeArc(), elliptical_arc->sweep(),
                   elliptical_arc->finalPoint() );
    } else { 
        //this case handles sbasis as well as all other curve types
        Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c->toSBasis(), 0.1);

        //recurse to convert the new path resulting from the sbasis to svgd
        for(const auto & iter : sbasis_path) {
            sp_svg_write_curve(str, &iter, normalize);
        }
    }
}

static void sp_svg_write_path(Inkscape::SVG::PathString & str, Geom::Path const & p, bool normalize = false) {
    str.moveTo( p.initialPoint()[0], p.initialPoint()[1] );

    for(Geom::Path::const_iterator cit = p.begin(); cit != p.end_open(); ++cit) {
        sp_svg_write_curve(str, &(*cit), normalize);
    }

    if (p.closed()) {
        str.closePath();
    }
}

std::string sp_svg_write_path(Geom::PathVector const &p, bool normalize) {
    Inkscape::SVG::PathString str;
    if (normalize) {
        str = Inkscape::SVG::PathString(Inkscape::SVG::PATHSTRING_ABSOLUTE, 4, -2, true);
    }

    for(const auto & pit : p) {
        sp_svg_write_path(str, pit, normalize);
    }

    return str;
}

std::string sp_svg_write_path(Geom::Path const &p) {
    Inkscape::SVG::PathString str;

    sp_svg_write_path(str, p);

    return str;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
