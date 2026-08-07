#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSColorNames.hh
 * \brief Human-friendly color aliases built on GGEMSColor palette.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright GNU GPL v3.0
 *
 * This header builds named color aliases on top of the generic color
 * system defined in \ref GGEMSColor.hh. For each color family, an
 * enumeration of semantic shade names is provided (for example Matrix,
 * Emerald, Jade for green), corresponding to specific shade indices.
 *
 * A constexpr metafunction maps shade enums to their color families, and
 * a helper generates \ref ggems::render::ColorKey values at compile time. A
 * macro creates a dense set of aliases for each name, covering both foreground
 * and background, and all variants Normal, Bright, and Faint.
 *
 * These facilities are intended to provide:
 *  - readability in UI and logging code,
 *  - a stable color vocabulary across terminal and GUI back-ends.
 */

#include <cstdint>

#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::render {
/*!
 * \enum GreenShade
 * \brief Named green shades mapped to palette indices.
 *
 * Each enumerator corresponds to a shade index in the green family
 * defined in \ref ggems::render::MakeGreenScale. The numeric values match the
 * shade index in the underlying palette.
 */
enum class GreenShade : std::uint8_t {
  Acid = 12,
  Matrix = 8,  /*!< Bright neon-green, high energy */
  Emerald = 5, /*!< Balanced vivid green */
  Jade = 4,    /*!< Deep mineral green */
  Lime = 10,   /*!< Fresh light green with yellow tint */
  Moss = 2,    /*!< Muted earthy green */
  Mint = 7,    /*!< Pale cold green */
  Forest = 11, /*!< Dark evergreen tone */
  Neon = 9,    /*!< High-intensity green highlight */
  Olive = 3,   /*!< Warm yellow-green */
  Pale = 1,    /*!< Soft desaturated green */
  Dark = 6,    /*!< Deep low-value green */
  Deep = 0     /*!< Maximum depth baseline green */
};

/*!
 * \enum BlueShade
 * \brief Named blue shades mapped to palette indices.
 */
enum class BlueShade : std::uint8_t {
  Abyss = 12,
  Azure = 9,  /*!< Bright light blue */
  Dodger = 8, /*!< Strong electric blue */
  Ice = 5,    /*!< Very pale icy blue */
  Deep = 11,  /*!< Highly saturated deep blue */
  Gunmetal = 7,
  Navy = 10,  /*!< Dark navy blue */
  Pale = 3,   /*!< Soft low-saturation blue */
  Steel = 4,  /*!< Slightly grayish steel blue */
  Soft = 2,   /*!< Gentle, unobtrusive blue */
  Dark = 6,   /*!< Deepened blue for shadows or edges */
  Royal = 1,  /*!< Balanced royal blue */
  Vibrant = 0 /*!< Energetic, vivid blue baseline */
};

/*!
 * \enum RedShade
 * \brief Named red shades mapped to palette indices.
 */
enum class RedShade : std::uint8_t {
  XenoBlood = 12,
  Crimson = 9, /*!< Intense deep crimson red */
  Ruby = 8,    /*!< Strong ruby-like red */
  Blood = 11,  /*!< Very dark dramatic red */
  Tomato = 5,  /*!< Warm tomato-like red with an orange note */
  Coral = 4,   /*!< Softer coral red */
  Cherry = 10, /*!< Bright cherry red */
  Pale = 3,    /*!< Soft low-saturation red */
  Soft = 2,    /*!< Gentle pastel red */
  Dark = 6,    /*!< Deep red for contrast or structural elements */
  Deep = 7,    /*!< Mineral-like deep red */
  Neon = 1,    /*!< High-brightness neon red */
  Vibrant = 0  /*!< Vivid energetic baseline red */
};

/*!
 * \enum CyanShade
 * \brief Named cyan shades mapped to palette indices.
 */
enum class CyanShade : std::uint8_t {
  Cryo = 12,
  Aqua = 9,    /*!< Bright aqua cyan */
  Ice = 5,     /*!< Pale icy cyan tone */
  Sky = 8,     /*!< Cyan leaning toward light sky blue */
  Neon = 10,   /*!< Very intense neon cyan */
  Deep = 11,   /*!< Saturated deep cyan */
  Soft = 2,    /*!< Gentle low-intensity cyan */
  Pale = 3,    /*!< Soft desaturated cyan */
  Frost = 4,   /*!< Cool frosted cyan */
  Dark = 6,    /*!< Dark cyan for contrast */
  Marine = 7,  /*!< Marine-leaning cyan tone */
  Radiant = 1, /*!< Bright high-contrast cyan */
  Pure = 0     /*!< Neutral baseline cyan */
};

/*!
 * \enum MagentaShade
 * \brief Named magenta shades mapped to palette indices.
 */
enum class MagentaShade : std::uint8_t {
  FleshSignal = 12,
  Pink = 9,     /*!< Light pink-tinted magenta */
  Fuchsia = 10, /*!< Strong saturated fuchsia */
  Deep = 11,    /*!< Very dark intense magenta */
  Soft = 5,     /*!< Soft pastel magenta */
  Orchid = 4,   /*!< Orchid-like purple-magenta */
  Pale = 3,     /*!< Pale low-saturation magenta */
  Dark = 6,     /*!< Dark magenta for deep accents */
  Neon = 8,     /*!< Bright neon magenta */
  Electric = 7, /*!< Highly energetic electric magenta */
  Rose = 2,     /*!< Rose-leaning magenta */
  Sharp = 1,    /*!< Sharp, high-contrast magenta */
  Pure = 0      /*!< Neutral baseline magenta */
};

/*!
 * \enum YellowShade
 * \brief Named yellow shades mapped to palette indices.
 */
enum class YellowShade : std::uint8_t {
  MotherAmber = 12,
  Gold = 9,   /*!< Deep golden yellow */
  Amber = 10, /*!< Warm amber yellow */
  Lemon = 8,  /*!< Vivid lemon yellow */
  Pale = 5,   /*!< Pale desaturated yellow */
  Soft = 4,   /*!< Soft gentle yellow */
  Sand = 2,   /*!< Sandy muted yellow */
  Dark = 6,   /*!< Dark yellow for strong contrast */
  Deep = 11,  /*!< Deep saturated yellow */
  Neon = 7,   /*!< High-intensity neon yellow */
  Solar = 3,  /*!< Radiant sun-like yellow */
  Bright = 1, /*!< Very bright yellow */
  Pure = 0    /*!< Neutral baseline yellow */
};

/*!
 * \enum GrayShade
 * \brief Named gray shades mapped to palette indices.
 */
enum class GrayShade : std::uint8_t {
  Void = 12,
  Light = 9,    /*!< Very light near-white gray */
  Soft = 8,     /*!< Soft mid-light neutral gray */
  Silver = 7,   /*!< Metallic silver-like gray */
  Concrete = 6, /*!< Concrete-like mid-dark gray */
  Dark = 5,     /*!< Dark neutral gray */
  Charcoal = 4, /*!< Charcoal deep gray */
  Ash = 3,      /*!< Ash-like gray with low saturation */
  Steel = 2,    /*!< Industrial steel gray */
  Pale = 1,     /*!< Pale low-intensity gray */
  Deep = 0,     /*!< Deepest gray of the family */
  Fog = 10,     /*!< Fog-like soft gray with slight cool tone */
  Smoke = 11    /*!< Smoke-deepened dark gray */
};

/*!
 * \enum WhiteShade
 * \brief Named white/near-white shades mapped to palette indices.
 */
enum class WhiteShade : std::uint8_t {
  Bone = 12,
  Pure = 0,    /*!< Neutral baseline white */
  Snow = 1,    /*!< Cold snow-like white */
  Pearl = 2,   /*!< Slightly warm pearlescent white */
  Ivory = 3,   /*!< Natural ivory-toned white */
  Cream = 4,   /*!< Soft creamy off-white */
  Frost = 5,   /*!< Frosted cool white */
  Ice = 6,     /*!< Icy white with a subtle bluish hint */
  Soft = 7,    /*!< Softened white with reduced contrast */
  Pale = 8,    /*!< Very pale off-white */
  Cold = 9,    /*!< Cool white leaning toward gray */
  Bright = 10, /*!< High-luminance bright white */
  Glare = 11   /*!< Extremely intense white with strong glare */
};

/*!
 * \brief Compile-time mapping from shade enum to color family.
 *
 * The primary template is left undefined and is specialized for each
 * shade enumeration. Attempting to use it with an unsupported shade
 * type will result in a compile-time error.
 *
 * \tparam ShadeEnum Shade enumeration type.
 * \return Color family corresponding to the shade enumeration.
 */
template <typename ShadeEnum>
consteval auto FamilyOf(ShadeEnum) -> ColorFamily;

/*!
 * \def DEF_FAMILY
 * \brief Helper macro to specialize FamilyOf for a shade enumeration.
 *
 * \param ShadeEnum Shade enumeration type (for example GreenShade).
 * \param FamilyName Corresponding \ref ggems::render::ColorFamily value.
 */
#define DEF_FAMILY(ShadeEnum, FamilyName)                                      \
  template <> consteval auto FamilyOf(ShadeEnum) -> ColorFamily {             \
    return FamilyName;                                                         \
  }

DEF_FAMILY(GreenShade, ColorFamily::Green)
DEF_FAMILY(BlueShade, ColorFamily::Blue)
DEF_FAMILY(RedShade, ColorFamily::Red)
DEF_FAMILY(CyanShade, ColorFamily::Cyan)
DEF_FAMILY(MagentaShade, ColorFamily::Magenta)
DEF_FAMILY(YellowShade, ColorFamily::Yellow)
DEF_FAMILY(GrayShade, ColorFamily::Gray)
DEF_FAMILY(WhiteShade, ColorFamily::White)

#undef DEF_FAMILY

/*!
 * \brief Define a \ref ggems::render::ColorKey from a named shade enumeration.
 *
 * \tparam ShadeEnum Shade enumeration type (for example GreenShade).
 * \param shade Shade enumerator selecting the palette index.
 * \param variant Intensity variant (Normal, Bright, Faint).
 * \param layer Target layer (Foreground or Background).
 * \return Color key combining the family, shade index, variant and layer.
 */
template <typename ShadeEnum>
consteval auto DefineColor(ShadeEnum shade,
                            ColorVariant variant = ColorVariant::Normal,
                            ColorLayer layer = ColorLayer::Foreground)
    -> ColorKey {
  return MakeColor(FamilyOf(shade), static_cast<std::uint8_t>(shade), variant,
                    layer);
}

/*!
 * \brief Default GGEMS UI background color.
 *
 * This is a neutral gray intended to provide comfortable contrast
 * with the default foreground.
 */
inline constexpr ColorKey DEFAULT_BG =
    MakeColor(ColorFamily::Gray, static_cast<std::uint8_t>(GrayShade::Steel),
               ColorVariant::Normal, ColorLayer::Background);

/*!
 * \brief Default GGEMS UI foreground color.
 *
 * This is a soft off-white selected for readability on \ref
 * ggems::render::DEFAULT_BG.
 */
inline constexpr ColorKey DEFAULT_FG = MakeColor(
    ColorFamily::White, static_cast<std::uint8_t>(WhiteShade::Ivory),
    ColorVariant::Normal, ColorLayer::Foreground);

/*!
 * \def GEN_COLOR_NAME
 * \brief Generate a full set of color aliases for a named shade.
 *
 * For a given family label, shade name, enumeration type and enumerator
 * value, this macro declares six \ref ggems::render::ColorKey constants:
 *
 *  - FAMILYNAME_SHADENAME: normal variant, foreground layer
 *  - FAMILYNAME_SHADENAME_B: bright variant, foreground layer
 *  - FAMILYNAME_SHADENAME_F: faint variant, foreground layer
 *  - FAMILYNAME_SHADENAME_BG: normal variant, background layer
 *  - FAMILYNAME_SHADENAME_B_BG: bright variant, background layer
 *  - FAMILYNAME_SHADENAME_F_BG: faint variant, background layer
 *
 * These aliases are intended for direct use in UI and logging code.
 *
 * \param FAMILYNAME Uppercase family label (for example GREEN).
 * \param SHADENAME Shade label (for example Matrix).
 * \param ENUMTYPE Shade enumeration type (for example GreenShade).
 * \param VALUE Enumerated shade value (for example Matrix).
 */
#define GEN_COLOR_NAME(FAMILYNAME, SHADENAME, ENUMTYPE, VALUE)                \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME =                        \
      DefineColor(ENUMTYPE::VALUE);                                           \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B =                    \
      DefineColor(ENUMTYPE::VALUE, ColorVariant::Bright);                    \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F =                    \
      DefineColor(ENUMTYPE::VALUE, ColorVariant::Faint);                     \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_BG = DefineColor(     \
      ENUMTYPE::VALUE, ColorVariant::Normal, ColorLayer::Background);        \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_B_BG = DefineColor(   \
      ENUMTYPE::VALUE, ColorVariant::Bright, ColorLayer::Background);        \
  inline constexpr ColorKey FAMILYNAME##_##SHADENAME##_F_BG = DefineColor(   \
      ENUMTYPE::VALUE, ColorVariant::Faint, ColorLayer::Background);

// Green
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

// Blue
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

// Red
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

// Cyan
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

// Magenta
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

// Yellow
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

// Gray
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

// White
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

#undef GEN_COLOR_NAME
} // namespace ggems::render
