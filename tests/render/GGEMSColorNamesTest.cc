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
 * \brief Checks named palette aliases and color authoring.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <set>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorMagenta.hh"
#include "GGEMS/render/GGEMSColorOrange.hh"
#include "GGEMS/render/GGEMSColorBlue.hh"
#include "GGEMS/render/GGEMSColorGreen.hh"
#include "GGEMS/render/GGEMSColorWhite.hh"
#include "GGEMS/render/GGEMSColorCyan.hh"
#include "GGEMS/render/GGEMSColorRed.hh"

namespace {

namespace render = ggems::render;

// =============================================================================
// =============================================================================

constexpr auto k_defined_color = render::DefineColor(
  render::OrangeShade::CopperSignal, render::ColorVariant::Faint,
  render::ColorLayer::Background);

// =============================================================================
// =============================================================================

static_assert(k_defined_color == render::ColorKey{
                                   .family = render::ColorFamily::Orange,
                                   .shade = static_cast<std::uint8_t>(
                                     render::OrangeShade::CopperSignal),
                                   .variant = render::ColorVariant::Faint,
                                   .layer = render::ColorLayer::Background,
                                 });

} // namespace

// =============================================================================
// =============================================================================

// Typed shades must resolve to the intended palette family.
TEST(GGEMSColorNamesTest, MapsRepresentativeShadeEnumsToFamilies) {
  EXPECT_EQ(render::FamilyOf(render::GreenShade::Acid),
            render::ColorFamily::Green);
  EXPECT_EQ(render::FamilyOf(render::OrangeShade::CopperSignal),
            render::ColorFamily::Orange);
  EXPECT_EQ(render::FamilyOf(render::WhiteShade::White),
            render::ColorFamily::White);
}

// =============================================================================
// =============================================================================

// Color authoring must retain independently chosen shade, intensity, and output
// layer.
TEST(GGEMSColorNamesTest, DefineColorPreservesShadeVariantAndLayer) {
  constexpr auto color = render::DefineColor(render::MagentaShade::FleshSignal,
                                             render::ColorVariant::Bright,
                                             render::ColorLayer::Background);

  EXPECT_EQ(color.family, render::ColorFamily::Magenta);
  EXPECT_EQ(color.shade,
            static_cast<std::uint8_t>(render::MagentaShade::FleshSignal));
  EXPECT_EQ(color.variant, render::ColorVariant::Bright);
  EXPECT_EQ(color.layer, render::ColorLayer::Background);
}

// =============================================================================
// =============================================================================

// Default foreground and background must preserve the project's readable
// terminal palette.
TEST(GGEMSColorNamesTest, DefaultsUseExpectedNamedColors) {
  EXPECT_EQ(render::DEFAULT_FG, render::WHITE_Eggshell);
  EXPECT_EQ(render::DEFAULT_BG, render::GRAY_Neutral48_BG);
}

// =============================================================================
// =============================================================================

// Named color aliases must retain their requested intensity and
// foreground/background roles.
TEST(GGEMSColorNamesTest, RepresentativeNamedConstantsPreserveTheirKeys) {
  struct NamedColorCase {
    std::string_view label;
    render::ColorKey actual;
    render::ColorKey expected;
  };

  constexpr std::array test_cases{
    NamedColorCase{
      .label = "green acid",
      .actual = render::GREEN_Acid,
      .expected = render::DefineColor(render::GreenShade::Acid),
    },
    NamedColorCase{
      .label = "bright blue abyss",
      .actual = render::BLUE_Abyss_B,
      .expected = render::DefineColor(render::BlueShade::Abyss,
                                      render::ColorVariant::Bright),
    },
    NamedColorCase{
      .label = "faint red tomato",
      .actual = render::RED_Tomato_F,
      .expected = render::DefineColor(render::RedShade::Tomato,
                                      render::ColorVariant::Faint),
    },
    NamedColorCase{
      .label = "orange copper signal background",
      .actual = render::ORANGE_CopperSignal_BG,
      .expected = render::DefineColor(render::OrangeShade::CopperSignal,
                                      render::ColorVariant::Normal,
                                      render::ColorLayer::Background),
    },
    NamedColorCase{
      .label = "bright white background",
      .actual = render::WHITE_White_B_BG,
      .expected = render::DefineColor(render::WhiteShade::White,
                                      render::ColorVariant::Bright,
                                      render::ColorLayer::Background),
    },
    NamedColorCase{
      .label = "faint cyan cryo background",
      .actual = render::CYAN_Cryo_F_BG,
      .expected = render::DefineColor(render::CyanShade::Cryo,
                                      render::ColorVariant::Faint,
                                      render::ColorLayer::Background),
    },
  };

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.label);
    EXPECT_EQ(test_case.actual, test_case.expected);
  }
}

// =============================================================================
// =============================================================================

// Host input and Python must use all existing normal names without aliases.
TEST(GGEMSColorNamesTest, CanonicalCatalogRoundTripsEveryExistingNormalKey) {
  auto const colors = render::GetNamedColors();
  ASSERT_EQ(colors.size(), 576U);
  std::set<std::string_view> names;
  std::set<render::ColorKey> keys;

  for (auto const &entry : colors) {
    SCOPED_TRACE(entry.name);
    EXPECT_TRUE(names.insert(entry.name).second);
    EXPECT_TRUE(keys.insert(entry.key).second);
    EXPECT_EQ(entry.key.variant, render::ColorVariant::Normal);
    EXPECT_EQ(entry.key.layer, render::ColorLayer::Foreground);
    EXPECT_LT(entry.key.family, render::ColorFamily::Count);
    EXPECT_LT(entry.key.shade, render::color_shade_count);
    EXPECT_EQ(render::ParseColorName(entry.name), entry.key);
    EXPECT_EQ(render::GetColorName(entry.key), entry.name);
  }

  EXPECT_EQ(render::ParseColorName("red_ruby"), render::RED_Ruby);
  EXPECT_EQ(render::ParseColorName("blue_lightsteelblue"),
            render::BLUE_LightSteelBlue);
  EXPECT_EQ(render::ParseColorName("white_white"), render::WHITE_White);
}

// =============================================================================
// =============================================================================

// Authoring rejects misspellings and unsupported variants rather than guessing.
TEST(GGEMSColorNamesTest, RejectsNonCanonicalColorNamesAndKeys) {
  for (std::string_view const name : {
         "",
         "RED_Ruby",
         "red_Ruby",
         "red-ruby",
         " red_ruby",
         "red_ruby ",
         "red_ruby_b",
         "white_white_bg",
         "unknown_color",
       }) {
    SCOPED_TRACE(name);
    EXPECT_FALSE(render::ParseColorName(name));
  }
  EXPECT_TRUE(render::GetColorName(render::RED_Ruby_B).empty());
  EXPECT_TRUE(render::GetColorName(render::WHITE_White_BG).empty());
}
