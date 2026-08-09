#pragma once
// ************************************************************************
// ************************************************************************


#include <cstdint>

#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::render {
enum class GreenShade : std::uint8_t {
  Acid = 12,
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
  Abyss = 12,
  Azure = 9,
  Dodger = 8,
  Ice = 5,
  Deep = 11,
  Gunmetal = 7,
  Navy = 10,
  Pale = 3,
  Steel = 4,
  Soft = 2,
  Dark = 6,
  Royal = 1,
  Vibrant = 0
};

enum class RedShade : std::uint8_t {
  XenoBlood = 12,
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
  Cryo = 12,
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
  FleshSignal = 12,
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
  MotherAmber = 12,
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

enum class GrayShade : std::uint8_t {
  Void = 12,
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
  Bone = 12,
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

template <typename ShadeEnum>
consteval auto FamilyOf(ShadeEnum) -> ColorFamily;

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

template <typename ShadeEnum>
consteval auto DefineColor(ShadeEnum shade,
                            ColorVariant variant = ColorVariant::Normal,
                            ColorLayer layer = ColorLayer::Foreground)
    -> ColorKey {
  return MakeColor(FamilyOf(shade), static_cast<std::uint8_t>(shade), variant,
                    layer);
}

inline constexpr ColorKey DEFAULT_BG =
    MakeColor(ColorFamily::Gray, static_cast<std::uint8_t>(GrayShade::Steel),
               ColorVariant::Normal, ColorLayer::Background);

inline constexpr ColorKey DEFAULT_FG = MakeColor(
    ColorFamily::White, static_cast<std::uint8_t>(WhiteShade::Ivory),
    ColorVariant::Normal, ColorLayer::Foreground);

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
