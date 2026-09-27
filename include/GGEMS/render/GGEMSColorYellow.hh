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
#define GGEMS_COLOR_YELLOW_SHADES(X)                                           \
  X(Pure, 255, 255, 0)                                                         \
  X(Gold, 255, 215, 0)                                                         \
  X(Goldenrod, 218, 165, 32)                                                   \
  X(DarkGoldenrod, 184, 134, 11)                                               \
  X(Khaki, 240, 230, 140)                                                      \
  X(DarkKhaki, 189, 183, 107)                                                  \
  X(PaleGoldenrod, 238, 232, 170)                                              \
  X(LightGoldenrodYellow, 250, 250, 210)                                       \
  X(Olive, 128, 128, 0)                                                        \
  X(Wheat, 245, 222, 179)                                                      \
  X(MotherAmber, 190, 145, 45)                                                 \
  X(DeepOlive, 96, 96, 0)                                                      \
  X(DarkGold, 160, 144, 0)                                                     \
  X(AntiqueGold, 192, 160, 0)                                                  \
  X(OldGold, 210, 180, 0)                                                      \
  X(Flax, 238, 221, 130)                                                       \
  X(PaleLemon, 250, 250, 120)                                                  \
  X(Lemon, 255, 255, 80)                                                       \
  X(PaleYellow, 255, 255, 160)                                                 \
  X(PaleCream, 255, 255, 220)                                                  \
  X(Amber, 255, 191, 0)                                                        \
  X(Mustard, 255, 219, 88)                                                     \
  X(Saffron, 244, 196, 48)                                                     \
  X(Canary, 255, 239, 0)                                                       \
  X(Maize, 251, 236, 93)                                                       \
  X(Dandelion, 240, 225, 48)                                                   \
  X(Banana, 255, 225, 53)                                                      \
  X(Brass, 181, 166, 66)                                                       \
  X(Citrine, 228, 208, 10)                                                     \
  X(Jasmine, 248, 222, 126)                                                    \
  X(Buff, 218, 190, 130)                                                       \
  X(Honey, 235, 180, 40)                                                       \
  X(Sunflower, 255, 218, 51)                                                   \
  X(Straw, 228, 217, 111)                                                      \
  X(Champagne, 247, 231, 206)                                                  \
  X(Vanilla, 243, 229, 171)                                                    \
  X(Pear, 209, 226, 49)                                                        \
  X(Shade5, 242, 242, 0)                                                       \
  X(Shade10, 230, 230, 0)                                                      \
  X(Shade20, 204, 204, 0)                                                      \
  X(Shade30, 178, 178, 0)                                                      \
  X(Shade40, 153, 153, 0)                                                      \
  X(Shade60, 102, 102, 0)                                                      \
  X(Shade70, 76, 76, 0)                                                        \
  X(Shade80, 51, 51, 0)                                                        \
  X(Shade90, 26, 26, 0)                                                        \
  X(Tint10, 255, 255, 26)                                                      \
  X(Tint20, 255, 255, 51)                                                      \
  X(Tint30, 255, 255, 76)                                                      \
  X(Tint40, 255, 255, 102)                                                     \
  X(Tint50, 255, 255, 128)                                                     \
  X(Tint60, 255, 255, 153)                                                     \
  X(Tint70, 255, 255, 178)                                                     \
  X(Tint80, 255, 255, 204)                                                     \
  X(Tint90, 255, 255, 230)                                                     \
  X(Tone10, 242, 242, 13)                                                      \
  X(Tone20, 230, 230, 26)                                                      \
  X(Tone30, 217, 217, 38)                                                      \
  X(Tone40, 204, 204, 51)                                                      \
  X(Tone50, 192, 192, 64)                                                      \
  X(Tone60, 179, 179, 77)                                                      \
  X(Tone70, 166, 166, 90)                                                      \
  X(Tone80, 153, 153, 102)                                                     \
  X(Tone90, 141, 141, 115)

/*!
 * \enum YellowShade
 * \brief Named shades in the yellow family, in palette index order.
 */
enum class YellowShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_YELLOW_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the yellow family palette.
 *
 * \return Yellow scale palette, indexed by YellowShade.
 */
constexpr auto MakeYellowScale() noexcept
  -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_YELLOW_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(YellowShade) -> ColorFamily {
  return ColorFamily::Yellow;
}
/// \endcond

} // namespace ggems::render
