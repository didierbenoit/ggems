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
 * \brief Documents tests for GGEMS UTF conversion helpers.
 *
 * Validates UTF-8/UTF-32 scalar boundaries, embedded NUL values, GGEMS Unicode characters, invalid UTF-8 sequences, surrogate rejection, and out-of-range replacement behavior.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/utf/GGEMSUTF.hh"

/// \cond

namespace {

// =============================================================================
// =============================================================================

struct ScalarEncodingCase {
  std::string_view name;
  char32_t code_point;
  std::string_view expected_utf8;
};

// =============================================================================
// =============================================================================

constexpr std::array<ScalarEncodingCase, 29U> k_scalar_encoding_cases{{
    {.name = "U+007F",
     .code_point = static_cast<char32_t>(0x007F),
     .expected_utf8 = "\x7F"},
    {.name = "U+0080",
     .code_point = static_cast<char32_t>(0x0080),
     .expected_utf8 = "\xC2\x80"},
    {.name = "U+07FF",
     .code_point = static_cast<char32_t>(0x07FF),
     .expected_utf8 = "\xDF\xBF"},
    {.name = "U+0800",
     .code_point = static_cast<char32_t>(0x0800),
     .expected_utf8 = "\xE0\xA0\x80"},
    {.name = "U+D7FF",
     .code_point = static_cast<char32_t>(0xD7FF),
     .expected_utf8 = "\xED\x9F\xBF"},
    {.name = "U+E000",
     .code_point = static_cast<char32_t>(0xE000),
     .expected_utf8 = "\xEE\x80\x80"},
    {.name = "U+FFFF",
     .code_point = static_cast<char32_t>(0xFFFF),
     .expected_utf8 = "\xEF\xBF\xBF"},
    {.name = "U+10000",
     .code_point = static_cast<char32_t>(0x10000),
     .expected_utf8 = "\xF0\x90\x80\x80"},
    {.name = "U+10FFFF",
     .code_point = static_cast<char32_t>(0x10FFFF),
     .expected_utf8 = "\xF4\x8F\xBF\xBF"},
    {.name = "above Unicode maximum",
     .code_point = static_cast<char32_t>(0x110000),
     .expected_utf8 = "?"},
    {.name = "micro sign", .code_point = U'µ', .expected_utf8 = "\xC2\xB5"},
    {.name = "superscript two",
     .code_point = U'²',
     .expected_utf8 = "\xC2\xB2"},
    {.name = "superscript three",
     .code_point = U'³',
     .expected_utf8 = "\xC2\xB3"},
    {.name = "gamma", .code_point = U'γ', .expected_utf8 = "\xCE\xB3"},
    {.name = "beta", .code_point = U'β', .expected_utf8 = "\xCE\xB2"},
    {.name = "lambda", .code_point = U'λ', .expected_utf8 = "\xCE\xBB"},
    {.name = "alpha", .code_point = U'α', .expected_utf8 = "\xCE\xB1"},
    {.name = "nu", .code_point = U'ν', .expected_utf8 = "\xCE\xBD"},
    {.name = "bullet", .code_point = U'•', .expected_utf8 = "\xE2\x80\xA2"},
    {.name = "box drawings light horizontal",
     .code_point = U'─',
     .expected_utf8 = "\xE2\x94\x80"},
    {.name = "full block", .code_point = U'█', .expected_utf8 = "\xE2\x96\x88"},
    {.name = "box drawings double down and right",
     .code_point = U'╔',
     .expected_utf8 = "\xE2\x95\x94"},
    {.name = "box drawings double down and left",
     .code_point = U'╗',
     .expected_utf8 = "\xE2\x95\x97"},
    {.name = "box drawings double up and right",
     .code_point = U'╚',
     .expected_utf8 = "\xE2\x95\x9A"},
    {.name = "box drawings double up and left",
     .code_point = U'╝',
     .expected_utf8 = "\xE2\x95\x9D"},
    {.name = "box drawings double horizontal",
     .code_point = U'═',
     .expected_utf8 = "\xE2\x95\x90"},
    {.name = "box drawings double vertical",
     .code_point = U'║',
     .expected_utf8 = "\xE2\x95\x91"},
    {.name = "box drawings vertical single and right double",
     .code_point = U'╟',
     .expected_utf8 = "\xE2\x95\x9F"},
    {.name = "box drawings vertical single and left double",
     .code_point = U'╢',
     .expected_utf8 = "\xE2\x95\xA2"},
}};

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, EncodesScalarBoundariesAndGgemsCharacters) {
  for (auto const &encoding_case : k_scalar_encoding_cases) {
    SCOPED_TRACE(std::string{encoding_case.name});
    EXPECT_EQ(ggems::utf::UTF32ToUTF8(encoding_case.code_point),
              encoding_case.expected_utf8);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, EncodesScalarNullAsOneByte) {
  std::string const expected(1U, '\0');
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(U'\0'), expected);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, EncodesUtf32StringViews) {
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(std::u32string_view{}), "");

  std::u32string const complete{U"Aµ─\U00010000"};
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(complete),
            "A\xC2\xB5\xE2\x94\x80\xF0\x90\x80\x80");

  std::u32string const with_null{U'A', U'\0', U'µ'};
  std::string const expected_with_null{"A\0\xC2\xB5", 4U};
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(with_null), expected_with_null);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, DecodesPortableAsciiUtf8) {
  EXPECT_EQ(ggems::utf::UTF8ToUTF32(std::string_view{""}), std::u32string{});
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("GGEMS 2.0"), U"GGEMS 2.0");

  std::string const with_null{'A', '\0', 'B'};
  std::u32string const expected_with_null{U'A', U'\0', U'B'};
  EXPECT_EQ(ggems::utf::UTF8ToUTF32(with_null), expected_with_null);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, DecodesMultibyteUtf8) {
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xC2\xB5"), U"µ");
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xCE\xB3"), U"γ");
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xE2\x94\x80"), U"─");
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF0\x90\x80\x80"),
            std::u32string{static_cast<char32_t>(0x10000)});
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, RejectsUtf32Surrogates) {
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(static_cast<char32_t>(0xD800)), "?");
  EXPECT_EQ(ggems::utf::UTF32ToUTF8(static_cast<char32_t>(0xDFFF)), "?");
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, RejectsInvalidUtf8) {
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\x80"), U"?");         // lone continuation
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xC2"), U"?");         // truncated 2-byte
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xE2\x82"), U"?");     // truncated 3-byte
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF0\x9F\x92"), U"?"); // truncated 4-byte

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xC0\x80"), U"?");     // overlong NUL
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xE0\x80\x80"), U"?"); // overlong
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF0\x80\x80\x80"),
            U"?"); // overlong

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xED\xA0\x80"), U"?"); // U+D800
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF4\x90\x80\x80"),
            U"?");                                          // > U+10FFFF
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xED\xBF\xBF"), U"?"); // U+DFFF
}

// =============================================================================
// =============================================================================

TEST(GGEMSUTFTest, DecodesUnicodeScalarBoundaries) {
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\x7F"),
            std::u32string{static_cast<char32_t>(0x007F)});
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xC2\x80"),
            std::u32string{static_cast<char32_t>(0x0080)});

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xDF\xBF"),
            std::u32string{static_cast<char32_t>(0x07FF)});
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xE0\xA0\x80"),
            std::u32string{static_cast<char32_t>(0x0800)});

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xED\x9F\xBF"),
            std::u32string{static_cast<char32_t>(0xD7FF)});
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xEE\x80\x80"),
            std::u32string{static_cast<char32_t>(0xE000)});

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xEF\xBF\xBF"),
            std::u32string{static_cast<char32_t>(0xFFFF)});
  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF0\x90\x80\x80"),
            std::u32string{static_cast<char32_t>(0x10000)});

  EXPECT_EQ(ggems::utf::UTF8ToUTF32("\xF4\x8F\xBF\xBF"),
            std::u32string{static_cast<char32_t>(0x10FFFF)});
}

} // namespace
/// \endcond
