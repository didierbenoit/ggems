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

// Neutral progression: black, the signature Void, the CSS grays and
// Neutral<level> entries (level = channel value) every fourth level
// from 8 to 252.
#define GGEMS_COLOR_GRAY_SHADES(X)                                             \
  X(Black, 0, 0, 0)                                                            \
  X(Void, 3, 3, 3)                                                             \
  X(Neutral8, 8, 8, 8)                                                         \
  X(Neutral12, 12, 12, 12)                                                     \
  X(Deep, 16, 16, 16)                                                          \
  X(Neutral20, 20, 20, 20)                                                     \
  X(Neutral24, 24, 24, 24)                                                     \
  X(Neutral28, 28, 28, 28)                                                     \
  X(Neutral32, 32, 32, 32)                                                     \
  X(Neutral36, 36, 36, 36)                                                     \
  X(Neutral40, 40, 40, 40)                                                     \
  X(Neutral44, 44, 44, 44)                                                     \
  X(Neutral48, 48, 48, 48)                                                     \
  X(Neutral52, 52, 52, 52)                                                     \
  X(Neutral56, 56, 56, 56)                                                     \
  X(Neutral60, 60, 60, 60)                                                     \
  X(Neutral64, 64, 64, 64)                                                     \
  X(Neutral68, 68, 68, 68)                                                     \
  X(Neutral72, 72, 72, 72)                                                     \
  X(Neutral76, 76, 76, 76)                                                     \
  X(Neutral80, 80, 80, 80)                                                     \
  X(Neutral84, 84, 84, 84)                                                     \
  X(Neutral88, 88, 88, 88)                                                     \
  X(Neutral92, 92, 92, 92)                                                     \
  X(Neutral96, 96, 96, 96)                                                     \
  X(Neutral100, 100, 100, 100)                                                 \
  X(DimGray, 105, 105, 105)                                                    \
  X(Neutral108, 108, 108, 108)                                                 \
  X(Neutral112, 112, 112, 112)                                                 \
  X(Neutral116, 116, 116, 116)                                                 \
  X(Neutral120, 120, 120, 120)                                                 \
  X(Neutral124, 124, 124, 124)                                                 \
  X(Gray, 128, 128, 128)                                                       \
  X(Neutral132, 132, 132, 132)                                                 \
  X(Neutral136, 136, 136, 136)                                                 \
  X(Neutral140, 140, 140, 140)                                                 \
  X(Neutral144, 144, 144, 144)                                                 \
  X(Neutral148, 148, 148, 148)                                                 \
  X(Neutral152, 152, 152, 152)                                                 \
  X(Neutral156, 156, 156, 156)                                                 \
  X(Concrete, 160, 160, 160)                                                   \
  X(Neutral164, 164, 164, 164)                                                 \
  X(DarkGray, 169, 169, 169)                                                   \
  X(Neutral172, 172, 172, 172)                                                 \
  X(Neutral176, 176, 176, 176)                                                 \
  X(Neutral180, 180, 180, 180)                                                 \
  X(Neutral184, 184, 184, 184)                                                 \
  X(Neutral188, 188, 188, 188)                                                 \
  X(Silver, 192, 192, 192)                                                     \
  X(Neutral196, 196, 196, 196)                                                 \
  X(Neutral200, 200, 200, 200)                                                 \
  X(Neutral204, 204, 204, 204)                                                 \
  X(Neutral208, 208, 208, 208)                                                 \
  X(LightGray, 211, 211, 211)                                                  \
  X(Neutral216, 216, 216, 216)                                                 \
  X(Gainsboro, 220, 220, 220)                                                  \
  X(Neutral224, 224, 224, 224)                                                 \
  X(Neutral228, 228, 228, 228)                                                 \
  X(Neutral232, 232, 232, 232)                                                 \
  X(Neutral236, 236, 236, 236)                                                 \
  X(Neutral240, 240, 240, 240)                                                 \
  X(Neutral244, 244, 244, 244)                                                 \
  X(Neutral248, 248, 248, 248)                                                 \
  X(Neutral252, 252, 252, 252)

/*!
 * \enum GrayShade
 * \brief Named shades in the gray family, in palette index order.
 */
enum class GrayShade : std::uint8_t {
#define GGEMS_COLOR_SHADE_ENUMERATOR(NAME, R, G, B) NAME,
  GGEMS_COLOR_GRAY_SHADES(GGEMS_COLOR_SHADE_ENUMERATOR)
#undef GGEMS_COLOR_SHADE_ENUMERATOR
};

/*!
 * \brief Builds the gray family palette.
 *
 * \return Gray scale palette, indexed by GrayShade.
 */
constexpr auto MakeGrayScale() noexcept -> std::array<RGB, color_shade_count> {
#define GGEMS_COLOR_SHADE_RGB(NAME, R, G, B) MakeRGB(R, G, B),
  return {{GGEMS_COLOR_GRAY_SHADES(GGEMS_COLOR_SHADE_RGB)}};
#undef GGEMS_COLOR_SHADE_RGB
}

/// \cond
template <> consteval auto FamilyOf(GrayShade) -> ColorFamily {
  return ColorFamily::Gray;
}
/// \endcond

} // namespace ggems::render
