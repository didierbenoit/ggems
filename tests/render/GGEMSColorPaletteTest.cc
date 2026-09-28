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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <set>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSParticleColors.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace {

namespace render = ggems::render;

// =============================================================================
// =============================================================================

TEST(GGEMSColorPaletteTest, EveryFamilyContains64DistinctShades) {
  ASSERT_EQ(render::base_palette.size(), 9U);
  for (auto const &family : render::base_palette) {
    ASSERT_EQ(family.size(), 64U);
    std::set<std::array<std::uint8_t, 3U>> colors;
    for (auto const &color : family) {
      colors.insert({color.red, color.green, color.blue});
    }
    EXPECT_EQ(colors.size(), 64U);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorPaletteTest, GrayRampIsMonotoneFromBlackToNeutral252) {
  auto const &gray =
    render::base_palette[static_cast<std::size_t>(render::ColorFamily::Gray)];
  EXPECT_EQ(gray.front().red, 0U);
  EXPECT_EQ(gray.back().red, 252U);
  for (std::size_t index = 0U; index < gray.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(gray[index].red, gray[index].green);
    EXPECT_EQ(gray[index].red, gray[index].blue);
    if (index != 0U) {
      EXPECT_LT(gray[index - 1U].red, gray[index].red);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorPaletteTest, EveryShadeIsAccessibleAndOutOfRangeClampsToLast) {
  for (std::size_t family = 0U; family < 9U; ++family) {
    SCOPED_TRACE(family);
    auto const kind = static_cast<render::ColorFamily>(family);
    for (std::uint32_t shade = 0U; shade <= 255U; ++shade) {
      SCOPED_TRACE(shade);
      auto const actual = render::GetColorRGB(
        kind, static_cast<std::uint8_t>(shade), render::ColorVariant::Normal);
      auto const &expected =
        render::base_palette[family][shade < 64U ? shade : 63U];
      EXPECT_EQ(actual.red, expected.red);
      EXPECT_EQ(actual.green, expected.green);
      EXPECT_EQ(actual.blue, expected.blue);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorPaletteTest, NamedCssColorsResolveToTheirDeclaredRgb) {
  struct ColorCase {
    render::ColorKey key;
    std::array<std::uint8_t, 3U> rgb;
  };
  constexpr std::array cases{
    ColorCase{.key = render::GRAY_Black, .rgb = {0U, 0U, 0U}},
    ColorCase{.key = render::RED_Tomato, .rgb = {255U, 99U, 71U}},
    ColorCase{.key = render::ORANGE_Pure, .rgb = {255U, 165U, 0U}},
    ColorCase{.key = render::YELLOW_Gold, .rgb = {255U, 215U, 0U}},
    ColorCase{.key = render::GREEN_SeaGreen, .rgb = {46U, 139U, 87U}},
    ColorCase{.key = render::CYAN_Pure, .rgb = {0U, 255U, 255U}},
    ColorCase{.key = render::BLUE_Navy, .rgb = {0U, 0U, 128U}},
    ColorCase{.key = render::MAGENTA_Pure, .rgb = {255U, 0U, 255U}},
    ColorCase{.key = render::WHITE_Ivory, .rgb = {255U, 255U, 240U}},
  };
  for (auto const &test_case : cases) {
    SCOPED_TRACE(static_cast<std::uint32_t>(test_case.key.family));
    auto const actual = render::GetColorRGB(
      test_case.key.family, test_case.key.shade, test_case.key.variant);
    EXPECT_EQ(actual.red, test_case.rgb[0]);
    EXPECT_EQ(actual.green, test_case.rgb[1]);
    EXPECT_EQ(actual.blue, test_case.rgb[2]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorPaletteTest, ParticleColorsUseCurrentNamedPalette) {
  using Particle = ggems::core::particles::GGEMSParticleType;
  constexpr std::array particles{
    Particle::Unknown,  Particle::Aionino,  Particle::Gamma,
    Particle::Electron, Particle::Positron, Particle::Proton,
    Particle::Neutron,  Particle::Alpha,
  };

  constexpr std::array keys{
    render::GRAY_Neutral200_F,    render::MAGENTA_Electric,
    render::GREEN_SeaGreen,       render::CYAN_Cryo,
    render::MAGENTA_LightMagenta, render::RED_Tomato,
    render::GRAY_Silver,          render::YELLOW_Gold,
  };

  for (std::size_t index = 0U; index < particles.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(render::GetParticleColorKey(particles[index]), keys[index]);
    auto const actual = render::GetParticleRGB(particles[index]);
    auto const expected = render::GetColorRGB(
      keys[index].family, keys[index].shade, keys[index].variant);
    EXPECT_EQ(actual.red, expected.red);
    EXPECT_EQ(actual.green, expected.green);
    EXPECT_EQ(actual.blue, expected.blue);
  }
}

} // namespace
