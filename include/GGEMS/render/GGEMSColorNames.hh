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
 * \brief Declares named GGEMS color shades and ready-to-use color constants.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
/// \endcond

#include "GGEMS/render/GGEMSColorBlue.hh"
#include "GGEMS/render/GGEMSColorRed.hh"
#include "GGEMS/render/GGEMSColorCyan.hh"
#include "GGEMS/render/GGEMSColorGreen.hh"
#include "GGEMS/render/GGEMSColorGray.hh"
#include "GGEMS/render/GGEMSColorMagenta.hh"
#include "GGEMS/render/GGEMSColorOrange.hh"
#include "GGEMS/render/GGEMSColorWhite.hh"
#include "GGEMS/render/GGEMSColorYellow.hh"

#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::render {

/*! \brief Default background color used by GGEMS text rendering. */
inline constexpr ColorKey DEFAULT_BG =
  MakeColor(ColorFamily::Gray, static_cast<std::uint8_t>(GrayShade::Neutral48),
            ColorVariant::Normal, ColorLayer::Background);

/*! \brief Default foreground color used by GGEMS text rendering. */
inline constexpr ColorKey DEFAULT_FG =
  MakeColor(ColorFamily::White, static_cast<std::uint8_t>(WhiteShade::Eggshell),
            ColorVariant::Normal, ColorLayer::Foreground);

/*!
 * \brief Generates one family of named color constants.
 *
 * Creates six ready-to-use ColorKey constants for one named shade: normal,
 * bright, faint, and their three background equivalents. The suffixes are
 * empty, _B, _F, _BG, _B_BG, and _F_BG respectively.
 *
 * \param[in] FAMILYNAME Family prefix in the exported key name.
 * \param[in] SHADENAME Named shade suffix in the exported key name.
 * \param[in] ENUMTYPE Shade enumeration selecting the family.
 * \param[in] VALUE Shade enumerator selecting the palette row.
 */
#define GEN_COLOR_NAME(FAMILYNAME, SHADENAME, ENUMTYPE, VALUE)                 \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME =                         \
    DefineColor(ENUMTYPE::VALUE);                                              \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B =                     \
    DefineColor(ENUMTYPE::VALUE, ColorVariant::Bright);                        \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F =                     \
    DefineColor(ENUMTYPE::VALUE, ColorVariant::Faint);                         \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_BG = DefineColor(       \
    ENUMTYPE::VALUE, ColorVariant::Normal, ColorLayer::Background);            \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B_BG = DefineColor(     \
    ENUMTYPE::VALUE, ColorVariant::Bright, ColorLayer::Background);            \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F_BG =                  \
    DefineColor(ENUMTYPE::VALUE, ColorVariant::Faint, ColorLayer::Background);

/*!
 * \brief Expands one gray shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_GRAY_KEY(NAME, R, G, B)                                    \
  GEN_COLOR_NAME(GRAY, NAME, GrayShade, NAME)
GGEMS_COLOR_GRAY_SHADES(GGEMS_COLOR_GRAY_KEY)
#undef GGEMS_COLOR_GRAY_KEY

/*!
 * \brief Expands one red shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_RED_KEY(NAME, R, G, B)                                     \
  GEN_COLOR_NAME(RED, NAME, RedShade, NAME)
GGEMS_COLOR_RED_SHADES(GGEMS_COLOR_RED_KEY)
#undef GGEMS_COLOR_RED_KEY

/*!
 * \brief Expands one orange shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_ORANGE_KEY(NAME, R, G, B)                                  \
  GEN_COLOR_NAME(ORANGE, NAME, OrangeShade, NAME)
GGEMS_COLOR_ORANGE_SHADES(GGEMS_COLOR_ORANGE_KEY)
#undef GGEMS_COLOR_ORANGE_KEY

/*!
 * \brief Expands one yellow shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_YELLOW_KEY(NAME, R, G, B)                                  \
  GEN_COLOR_NAME(YELLOW, NAME, YellowShade, NAME)
GGEMS_COLOR_YELLOW_SHADES(GGEMS_COLOR_YELLOW_KEY)
#undef GGEMS_COLOR_YELLOW_KEY

/*!
 * \brief Expands one green shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_GREEN_KEY(NAME, R, G, B)                                   \
  GEN_COLOR_NAME(GREEN, NAME, GreenShade, NAME)
GGEMS_COLOR_GREEN_SHADES(GGEMS_COLOR_GREEN_KEY)
#undef GGEMS_COLOR_GREEN_KEY

/*!
 * \brief Expands one cyan shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_CYAN_KEY(NAME, R, G, B)                                    \
  GEN_COLOR_NAME(CYAN, NAME, CyanShade, NAME)
GGEMS_COLOR_CYAN_SHADES(GGEMS_COLOR_CYAN_KEY)
#undef GGEMS_COLOR_CYAN_KEY

/*!
 * \brief Expands one blue shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_BLUE_KEY(NAME, R, G, B)                                    \
  GEN_COLOR_NAME(BLUE, NAME, BlueShade, NAME)
GGEMS_COLOR_BLUE_SHADES(GGEMS_COLOR_BLUE_KEY)
#undef GGEMS_COLOR_BLUE_KEY

/*!
 * \brief Expands one magenta shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_MAGENTA_KEY(NAME, R, G, B)                                 \
  GEN_COLOR_NAME(MAGENTA, NAME, MagentaShade, NAME)
GGEMS_COLOR_MAGENTA_SHADES(GGEMS_COLOR_MAGENTA_KEY)
#undef GGEMS_COLOR_MAGENTA_KEY

/*!
 * \brief Expands one white shade row into its six named color keys.
 * \param[in] NAME Shade enumerator and key-name suffix.
 * \param[in] R Red table channel, unused by key generation.
 * \param[in] G Green table channel, unused by key generation.
 * \param[in] B Blue table channel, unused by key generation.
 */
#define GGEMS_COLOR_WHITE_KEY(NAME, R, G, B)                                   \
  GEN_COLOR_NAME(WHITE, NAME, WhiteShade, NAME)
GGEMS_COLOR_WHITE_SHADES(GGEMS_COLOR_WHITE_KEY)
#undef GGEMS_COLOR_WHITE_KEY

#undef GEN_COLOR_NAME
} // namespace ggems::render
