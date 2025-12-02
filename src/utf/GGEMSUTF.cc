// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSUTF.cc
 * \brief UTF-8 and UTF-32 conversion utilities for GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 */

#include <cuchar>
#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::utf {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::u32string UTF8ToUTF32(std::string_view str8) {
  std::u32string out;
  std::mbstate_t st{};
  char const *src = str8.data();
  char const *end = src + str8.size();
  char32_t cp = 0;

  while (src < end) {
    std::size_t r =
        mbrtoc32(&cp, src, static_cast<std::size_t>(end - src), &st);
    if (r == static_cast<std::size_t>(-1) ||
        r == static_cast<std::size_t>(-2)) {
      cp = U'?';
      ++src;
    } else if (r == 0) {
      ++src;
    } else {
      src += r;
    }
    out.push_back(cp);
  }
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string UTF32ToUTF8(char32_t ch32) {
  std::string out;
  if (ch32 <= 0x7F) {
    out.push_back(static_cast<char>(ch32));
  } else if (ch32 <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | ((ch32 >> 6) & 0x1F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else if (ch32 <= 0xFFFF) {
    out.push_back(static_cast<char>(0xE0 | ((ch32 >> 12) & 0x0F)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else if (ch32 <= 0x10FFFF) {
    out.push_back(static_cast<char>(0xF0 | ((ch32 >> 18) & 0x07)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else {
    out = "?";
  }
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string UTF32ToUTF8(std::u32string_view str32) {
  std::string out;
  out.reserve(str32.size() * 4);
  for (char32_t cp : str32) {
    out += UTF32ToUTF8(cp);
  }
  return out;
}
} // namespace ggems::utf
