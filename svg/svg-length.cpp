// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * SVG data parser
 *//*
 * Authors: see git history
 
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <cmath>
#include <cstring>
#include <string>
#include <glib.h>
#include <iostream>
#include <vector>

#include "svg.h"
#include "stringstream.h"
#include "util/units.h"
#include "util/numeric/converters.h"

using std::pow;

static unsigned sp_svg_length_read_lff(gchar const *str, SVGLength::Unit *unit, float *val, float *computed, char **next);

#ifndef MAX
# define MAX(a,b) ((a < b) ? (b) : (a))
#endif

unsigned int sp_svg_number_read_f(gchar const *str, float *val)
{
    if (!str) {
        return 0;
    }

    char *e;
    float const v = g_ascii_strtod(str, &e);
    if ((gchar const *) e == str) {
        return 0;
    }

    *val = v;
    return 1;
}

unsigned int sp_svg_number_read_d(gchar const *str, double *val)
{
    if (!str) {
        return 0;
    }

    char *e;
    double const v = g_ascii_strtod(str, &e);
    if ((gchar const *) e == str) {
        return 0;
    }

    *val = v;
    return 1;
}

static std::string sp_svg_number_write_d( double val, unsigned int tprec, unsigned int fprec)
{

    std::string buf;
    /* Process sign */
    if (val < 0.0) {
        buf.append("-");
        val = fabs(val);
    }

    /* Determine number of integral digits */
    int idigits = 0;
    if (val >= 1.0) {
        idigits = (int) floor(log10(val)) + 1;
    }

    /* Determine the actual number of fractional digits */
    fprec = MAX(static_cast<int>(fprec), static_cast<int>(tprec) - idigits);
    /* Round value */
    val += 0.5 / pow(10.0, fprec);
    /* Extract integral and fractional parts */
    double dival = floor(val);
    double fval = val - dival;
    /* Write integra */
    if (idigits > (int)tprec) {
        buf.append(std::to_string((unsigned int)floor(dival/pow(10.0, idigits-tprec) + .5)));
        for(unsigned int j=0; j<(unsigned int)idigits-tprec; j++) {
            buf.append("0");
        }
    } else {
       buf.append(std::to_string((unsigned int)dival));
    }

    if (fprec > 0 && fval > 0.0) {
        std::string s(".");
        do {
            fval *= 10.0;
            dival = floor(fval);
            fval -= dival;
            int const int_dival = (int) dival;
            s.append(std::to_string(int_dival));
            if(int_dival != 0){
                buf.append(s);
                s="";
            }
            fprec -= 1;
        } while(fprec > 0 && fval > 0.0);
    }
    return buf;
}

std::string sp_svg_number_write_de(double val, unsigned int tprec, int min_exp)
{
    std::string buf;
    int eval = (int)floor(log10(fabs(val)));
    if (val == 0.0 || eval < min_exp) {
        buf.append("0");
        return buf;
    }
    unsigned int maxnumdigitsWithoutExp = // This doesn't include the sign because it is included in either representation
        eval<0?tprec+(unsigned int)-eval+1:
        eval+1<(int)tprec?tprec+1:
        (unsigned int)eval+1;
    unsigned int maxnumdigitsWithExp = tprec + ( eval<0 ? 4 : 3 ); // It's not necessary to take larger exponents into account, because then maxnumdigitsWithoutExp is DEFINITELY larger
    if (maxnumdigitsWithoutExp <= maxnumdigitsWithExp) {
        buf.append(sp_svg_number_write_d(val, tprec, 0));
    } else {
        val = eval < 0 ? val * pow(10.0, -eval) : val / pow(10.0, eval);
        buf.append(sp_svg_number_write_d(val, tprec, 0));
        buf.append("e");
        buf.append(std::to_string(eval));
    }
    return buf;

}

SVGLength::SVGLength()
    : _set(false)
    , unit(NONE)
    , value(0)
    , computed(0)
{
}

/* Length */

bool SVGLength::read(gchar const *str)
{
    if (!str) {
        return false;
    }

    SVGLength::Unit u;
    float v;
    float c;
    if (!sp_svg_length_read_lff(str, &u, &v, &c, nullptr)) {
        return false;
    }

    if (!std::isfinite(v)) {
        return false;
    }

    _set = true;
    unit = u;
    value = v;
    computed = c;

    return true;
}

bool SVGLength::readAbsolute(gchar const *str)
{
    if (!str) {
        return false;
    }

    SVGLength::Unit u;
    float v;
    float c;
    if (!sp_svg_length_read_lff(str, &u, &v, &c, nullptr)) {
        return false;
    }

    if (svg_length_absolute_unit(u) == false) {
        return false;
    }

    _set = true;
    unit = u;
    value = v;
    computed = c;

    return true;
}

/**
 * Returns the unit used as a string.
 *
 * @returns unit string
 */
std::string SVGLength::getUnit() const
{
    return sp_svg_length_get_css_units(unit);
}

/**
 * Is this length an absolute value (uses an absolute unit).
 *
 * @returns true if unit is not NONE and not a relative unit (percent etc)
 */
bool SVGLength::isAbsolute()
{
    return unit && svg_length_absolute_unit(unit);
}

unsigned int sp_svg_length_read_computed_absolute(gchar const *str, float *length)
{
    if (!str) {
        return 0;
    }

    SVGLength::Unit unit;
    float computed;
    if (!sp_svg_length_read_lff(str, &unit, nullptr, &computed, nullptr)) {
        // failed to read
        return 0;
    }

    if (svg_length_absolute_unit(unit) == false) {
        return 0;
    }

    *length = computed;

    return 1;
}

std::vector<SVGLength> sp_svg_length_list_read(gchar const *str)
{
    if (!str) {
        return std::vector<SVGLength>();
    }

    SVGLength::Unit unit;
    float value;
    float computed;
    char *next = (char *) str;
    std::vector<SVGLength> list;

    while (sp_svg_length_read_lff(next, &unit, &value, &computed, &next)) {

        SVGLength length;
        length.set(unit, value, computed);
        list.push_back(length);

        while (next && *next &&
               (*next == ',' || *next == ' ' || *next == '\n' || *next == '\r' || *next == '\t')) {
            // the list can be comma- or space-separated, but we will be generous and accept
            // a mix, including newlines and tabs
            next++;
        }

        if (!next || !*next) {
            break;
        }
    }

    return list;
}


#define UVAL(a,b) (((unsigned int) (a) << 8) | (unsigned int) (b))

static unsigned sp_svg_length_read_lff(gchar const *str, SVGLength::Unit *unit, float *val, float *computed, char **next)
{
/* note: this function is sometimes fed a string with several consecutive numbers, e.g. by sp_svg_length_list_read.
So after the number, the string does not necessarily have a \0 or a unit, it might also contain a space or comma and then the next number!
*/

    if (!str) {
        return 0;
    }

    gchar const *e;
    float const v = g_ascii_strtod(str, (char **) &e);
    if (e == str) {
        return 0;
    }

    if (!e[0]) {
        /* Unitless */
        if (unit) {
            *unit = SVGLength::NONE;
        }
        if (val) {
            *val = v;
        }
        if (computed) {
            *computed = v;
        }
        if (next) {
            *next = nullptr; // no more values
        }
        return 1;
    } else if (!g_ascii_isalnum(e[0])) {
        /* Unitless or percent */
        if (e[0] == '%') {
            /* Percent */
            if (e[1] && g_ascii_isalnum(e[1])) {
                return 0;
            }
            if (unit) {
                *unit = SVGLength::PERCENT;
            }
            if (val) {
                *val = v * 0.01;
            }
            if (computed) {
                *computed = v * 0.01;
            }
            if (next) {
                *next = (char *) e + 1;
            }
            return 1;
        } else if (g_ascii_isspace(e[0]) && e[1] && g_ascii_isalpha(e[1])) {
            return 0; // spaces between value and unit are not allowed
        } else {
            /* Unitless */
            if (unit) {
                *unit = SVGLength::NONE;
            }
            if (val) {
                *val = v;
            }
            if (computed) {
                *computed = v;
            }
            if (next) {
                *next = (char *) e;
            }
            return 1;
        }
    } else if (e[1] && !g_ascii_isalnum(e[2])) {
        /* TODO: Allow the number of px per inch to vary (document preferences, X server
         * or whatever).  E.g. don't fill in computed here, do it at the same time as
         * percentage units are done. */
        unsigned int const uval = UVAL(e[0], e[1]);
        switch (uval) {
            case UVAL('p','x'):
                if (unit) {
                    *unit = SVGLength::PX;
                }
                if (computed) {
                    *computed = v;
                }
                break;
            case UVAL('p','t'):
                if (unit) {
                    *unit = SVGLength::PT;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "pt", "px");
                }
                break;
            case UVAL('p','c'):
                if (unit) {
                    *unit = SVGLength::PC;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "pc", "px");
                }
                break;
            case UVAL('m','m'):
                if (unit) {
                    *unit = SVGLength::MM;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "mm", "px");
                }
                break;
            case UVAL('c','m'):
                if (unit) {
                    *unit = SVGLength::CM;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "cm", "px");
                }
                break;
            case UVAL('i','n'):
                if (unit) {
                    *unit = SVGLength::INCH;
                }
                if (computed) {
                    *computed = Inkscape::Util::Quantity::convert(v, "in", "px");
                }
                break;
            case UVAL('e','m'):
                if (unit) {
                    *unit = SVGLength::EM;
                }
                break;
            case UVAL('e','x'):
                if (unit) {
                    *unit = SVGLength::EX;
                }
                break;
            default:
                /* Invalid */
                return 0;
                break;
        }
        if (val) {
            *val = v;
        }
        if (next) {
            *next = (char *) e + 2;
        }
        return 1;
    }

    /* Invalid */
    return 0;
}

unsigned int sp_svg_length_read_ldd(gchar const *str, SVGLength::Unit *unit, double *value, double *computed)
{
    float a;
    float b;
    unsigned int r = sp_svg_length_read_lff(str, unit, &a, &b, nullptr);
    if (r) {
        if (value) {
            *value = a;
        }
        if (computed) {
            *computed = b;
        }
    }
    return r;
}

std::string SVGLength::write() const
{
    return sp_svg_length_write_with_units(*this);
}

/**
 * Write out length in user unit, for the user to use.
 *
 * @param out_unit - The unit to convert the computed px into
 * @returns a string containing the value in the given units
 */
std::string SVGLength::toString(const std::string &out_unit, double doc_scale, std::optional<unsigned int> precision, bool add_unit) const
{
    if (unit == SVGLength::PERCENT) {
        return write();
    }
    double value = toValue(out_unit) * doc_scale;
    Inkscape::SVGOStringStream os;
    if (precision) {
        os << Inkscape::Util::format_number(value, *precision);
    } else {
        os << value;
    }
    if (add_unit)
        os << out_unit;
    return os.str();
}

/**
 * Caulate the length in a user unit.
 *
 * @param out_unit - The unit to convert the computed px into
 * @returns a double of the computed value in this unit
 */
double SVGLength::toValue(const std::string &out_unit) const
{
    return Inkscape::Util::Quantity::convert(computed, "px", out_unit);
}

/**
 * Read from user input, any non-unitised value is converted internally.
 *
 * @param input - The string input
 * @param default_unit - The unit used by the display. Set to empty string for xml reading.
 * @param doc_scale - The scale values with units should apply to make those units correct for this document.
 */
bool SVGLength::fromString(const std::string &input, const std::string &default_unit, std::optional<double> doc_scale)
{
    if (!read((input + default_unit).c_str()))
        if (!read(input.c_str()))
            return false;
    // Rescale real units to document, since user input is not scaled
    if (doc_scale && unit != SVGLength::PERCENT && unit != SVGLength::NONE) {
        value = computed;
        unit = SVGLength::NONE;
        scale(1 / *doc_scale);
    }
    return true;
}

void SVGLength::set(SVGLength::Unit u, float v)
{
    _set = true;
    unit = u;
    Glib::ustring hack("px");
    switch( unit ) {
        case NONE:
        case PX:
        case EM:
        case EX:
        case PERCENT:
            break;
        case PT:
            hack = "pt";
            break;
        case PC:
            hack = "pc";
            break;
        case MM:
            hack = "mm";
            break;
        case CM:
            hack = "cm";
            break;
        case INCH:
            hack = "in";
            break;
        default:
            break;
    }
    value = v;
    computed =  Inkscape::Util::Quantity::convert(v, hack, "px");
}

void SVGLength::set(SVGLength::Unit u, float v, float c)
{
    _set = true;
    unit = u;
    value = v;
    computed = c;
}

void SVGLength::unset(SVGLength::Unit u, float v, float c)
{
    _set = false;
    unit = u;
    value = v;
    computed = c;
}

void SVGLength::scale(double scale)
{
    value *= scale;
    computed *= scale;
}

void SVGLength::update(double em, double ex, double scale)
{
    if (unit == EM) {
        computed = value * em;
    } else if (unit == EX) {
        computed = value * ex;
    } else if (unit == PERCENT) {
        computed = value * scale;
    }
}

double sp_svg_read_percentage(char const *str, double def)
{
    if (str == nullptr) {
        return def;
    }

    char *u;
    double v = g_ascii_strtod(str, &u);
    while (isspace(*u)) {
        if (*u == '\0') {
            return v;
        }
        u++;
    }
    if (*u == '%') {
        v /= 100.0;
    }

    return v;
}

gchar const *sp_svg_length_get_css_units(SVGLength::Unit unit)
{
    switch (unit) {
        case SVGLength::NONE: return "";
        case SVGLength::PX: return "";
        case SVGLength::PT: return "pt";
        case SVGLength::PC: return "pc";
        case SVGLength::MM: return "mm";
        case SVGLength::CM: return "cm";
        case SVGLength::INCH: return "in";
        case SVGLength::EM: return "em";
        case SVGLength::EX: return "ex";
        case SVGLength::PERCENT: return "%";
    }
    return "";
}

bool svg_length_absolute_unit(SVGLength::Unit u)
{
    return (u != SVGLength::EM && u != SVGLength::EX && u != SVGLength::PERCENT);
}

/**
 * N.B.\ This routine will sometimes return strings with `e' notation, so is unsuitable for CSS
 * lengths (which don't allow scientific `e' notation).
 */
std::string sp_svg_length_write_with_units(SVGLength const &length)
{
    Inkscape::SVGOStringStream os;
    if (length.unit == SVGLength::PERCENT) {
        os << 100*length.value << sp_svg_length_get_css_units(length.unit);
    } else {
        os << length.value << sp_svg_length_get_css_units(length.unit);
    }
    return os.str();
}


void SVGLength::readOrUnset(gchar const *str, Unit u, float v, float c)
{
    if (!read(str)) {
        unset(u, v, c);
    }
}

namespace Inkscape {
char const *refX_named_to_percent(char const *str)
{
    if (str) {
        if (g_str_equal(str, "left")) {
            return "0%";
        } else if (g_str_equal(str, "center")) {
            return "50%";
        } else if (g_str_equal(str, "right")) {
            return "100%";
        }
    }
    return str;
}

char const *refY_named_to_percent(char const *str)
{
    if (str) {
        if (g_str_equal(str, "top")) {
            return "0%";
        } else if (g_str_equal(str, "center")) {
            return "50%";
        } else if (g_str_equal(str, "bottom")) {
            return "100%";
        }
    }
    return str;
}
} // namespace Inkscape

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
