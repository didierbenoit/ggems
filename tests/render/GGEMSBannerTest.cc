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
 * \brief Checks project identity and character encoding in the startup banner.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

auto BannerText(ggems::core::Encoding encoding) -> std::u32string {
  ggems::test::ScopedLoggerEncoding const scoped_encoding{encoding};
  std::u32string text;
  for (auto const &line : ggems::render::BuildBannerLines()) {
    for (auto const &segment : line.segments) {
      text.append(segment.text);
    }
    text.push_back(U'\n');
  }
  return text;
}

auto ExpectProjectIdentity(std::u32string_view text) -> void {
  EXPECT_NE(text.find(U"GPU Geant4-based Monte Carlo Simulations"), text.npos);
  EXPECT_NE(text.find(U"https://ggems.fr"), text.npos);
  EXPECT_NE(text.find(U"Julien Bert"), text.npos);
  EXPECT_NE(text.find(U"Didier Benoit"), text.npos);
  EXPECT_NE(text.find(U"GNU GPL"), text.npos);
}

} // namespace

// ASCII output must identify GGEMS and its license without emitting non-ASCII
// characters.
TEST(GGEMSBannerTest, BuildsCompleteAsciiBanner) {
  auto const text = BannerText(ggems::core::Encoding::Ascii);

  ExpectProjectIdentity(text);
  EXPECT_TRUE(std::ranges::all_of(
    text, [](char32_t value) -> bool { return value <= 0x7FU; }));
}

// Unicode output must retain project identity and use the selected Unicode
// artwork.
TEST(GGEMSBannerTest, BuildsCompleteUnicodeBanner) {
  auto const text = BannerText(ggems::core::Encoding::Unicode);

  ExpectProjectIdentity(text);
  EXPECT_TRUE(std::ranges::any_of(
    text, [](char32_t value) -> bool { return value > 0x7FU; }));
}
