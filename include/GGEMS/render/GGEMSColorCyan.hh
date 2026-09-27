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
#define GGEMS_COLOR_CYAN_SHADES(X)                                             \
  X(Pure, 0, 255, 255)                                                         \
  X(Teal, 0, 128, 128)                                                         \
  X(DarkCyan, 0, 139, 139)                                                     \
  X(LightSeaGreen, 32, 178, 170)                                               \
  X(MediumTurquoise, 72, 209, 204)                                             \
  X(Turquoise, 64, 224, 208)                                                   \
  X(DarkTurquoise, 0, 206, 209)                                                \
  X(PaleTurquoise, 175, 238, 238)                                              \
  X(Aquamarine, 127, 255, 212)                                                 \
  X(MediumAquamarine, 102, 205, 170)                                           \
  X(CadetBlue, 95, 158, 160)                                                   \
  X(PowderBlue, 176, 224, 230)                                                 \
  X(LightBlue, 173, 216, 230)                                                  \
  X(SkyBlue, 135, 206, 235)                                                    \
  X(LightSkyBlue, 135, 206, 250)                                               \
  X(DeepSkyBlue, 0, 191, 255)                                                  \
  X(LightCyan, 224, 255, 255)                                                  \
  X(Cryo, 58, 178, 190)                                                        \
  X(DeepTeal, 0, 48, 48)                                                       \
  X(DarkTeal, 0, 80, 80)                                                       \
  X(Marine, 0, 100, 100)                                                       \
  X(LightTeal, 0, 160, 160)                                                    \
  X(Vivid, 0, 183, 235)                                                        \
  X(Neon, 0, 200, 255)                                                         \
  X(Electric, 125, 249, 255)                                                   \
  X(Radiant, 80, 255, 255)                                                     \
  X(PaleSky, 180, 230, 255)                                                    \
  X(Ice, 210, 245, 255)                                                        \
  X(Cerulean, 0, 123, 167)                                                     \
  X(Peacock, 0, 164, 180)                                                      \
  X(Verdigris, 67, 179, 174)                                                   \
  X(RobinEgg, 0, 204, 204)                                                     \
  X(Arctic, 130, 210, 230)                                                     \
  X(Lagoon, 0, 150, 170)                                                       \
  X(Tiffany, 10, 186, 181)                                                     \
  X(Celeste, 178, 255, 255)                                                    \
  X(Glacier, 120, 200, 220)                                                    \
  X(Aegean, 30, 140, 160)                                                      \
  X(Shade5, 0, 242, 242)                                                       \
  X(Shade10, 0, 230, 230)                                                      \
  X(Shade30, 0, 178, 178)                                                      \
  X(Shade40, 0, 153, 153)                                                      \
  X(Shade60, 0, 102, 102)                                                      \
  X(Shade70, 0, 76, 76)                                                        \
  X(Shade80, 0, 51, 51)                                                        \
  X(Shade90, 0, 26, 26)                                                        \
  X(Tint5, 13, 255, 255)                                                       \
  X(Tint10, 26, 255, 255)                                                      \
  X(Tint20, 51, 255, 255)                                                      \
  X(Tint30, 76, 255, 255)                                                      \
  X(Tint40, 102, 255, 255)                                                     \
  X(Tint50, 128, 255, 255)                                                     \
  X(Tint60, 153, 255, 255)                                                     \
  X(Tint80, 204, 255, 255)                                                     \
  X(Tint90, 230, 255, 255)                                                     \
  X(Tone10, 13, 242, 242)                                                      \
  X(Tone20, 26, 230, 230)                                                      \
  X(Tone30, 38, 217, 217)                                                      \
  X(Tone40, 51, 204, 204)                                                      \
  X(Tone50, 64, 192, 192)                                                      \
  X(Tone60, 77, 179, 179)                                                      \
  X(Tone70, 90, 166, 166)                                                      \
  X(Tone80, 102, 153, 153)                                                     \
  X(Tone90, 115, 141, 141)

/*!
 * \enum CyanShade
 * \brief Named shades in the cyan family, in palette index order.
 */
enum class CyanShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_CYAN_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the cyan family palette.
 *
 * \return Cyan scale palette, indexed by CyanShade.
 */
constexpr auto MakeCyanScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_CYAN_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(CyanShade) -> ColorFamily {
  return ColorFamily::Cyan;
}
/// \endcond

} // namespace ggems::render
