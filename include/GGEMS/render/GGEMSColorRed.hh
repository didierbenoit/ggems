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
#define GGEMS_COLOR_RED_SHADES(X)                                              \
  X(Pure, 255, 0, 0)                                                           \
  X(DarkRed, 139, 0, 0)                                                        \
  X(FireBrick, 178, 34, 34)                                                    \
  X(Crimson, 220, 20, 60)                                                      \
  X(IndianRed, 205, 92, 92)                                                    \
  X(LightCoral, 240, 128, 128)                                                 \
  X(Salmon, 250, 128, 114)                                                     \
  X(DarkSalmon, 233, 150, 122)                                                 \
  X(LightSalmon, 255, 160, 122)                                                \
  X(Tomato, 255, 99, 71)                                                       \
  X(Maroon, 128, 0, 0)                                                         \
  X(Brown, 165, 42, 42)                                                        \
  X(RosyBrown, 188, 143, 143)                                                  \
  X(XenoBlood, 170, 45, 40)                                                    \
  X(Blood, 64, 0, 0)                                                           \
  X(Garnet, 96, 0, 0)                                                          \
  X(Ruby, 160, 0, 0)                                                           \
  X(Cherry, 192, 16, 16)                                                       \
  X(Neon, 255, 40, 40)                                                         \
  X(LightRed, 255, 80, 80)                                                     \
  X(PaleSalmon, 255, 192, 160)                                                 \
  X(Scarlet, 255, 36, 0)                                                       \
  X(Vermilion, 227, 66, 52)                                                    \
  X(Cardinal, 196, 30, 58)                                                     \
  X(Carmine, 150, 0, 24)                                                       \
  X(Burgundy, 128, 0, 32)                                                      \
  X(Wine, 114, 47, 55)                                                         \
  X(Claret, 127, 23, 52)                                                       \
  X(Cranberry, 159, 0, 52)                                                     \
  X(PersianRed, 204, 51, 51)                                                   \
  X(Chili, 194, 24, 7)                                                         \
  X(TuscanRed, 124, 48, 48)                                                    \
  X(Rosewood, 101, 0, 11)                                                      \
  X(Redwood, 164, 90, 82)                                                      \
  X(Sangria, 146, 0, 10)                                                       \
  X(CandyApple, 255, 8, 0)                                                     \
  X(Shade5, 242, 0, 0)                                                         \
  X(Shade10, 230, 0, 0)                                                        \
  X(Shade20, 204, 0, 0)                                                        \
  X(Shade30, 178, 0, 0)                                                        \
  X(Shade40, 153, 0, 0)                                                        \
  X(Shade60, 102, 0, 0)                                                        \
  X(Shade70, 76, 0, 0)                                                         \
  X(Shade80, 51, 0, 0)                                                         \
  X(Shade90, 26, 0, 0)                                                         \
  X(Tint5, 255, 13, 13)                                                        \
  X(Tint10, 255, 26, 26)                                                       \
  X(Tint20, 255, 51, 51)                                                       \
  X(Tint30, 255, 76, 76)                                                       \
  X(Tint40, 255, 102, 102)                                                     \
  X(Tint50, 255, 128, 128)                                                     \
  X(Tint60, 255, 153, 153)                                                     \
  X(Tint70, 255, 178, 178)                                                     \
  X(Tint80, 255, 204, 204)                                                     \
  X(Tint90, 255, 230, 230)                                                     \
  X(Tone5, 249, 6, 6)                                                          \
  X(Tone10, 242, 13, 13)                                                       \
  X(Tone20, 230, 26, 26)                                                       \
  X(Tone30, 217, 38, 38)                                                       \
  X(Tone50, 192, 64, 64)                                                       \
  X(Tone60, 179, 77, 77)                                                       \
  X(Tone70, 166, 90, 90)                                                       \
  X(Tone80, 153, 102, 102)                                                     \
  X(Tone90, 141, 115, 115)

/*!
 * \enum RedShade
 * \brief Named shades in the red family, in palette index order.
 */
enum class RedShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_RED_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the red family palette.
 *
 * \return Red scale palette, indexed by RedShade.
 */
constexpr auto MakeRedScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_RED_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(RedShade) -> ColorFamily {
  return ColorFamily::Red;
}
/// \endcond

} // namespace ggems::render
