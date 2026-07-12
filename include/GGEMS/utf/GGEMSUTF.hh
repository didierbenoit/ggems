#pragma once
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
 * \file GGEMSUTF.hh
 * \brief UTF-8 and UTF-32 conversion utilities for GGEMS.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright GNU GPL v3
 *
 * This header provides small, allocation-based helpers to convert between
 * UTF-8 encoded byte sequences and UTF-32 code point strings. These utilities
 * are intended for internal text handling in GGEMS, particularly when dealing
 * with glyph rendering in terminal or GUI front-ends.
 *
 * Invalid UTF-8 byte sequences are normalised to the replacement character
 * U'?' so that the resulting UTF-32 string is always well-formed. When
 * converting back to UTF-8, out-of-range code points are mapped to the
 * ASCII '?' (0x3F).
 */

/// \cond
#include <format>
#include <string>
#include <string_view>
/// \endcond

namespace ggems::utf {
/*!
 * \brief Convert a UTF-8 encoded string into a UTF-32 code point sequence.
 *
 * \param str8 UTF-8 encoded input string view.
 * \return A \c std::u32string containing the decoded Unicode code points.
 *
 * Invalid byte sequences are replaced by the replacement code point U'?'.
 * The implementation relies on \c mbrtoc32 and respects the current C
 * locale's multibyte conversion rules.
 */
std::u32string UTF8ToUTF32(std::string_view str8);

/*!
 * \brief Convert a single UTF-32 code point to UTF-8.
 *
 * \param ch32 UTF-32 code point to convert.
 * \return A UTF-8 encoded \c std::string representing \p ch32.
 *
 * If \p ch32 is outside the valid Unicode scalar value range
 * [0x0, 0x10FFFF], the function returns the single-character string "?".
 */
std::string UTF32ToUTF8(char32_t ch32);

/*!
 * \brief Convert a UTF-32 string view to a UTF-8 encoded string.
 *
 * \param str32 UTF-32 input string view.
 * \return A UTF-8 encoded \c std::string containing the encoding of
 *         all code points in \p str32, in order.
 *
 * Each code point is converted with \ref ggems::utf::UTF32ToUTF8, so invalid
 * values are mapped to "?" in the resulting UTF-8 string.
 */
std::string UTF32ToUTF8(std::u32string_view str32);
} // namespace ggems::utf

/// \endcond
