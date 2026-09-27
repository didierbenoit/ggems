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
 * \brief Defines the 64 named shades and RGB rows of the magenta family.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
/// \endcond

#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::render {

/*!
 * \brief Enumerates the 64 magenta palette rows in shade-index order.
 *
 * Named hues (CSS values where a CSS name is used, GGEMS signature and kept
 * baseline colors, descriptive names), completed by Shade followed by a
 * percentage, Tint followed by a percentage and Tone followed by a percentage:
 * the Pure hue mixed p% toward black, white or mid gray (128); candidates equal
 * to an earlier entry are skipped.
 *
 * \param[in] X Row consumer called as X(NAME, R, G, B).
 */
#define GGEMS_COLOR_MAGENTA_SHADES(X)                                          \
  X(Pure, 255, 0, 255)                                                         \
  X(Purple, 128, 0, 128)                                                       \
  X(DarkMagenta, 139, 0, 139)                                                  \
  X(Indigo, 75, 0, 130)                                                        \
  X(RebeccaPurple, 102, 51, 153)                                               \
  X(BlueViolet, 138, 43, 226)                                                  \
  X(DarkViolet, 148, 0, 211)                                                   \
  X(DarkOrchid, 153, 50, 204)                                                  \
  X(MediumOrchid, 186, 85, 211)                                                \
  X(Orchid, 218, 112, 214)                                                     \
  X(MediumPurple, 147, 112, 219)                                               \
  X(Violet, 238, 130, 238)                                                     \
  X(Plum, 221, 160, 221)                                                       \
  X(Thistle, 216, 191, 216)                                                    \
  X(DeepPink, 255, 20, 147)                                                    \
  X(HotPink, 255, 105, 180)                                                    \
  X(LightPink, 255, 182, 193)                                                  \
  X(Pink, 255, 192, 203)                                                       \
  X(PaleVioletRed, 219, 112, 147)                                              \
  X(MediumVioletRed, 199, 21, 133)                                             \
  X(FleshSignal, 150, 62, 92)                                                  \
  X(DeepPurple, 64, 0, 64)                                                     \
  X(DarkPurple, 96, 0, 96)                                                     \
  X(Electric, 255, 30, 100)                                                    \
  X(LightMagenta, 255, 160, 255)                                               \
  X(PalePink, 255, 200, 240)                                                   \
  X(Cerise, 222, 49, 99)                                                       \
  X(Rose, 255, 0, 127)                                                         \
  X(Amethyst, 153, 102, 204)                                                   \
  X(Lilac, 200, 162, 200)                                                      \
  X(Mauve, 224, 176, 255)                                                      \
  X(Byzantium, 112, 41, 99)                                                    \
  X(Mulberry, 197, 75, 140)                                                    \
  X(Grape, 111, 45, 168)                                                       \
  X(Heliotrope, 223, 115, 255)                                                 \
  X(Wisteria, 201, 160, 220)                                                   \
  X(Eggplant, 97, 64, 81)                                                      \
  X(Shade5, 242, 0, 242)                                                       \
  X(Shade10, 230, 0, 230)                                                      \
  X(Shade20, 204, 0, 204)                                                      \
  X(Shade30, 178, 0, 178)                                                      \
  X(Shade40, 153, 0, 153)                                                      \
  X(Shade60, 102, 0, 102)                                                      \
  X(Shade70, 76, 0, 76)                                                        \
  X(Shade80, 51, 0, 51)                                                        \
  X(Shade90, 26, 0, 26)                                                        \
  X(Tint10, 255, 26, 255)                                                      \
  X(Tint20, 255, 51, 255)                                                      \
  X(Tint30, 255, 76, 255)                                                      \
  X(Tint40, 255, 102, 255)                                                     \
  X(Tint50, 255, 128, 255)                                                     \
  X(Tint60, 255, 153, 255)                                                     \
  X(Tint70, 255, 178, 255)                                                     \
  X(Tint80, 255, 204, 255)                                                     \
  X(Tint90, 255, 230, 255)                                                     \
  X(Tone10, 242, 13, 242)                                                      \
  X(Tone20, 230, 26, 230)                                                      \
  X(Tone30, 217, 38, 217)                                                      \
  X(Tone40, 204, 51, 204)                                                      \
  X(Tone50, 192, 64, 192)                                                      \
  X(Tone60, 179, 77, 179)                                                      \
  X(Tone70, 166, 90, 166)                                                      \
  X(Tone80, 153, 102, 153)                                                     \
  X(Tone90, 141, 115, 141)

/*! \brief Named shades in the magenta family, in palette index order. */
enum class MagentaShade : std::uint8_t {
/*!
 * \brief Projects a palette row into its shade enumerator.
 * \param[in] NAME Named shade identifier.
 * \param[in] R Red channel, unused by the enumerator projection.
 * \param[in] G Green channel, unused by the enumerator projection.
 * \param[in] B Blue channel, unused by the enumerator projection.
 */
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_MAGENTA_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \def GGEMS_COLOR_SHADE_RGB(NAME, R, G, B)
 * \brief Projects a palette row into its RGB initializer.
 * \param[in] NAME Named shade, unused by the RGB projection.
 * \param[in] R Red channel in [0, 255].
 * \param[in] G Green channel in [0, 255].
 * \param[in] B Blue channel in [0, 255].
 */

/*!
 * \brief Builds the magenta family palette.
 *
 * \return Magenta scale palette, indexed by MagentaShade.
 */
constexpr auto MakeMagentaScale() noexcept
  -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_MAGENTA_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(MagentaShade) -> ColorFamily {
  return ColorFamily::Magenta;
}
/// \endcond

} // namespace ggems::render
