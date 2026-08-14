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

#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::render {

/*!
 * \enum GreenShade
 * \brief Named shades in the green family.
 */
enum class GreenShade : std::uint8_t {
  Acid = 12,   /*!< Signature acid green. */
  Matrix = 8,  /*!< Matrix green. */
  Emerald = 5, /*!< Emerald green. */
  Jade = 4,    /*!< Jade green. */
  Lime = 10,   /*!< Lime green. */
  Moss = 2,    /*!< Moss green. */
  Mint = 7,    /*!< Mint green. */
  Forest = 11, /*!< Forest green. */
  Neon = 9,    /*!< Neon green. */
  Olive = 3,   /*!< Olive green. */
  Pale = 1,    /*!< Pale green. */
  Dark = 6,    /*!< Dark green. */
  Deep = 0     /*!< Deep green. */
};

/*!
 * \enum BlueShade
 * \brief Named shades in the blue family.
 */
enum class BlueShade : std::uint8_t {
  Abyss = 12,   /*!< Signature abyss blue. */
  Azure = 9,    /*!< Azure blue. */
  Dodger = 8,   /*!< Dodger blue. */
  Ice = 5,      /*!< Ice blue. */
  Deep = 11,    /*!< Deep blue. */
  Gunmetal = 7, /*!< Gunmetal blue. */
  Navy = 10,    /*!< Navy blue. */
  Pale = 3,     /*!< Pale blue. */
  Steel = 4,    /*!< Steel blue. */
  Soft = 2,     /*!< Soft blue. */
  Dark = 6,     /*!< Dark blue. */
  Royal = 1,    /*!< Royal blue. */
  Vibrant = 0   /*!< Vibrant blue. */
};

/*!
 * \enum RedShade
 * \brief Named shades in the red family.
 */
enum class RedShade : std::uint8_t {
  XenoBlood = 12, /*!< Signature xeno-blood red. */
  Crimson = 9,    /*!< Crimson red. */
  Ruby = 8,       /*!< Ruby red. */
  Blood = 11,     /*!< Blood red. */
  Tomato = 5,     /*!< Tomato red. */
  Coral = 4,      /*!< Coral red. */
  Cherry = 10,    /*!< Cherry red. */
  Pale = 3,       /*!< Pale red. */
  Soft = 2,       /*!< Soft red. */
  Dark = 6,       /*!< Dark red. */
  Deep = 7,       /*!< Deep red. */
  Neon = 1,       /*!< Neon red. */
  Vibrant = 0     /*!< Vibrant red. */
};

/*!
 * \enum CyanShade
 * \brief Named shades in the cyan family.
 */
enum class CyanShade : std::uint8_t {
  Cryo = 12,   /*!< Signature cryo cyan. */
  Aqua = 9,    /*!< Aqua cyan. */
  Ice = 5,     /*!< Ice cyan. */
  Sky = 8,     /*!< Sky cyan. */
  Neon = 10,   /*!< Neon cyan. */
  Deep = 11,   /*!< Deep cyan. */
  Soft = 2,    /*!< Soft cyan. */
  Pale = 3,    /*!< Pale cyan. */
  Frost = 4,   /*!< Frost cyan. */
  Dark = 6,    /*!< Dark cyan. */
  Marine = 7,  /*!< Marine cyan. */
  Radiant = 1, /*!< Radiant cyan. */
  Pure = 0     /*!< Pure cyan. */
};

/*!
 * \enum MagentaShade
 * \brief Named shades in the magenta family.
 */
enum class MagentaShade : std::uint8_t {
  FleshSignal = 12, /*!< Signature flesh-signal magenta. */
  Pink = 9,         /*!< Pink magenta. */
  Fuchsia = 10,     /*!< Fuchsia magenta. */
  Deep = 11,        /*!< Deep magenta. */
  Soft = 5,         /*!< Soft magenta. */
  Orchid = 4,       /*!< Orchid magenta. */
  Pale = 3,         /*!< Pale magenta. */
  Dark = 6,         /*!< Dark magenta. */
  Neon = 8,         /*!< Neon magenta. */
  Electric = 7,     /*!< Electric magenta. */
  Rose = 2,         /*!< Rose magenta. */
  Sharp = 1,        /*!< Sharp magenta. */
  Pure = 0          /*!< Pure magenta. */
};

/*!
 * \enum YellowShade
 * \brief Named shades in the yellow family.
 */
enum class YellowShade : std::uint8_t {
  MotherAmber = 12, /*!< Signature mother-amber yellow. */
  Gold = 9,         /*!< Gold yellow. */
  Amber = 10,       /*!< Amber yellow. */
  Lemon = 8,        /*!< Lemon yellow. */
  Pale = 5,         /*!< Pale yellow. */
  Soft = 4,         /*!< Soft yellow. */
  Sand = 2,         /*!< Sand yellow. */
  Dark = 6,         /*!< Dark yellow. */
  Deep = 11,        /*!< Deep yellow. */
  Neon = 7,         /*!< Neon yellow. */
  Solar = 3,        /*!< Solar yellow. */
  Bright = 1,       /*!< Bright yellow. */
  Pure = 0          /*!< Pure yellow. */
};

/*!
 * \enum GrayShade
 * \brief Named shades in the gray family.
 */
enum class GrayShade : std::uint8_t {
  Void = 12,    /*!< Signature void gray. */
  Light = 9,    /*!< Light gray. */
  Soft = 8,     /*!< Soft gray. */
  Silver = 7,   /*!< Silver gray. */
  Concrete = 6, /*!< Concrete gray. */
  Dark = 5,     /*!< Dark gray. */
  Charcoal = 4, /*!< Charcoal gray. */
  Ash = 3,      /*!< Ash gray. */
  Steel = 2,    /*!< Steel gray. */
  Pale = 1,     /*!< Pale gray. */
  Deep = 0,     /*!< Deep gray. */
  Fog = 10,     /*!< Fog gray. */
  Smoke = 11    /*!< Smoke gray. */
};

/*!
 * \enum WhiteShade
 * \brief Named shades in the white family.
 */
enum class WhiteShade : std::uint8_t {
  Bone = 12,   /*!< Signature bone white. */
  Pure = 0,    /*!< Pure white. */
  Snow = 1,    /*!< Snow white. */
  Pearl = 2,   /*!< Pearl white. */
  Ivory = 3,   /*!< Ivory white. */
  Cream = 4,   /*!< Cream white. */
  Frost = 5,   /*!< Frost white. */
  Ice = 6,     /*!< Ice white. */
  Soft = 7,    /*!< Soft white. */
  Pale = 8,    /*!< Pale white. */
  Cold = 9,    /*!< Cold white. */
  Bright = 10, /*!< Bright white. */
  Glare = 11   /*!< Glare white. */
};

/*!
 * \enum OrangeShade
 * \brief Named shades in the orange family.
 */
enum class OrangeShade : std::uint8_t {
  Deep = 0,         /*!< Deep orange. */
  Dark = 1,         /*!< Dark orange. */
  Burnt = 2,        /*!< Burnt orange. */
  Copper = 3,       /*!< Copper orange. */
  Rust = 4,         /*!< Rust orange. */
  Chocolate = 5,    /*!< Chocolate orange. */
  Coral = 6,        /*!< Coral orange. */
  Flame = 7,        /*!< Flame orange. */
  Pure = 8,         /*!< Pure orange. */
  Tangerine = 9,    /*!< Tangerine orange. */
  Soft = 10,        /*!< Soft orange. */
  Pale = 11,        /*!< Pale orange. */
  CopperSignal = 12 /*!< Signature copper-signal orange. */
};

/// \cond
template <typename ShadeEnum> consteval auto FamilyOf(ShadeEnum) -> ColorFamily;

template <> consteval auto FamilyOf(GreenShade) -> ColorFamily {
  return ColorFamily::Green;
}

template <> consteval auto FamilyOf(BlueShade) -> ColorFamily {
  return ColorFamily::Blue;
}

template <> consteval auto FamilyOf(RedShade) -> ColorFamily {
  return ColorFamily::Red;
}

template <> consteval auto FamilyOf(CyanShade) -> ColorFamily {
  return ColorFamily::Cyan;
}

template <> consteval auto FamilyOf(MagentaShade) -> ColorFamily {
  return ColorFamily::Magenta;
}

template <> consteval auto FamilyOf(YellowShade) -> ColorFamily {
  return ColorFamily::Yellow;
}

template <> consteval auto FamilyOf(GrayShade) -> ColorFamily {
  return ColorFamily::Gray;
}

template <> consteval auto FamilyOf(WhiteShade) -> ColorFamily {
  return ColorFamily::White;
}

template <> consteval auto FamilyOf(OrangeShade) -> ColorFamily {
  return ColorFamily::Orange;
}
/// \endcond

/*!
 * \brief Builds one named GGEMS color key.
 *
 * \tparam ShadeEnum Shade enumeration type.
 * \param[in] shade Selected named shade.
 * \param[in] variant Color variant.
 * \param[in] layer Target color layer.
 * \return Named color key.
 */
template <typename ShadeEnum>
consteval auto DefineColor(ShadeEnum shade,
                           ColorVariant variant = ColorVariant::Normal,
                           ColorLayer layer = ColorLayer::Foreground)
    -> ColorKey {
  return MakeColor(FamilyOf(shade), static_cast<std::uint8_t>(shade), variant,
                   layer);
}

/*! \brief Default background color used by GGEMS text rendering. */
inline constexpr ColorKey DEFAULT_BG =
    MakeColor(ColorFamily::Gray, static_cast<std::uint8_t>(GrayShade::Steel),
              ColorVariant::Normal, ColorLayer::Background);

/*! \brief Default foreground color used by GGEMS text rendering. */
inline constexpr ColorKey DEFAULT_FG =
    MakeColor(ColorFamily::White, static_cast<std::uint8_t>(WhiteShade::Ivory),
              ColorVariant::Normal, ColorLayer::Foreground);

/*!
 * \brief Generates one family of named color constants.
 *
 * Creates six ready-to-use ColorKey constants for one named shade:
 * normal, bright, faint, and their three background equivalents.
 */
#define GEN_COLOR_NAME(FAMILYNAME, SHADENAME, ENUMTYPE, VALUE)                 \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME =                         \
      DefineColor(ENUMTYPE::VALUE);                                            \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B =                     \
      DefineColor(ENUMTYPE::VALUE, ColorVariant::Bright);                      \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F =                     \
      DefineColor(ENUMTYPE::VALUE, ColorVariant::Faint);                       \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_BG = DefineColor(       \
      ENUMTYPE::VALUE, ColorVariant::Normal, ColorLayer::Background);          \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B_BG = DefineColor(     \
      ENUMTYPE::VALUE, ColorVariant::Bright, ColorLayer::Background);          \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F_BG = DefineColor(     \
      ENUMTYPE::VALUE, ColorVariant::Faint, ColorLayer::Background);

GEN_COLOR_NAME(GREEN, Matrix, GreenShade, Matrix)
GEN_COLOR_NAME(GREEN, Emerald, GreenShade, Emerald)
GEN_COLOR_NAME(GREEN, Jade, GreenShade, Jade)
GEN_COLOR_NAME(GREEN, Lime, GreenShade, Lime)
GEN_COLOR_NAME(GREEN, Moss, GreenShade, Moss)
GEN_COLOR_NAME(GREEN, Mint, GreenShade, Mint)
GEN_COLOR_NAME(GREEN, Forest, GreenShade, Forest)
GEN_COLOR_NAME(GREEN, Neon, GreenShade, Neon)
GEN_COLOR_NAME(GREEN, Olive, GreenShade, Olive)
GEN_COLOR_NAME(GREEN, Pale, GreenShade, Pale)
GEN_COLOR_NAME(GREEN, Dark, GreenShade, Dark)
GEN_COLOR_NAME(GREEN, Deep, GreenShade, Deep)
GEN_COLOR_NAME(GREEN, Acid, GreenShade, Acid)

GEN_COLOR_NAME(BLUE, Azure, BlueShade, Azure)
GEN_COLOR_NAME(BLUE, Dodger, BlueShade, Dodger)
GEN_COLOR_NAME(BLUE, Ice, BlueShade, Ice)
GEN_COLOR_NAME(BLUE, Deep, BlueShade, Deep)
GEN_COLOR_NAME(BLUE, Gunmetal, BlueShade, Gunmetal)
GEN_COLOR_NAME(BLUE, Navy, BlueShade, Navy)
GEN_COLOR_NAME(BLUE, Pale, BlueShade, Pale)
GEN_COLOR_NAME(BLUE, Steel, BlueShade, Steel)
GEN_COLOR_NAME(BLUE, Soft, BlueShade, Soft)
GEN_COLOR_NAME(BLUE, Dark, BlueShade, Dark)
GEN_COLOR_NAME(BLUE, Royal, BlueShade, Royal)
GEN_COLOR_NAME(BLUE, Vibrant, BlueShade, Vibrant)
GEN_COLOR_NAME(BLUE, Abyss, BlueShade, Abyss)

GEN_COLOR_NAME(RED, Crimson, RedShade, Crimson)
GEN_COLOR_NAME(RED, Ruby, RedShade, Ruby)
GEN_COLOR_NAME(RED, Blood, RedShade, Blood)
GEN_COLOR_NAME(RED, Tomato, RedShade, Tomato)
GEN_COLOR_NAME(RED, Coral, RedShade, Coral)
GEN_COLOR_NAME(RED, Cherry, RedShade, Cherry)
GEN_COLOR_NAME(RED, Pale, RedShade, Pale)
GEN_COLOR_NAME(RED, Soft, RedShade, Soft)
GEN_COLOR_NAME(RED, Dark, RedShade, Dark)
GEN_COLOR_NAME(RED, Deep, RedShade, Deep)
GEN_COLOR_NAME(RED, Neon, RedShade, Neon)
GEN_COLOR_NAME(RED, Vibrant, RedShade, Vibrant)
GEN_COLOR_NAME(RED, XenoBlood, RedShade, XenoBlood)

GEN_COLOR_NAME(CYAN, Aqua, CyanShade, Aqua)
GEN_COLOR_NAME(CYAN, Ice, CyanShade, Ice)
GEN_COLOR_NAME(CYAN, Sky, CyanShade, Sky)
GEN_COLOR_NAME(CYAN, Neon, CyanShade, Neon)
GEN_COLOR_NAME(CYAN, Deep, CyanShade, Deep)
GEN_COLOR_NAME(CYAN, Soft, CyanShade, Soft)
GEN_COLOR_NAME(CYAN, Pale, CyanShade, Pale)
GEN_COLOR_NAME(CYAN, Frost, CyanShade, Frost)
GEN_COLOR_NAME(CYAN, Dark, CyanShade, Dark)
GEN_COLOR_NAME(CYAN, Marine, CyanShade, Marine)
GEN_COLOR_NAME(CYAN, Radiant, CyanShade, Radiant)
GEN_COLOR_NAME(CYAN, Pure, CyanShade, Pure)
GEN_COLOR_NAME(CYAN, Cryo, CyanShade, Cryo)

GEN_COLOR_NAME(MAGENTA, Pink, MagentaShade, Pink)
GEN_COLOR_NAME(MAGENTA, Fuchsia, MagentaShade, Fuchsia)
GEN_COLOR_NAME(MAGENTA, Deep, MagentaShade, Deep)
GEN_COLOR_NAME(MAGENTA, Soft, MagentaShade, Soft)
GEN_COLOR_NAME(MAGENTA, Orchid, MagentaShade, Orchid)
GEN_COLOR_NAME(MAGENTA, Pale, MagentaShade, Pale)
GEN_COLOR_NAME(MAGENTA, Dark, MagentaShade, Dark)
GEN_COLOR_NAME(MAGENTA, Neon, MagentaShade, Neon)
GEN_COLOR_NAME(MAGENTA, Electric, MagentaShade, Electric)
GEN_COLOR_NAME(MAGENTA, Rose, MagentaShade, Rose)
GEN_COLOR_NAME(MAGENTA, Sharp, MagentaShade, Sharp)
GEN_COLOR_NAME(MAGENTA, Pure, MagentaShade, Pure)
GEN_COLOR_NAME(MAGENTA, FleshSignal, MagentaShade, FleshSignal)

GEN_COLOR_NAME(YELLOW, Gold, YellowShade, Gold)
GEN_COLOR_NAME(YELLOW, Amber, YellowShade, Amber)
GEN_COLOR_NAME(YELLOW, Lemon, YellowShade, Lemon)
GEN_COLOR_NAME(YELLOW, Pale, YellowShade, Pale)
GEN_COLOR_NAME(YELLOW, Soft, YellowShade, Soft)
GEN_COLOR_NAME(YELLOW, Sand, YellowShade, Sand)
GEN_COLOR_NAME(YELLOW, Dark, YellowShade, Dark)
GEN_COLOR_NAME(YELLOW, Deep, YellowShade, Deep)
GEN_COLOR_NAME(YELLOW, Neon, YellowShade, Neon)
GEN_COLOR_NAME(YELLOW, Solar, YellowShade, Solar)
GEN_COLOR_NAME(YELLOW, Bright, YellowShade, Bright)
GEN_COLOR_NAME(YELLOW, Pure, YellowShade, Pure)
GEN_COLOR_NAME(YELLOW, MotherAmber, YellowShade, MotherAmber)

GEN_COLOR_NAME(GRAY, Light, GrayShade, Light)
GEN_COLOR_NAME(GRAY, Soft, GrayShade, Soft)
GEN_COLOR_NAME(GRAY, Silver, GrayShade, Silver)
GEN_COLOR_NAME(GRAY, Concrete, GrayShade, Concrete)
GEN_COLOR_NAME(GRAY, Dark, GrayShade, Dark)
GEN_COLOR_NAME(GRAY, Charcoal, GrayShade, Charcoal)
GEN_COLOR_NAME(GRAY, Ash, GrayShade, Ash)
GEN_COLOR_NAME(GRAY, Steel, GrayShade, Steel)
GEN_COLOR_NAME(GRAY, Pale, GrayShade, Pale)
GEN_COLOR_NAME(GRAY, Deep, GrayShade, Deep)
GEN_COLOR_NAME(GRAY, Fog, GrayShade, Fog)
GEN_COLOR_NAME(GRAY, Smoke, GrayShade, Smoke)
GEN_COLOR_NAME(GRAY, Void, GrayShade, Void)

GEN_COLOR_NAME(WHITE, Pure, WhiteShade, Pure)
GEN_COLOR_NAME(WHITE, Snow, WhiteShade, Snow)
GEN_COLOR_NAME(WHITE, Pearl, WhiteShade, Pearl)
GEN_COLOR_NAME(WHITE, Ivory, WhiteShade, Ivory)
GEN_COLOR_NAME(WHITE, Cream, WhiteShade, Cream)
GEN_COLOR_NAME(WHITE, Frost, WhiteShade, Frost)
GEN_COLOR_NAME(WHITE, Ice, WhiteShade, Ice)
GEN_COLOR_NAME(WHITE, Soft, WhiteShade, Soft)
GEN_COLOR_NAME(WHITE, Pale, WhiteShade, Pale)
GEN_COLOR_NAME(WHITE, Cold, WhiteShade, Cold)
GEN_COLOR_NAME(WHITE, Bright, WhiteShade, Bright)
GEN_COLOR_NAME(WHITE, Glare, WhiteShade, Glare)
GEN_COLOR_NAME(WHITE, Bone, WhiteShade, Bone)

GEN_COLOR_NAME(ORANGE, Deep, OrangeShade, Deep)
GEN_COLOR_NAME(ORANGE, Dark, OrangeShade, Dark)
GEN_COLOR_NAME(ORANGE, Burnt, OrangeShade, Burnt)
GEN_COLOR_NAME(ORANGE, Copper, OrangeShade, Copper)
GEN_COLOR_NAME(ORANGE, Rust, OrangeShade, Rust)
GEN_COLOR_NAME(ORANGE, Chocolate, OrangeShade, Chocolate)
GEN_COLOR_NAME(ORANGE, Coral, OrangeShade, Coral)
GEN_COLOR_NAME(ORANGE, Flame, OrangeShade, Flame)
GEN_COLOR_NAME(ORANGE, Pure, OrangeShade, Pure)
GEN_COLOR_NAME(ORANGE, Soft, OrangeShade, Soft)
GEN_COLOR_NAME(ORANGE, Tangerine, OrangeShade, Tangerine)
GEN_COLOR_NAME(ORANGE, Pale, OrangeShade, Pale)
GEN_COLOR_NAME(ORANGE, CopperSignal, OrangeShade, CopperSignal)

#undef GEN_COLOR_NAME
} // namespace ggems::render
