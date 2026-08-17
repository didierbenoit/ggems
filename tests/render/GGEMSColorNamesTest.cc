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
 * \brief Unit tests for named GGEMS colors.
 *
 * Validates shade-family mapping, color-key construction, default colors, and representative named constants across variants and layers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

/// \cond

namespace {

namespace render = ggems::render;

// =============================================================================
// =============================================================================

constexpr auto k_defined_color = render::DefineColor(
    render::OrangeShade::CopperSignal, render::ColorVariant::Faint,
    render::ColorLayer::Background);

// =============================================================================
// =============================================================================

static_assert(k_defined_color ==
              render::ColorKey{.family = render::ColorFamily::Orange,
                               .shade = static_cast<std::uint8_t>(
                                   render::OrangeShade::CopperSignal),
                               .variant = render::ColorVariant::Faint,
                               .layer = render::ColorLayer::Background});

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSColorNamesTest, MapsRepresentativeShadeEnumsToFamilies) {
  EXPECT_EQ(render::FamilyOf(render::GreenShade::Acid),
            render::ColorFamily::Green);
  EXPECT_EQ(render::FamilyOf(render::OrangeShade::CopperSignal),
            render::ColorFamily::Orange);
  EXPECT_EQ(render::FamilyOf(render::WhiteShade::Glare),
            render::ColorFamily::White);
}

// =============================================================================
// =============================================================================

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

TEST(GGEMSColorNamesTest, DefaultsUseExpectedNamedColors) {
  EXPECT_EQ(render::DEFAULT_FG, render::WHITE_Ivory);
  EXPECT_EQ(render::DEFAULT_BG, render::GRAY_Steel_BG);
}

// =============================================================================
// =============================================================================

TEST(GGEMSColorNamesTest, RepresentativeNamedConstantsPreserveTheirKeys) {
  struct NamedColorCase {
    std::string_view label;
    render::ColorKey actual;
    render::ColorKey expected;
  };

  constexpr std::array test_cases{
      NamedColorCase{.label = "green acid",
                     .actual = render::GREEN_Acid,
                     .expected = render::DefineColor(render::GreenShade::Acid)},
      NamedColorCase{.label = "bright blue abyss",
                     .actual = render::BLUE_Abyss_B,
                     .expected =
                         render::DefineColor(render::BlueShade::Abyss,
                                             render::ColorVariant::Bright)},
      NamedColorCase{.label = "faint red tomato",
                     .actual = render::RED_Tomato_F,
                     .expected =
                         render::DefineColor(render::RedShade::Tomato,
                                             render::ColorVariant::Faint)},
      NamedColorCase{.label = "orange copper signal background",
                     .actual = render::ORANGE_CopperSignal_BG,
                     .expected =
                         render::DefineColor(render::OrangeShade::CopperSignal,
                                             render::ColorVariant::Normal,
                                             render::ColorLayer::Background)},
      NamedColorCase{.label = "bright white glare background",
                     .actual = render::WHITE_Glare_B_BG,
                     .expected =
                         render::DefineColor(render::WhiteShade::Glare,
                                             render::ColorVariant::Bright,
                                             render::ColorLayer::Background)},
      NamedColorCase{.label = "faint cyan cryo background",
                     .actual = render::CYAN_Cryo_F_BG,
                     .expected = render::DefineColor(
                         render::CyanShade::Cryo, render::ColorVariant::Faint,
                         render::ColorLayer::Background)},
  };

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.label);
    EXPECT_EQ(test_case.actual, test_case.expected);
  }
}
/// \endcond
