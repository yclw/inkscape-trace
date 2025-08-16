// SPDX-License-Identifier: GPL-2.0-or-later
//

#include "string-convert.h"

#include <assert.h>
#include <codecvt>
#include <locale>
#include <string>
#include <glibmm/ustring.h>

namespace Inkscape {

// std::wstring_convert is deprecated and will be removed in C++26,
// but it doesn't have a replacement yet in std library.

std::wstring utf8_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_utf8(const wchar_t* wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::string wstring_to_utf8(const gunichar* wstr, unsigned int count) {
    assert(wstr);
    Glib::ustring str{wstr, wstr + count};
    return str.raw();
}

std::string unicode_char_to_utf8(unsigned int unicode) {
    gunichar chr = unicode;
    Glib::ustring str{&chr, &chr + 1};
    return str.raw();
}

} // Inkscape
