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
 * \brief Declares UTF-8 and UTF-32 conversion helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string>
#include <string_view>

/// \endcond
/*!
 * \namespace ggems::utf
 * \brief Provides UTF conversion helpers used by GGEMS textual interfaces.
 */
namespace ggems::utf {

/*!
 * \brief Decodes a UTF-8 byte sequence into Unicode scalar values.
 *
 * Invalid, truncated, overlong, surrogate, and out-of-range sequences are
 * replaced by U+003F (``?``).
 *
 * \param[in] str8 UTF-8 byte sequence.
 * \return Decoded UTF-32 string.
 */
auto UTF8ToUTF32(std::string_view str8) -> std::u32string;

/*!
 * \brief Encodes one Unicode scalar value as UTF-8.
 *
 * UTF-32 surrogate values and values above U+10FFFF are replaced by ``?``.
 *
 * \param[in] ch32 UTF-32 code point to encode.
 * \return UTF-8 byte sequence for the code point or ``?`` when invalid.
 */
auto UTF32ToUTF8(char32_t ch32) -> std::string;

/*!
 * \brief Encodes a sequence of UTF-32 code points as UTF-8.
 *
 * Invalid UTF-32 values are replaced independently by ``?``.
 *
 * \param[in] str32 UTF-32 sequence to encode.
 * \return UTF-8 byte sequence.
 */
auto UTF32ToUTF8(std::u32string_view str32) -> std::string;

} // namespace ggems::utf
