#pragma once
// ************************************************************************
// ************************************************************************


#include <array>
#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <cstddef>

namespace ggems::render {
struct RGB {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

enum class ColorFamily : std::uint8_t {
  Gray = 0,
  Red,
  Orange,
  Yellow,
  Green,
  Cyan,
  Blue,
  Magenta,
  White,
  Count
};

enum class ColorVariant : std::uint8_t {
  Normal = 0,
  Bright,
  Faint,
  Count
};

enum class ColorLayer : std::uint8_t {
  Foreground = 0,
  Background
};

struct ColorKey {
  ColorFamily family{};
  std::uint8_t shade{};
  ColorVariant variant{ColorVariant::Normal};
  ColorLayer layer{ColorLayer::Foreground};

  constexpr auto operator<=>(ColorKey const &) const = default;
};

inline constexpr std::size_t kColorFamilyCount =
    static_cast<std::size_t>(ColorFamily::Count);

inline constexpr std::size_t kColorVariantCount =
    static_cast<std::size_t>(ColorVariant::Count);

inline constexpr std::size_t kColorShadeCount = 13U;

using ColorFamilyPalette =
    std::array<std::array<RGB, kColorShadeCount>, kColorFamilyCount>;

constexpr RGB MakeRGB(std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
  return RGB{r, g, b};
}

constexpr std::array<RGB, kColorShadeCount> MakeGrayScale() noexcept {
  return {
      MakeRGB(16, 16, 16),    MakeRGB(32, 32, 32),    MakeRGB(48, 48, 48),
      MakeRGB(64, 64, 64),    MakeRGB(96, 96, 96),    MakeRGB(128, 128, 128),
      MakeRGB(160, 160, 160), MakeRGB(192, 192, 192), MakeRGB(208, 208, 208),
      MakeRGB(224, 224, 224), MakeRGB(240, 240, 240), MakeRGB(252, 252, 252),
      MakeRGB(3, 3, 3)};
}

constexpr std::array<RGB, kColorShadeCount> MakeRedScale() noexcept {
  return {MakeRGB(64, 0, 0),     MakeRGB(96, 0, 0),      MakeRGB(128, 0, 0),
          MakeRGB(160, 0, 0),    MakeRGB(192, 16, 16),   MakeRGB(220, 20, 60),
          MakeRGB(255, 40, 40),  MakeRGB(255, 80, 80),   MakeRGB(255, 99, 71),
          MakeRGB(255, 127, 80), MakeRGB(255, 160, 122), MakeRGB(255, 192, 160),
          MakeRGB(170, 45, 40)};
}

constexpr std::array<RGB, kColorShadeCount> MakeOrangeScale() noexcept {
  return {MakeRGB(80, 32, 0),    MakeRGB(96, 40, 0),    MakeRGB(128, 64, 0),
          MakeRGB(160, 80, 0),   MakeRGB(192, 96, 0),   MakeRGB(210, 105, 30),
          MakeRGB(255, 127, 80), MakeRGB(255, 140, 0),  MakeRGB(255, 165, 0),
          MakeRGB(255, 180, 40), MakeRGB(255, 200, 80), MakeRGB(255, 215, 120),
          MakeRGB(180, 88, 38)};
}

constexpr std::array<RGB, kColorShadeCount> MakeYellowScale() noexcept {
  return {
      MakeRGB(96, 96, 0),     MakeRGB(128, 128, 0),   MakeRGB(160, 144, 0),
      MakeRGB(192, 160, 0),   MakeRGB(210, 180, 0),   MakeRGB(238, 221, 130),
      MakeRGB(240, 230, 140), MakeRGB(250, 250, 120), MakeRGB(255, 255, 0),
      MakeRGB(255, 255, 80),  MakeRGB(255, 255, 160), MakeRGB(255, 255, 220),
      MakeRGB(190, 145, 45)};
}

constexpr std::array<RGB, kColorShadeCount> MakeGreenScale() noexcept {
  return {MakeRGB(0, 48, 0),    MakeRGB(0, 80, 0),      MakeRGB(0, 100, 0),
          MakeRGB(0, 128, 0),   MakeRGB(0, 160, 64),    MakeRGB(0, 201, 87),
          MakeRGB(0, 255, 0),   MakeRGB(60, 255, 120),  MakeRGB(0, 255, 170),
          MakeRGB(46, 139, 87), MakeRGB(144, 238, 144), MakeRGB(204, 255, 204),
          MakeRGB(48, 220, 160)};
}

constexpr std::array<RGB, kColorShadeCount> MakeCyanScale() noexcept {
  return {
      MakeRGB(0, 48, 48),     MakeRGB(0, 80, 80),     MakeRGB(0, 100, 100),
      MakeRGB(0, 128, 128),   MakeRGB(0, 160, 160),   MakeRGB(0, 183, 235),
      MakeRGB(0, 200, 255),   MakeRGB(0, 255, 255),   MakeRGB(80, 255, 255),
      MakeRGB(135, 206, 235), MakeRGB(180, 230, 255), MakeRGB(210, 245, 255),
      MakeRGB(58, 178, 190)};
}

constexpr std::array<RGB, kColorShadeCount> MakeBlueScale() noexcept {
  return {MakeRGB(0, 0, 64),      MakeRGB(0, 0, 96),     MakeRGB(0, 0, 139),
          MakeRGB(25, 25, 112),   MakeRGB(0, 71, 171),   MakeRGB(30, 144, 255),
          MakeRGB(0, 127, 255),   MakeRGB(30, 38, 46),   MakeRGB(80, 120, 255),
          MakeRGB(125, 249, 255), MakeRGB(70, 130, 180), MakeRGB(160, 200, 255),
          MakeRGB(3, 5, 7)};
}

constexpr std::array<RGB, kColorShadeCount> MakeMagentaScale() noexcept {
  return {
      MakeRGB(64, 0, 64),     MakeRGB(96, 0, 96),     MakeRGB(128, 0, 128),
      MakeRGB(139, 0, 139),   MakeRGB(186, 85, 211),  MakeRGB(199, 21, 133),
      MakeRGB(255, 0, 255),   MakeRGB(255, 30, 100),  MakeRGB(255, 105, 180),
      MakeRGB(238, 130, 238), MakeRGB(255, 160, 255), MakeRGB(255, 200, 240),
      MakeRGB(150, 62, 92)};
}

constexpr std::array<RGB, kColorShadeCount> MakeWhiteScale() noexcept {
  return {
      MakeRGB(255, 255, 255), MakeRGB(250, 250, 250), MakeRGB(245, 245, 245),
      MakeRGB(240, 240, 235), MakeRGB(235, 232, 220), MakeRGB(230, 230, 230),
      MakeRGB(220, 220, 220), MakeRGB(210, 210, 210), MakeRGB(200, 200, 200),
      MakeRGB(185, 185, 185), MakeRGB(255, 255, 240), MakeRGB(255, 255, 200),
      MakeRGB(218, 214, 196)};
}

constexpr ColorFamilyPalette MakeBasePalette() noexcept {
  return ColorFamilyPalette{
      MakeGrayScale(),   MakeRedScale(),     MakeOrangeScale(),
      MakeYellowScale(), MakeGreenScale(),   MakeCyanScale(),
      MakeBlueScale(),   MakeMagentaScale(), MakeWhiteScale()};
}

inline constexpr ColorFamilyPalette kBasePalette = MakeBasePalette();

constexpr std::uint8_t BrightenChannel(std::uint8_t c) noexcept {
  return static_cast<std::uint8_t>(c + (255U - c) / 3U);
}

constexpr std::uint8_t FaintChannel(std::uint8_t c) noexcept {
  return static_cast<std::uint8_t>((static_cast<std::uint16_t>(c) * 2U) / 3U);
}

constexpr RGB ApplyVariant(RGB base, ColorVariant v) noexcept {
  switch (v) {
  case ColorVariant::Normal:
    return base;
  case ColorVariant::Bright:
    return RGB{BrightenChannel(base.r), BrightenChannel(base.g),
               BrightenChannel(base.b)};
  case ColorVariant::Faint:
    return RGB{FaintChannel(base.r), FaintChannel(base.g),
               FaintChannel(base.b)};
  default:
    return base;
  }
}

constexpr RGB GetColorRGB(ColorFamily family, std::uint8_t shade,
                           ColorVariant variant) noexcept {
  const auto family_index = static_cast<std::size_t>(family);
  const auto shade_index = static_cast<std::size_t>(
      std::min<std::uint8_t>(shade, kColorShadeCount - 1U));

  const RGB base = kBasePalette[family_index][shade_index];
  return ApplyVariant(base, variant);
}

enum class AnsiControl : std::uint8_t {
  ResetAll,
  ResetColor,
  Bold,
  Faint
};

inline std::string_view AnsiControlCode(AnsiControl c) noexcept {
  switch (c) {
  case AnsiControl::ResetAll:
    return "\033[0m";
  case AnsiControl::ResetColor:
    return "\033[39;49m";
  case AnsiControl::Bold:
    return "\033[1m";
  case AnsiControl::Faint:
    return "\033[2m";
  default:
    return "\033[0m";
  }
}

inline void AppendAnsiColor(std::string &out, ColorKey const &key) {
  const RGB rgb = GetColorRGB(key.family, key.shade, key.variant);

  const int code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  std::format_to(std::back_inserter(out), "\033[{};2;{};{};{}m", code,
                 static_cast<unsigned int>(rgb.r),
                 static_cast<unsigned int>(rgb.g),
                 static_cast<unsigned int>(rgb.b));
}

inline std::string AnsiColor(ColorKey const &key) {
  const RGB rgb = GetColorRGB(key.family, key.shade, key.variant);

  const int code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  // Retour direct : une seule allocation très légère
  return std::format("\033[{};2;{};{};{}m", code, static_cast<unsigned>(rgb.r),
                     static_cast<unsigned>(rgb.g),
                     static_cast<unsigned>(rgb.b));
}

inline void AppendAnsiControl(std::string &out, AnsiControl c) {
  out.append(AnsiControlCode(c));
}

inline std::string_view AnsiControl(AnsiControl c) {
  switch (c) {
  case AnsiControl::ResetAll:
    return "\033[0m";
  case AnsiControl::ResetColor:
    return "\033[39;49m";
  case AnsiControl::Bold:
    return "\033[1m";
  case AnsiControl::Faint:
    return "\033[2m";
  default:
    return "\033[0m";
  }
}

constexpr ColorKey
MakeColor(ColorFamily family, std::uint8_t shade,
           ColorVariant variant = ColorVariant::Normal,
           ColorLayer layer = ColorLayer::Foreground) noexcept {
  return ColorKey{family, shade, variant, layer};
}
} // namespace ggems::render
