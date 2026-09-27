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

#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorGray.hh"
#include "GGEMS/render/GGEMSColorRed.hh"
#include "GGEMS/render/GGEMSColorOrange.hh"
#include "GGEMS/render/GGEMSColorYellow.hh"
#include "GGEMS/render/GGEMSColorGreen.hh"
#include "GGEMS/render/GGEMSColorCyan.hh"
#include "GGEMS/render/GGEMSColorBlue.hh"
#include "GGEMS/render/GGEMSColorMagenta.hh"
#include "GGEMS/render/GGEMSColorWhite.hh"

namespace ggems::render {

/*!
 * \brief Builds the full GGEMS base palette.
 *
 * \return Palette containing every color family.
 */
constexpr auto MakeBasePalette() noexcept -> ColorFamilyPalette {
  return ColorFamilyPalette{
    MakeGrayScale(),   MakeRedScale(),     MakeOrangeScale(),
    MakeYellowScale(), MakeGreenScale(),   MakeCyanScale(),
    MakeBlueScale(),   MakeMagentaScale(), MakeWhiteScale(),
  };
}

/*! \brief Base palette for all GGEMS colors. */
inline constexpr ColorFamilyPalette base_palette = MakeBasePalette();

/*!
 * \brief Brightens one color channel.
 *
 * \param[in] color Base channel value.
 * \return color + floor((255 - color) / 3).
 */
constexpr auto BrightenChannel(std::uint8_t color) noexcept -> std::uint8_t {
  return static_cast<std::uint8_t>(color + ((255U - color) / 3U));
}

/*!
 * \brief Dims one color channel.
 *
 * \param[in] color Base channel value.
 * \return floor(2 * color / 3).
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
    return RGB{
      .red = BrightenChannel(base.red),
      .green = BrightenChannel(base.green),
      .blue = BrightenChannel(base.blue),
    };
  case ColorVariant::Faint:
    return RGB{
      .red = FaintChannel(base.red),
      .green = FaintChannel(base.green),
      .blue = FaintChannel(base.blue),
    };
  default:
    return base;
  }
}

/*!
 * \brief Retrieves one RGB color from the GGEMS palette.
 *
 * \param[in] family Valid palette family, excluding Count.
 * \param[in] shade Family row index; values above 63 are clamped to 63.
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

/*! \brief Identifies one ANSI text-control sequence. */
enum class AnsiControl : std::uint8_t {
  /*! \brief Resets all terminal styling. */
  ResetAll,

  /*! \brief Resets only color styling. */
  ResetColor,

  /*! \brief Enables bold styling. */
  Bold,

  /*! \brief Enables faint styling. */
  Faint,
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
 * \param[in] key Key with a valid palette family, excluding Count.
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
 * \param[in] key Key with a valid palette family, excluding Count.
 * \return ANSI escape sequence.
 */
inline auto AnsiColor(ColorKey const &key) -> std::string {
  std::string result;
  AppendAnsiColor(result, key);
  return result;
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

} // namespace ggems::render
