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
 * \brief Defines GGEMS color families, palettes, and ANSI rendering helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iterator>
#include <string>
#include <string_view>
/// \endcond

namespace ggems::render {

/*!
 * \brief Stores one RGB color.
 */
struct RGB {
  std::uint8_t red;   /*!< Red channel in the range [0, 255]. */
  std::uint8_t green; /*!< Green channel in the range [0, 255]. */
  std::uint8_t blue;  /*!< Blue channel in the range [0, 255]. */
};

/*!
 * \enum ColorFamily
 * \brief Identifies a GGEMS color family.
 */
enum class ColorFamily : std::uint8_t {
  Gray = 0, /*!< Gray family. */
  Red,      /*!< Red family. */
  Orange,   /*!< Orange family. */
  Yellow,   /*!< Yellow family. */
  Green,    /*!< Green family. */
  Cyan,     /*!< Cyan family. */
  Blue,     /*!< Blue family. */
  Magenta,  /*!< Magenta family. */
  White,    /*!< White family. */
  Count     /*!< Number of color families. */
};

/*!
 * \enum ColorVariant
 * \brief Selects a visual variant derived from one base shade.
 */
enum class ColorVariant : std::uint8_t {
  Normal = 0, /*!< Base color variant. */
  Bright,     /*!< Brightened color variant. */
  Faint,      /*!< Dimmed color variant. */
  Count       /*!< Number of color variants. */
};

/*!
 * \enum ColorLayer
 * \brief Selects whether a color is applied to foreground or background.
 */
enum class ColorLayer : std::uint8_t {
  Foreground = 0, /*!< Foreground text color. */
  Background      /*!< Background color. */
};

/*!
 * \brief Identifies one GGEMS color entry.
 *
 * Combines family, shade, variant, and layer into one compact key.
 */
struct ColorKey {
  ColorFamily family{}; /*!< Color family. */
  std::uint8_t shade{}; /*!< Shade index in the selected family. */
  ColorVariant variant{ColorVariant::Normal}; /*!< Color variant. */
  ColorLayer layer{ColorLayer::Foreground};   /*!< Target color layer. */

  /*!
   * \brief Compares two color keys.
   *
   * \return Comparison result for the two color keys.
   */
  constexpr auto operator<=>(ColorKey const &) const = default;
};

/*! \brief Number of color families. */
inline constexpr std::size_t color_family_count =
    static_cast<std::size_t>(ColorFamily::Count);

/*! \brief Number of color variants. */
inline constexpr std::size_t color_variant_count =
    static_cast<std::size_t>(ColorVariant::Count);

/*! \brief Number of shades per family. */
inline constexpr std::size_t color_shade_count = 13U;

/*! \brief Palette storage for all GGEMS color families. */
using ColorFamilyPalette =
    std::array<std::array<RGB, color_shade_count>, color_family_count>;

/*!
 * \brief Builds one RGB triplet.
 *
 * \param[in] red Red channel.
 * \param[in] green Green channel.
 * \param[in] blue Blue channel.
 * \return RGB color.
 */
constexpr auto MakeRGB(std::uint8_t red, std::uint8_t green,
                       std::uint8_t blue) noexcept -> RGB {
  return RGB{.red = red, .green = green, .blue = blue};
}

/*!
 * \brief Builds the gray family palette.
 *
 * \return Gray scale palette.
 */
constexpr auto MakeGrayScale() noexcept -> std::array<RGB, color_shade_count> {
  return {
      MakeRGB(16, 16, 16),    MakeRGB(32, 32, 32),    MakeRGB(48, 48, 48),
      MakeRGB(64, 64, 64),    MakeRGB(96, 96, 96),    MakeRGB(128, 128, 128),
      MakeRGB(160, 160, 160), MakeRGB(192, 192, 192), MakeRGB(208, 208, 208),
      MakeRGB(224, 224, 224), MakeRGB(240, 240, 240), MakeRGB(252, 252, 252),
      MakeRGB(3, 3, 3)};
}

/*!
 * \brief Builds the red family palette.
 *
 * \return Red scale palette.
 */
constexpr auto MakeRedScale() noexcept -> std::array<RGB, color_shade_count> {
  return {MakeRGB(64, 0, 0),     MakeRGB(96, 0, 0),      MakeRGB(128, 0, 0),
          MakeRGB(160, 0, 0),    MakeRGB(192, 16, 16),   MakeRGB(220, 20, 60),
          MakeRGB(255, 40, 40),  MakeRGB(255, 80, 80),   MakeRGB(255, 99, 71),
          MakeRGB(255, 127, 80), MakeRGB(255, 160, 122), MakeRGB(255, 192, 160),
          MakeRGB(170, 45, 40)};
}

/*!
 * \brief Builds the orange family palette.
 *
 * \return Orange scale palette.
 */
constexpr auto MakeOrangeScale() noexcept
    -> std::array<RGB, color_shade_count> {
  return {MakeRGB(80, 32, 0),    MakeRGB(96, 40, 0),    MakeRGB(128, 64, 0),
          MakeRGB(160, 80, 0),   MakeRGB(192, 96, 0),   MakeRGB(210, 105, 30),
          MakeRGB(255, 127, 80), MakeRGB(255, 140, 0),  MakeRGB(255, 165, 0),
          MakeRGB(255, 180, 40), MakeRGB(255, 200, 80), MakeRGB(255, 215, 120),
          MakeRGB(180, 88, 38)};
}

/*!
 * \brief Builds the yellow family palette.
 *
 * \return Yellow scale palette.
 */
constexpr auto MakeYellowScale() noexcept
    -> std::array<RGB, color_shade_count> {
  return {
      MakeRGB(96, 96, 0),     MakeRGB(128, 128, 0),   MakeRGB(160, 144, 0),
      MakeRGB(192, 160, 0),   MakeRGB(210, 180, 0),   MakeRGB(238, 221, 130),
      MakeRGB(240, 230, 140), MakeRGB(250, 250, 120), MakeRGB(255, 255, 0),
      MakeRGB(255, 255, 80),  MakeRGB(255, 255, 160), MakeRGB(255, 255, 220),
      MakeRGB(190, 145, 45)};
}

/*!
 * \brief Builds the green family palette.
 *
 * \return Green scale palette.
 */
constexpr auto MakeGreenScale() noexcept -> std::array<RGB, color_shade_count> {
  return {MakeRGB(0, 48, 0),    MakeRGB(0, 80, 0),      MakeRGB(0, 100, 0),
          MakeRGB(0, 128, 0),   MakeRGB(0, 160, 64),    MakeRGB(0, 201, 87),
          MakeRGB(0, 255, 0),   MakeRGB(60, 255, 120),  MakeRGB(0, 255, 170),
          MakeRGB(46, 139, 87), MakeRGB(144, 238, 144), MakeRGB(204, 255, 204),
          MakeRGB(48, 220, 160)};
}

/*!
 * \brief Builds the cyan family palette.
 *
 * \return Cyan scale palette.
 */
constexpr auto MakeCyanScale() noexcept -> std::array<RGB, color_shade_count> {
  return {
      MakeRGB(0, 48, 48),     MakeRGB(0, 80, 80),     MakeRGB(0, 100, 100),
      MakeRGB(0, 128, 128),   MakeRGB(0, 160, 160),   MakeRGB(0, 183, 235),
      MakeRGB(0, 200, 255),   MakeRGB(0, 255, 255),   MakeRGB(80, 255, 255),
      MakeRGB(135, 206, 235), MakeRGB(180, 230, 255), MakeRGB(210, 245, 255),
      MakeRGB(58, 178, 190)};
}

/*!
 * \brief Builds the blue family palette.
 *
 * \return Blue scale palette.
 */
constexpr auto MakeBlueScale() noexcept -> std::array<RGB, color_shade_count> {
  return {MakeRGB(0, 0, 64),      MakeRGB(0, 0, 96),     MakeRGB(0, 0, 139),
          MakeRGB(25, 25, 112),   MakeRGB(0, 71, 171),   MakeRGB(30, 144, 255),
          MakeRGB(0, 127, 255),   MakeRGB(30, 38, 46),   MakeRGB(80, 120, 255),
          MakeRGB(125, 249, 255), MakeRGB(70, 130, 180), MakeRGB(160, 200, 255),
          MakeRGB(3, 5, 7)};
}

/*!
 * \brief Builds the magenta family palette.
 *
 * \return Magenta scale palette.
 */
constexpr auto MakeMagentaScale() noexcept
    -> std::array<RGB, color_shade_count> {
  return {
      MakeRGB(64, 0, 64),     MakeRGB(96, 0, 96),     MakeRGB(128, 0, 128),
      MakeRGB(139, 0, 139),   MakeRGB(186, 85, 211),  MakeRGB(199, 21, 133),
      MakeRGB(255, 0, 255),   MakeRGB(255, 30, 100),  MakeRGB(255, 105, 180),
      MakeRGB(238, 130, 238), MakeRGB(255, 160, 255), MakeRGB(255, 200, 240),
      MakeRGB(150, 62, 92)};
}

/*!
 * \brief Builds the white family palette.
 *
 * \return White scale palette.
 */
constexpr auto MakeWhiteScale() noexcept -> std::array<RGB, color_shade_count> {
  return {
      MakeRGB(255, 255, 255), MakeRGB(250, 250, 250), MakeRGB(245, 245, 245),
      MakeRGB(240, 240, 235), MakeRGB(235, 232, 220), MakeRGB(230, 230, 230),
      MakeRGB(220, 220, 220), MakeRGB(210, 210, 210), MakeRGB(200, 200, 200),
      MakeRGB(185, 185, 185), MakeRGB(255, 255, 240), MakeRGB(255, 255, 200),
      MakeRGB(218, 214, 196)};
}

/*!
 * \brief Builds the full GGEMS base palette.
 *
 * \return Palette containing every color family.
 */
constexpr auto MakeBasePalette() noexcept -> ColorFamilyPalette {
  return ColorFamilyPalette{
      MakeGrayScale(),   MakeRedScale(),     MakeOrangeScale(),
      MakeYellowScale(), MakeGreenScale(),   MakeCyanScale(),
      MakeBlueScale(),   MakeMagentaScale(), MakeWhiteScale()};
}

/*! \brief Base palette for all GGEMS colors. */
inline constexpr ColorFamilyPalette base_palette = MakeBasePalette();

/*!
 * \brief Brightens one color channel.
 *
 * \param[in] color Base channel value.
 * \return Brightened channel value.
 */
constexpr auto BrightenChannel(std::uint8_t color) noexcept -> std::uint8_t {
  return static_cast<std::uint8_t>(color + ((255U - color) / 3U));
}

/*!
 * \brief Dims one color channel.
 *
 * \param[in] color Base channel value.
 * \return Faint channel value.
 */
constexpr auto FaintChannel(std::uint8_t color) noexcept -> std::uint8_t {
  return static_cast<std::uint8_t>((static_cast<std::uint16_t>(color) * 2U) /
                                   3U);
}

/*!
 * \brief Applies a color variant to one base RGB color.
 *
 * \param[in] base Base color.
 * \param[in] variant Variant to apply.
 * \return Variant-adjusted color.
 */
constexpr auto ApplyVariant(RGB base, ColorVariant variant) noexcept -> RGB {
  switch (variant) {
  case ColorVariant::Normal:
    return base;
  case ColorVariant::Bright:
    return RGB{.red = BrightenChannel(base.red),
               .green = BrightenChannel(base.green),
               .blue = BrightenChannel(base.blue)};
  case ColorVariant::Faint:
    return RGB{.red = FaintChannel(base.red),
               .green = FaintChannel(base.green),
               .blue = FaintChannel(base.blue)};
  default:
    return base;
  }
}

/*!
 * \brief Retrieves one RGB color from the GGEMS palette.
 *
 * \param[in] family Color family.
 * \param[in] shade Shade index in the selected family.
 * \param[in] variant Variant to apply.
 * \return RGB color associated with the key.
 */
constexpr auto GetColorRGB(ColorFamily family, std::uint8_t shade,
                           ColorVariant variant) noexcept -> RGB {
  auto const family_index = static_cast<std::size_t>(family);
  auto const shade_index = static_cast<std::size_t>(
      std::min<std::uint8_t>(shade, color_shade_count - 1U));

  RGB const base = base_palette[family_index][shade_index];
  return ApplyVariant(base, variant);
}

/*!
 * \enum AnsiControl
 * \brief Identifies one ANSI text-control sequence.
 */
enum class AnsiControl : std::uint8_t {
  ResetAll,   /*!< Resets all terminal styling. */
  ResetColor, /*!< Resets only color styling. */
  Bold,       /*!< Enables bold styling. */
  Faint       /*!< Enables faint styling. */
};

/*!
 * \brief Returns the ANSI escape sequence for one control code.
 *
 * \param[in] control Control code to encode.
 * \return ANSI escape sequence.
 */
inline auto AnsiControlCode(AnsiControl control) noexcept -> std::string_view {
  switch (control) {
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

/*!
 * \brief Appends one ANSI color escape sequence to a string.
 *
 * \param[in,out] out Destination string.
 * \param[in] key Color key to encode.
 */
inline auto AppendAnsiColor(std::string &out, ColorKey const &key) -> void {
  RGB const rgb = GetColorRGB(key.family, key.shade, key.variant);

  int const code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  std::format_to(std::back_inserter(out), "\033[{};2;{};{};{}m", code,
                 static_cast<unsigned int>(rgb.red),
                 static_cast<unsigned int>(rgb.green),
                 static_cast<unsigned int>(rgb.blue));
}

/*!
 * \brief Builds one ANSI color escape sequence.
 *
 * \param[in] key Color key to encode.
 * \return ANSI escape sequence.
 */
inline auto AnsiColor(ColorKey const &key) -> std::string {
  RGB const rgb = GetColorRGB(key.family, key.shade, key.variant);

  int const code = (key.layer == ColorLayer::Foreground) ? 38 : 48;

  return std::format(
      "\033[{};2;{};{};{}m", code, static_cast<unsigned>(rgb.red),
      static_cast<unsigned>(rgb.green), static_cast<unsigned>(rgb.blue));
}

/*!
 * \brief Appends one ANSI control escape sequence to a string.
 *
 * \param[in,out] out Destination string.
 * \param[in] control Control code to append.
 */
inline auto AppendAnsiControl(std::string &out, AnsiControl control) -> void {
  out.append(AnsiControlCode(control));
}

/*!
 * \brief Builds one GGEMS color key.
 *
 * \param[in] family Color family.
 * \param[in] shade Shade index.
 * \param[in] variant Color variant.
 * \param[in] layer Target color layer.
 * \return Constructed color key.
 */
constexpr auto MakeColor(ColorFamily family, std::uint8_t shade,
                         ColorVariant variant = ColorVariant::Normal,
                         ColorLayer layer = ColorLayer::Foreground) noexcept
    -> ColorKey {
  return ColorKey{
      .family = family, .shade = shade, .variant = variant, .layer = layer};
}
} // namespace ggems::render
