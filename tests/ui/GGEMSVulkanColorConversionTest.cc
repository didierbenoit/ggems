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
 * \brief Checks independent sRGB-to-linear clear-color expectations.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

namespace render = ggems::render;
using ggems::ui::detail::ToVulkanClearColor;

// Rounded independent sRGB references have nine decimals; FP32 conversion
// adds a few ULPs over the unit interval. Alpha is an exact discrete contract.
constexpr float k_color_tolerance{1.0e-6F};

constexpr std::array<float, 4U> k_expected_blue_abyss{
  0.000910581F,
  0.001517635F,
  0.002124689F,
  1.0F,
};

// =============================================================================
// =============================================================================

auto ExpectVulkanColor(std::array<float, 4U> const &actual,
                       std::array<float, 4U> const &expected) -> void {
  EXPECT_NEAR(actual[0U], expected[0U], k_color_tolerance);
  EXPECT_NEAR(actual[1U], expected[1U], k_color_tolerance);
  EXPECT_NEAR(actual[2U], expected[2U], k_color_tolerance);
  EXPECT_EQ(actual[3U], expected[3U]);
}
} // namespace

// =============================================================================
// =============================================================================

// sRGB decoding must preserve black channels while converting a nonzero green
// channel.
TEST(GGEMSVulkanColorConversion, ZeroByteChannelsMapToZeroLinearChannels) {
  ExpectVulkanColor(ToVulkanClearColor(render::GREEN_DeepGreen),
                    {0.0F, 0.029556835F, 0.0F, 1.0F});
}

// =============================================================================
// =============================================================================

// Full-intensity sRGB white must remain exact opaque white after conversion.
TEST(GGEMSVulkanColorConversion, AllMaximumByteChannelsMapToOne) {
  ExpectVulkanColor(ToVulkanClearColor(render::WHITE_White),
                    {1.0F, 1.0F, 1.0F, 1.0F});
}

// =============================================================================
// =============================================================================

// Unequal RGB channels must match independently calculated linear-light values.
TEST(GGEMSVulkanColorConversion, NamedColorMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato),
                    {1.0F, 0.124771819F, 0.063010015F, 1.0F});
}

// =============================================================================
// =============================================================================

// Dark palette values must use the linear branch of sRGB decoding.
TEST(GGEMSVulkanColorConversion, BlueAbyssMatchesLinearClearColor) {
  ExpectVulkanColor(ToVulkanClearColor(render::BLUE_Abyss),
                    k_expected_blue_abyss);
}

// =============================================================================
// =============================================================================

// Brightness adjustment must occur in palette space before conversion to linear
// light.
TEST(GGEMSVulkanColorConversion, BrightVariantMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_B),
                    {1.0F, 0.309468925F, 0.230740055F, 1.0F});
}

// =============================================================================
// =============================================================================

// Faint colors must match independent linear-light channel values and remain
// opaque.
TEST(GGEMSVulkanColorConversion, FaintVariantMapsToLinearComponents) {
  ExpectVulkanColor(ToVulkanClearColor(render::RED_Tomato_F),
                    {0.401977777F, 0.054480277F, 0.028426040F, 1.0F});
}

// =============================================================================
// =============================================================================

// Terminal layer metadata must not change the corresponding Vulkan clear color.
TEST(GGEMSVulkanColorConversion, IgnoresForegroundAndBackgroundLayer) {
  auto const foreground = ToVulkanClearColor(render::BLUE_Abyss);
  auto const background = ToVulkanClearColor(render::BLUE_Abyss_BG);

  ExpectVulkanColor(foreground, k_expected_blue_abyss);
  ExpectVulkanColor(background, k_expected_blue_abyss);
  EXPECT_EQ(foreground, background);
}

// =============================================================================
// =============================================================================

// An oversized shade must resolve consistently before conversion to linear
// light.
TEST(GGEMSVulkanColorConversion, ClampsShadeAbovePaletteRange) {
  constexpr render::ColorKey out_of_range_color{
    .family = render::ColorFamily::Blue,
    .shade = std::numeric_limits<std::uint8_t>::max(),
    .variant = render::ColorVariant::Normal,
    .layer = render::ColorLayer::Foreground,
  };

  EXPECT_EQ(ToVulkanClearColor(out_of_range_color),
            ToVulkanClearColor(render::BLUE_Tone90));
}

// =============================================================================
// =============================================================================
