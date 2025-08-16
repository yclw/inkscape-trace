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
#include <string>
#include <glib.h>

#ifndef MAX
# define MAX(a,b) ((a < b) ? (b) : (a))
#endif

using std::pow;

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

static std::string sp_svg_number_write_d(double val, unsigned int tprec, unsigned int fprec)
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