#pragma once

#include "GGEMS/render/GGEMSColour.hh"

namespace ggems::render {

enum class GreenShade : std::uint8_t {
  Matrix = 8,
  Emerald = 5,
  Jade = 4,
  Lime = 10,
  Moss = 2,
  Mint = 7,
  Forest = 11,
  Neon = 9,
  Olive = 3,
  Pale = 1,
  Dark = 6,
  Deep = 0
};

enum class BlueShade : std::uint8_t {
  Azure = 9,
  Dodger = 8,
  Ice = 5,
  Deep = 11,
  Sky = 7,
  Navy = 10,
  Pale = 3,
  Steel = 4,
  Soft = 2,
  Dark = 6,
  Royal = 1,
  Vibrant = 0
};

enum class RedShade : std::uint8_t {
  Crimson = 9,
  Ruby = 8,
  Blood = 11,
  Tomato = 5,
  Coral = 4,
  Cherry = 10,
  Pale = 3,
  Soft = 2,
  Dark = 6,
  Deep = 7,
  Neon = 1,
  Vibrant = 0
};

enum class CyanShade : std::uint8_t {
  Aqua = 9,
  Ice = 5,
  Sky = 8,
  Neon = 10,
  Deep = 11,
  Soft = 2,
  Pale = 3,
  Frost = 4,
  Dark = 6,
  Marine = 7,
  Radiant = 1,
  Pure = 0
};

enum class MagentaShade : std::uint8_t {
  Pink = 9,
  Fuchsia = 10,
  Deep = 11,
  Soft = 5,
  Orchid = 4,
  Pale = 3,
  Dark = 6,
  Neon = 8,
  Electric = 7,
  Rose = 2,
  Sharp = 1,
  Pure = 0
};

enum class YellowShade : std::uint8_t {
  Gold = 9,
  Amber = 10,
  Lemon = 8,
  Pale = 5,
  Soft = 4,
  Sand = 2,
  Dark = 6,
  Deep = 11,
  Neon = 7,
  Solar = 3,
  Bright = 1,
  Pure = 0
};

enum class GreyShade : std::uint8_t {
  Light = 9,
  Soft = 8,
  Silver = 7,
  Concrete = 6,
  Dark = 5,
  Charcoal = 4,
  Ash = 3,
  Steel = 2,
  Pale = 1,
  Deep = 0,
  Fog = 10,
  Smoke = 11
};

enum class WhiteShade : std::uint8_t {
  Pure = 0,
  Snow = 1,
  Pearl = 2,
  Ivory = 3,
  Cream = 4,
  Frost = 5,
  Ice = 6,
  Soft = 7,
  Pale = 8,
  Cold = 9,
  Bright = 10,
  Glare = 11
};

// ------------------------------------------------------------
// 2) Metafunction : FamilyOf(Shade)
// ------------------------------------------------------------

template <typename ShadeEnum> consteval ColourFamily FamilyOf(ShadeEnum);

#define DEF_FAMILY(ShadeEnum, FamilyName)                                      \
  template <> consteval ColourFamily FamilyOf(ShadeEnum) { return FamilyName; }

DEF_FAMILY(GreenShade, ColourFamily::Green)
DEF_FAMILY(BlueShade, ColourFamily::Blue)
DEF_FAMILY(RedShade, ColourFamily::Red)
DEF_FAMILY(CyanShade, ColourFamily::Cyan)
DEF_FAMILY(MagentaShade, ColourFamily::Magenta)
DEF_FAMILY(YellowShade, ColourFamily::Yellow)
DEF_FAMILY(GreyShade, ColourFamily::Grey)
DEF_FAMILY(WhiteShade, ColourFamily::White)

#undef DEF_FAMILY

// ------------------------------------------------------------
// 3) Generate colour name
// ------------------------------------------------------------

template <typename ShadeEnum>
consteval ColourKey DefineColour(ShadeEnum shade,
                                 ColourVariant variant = ColourVariant::Normal,
                                 ColourLayer layer = ColourLayer::Foreground) {
  return MakeColour(FamilyOf(shade), static_cast<std::uint8_t>(shade), variant,
                    layer);
}

inline constexpr ColourKey DEFAULT_BG =
    MakeColour(ColourFamily::Grey, static_cast<std::uint8_t>(GreyShade::Steel),
               ColourVariant::Normal, ColourLayer::Background);

inline constexpr ColourKey DEFAULT_FG = MakeColour(
    ColourFamily::White, static_cast<std::uint8_t>(WhiteShade::Ivory),
    ColourVariant::Normal, ColourLayer::Foreground);

// ------------------------------------------------------------
// 4) Macro for all aliases
// ------------------------------------------------------------
//  Produit :
//      NAME
//      NAME_B
//      NAME_F
//      NAME_BG
//      NAME_B_BG
//      NAME_F_BG
// ------------------------------------------------------------

#define GEN_COLOUR_NAME(FAMILYNAME, SHADENAME, ENUMTYPE, VALUE)                \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME =                        \
      DefineColour(ENUMTYPE::VALUE);                                           \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME##_B =                    \
      DefineColour(ENUMTYPE::VALUE, ColourVariant::Bright);                    \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME##_F =                    \
      DefineColour(ENUMTYPE::VALUE, ColourVariant::Faint);                     \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME##_BG = DefineColour(     \
      ENUMTYPE::VALUE, ColourVariant::Normal, ColourLayer::Background);        \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME##_B_BG = DefineColour(   \
      ENUMTYPE::VALUE, ColourVariant::Bright, ColourLayer::Background);        \
  inline constexpr ColourKey FAMILYNAME##_##SHADENAME##_F_BG = DefineColour(   \
      ENUMTYPE::VALUE, ColourVariant::Faint, ColourLayer::Background);

// ------------------------------------------------------------
// 5) Instanciation
// ------------------------------------------------------------

// Green
GEN_COLOUR_NAME(GREEN, Matrix, GreenShade, Matrix)
GEN_COLOUR_NAME(GREEN, Emerald, GreenShade, Emerald)
GEN_COLOUR_NAME(GREEN, Jade, GreenShade, Jade)
GEN_COLOUR_NAME(GREEN, Lime, GreenShade, Lime)
GEN_COLOUR_NAME(GREEN, Moss, GreenShade, Moss)
GEN_COLOUR_NAME(GREEN, Mint, GreenShade, Mint)
GEN_COLOUR_NAME(GREEN, Forest, GreenShade, Forest)
GEN_COLOUR_NAME(GREEN, Neon, GreenShade, Neon)
GEN_COLOUR_NAME(GREEN, Olive, GreenShade, Olive)
GEN_COLOUR_NAME(GREEN, Pale, GreenShade, Pale)
GEN_COLOUR_NAME(GREEN, Dark, GreenShade, Dark)
GEN_COLOUR_NAME(GREEN, Deep, GreenShade, Deep)

// Blue
GEN_COLOUR_NAME(BLUE, Azure, BlueShade, Azure)
GEN_COLOUR_NAME(BLUE, Dodger, BlueShade, Dodger)
GEN_COLOUR_NAME(BLUE, Ice, BlueShade, Ice)
GEN_COLOUR_NAME(BLUE, Deep, BlueShade, Deep)
GEN_COLOUR_NAME(BLUE, Sky, BlueShade, Sky)
GEN_COLOUR_NAME(BLUE, Navy, BlueShade, Navy)
GEN_COLOUR_NAME(BLUE, Pale, BlueShade, Pale)
GEN_COLOUR_NAME(BLUE, Steel, BlueShade, Steel)
GEN_COLOUR_NAME(BLUE, Soft, BlueShade, Soft)
GEN_COLOUR_NAME(BLUE, Dark, BlueShade, Dark)
GEN_COLOUR_NAME(BLUE, Royal, BlueShade, Royal)
GEN_COLOUR_NAME(BLUE, Vibrant, BlueShade, Vibrant)

// Red
GEN_COLOUR_NAME(RED, Crimson, RedShade, Crimson)
GEN_COLOUR_NAME(RED, Ruby, RedShade, Ruby)
GEN_COLOUR_NAME(RED, Blood, RedShade, Blood)
GEN_COLOUR_NAME(RED, Tomato, RedShade, Tomato)
GEN_COLOUR_NAME(RED, Coral, RedShade, Coral)
GEN_COLOUR_NAME(RED, Cherry, RedShade, Cherry)
GEN_COLOUR_NAME(RED, Pale, RedShade, Pale)
GEN_COLOUR_NAME(RED, Soft, RedShade, Soft)
GEN_COLOUR_NAME(RED, Dark, RedShade, Dark)
GEN_COLOUR_NAME(RED, Deep, RedShade, Deep)
GEN_COLOUR_NAME(RED, Neon, RedShade, Neon)
GEN_COLOUR_NAME(RED, Vibrant, RedShade, Vibrant)

// Cyan
GEN_COLOUR_NAME(CYAN, Aqua, CyanShade, Aqua)
GEN_COLOUR_NAME(CYAN, Ice, CyanShade, Ice)
GEN_COLOUR_NAME(CYAN, Sky, CyanShade, Sky)
GEN_COLOUR_NAME(CYAN, Neon, CyanShade, Neon)
GEN_COLOUR_NAME(CYAN, Deep, CyanShade, Deep)
GEN_COLOUR_NAME(CYAN, Soft, CyanShade, Soft)
GEN_COLOUR_NAME(CYAN, Pale, CyanShade, Pale)
GEN_COLOUR_NAME(CYAN, Frost, CyanShade, Frost)
GEN_COLOUR_NAME(CYAN, Dark, CyanShade, Dark)
GEN_COLOUR_NAME(CYAN, Marine, CyanShade, Marine)
GEN_COLOUR_NAME(CYAN, Radiant, CyanShade, Radiant)
GEN_COLOUR_NAME(CYAN, Pure, CyanShade, Pure)

// Magenta
GEN_COLOUR_NAME(MAGENTA, Pink, MagentaShade, Pink)
GEN_COLOUR_NAME(MAGENTA, Fuchsia, MagentaShade, Fuchsia)
GEN_COLOUR_NAME(MAGENTA, Deep, MagentaShade, Deep)
GEN_COLOUR_NAME(MAGENTA, Soft, MagentaShade, Soft)
GEN_COLOUR_NAME(MAGENTA, Orchid, MagentaShade, Orchid)
GEN_COLOUR_NAME(MAGENTA, Pale, MagentaShade, Pale)
GEN_COLOUR_NAME(MAGENTA, Dark, MagentaShade, Dark)
GEN_COLOUR_NAME(MAGENTA, Neon, MagentaShade, Neon)
GEN_COLOUR_NAME(MAGENTA, Electric, MagentaShade, Electric)
GEN_COLOUR_NAME(MAGENTA, Rose, MagentaShade, Rose)
GEN_COLOUR_NAME(MAGENTA, Sharp, MagentaShade, Sharp)
GEN_COLOUR_NAME(MAGENTA, Pure, MagentaShade, Pure)

// Yellow
GEN_COLOUR_NAME(YELLOW, Gold, YellowShade, Gold)
GEN_COLOUR_NAME(YELLOW, Amber, YellowShade, Amber)
GEN_COLOUR_NAME(YELLOW, Lemon, YellowShade, Lemon)
GEN_COLOUR_NAME(YELLOW, Pale, YellowShade, Pale)
GEN_COLOUR_NAME(YELLOW, Soft, YellowShade, Soft)
GEN_COLOUR_NAME(YELLOW, Sand, YellowShade, Sand)
GEN_COLOUR_NAME(YELLOW, Dark, YellowShade, Dark)
GEN_COLOUR_NAME(YELLOW, Deep, YellowShade, Deep)
GEN_COLOUR_NAME(YELLOW, Neon, YellowShade, Neon)
GEN_COLOUR_NAME(YELLOW, Solar, YellowShade, Solar)
GEN_COLOUR_NAME(YELLOW, Bright, YellowShade, Bright)
GEN_COLOUR_NAME(YELLOW, Pure, YellowShade, Pure)

// Grey
GEN_COLOUR_NAME(GREY, Light, GreyShade, Light)
GEN_COLOUR_NAME(GREY, Soft, GreyShade, Soft)
GEN_COLOUR_NAME(GREY, Silver, GreyShade, Silver)
GEN_COLOUR_NAME(GREY, Concrete, GreyShade, Concrete)
GEN_COLOUR_NAME(GREY, Dark, GreyShade, Dark)
GEN_COLOUR_NAME(GREY, Charcoal, GreyShade, Charcoal)
GEN_COLOUR_NAME(GREY, Ash, GreyShade, Ash)
GEN_COLOUR_NAME(GREY, Steel, GreyShade, Steel)
GEN_COLOUR_NAME(GREY, Pale, GreyShade, Pale)
GEN_COLOUR_NAME(GREY, Deep, GreyShade, Deep)
GEN_COLOUR_NAME(GREY, Fog, GreyShade, Fog)
GEN_COLOUR_NAME(GREY, Smoke, GreyShade, Smoke)

// White
GEN_COLOUR_NAME(WHITE, Pure, WhiteShade, Pure)
GEN_COLOUR_NAME(WHITE, Snow, WhiteShade, Snow)
GEN_COLOUR_NAME(WHITE, Pearl, WhiteShade, Pearl)
GEN_COLOUR_NAME(WHITE, Ivory, WhiteShade, Ivory)
GEN_COLOUR_NAME(WHITE, Cream, WhiteShade, Cream)
GEN_COLOUR_NAME(WHITE, Frost, WhiteShade, Frost)
GEN_COLOUR_NAME(WHITE, Ice, WhiteShade, Ice)
GEN_COLOUR_NAME(WHITE, Soft, WhiteShade, Soft)
GEN_COLOUR_NAME(WHITE, Pale, WhiteShade, Pale)
GEN_COLOUR_NAME(WHITE, Cold, WhiteShade, Cold)
GEN_COLOUR_NAME(WHITE, Bright, WhiteShade, Bright)
GEN_COLOUR_NAME(WHITE, Glare, WhiteShade, Glare)

#undef GEN_COLOUR_NAME

} // namespace ggems::render
