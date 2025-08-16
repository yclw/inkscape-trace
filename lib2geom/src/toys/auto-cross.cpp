/* @brief
 * A toy for playing around with Path::intersectSelf().
 *
 * Authors:
 *   Rafał Siejakowski <rs@rs-math.net>
 *
 * Copyright 2022 the Authors.
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

#include <toys/toy-framework-2.h>
#include <2geom/path.h>
#include <2geom/elliptical-arc.h>
#include <2geom/svg-path-parser.h>

using namespace Geom;
using Color = uint32_t;

Color const RED   = 0x80000000;
Color const GREEN = 0x00800000;
Color const BROWN = 0x90500000;
Color const BLUE  = 0x0000ff00;
Color const BLACK = 0x00000000;

static void set_cairo_rgb(cairo_t *c, Color rgb)
{
    cairo_set_source_rgba(c, (double)((rgb & 0xFF000000) >> 24) / 255.0,
                             (double)((rgb & 0x00FF0000) >> 16) / 255.0,
                             (double)((rgb & 0x0000FF00) >> 8) / 255.0,
                             1.0);
}

static void write_text(cairo_t *c, const char *text, Point const &position, Color color)
{
    cairo_move_to(c, position);
    cairo_set_font_size(c, 12);
    set_cairo_rgb(c, color);
    cairo_show_text(c, text);
}

static std::string format_point(Point const &pt)
{
    std::ostringstream ss;
    ss.precision(4);
    ss << pt;
    return ss.str();
}

static EllipticalArc random_arc(Point from, Point to)
{
    double const dist = distance(from, to);
    auto angle = atan2(to - from);
    bool sweep = std::abs(angle) > M_PI_2;
    angle *= 2;
    angle = std::fmod(angle, 2.0 * M_PI);
    return EllipticalArc(from, Point(0.5 * dist, 2.0 * dist), angle, false, sweep, to);
}

class Item
{
private:
    Path _path;
    Color _color;
    std::string _d;

public:
    Item(Color color)
        : _color{color}
    {}

    void setPath(Path &&new_path)
    {
        _path = std::forward<Path>(new_path);
        std::ostringstream oss;
        oss << _path;
        _d = oss.str();
    }

    void draw(cairo_t *cr) const
    {
        cairo_set_line_width(cr, 2);
        set_cairo_rgb(cr, _color);
        cairo_path(cr, _path);
        cairo_stroke(cr);
        _drawBezierTangents(cr);
        _drawSelfIntersections(cr);
    }

    void write(cairo_t *cr, Point const &pos) const
    {
        write_text(cr, _d.c_str(), pos, _color);
    }

    std::string const& getSVGD() const { return _d; }

private:
    void _drawBezierTangents(cairo_t *c) const
    {
        cairo_set_line_width(c, 1);
        set_cairo_rgb(c, 0x0000b000);
        // Draw tangents for Beziers:
        for (auto const &curve : _path) {
            if (auto const *bezier = dynamic_cast<BezierCurve const *>(&curve)) {
                if (bezier->order() > 1) {
                    auto points = bezier->controlPoints();
                    cairo_move_to(c, points[0]);
                    cairo_line_to(c, points[1]);
                    cairo_stroke(c);
                    cairo_move_to(c, points.back());
                    cairo_line_to(c, points[points.size() - 2]);
                    cairo_stroke(c);
                }
            }
        }
    }

    void _drawSelfIntersections(cairo_t *cr) const
    {
        set_cairo_rgb(cr, BLACK);
        for (auto const &xing : _path.intersectSelf()) {
            draw_cross(cr, xing.point());
            auto const coords = format_point(xing.point());
            write_text(cr, coords.c_str(), xing.point() + Point(8, -8), BLACK);
        }
    }
};

class AutoCross : public Toy
{
public:
    AutoCross()
        : items{Item(RED), Item(GREEN), Item(BROWN)}
    {
        bezier_handles.pts = { {200, 400}, {100, 300}, {300, 400}, {300, 300}, {450, 300}, {500, 500}, {400, 400} };
        elliptical_handles.pts = { {500, 200}, {700, 400}, {600, 500} };
        mixed_handles.pts = { {100, 600}, {120, 690}, {300, 650}, {330, 600}, {500, 800} };
        handles.push_back(&bezier_handles);
        handles.push_back(&elliptical_handles);
        handles.push_back(&mixed_handles);
    }

    void draw(cairo_t *cr, std::ostringstream *notify, int width, int height, bool save,
              std::ostringstream *timer_stream) override
    {
        if (crashed) {
            draw_error(cr, width, height);
        } else {
            try {
                draw_impl(cr, width, height);
            } catch (Exception &e) {
                error = e.what();
                handles.clear();
                crashed = true;
            }
        }
        Toy::draw(cr, notify, width, height, save, timer_stream);
    }

    void key_hit(unsigned keyval, unsigned modifiers) override
    {
        if (keyval == GDK_KEY_space) {
            print_path_d();
        } else if ((keyval == GDK_KEY_V || keyval == GDK_KEY_v) && (modifiers & GDK_CONTROL_MASK)) {
            paste_d();
        }
    }

private:
    std::string error;
    std::vector<Item> items;
    PointSetHandle bezier_handles, elliptical_handles, mixed_handles;
    bool crashed = false;

    void paste_d()
    {
        get_clipboard_text([this] (char const *text) {
            if (!text) {
                return;
            }

            PathVector pv;
            try {
                pv = parse_svg_path(text);
            } catch (SVGPathParseError const &error) {
                std::cerr << "Error pasting path d: " << error.what() << std::endl;
                return;
            }
            if (pv.empty()) {
                return;
            }
            Item paste_item{RED}; // TODO: cycle through a color palette.
            paste_item.setPath(std::move(pv[0]));
            items.push_back(paste_item);
            redraw();
        });
    }

    void print_path_d()
    {
        std::cout << "Path snapshots:\n";
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            std::cout << it->getSVGD() << '\n';
        }
    }

    void refresh_geometry()
    {
        // Construct the 2-segment Bézier path
        auto const &cp = bezier_handles.pts;
        Path bezier;
        bezier.append(BezierCurveN<3>(cp[0].round(), cp[1].round(), cp[2].round(), cp[3].round()));
        bezier.append(BezierCurveN<3>(cp[3].round(), cp[4].round(), cp[5].round(), cp[6].round()));
        items[0].setPath(std::move(bezier));

        // Construct the elliptical arcs
        auto const &ae = elliptical_handles.pts;
        Path elliptical;
        elliptical.append(random_arc(ae[0], ae[1]));
        elliptical.append(random_arc(ae[1], ae[2]));
        items[1].setPath(std::move(elliptical));

        // Construct a mixed path
        auto const &mh = mixed_handles.pts;
        Path mixed;
        mixed.append(BezierCurveN<3>(mh[0], mh[1], mh[2], mh[3]));
        mixed.append(random_arc(mh[3], mh[4]));
        mixed.close();
        items[2].setPath(std::move(mixed));
    }

    void draw_impl(cairo_t *cr, int width, int height)
    {
        refresh_geometry();
        write_title(cr);

        auto text_pos = Point(20, height - 20);
        for (auto const &item : items) {
            item.draw(cr);
            item.write(cr, text_pos);
            text_pos -= Point(0, 20);
        }
    }

    void write_title(cairo_t *c)
    {
        cairo_move_to(c, 10, 40);
        cairo_set_font_size(c, 30);
        set_cairo_rgb(c, 0x0);
        cairo_show_text(c, "Self-intersection of paths in lib2geom!");
        cairo_set_font_size(c, 14);
        cairo_move_to(c, 10, 60);
        cairo_show_text(c, "[Space]: Print SVG 'd' attributes to stdout");
        cairo_move_to(c, 10, 80);
        cairo_show_text(c, "[Ctrl-V]: Paste a 'd' attribute from clipboard");
    }

    void draw_error(cairo_t *cr, int width, int height)
    {
        auto center = Point(0.5 * (double)width, 0.5 * (double)height);
        cairo_move_to(cr, center + Point(-90, -100));
        cairo_line_to(cr, center + Point(100, 90));
        cairo_line_to(cr, center + Point(90, 100));
        cairo_line_to(cr, center + Point(-100, -90));
        cairo_close_path(cr);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_fill(cr);

        cairo_move_to(cr, center + Point(90, -100));
        cairo_line_to(cr, center + Point(100, -90));
        cairo_line_to(cr, center + Point(-90, 100));
        cairo_line_to(cr, center + Point(-100, 90));
        cairo_close_path(cr);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_fill(cr);

        cairo_move_to(cr, center + Point(-90, 120));
        cairo_show_text(cr, "Sorry, your toy has just broken :-/");
        cairo_move_to(cr, Point(10, center[Y] + 150));
        cairo_show_text(cr, error.c_str());
    }
};

int main(int argc, char **argv)
{
    auto toy = AutoCross();
    init(argc, argv, &toy, 800, 800);
    return 0;
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
