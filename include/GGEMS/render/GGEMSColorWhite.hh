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
 * \brief Defines the 64 named shades and RGB rows of the white family.
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
 * \brief Enumerates the 64 white palette rows in shade-index order.
 *
 * White and near-white shades: CSS off-whites, Bone, the kept baseline tints
 * and generated Warm/Cool/Rose/Mint tint ramps (index 1..8).
 *
 * \param[in] X Row consumer called as X(NAME, R, G, B).
 */
#define GGEMS_COLOR_WHITE_SHADES(X)                                            \
  X(White, 255, 255, 255)                                                      \
  X(Snow, 255, 250, 250)                                                       \
  X(GhostWhite, 248, 248, 255)                                                 \
  X(WhiteSmoke, 245, 245, 245)                                                 \
  X(FloralWhite, 255, 250, 240)                                                \
  X(Ivory, 255, 255, 240)                                                      \
  X(Seashell, 255, 245, 238)                                                   \
  X(OldLace, 253, 245, 230)                                                    \
  X(Linen, 250, 240, 230)                                                      \
  X(AntiqueWhite, 250, 235, 215)                                               \
  X(Beige, 245, 245, 220)                                                      \
  X(Cornsilk, 255, 248, 220)                                                   \
  X(LemonChiffon, 255, 250, 205)                                               \
  X(LightYellow, 255, 255, 224)                                                \
  X(PapayaWhip, 255, 239, 213)                                                 \
  X(BlanchedAlmond, 255, 235, 205)                                             \
  X(Bisque, 255, 228, 196)                                                     \
  X(MistyRose, 255, 228, 225)                                                  \
  X(LavenderBlush, 255, 240, 245)                                              \
  X(Lavender, 230, 230, 250)                                                   \
  X(Honeydew, 240, 255, 240)                                                   \
  X(MintCream, 245, 255, 250)                                                  \
  X(Azure, 240, 255, 255)                                                      \
  X(AliceBlue, 240, 248, 255)                                                  \
  X(Bone, 218, 214, 196)                                                       \
  X(Eggshell, 240, 240, 235)                                                   \
  X(Cream, 235, 232, 220)                                                      \
  X(Paper, 250, 250, 250)                                                      \
  X(Pearl, 234, 224, 200)                                                      \
  X(Alabaster, 242, 240, 230)                                                  \
  X(Chalk, 250, 250, 245)                                                      \
  X(Parchment, 241, 231, 205)                                                  \
  X(Warm1, 255, 252, 249)                                                      \
  X(Warm2, 255, 249, 243)                                                      \
  X(Warm3, 255, 246, 237)                                                      \
  X(Warm4, 255, 243, 231)                                                      \
  X(Warm5, 255, 240, 225)                                                      \
  X(Warm6, 255, 237, 219)                                                      \
  X(Warm7, 255, 234, 213)                                                      \
  X(Warm8, 255, 231, 207)                                                      \
  X(Cool1, 249, 252, 255)                                                      \
  X(Cool2, 243, 249, 255)                                                      \
  X(Cool3, 237, 246, 255)                                                      \
  X(Cool4, 231, 243, 255)                                                      \
  X(Cool5, 225, 240, 255)                                                      \
  X(Cool6, 219, 237, 255)                                                      \
  X(Cool7, 213, 234, 255)                                                      \
  X(Cool8, 207, 231, 255)                                                      \
  X(Rose1, 255, 251, 253)                                                      \
  X(Rose2, 255, 247, 251)                                                      \
  X(Rose3, 255, 243, 249)                                                      \
  X(Rose4, 255, 239, 247)                                                      \
  X(Rose5, 255, 235, 245)                                                      \
  X(Rose6, 255, 231, 243)                                                      \
  X(Rose7, 255, 227, 241)                                                      \
  X(Rose8, 255, 223, 239)                                                      \
  X(Mint1, 250, 255, 252)                                                      \
  X(Mint2, 245, 255, 249)                                                      \
  X(Mint3, 240, 255, 246)                                                      \
  X(Mint4, 235, 255, 243)                                                      \
  X(Mint5, 230, 255, 240)                                                      \
  X(Mint6, 225, 255, 237)                                                      \
  X(Mint7, 220, 255, 234)                                                      \
  X(Mint8, 215, 255, 231)

/*! \brief Named shades in the white family, in palette index order. */
enum class WhiteShade : std::uint8_t {
/*!
 * \brief Projects a palette row into its shade enumerator.
 * \param[in] NAME Named shade identifier.
 * \param[in] R Red channel, unused by the enumerator projection.
 * \param[in] G Green channel, unused by the enumerator projection.
 * \param[in] B Blue channel, unused by the enumerator projection.
 */
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_WHITE_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
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
 * \brief Builds the white family palette.
 *
 * \return White scale palette, indexed by WhiteShade.
 */
constexpr auto MakeWhiteScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_WHITE_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(WhiteShade) -> ColorFamily {
  return ColorFamily::White;
}
/// \endcond

} // namespace ggems::render
