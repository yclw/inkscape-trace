// SPDX-License-Identifier: GPL-2.0-or-later
//
// String conversion routines

#ifndef STRINGCONVERT_H
#define STRINGCONVERT_H

#include <string>
#include <glib.h>

namespace Inkscape {

// Convert UTF8-encoded string to wide-character string
std::wstring utf8_to_wstring(const std::string& str);

// Convert nul-terminated wide string to UTF9-encoded string
std::string wstring_to_utf8(const wchar_t* wstr);

// Convert string of 'count' Unicode chars into UTF8-encoded string
std::string wstring_to_utf8(const gunichar* wstr, unsigned int count);

// Convert single Unicode character into UTF8-encoded string
std::string unicode_char_to_utf8(unsigned int unicode);

} // Inkscape

#endif //STRINGCONVERT_H
