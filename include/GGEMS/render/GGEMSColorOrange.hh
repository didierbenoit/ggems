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

#pragma once

/// \cond
#include <array>
#include <cstdint>
/// \endcond

#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::render {

// Named hues (CSS values where a CSS name is used, GGEMS signature and
// kept baseline colors, descriptive names), completed by Shade<p>,
// Tint<p> and Tone<p>: the Pure hue mixed p% toward black, white or mid
// gray (128); candidates equal to an earlier entry are skipped.
#define GGEMS_COLOR_ORANGE_SHADES(X)                                           \
  X(Pure, 255, 165, 0)                                                         \
  X(DarkOrange, 255, 140, 0)                                                   \
  X(OrangeRed, 255, 69, 0)                                                     \
  X(Coral, 255, 127, 80)                                                       \
  X(Chocolate, 210, 105, 30)                                                   \
  X(SaddleBrown, 139, 69, 19)                                                  \
  X(Sienna, 160, 82, 45)                                                       \
  X(Peru, 205, 133, 63)                                                        \
  X(SandyBrown, 244, 164, 96)                                                  \
  X(Tan, 210, 180, 140)                                                        \
  X(BurlyWood, 222, 184, 135)                                                  \
  X(PeachPuff, 255, 218, 185)                                                  \
  X(NavajoWhite, 255, 222, 173)                                                \
  X(Moccasin, 255, 228, 181)                                                   \
  X(CopperSignal, 180, 88, 38)                                                 \
  X(DeepBrown, 80, 32, 0)                                                      \
  X(DarkBrown, 96, 40, 0)                                                      \
  X(MediumBrown, 128, 64, 0)                                                   \
  X(Copper, 160, 80, 0)                                                        \
  X(Rust, 192, 96, 0)                                                          \
  X(Tangerine, 255, 180, 40)                                                   \
  X(LightOrange, 255, 200, 80)                                                 \
  X(PaleOrange, 255, 215, 120)                                                 \
  X(Pumpkin, 255, 117, 24)                                                     \
  X(Carrot, 237, 145, 33)                                                      \
  X(Apricot, 251, 206, 177)                                                    \
  X(Peach, 255, 229, 180)                                                      \
  X(Persimmon, 236, 88, 0)                                                     \
  X(Terracotta, 226, 114, 91)                                                  \
  X(BurntOrange, 204, 85, 0)                                                   \
  X(Mango, 255, 130, 67)                                                       \
  X(Ochre, 204, 119, 34)                                                       \
  X(Caramel, 255, 213, 154)                                                    \
  X(Bronze, 205, 127, 50)                                                      \
  X(Mahogany, 192, 64, 0)                                                      \
  X(Ginger, 176, 101, 0)                                                       \
  X(Tawny, 205, 87, 0)                                                         \
  X(Shade10, 230, 148, 0)                                                      \
  X(Shade20, 204, 132, 0)                                                      \
  X(Shade30, 178, 116, 0)                                                      \
  X(Shade40, 153, 99, 0)                                                       \
  X(Shade50, 128, 82, 0)                                                       \
  X(Shade60, 102, 66, 0)                                                       \
  X(Shade70, 76, 50, 0)                                                        \
  X(Shade80, 51, 33, 0)                                                        \
  X(Shade90, 26, 16, 0)                                                        \
  X(Tint10, 255, 174, 26)                                                      \
  X(Tint20, 255, 183, 51)                                                      \
  X(Tint30, 255, 192, 76)                                                      \
  X(Tint40, 255, 201, 102)                                                     \
  X(Tint50, 255, 210, 128)                                                     \
  X(Tint60, 255, 219, 153)                                                     \
  X(Tint70, 255, 228, 178)                                                     \
  X(Tint80, 255, 237, 204)                                                     \
  X(Tint90, 255, 246, 230)                                                     \
  X(Tone10, 242, 161, 13)                                                      \
  X(Tone20, 230, 158, 26)                                                      \
  X(Tone30, 217, 154, 38)                                                      \
  X(Tone40, 204, 150, 51)                                                      \
  X(Tone50, 192, 146, 64)                                                      \
  X(Tone60, 179, 143, 77)                                                      \
  X(Tone70, 166, 139, 90)                                                      \
  X(Tone80, 153, 135, 102)                                                     \
  X(Tone90, 141, 132, 115)

/*!
 * \enum OrangeShade
 * \brief Named shades in the orange family, in palette index order.
 */
enum class OrangeShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_ORANGE_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the orange family palette.
 *
 * \return Orange scale palette, indexed by OrangeShade.
 */
constexpr auto MakeOrangeScale() noexcept
  -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_ORANGE_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(OrangeShade) -> ColorFamily {
  return ColorFamily::Orange;
}
/// \endcond

} // namespace ggems::render
