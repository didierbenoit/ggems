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
 * \brief Defines the 64 named shades and RGB rows of the blue family.
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
 * \brief Enumerates the 64 blue palette rows in shade-index order.
 *
 * Named hues (CSS values where a CSS name is used, GGEMS signature and kept
 * baseline colors, descriptive names), completed by Shade followed by a
 * percentage, Tint followed by a percentage and Tone followed by a percentage:
 * the Pure hue mixed p% toward black, white or mid gray (128); candidates equal
 * to an earlier entry are skipped.
 *
 * \param[in] X Row consumer called as X(NAME, R, G, B).
 */
#define GGEMS_COLOR_BLUE_SHADES(X)                                             \
  X(Pure, 0, 0, 255)                                                           \
  X(Navy, 0, 0, 128)                                                           \
  X(DarkBlue, 0, 0, 139)                                                       \
  X(MediumBlue, 0, 0, 205)                                                     \
  X(MidnightBlue, 25, 25, 112)                                                 \
  X(RoyalBlue, 65, 105, 225)                                                   \
  X(DodgerBlue, 30, 144, 255)                                                  \
  X(SteelBlue, 70, 130, 180)                                                   \
  X(CornflowerBlue, 100, 149, 237)                                             \
  X(LightSteelBlue, 176, 196, 222)                                             \
  X(SlateBlue, 106, 90, 205)                                                   \
  X(DarkSlateBlue, 72, 61, 139)                                                \
  X(MediumSlateBlue, 123, 104, 238)                                            \
  X(Abyss, 3, 5, 7)                                                            \
  X(Gunmetal, 30, 38, 46)                                                      \
  X(DeepNavy, 0, 0, 64)                                                        \
  X(DarkNavy, 0, 0, 96)                                                        \
  X(Cobalt, 0, 71, 171)                                                        \
  X(AzureBlue, 0, 127, 255)                                                    \
  X(LightRoyal, 80, 120, 255)                                                  \
  X(PaleBlue, 160, 200, 255)                                                   \
  X(Sapphire, 15, 82, 186)                                                     \
  X(Denim, 21, 96, 189)                                                        \
  X(PrussianBlue, 0, 49, 83)                                                   \
  X(Ultramarine, 18, 10, 143)                                                  \
  X(Periwinkle, 204, 204, 255)                                                 \
  X(BabyBlue, 137, 207, 240)                                                   \
  X(OceanBlue, 0, 94, 184)                                                     \
  X(OxfordBlue, 0, 33, 71)                                                     \
  X(YaleBlue, 15, 77, 146)                                                     \
  X(Zaffre, 0, 20, 168)                                                        \
  X(Glaucous, 96, 130, 182)                                                    \
  X(Lapis, 38, 97, 156)                                                        \
  X(Ink, 0, 32, 64)                                                            \
  X(Iris, 90, 80, 220)                                                         \
  X(Twilight, 46, 52, 110)                                                     \
  X(Shade5, 0, 0, 242)                                                         \
  X(Shade10, 0, 0, 230)                                                        \
  X(Shade20, 0, 0, 204)                                                        \
  X(Shade30, 0, 0, 178)                                                        \
  X(Shade40, 0, 0, 153)                                                        \
  X(Shade60, 0, 0, 102)                                                        \
  X(Shade70, 0, 0, 76)                                                         \
  X(Shade80, 0, 0, 51)                                                         \
  X(Shade90, 0, 0, 26)                                                         \
  X(Tint5, 13, 13, 255)                                                        \
  X(Tint10, 26, 26, 255)                                                       \
  X(Tint20, 51, 51, 255)                                                       \
  X(Tint30, 76, 76, 255)                                                       \
  X(Tint40, 102, 102, 255)                                                     \
  X(Tint50, 128, 128, 255)                                                     \
  X(Tint60, 153, 153, 255)                                                     \
  X(Tint70, 178, 178, 255)                                                     \
  X(Tint90, 230, 230, 255)                                                     \
  X(Tone5, 6, 6, 249)                                                          \
  X(Tone10, 13, 13, 242)                                                       \
  X(Tone20, 26, 26, 230)                                                       \
  X(Tone30, 38, 38, 217)                                                       \
  X(Tone40, 51, 51, 204)                                                       \
  X(Tone50, 64, 64, 192)                                                       \
  X(Tone60, 77, 77, 179)                                                       \
  X(Tone70, 90, 90, 166)                                                       \
  X(Tone80, 102, 102, 153)                                                     \
  X(Tone90, 115, 115, 141)

/*! \brief Named shades in the blue family, in palette index order. */
enum class BlueShade : std::uint8_t {
/*!
 * \brief Projects a palette row into its shade enumerator.
 * \param[in] NAME Named shade identifier.
 * \param[in] R Red channel, unused by the enumerator projection.
 * \param[in] G Green channel, unused by the enumerator projection.
 * \param[in] B Blue channel, unused by the enumerator projection.
 */
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_BLUE_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
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
 * \brief Builds the blue family palette.
 *
 * \return Blue scale palette, indexed by BlueShade.
 */
constexpr auto MakeBlueScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_BLUE_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(BlueShade) -> ColorFamily {
  return ColorFamily::Blue;
}
/// \endcond

} // namespace ggems::render
