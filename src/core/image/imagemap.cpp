// SPDX-License-Identifier: GPL-2.0-or-later
/** @file
 * TODO: insert short description here
 *//*
 * Authors: see git history
 *
 * Copyright (C) 2018 Authors
 * Released under GNU GPL v2+, read the file 'COPYING' for more information.
 */
#include <cstdio>
#include "core/image/imagemap.h"

namespace Inkscape {
namespace Trace {

/*
 * GrayMap
 */

GrayMap::GrayMap(int width, int height)
    : MapBase(width, height)
{
}

bool GrayMap::writePPM(char const *fileName)
{
    if (!fileName) {
        return false;
    }

    auto f = std::fopen(fileName, "wb");
    if (!f) {
        return false;
    }

    std::fprintf(f, "P6 %d %d 255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto pix = getPixel(x, y) / 3;
            unsigned char pixb = pix & 0xff;
            std::fputc(pixb, f);
            std::fputc(pixb, f);
            std::fputc(pixb, f);
        }
    }

    std::fclose(f);

    return true;
}

/*
 * RgbMap
 */

RgbMap::RgbMap(int width, int height)
    : MapBase(width, height)
{
}

bool RgbMap::writePPM(char const *fileName)
{
    if (!fileName) {
        return false;
    }

    auto f = std::fopen(fileName, "wb");
    if (!f) {
        return false;
    }

    std::fprintf(f, "P6 %d %d 255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto rgb = getPixel(x, y);
            std::fputc(rgb.r, f);
            std::fputc(rgb.g, f);
            std::fputc(rgb.b, f);
        }
    }

    std::fclose(f);

    return true;
}

/*
 * IndexedMap
 */

IndexedMap::IndexedMap(int width, int height)
    : MapBase(width, height)
    , nrColors(0)
{
    clut.fill(RGB{0, 0, 0});
}

bool IndexedMap::writePPM(char const *fileName)
{
    if (!fileName) {
        return false;
    }

    auto f = std::fopen(fileName, "wb");
    if (!f) {
        return false;
    }

    std::fprintf(f, "P6 %d %d 255\n", width, height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto rgb = getPixelValue(x, y);
            std::fputc(rgb.r, f);
            std::fputc(rgb.g, f);
            std::fputc(rgb.b, f);
        }
    }

    std::fclose(f);

    return true;
}

GrayMap rgbMapToGrayMap(RgbMap const &rgbmap) {
    auto graymap = GrayMap(rgbmap.width, rgbmap.height);
    
    for (int y = 0; y < rgbmap.height; y++) {
        for (int x = 0; x < rgbmap.width; x++) {
            auto rgb = rgbmap.getPixel(x, y);
            int alpha = 255;
            int white = 3 * (255 - alpha);
            unsigned long sample = (int)rgb.r + (int)rgb.g + (int)rgb.b;
            unsigned long bright = sample * alpha / 256 + white;
            
            graymap.setPixel(x, y, bright);
        }
    }
    
    return graymap;
}

RgbMap grayMapToRgbMap(GrayMap const &graymap) {
    auto rgbmap = RgbMap(graymap.width, graymap.height);
    
    for (int y = 0; y < graymap.height; y++) {
        for (int x = 0; x < graymap.width; x++) {
            unsigned long pix = graymap.getPixel(x, y) / 3;
            unsigned char val = pix & 0xff;
            rgbmap.setPixel(x, y, {val, val, val});
        }
    }
    
    return rgbmap;
}

RgbMap indexedMapToRgbMap(IndexedMap const &indexedmap) {
    auto rgbmap = RgbMap(indexedmap.width, indexedmap.height);
    
    for (int y = 0; y < indexedmap.height; y++) {
        for (int x = 0; x < indexedmap.width; x++) {
            auto rgb = indexedmap.getPixelValue(x, y);
            unsigned char r = rgb.r & 0xff;
            unsigned char g = rgb.g & 0xff;
            unsigned char b = rgb.b & 0xff;
            rgbmap.setPixel(x, y, {r, g, b});
        }
    }
    
    return rgbmap;
}

} // namespace Trace
} // namespace Inkscape
