// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements UTF-8 and UTF-32 conversion helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cstddef>
#include <string>
#include <string_view>

/// \endcond
#include "GGEMS/utf/GGEMSUTF.hh"

namespace {

/// \cond
/*!
 * \brief Tests whether a byte has the UTF-8 continuation-byte prefix.
 *
 * \param[in] byte Byte to test.
 * \return True when the two most significant bits are ``10``.
 */
[[nodiscard]] auto IsUTF8ContinuationByte(unsigned char byte) noexcept -> bool {
  return (byte & 0xC0U) == 0x80U;
}
/// \endcond

} // namespace

namespace ggems::utf {

// =============================================================================
// =============================================================================

auto UTF8ToUTF32(std::string_view str8) -> std::u32string {
  std::u32string output;
  output.reserve(str8.size());

  std::size_t offset{0U};

  while (offset < str8.size()) {
    auto const first_byte = static_cast<unsigned char>(str8[offset]);

    if (first_byte <= 0x7FU) {
      output.push_back(static_cast<char32_t>(first_byte));
      ++offset;
      continue;
    }

    std::size_t expected_length{0U};
    char32_t code_point{0};
    char32_t minimum_code_point{0};
    bool valid_leading_byte{true};

    if (first_byte >= 0xC0U && first_byte <= 0xDFU) {
      expected_length = 2U;
      code_point = static_cast<char32_t>(first_byte & 0x1FU);
      minimum_code_point = 0x80;
      valid_leading_byte = first_byte >= 0xC2U;
    } else if (first_byte >= 0xE0U && first_byte <= 0xEFU) {
      expected_length = 3U;
      code_point = static_cast<char32_t>(first_byte & 0x0FU);
      minimum_code_point = 0x800;
    } else if (first_byte >= 0xF0U && first_byte <= 0xF7U) {
      expected_length = 4U;
      code_point = static_cast<char32_t>(first_byte & 0x07U);
      minimum_code_point = 0x10000;
      valid_leading_byte = first_byte <= 0xF4U;
    } else {
      output.push_back(U'?');
      ++offset;
      continue;
    }

    std::size_t consumed{1U};

    while (consumed < expected_length && offset + consumed < str8.size()) {
      auto const continuation_byte =
          static_cast<unsigned char>(str8[offset + consumed]);

      if (!IsUTF8ContinuationByte(continuation_byte)) {
        break;
      }

      code_point =
          (code_point << 6U) | static_cast<char32_t>(continuation_byte & 0x3FU);
      ++consumed;
    }

    if (consumed != expected_length) {
      output.push_back(U'?');
      offset += consumed;
      continue;
    }

    offset += expected_length;

    bool const is_surrogate = code_point >= 0xD800 && code_point <= 0xDFFF;

    if (!valid_leading_byte || code_point < minimum_code_point ||
        is_surrogate || code_point > 0x10FFFF) {
      output.push_back(U'?');
      continue;
    }

    output.push_back(code_point);
  }

  return output;
}

// =============================================================================
// =============================================================================

auto UTF32ToUTF8(char32_t ch32) -> std::string {
  std::string out;

  if (ch32 <= 0x7F) {
    out.push_back(static_cast<char>(ch32));
  } else if (ch32 <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | ((ch32 >> 6) & 0x1F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else if (ch32 >= 0xD800 && ch32 <= 0xDFFF) {
    return "?";
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

// =============================================================================
// =============================================================================

auto UTF32ToUTF8(std::u32string_view str32) -> std::string {
  std::string out;
  out.reserve(str32.size() * 4);
  for (char32_t code_point : str32) {
    out += UTF32ToUTF8(code_point);
  }
  return out;
}
} // namespace ggems::utf
