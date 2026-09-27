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
#define GGEMS_COLOR_GREEN_SHADES(X)                                            \
  X(Pure, 0, 255, 0)                                                           \
  X(WebGreen, 0, 128, 0)                                                       \
  X(DarkGreen, 0, 100, 0)                                                      \
  X(ForestGreen, 34, 139, 34)                                                  \
  X(SeaGreen, 46, 139, 87)                                                     \
  X(MediumSeaGreen, 60, 179, 113)                                              \
  X(LightGreen, 144, 238, 144)                                                 \
  X(PaleGreen, 152, 251, 152)                                                  \
  X(DarkSeaGreen, 143, 188, 143)                                               \
  X(LimeGreen, 50, 205, 50)                                                    \
  X(LawnGreen, 124, 252, 0)                                                    \
  X(Chartreuse, 127, 255, 0)                                                   \
  X(GreenYellow, 173, 255, 47)                                                 \
  X(YellowGreen, 154, 205, 50)                                                 \
  X(OliveDrab, 107, 142, 35)                                                   \
  X(DarkOliveGreen, 85, 107, 47)                                               \
  X(SpringGreen, 0, 255, 127)                                                  \
  X(MediumSpringGreen, 0, 250, 154)                                            \
  X(Matrix, 0, 255, 170)                                                       \
  X(Acid, 48, 220, 160)                                                        \
  X(DeepGreen, 0, 48, 0)                                                       \
  X(Pine, 0, 80, 0)                                                            \
  X(Jade, 0, 160, 64)                                                          \
  X(Emerald, 0, 201, 87)                                                       \
  X(LightLime, 60, 255, 120)                                                   \
  X(PaleMint, 204, 255, 204)                                                   \
  X(Sage, 158, 183, 133)                                                       \
  X(Moss, 138, 154, 91)                                                        \
  X(Fern, 79, 121, 66)                                                         \
  X(Kelly, 76, 187, 23)                                                        \
  X(Shamrock, 0, 158, 96)                                                      \
  X(Hunter, 53, 94, 59)                                                        \
  X(Avocado, 86, 130, 3)                                                       \
  X(Pistachio, 147, 197, 114)                                                  \
  X(Seafoam, 159, 226, 191)                                                    \
  X(Malachite, 11, 218, 81)                                                    \
  X(Laser, 57, 255, 20)                                                        \
  X(Shade5, 0, 242, 0)                                                         \
  X(Shade10, 0, 230, 0)                                                        \
  X(Shade20, 0, 204, 0)                                                        \
  X(Shade30, 0, 178, 0)                                                        \
  X(Shade40, 0, 153, 0)                                                        \
  X(Shade60, 0, 102, 0)                                                        \
  X(Shade70, 0, 76, 0)                                                         \
  X(Shade80, 0, 51, 0)                                                         \
  X(Shade90, 0, 26, 0)                                                         \
  X(Tint5, 13, 255, 13)                                                        \
  X(Tint10, 26, 255, 26)                                                       \
  X(Tint20, 51, 255, 51)                                                       \
  X(Tint30, 76, 255, 76)                                                       \
  X(Tint40, 102, 255, 102)                                                     \
  X(Tint50, 128, 255, 128)                                                     \
  X(Tint60, 153, 255, 153)                                                     \
  X(Tint70, 178, 255, 178)                                                     \
  X(Tint90, 230, 255, 230)                                                     \
  X(Tone10, 13, 242, 13)                                                       \
  X(Tone20, 26, 230, 26)                                                       \
  X(Tone30, 38, 217, 38)                                                       \
  X(Tone40, 51, 204, 51)                                                       \
  X(Tone50, 64, 192, 64)                                                       \
  X(Tone60, 77, 179, 77)                                                       \
  X(Tone70, 90, 166, 90)                                                       \
  X(Tone80, 102, 153, 102)                                                     \
  X(Tone90, 115, 141, 115)

/*!
 * \enum GreenShade
 * \brief Named shades in the green family, in palette index order.
 */
enum class GreenShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_GREEN_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the green family palette.
 *
 * \return Green scale palette, indexed by GreenShade.
 */
constexpr auto MakeGreenScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_GREEN_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(GreenShade) -> ColorFamily {
  return ColorFamily::Green;
}
/// \endcond

} // namespace ggems::render
