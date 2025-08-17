// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SEEN_SP_SVG_H
#define SEEN_SP_SVG_H

/*
 * SVG utilities for TraceResult → SVG conversion
 * Authors:
 *   Inkscape Trace Project
 *
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */

#include <string>

/* Core number formatting functions used by PathString */

/*
 * Format number with specified precision and minimum exponent
 * Used for SVG coordinate output
 */
std::string sp_svg_number_write_de(double val, unsigned int tprec, int min_exp);

/*
 * Read number from string (used in relative coordinate calculations)
 * Returns 1 on success, 0 on failure
 */
unsigned int sp_svg_number_read_d(const char *str, double *val);

#endif // SEEN_SP_SVG_H
