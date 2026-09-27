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
 * \brief Defines palette indices, RGB values, and typed display-color keys.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
/// \endcond

/*!
 * \namespace ggems::render
 * \brief Provides shared color, text-layout, and diagnostic trace data.
 *
 * The palette contains nine families of 64 shades. A shade index selects a row,
 * not a uniform brightness step. Normal, Bright, and Faint variants transform
 * that RGB row; foreground/background layers select the ANSI output target.
 */
namespace ggems::render {

/*! \brief Stores one RGB color. */
struct RGB {
  /*! \brief Red channel in the range [0, 255]. */
  std::uint8_t red;

  /*! \brief Green channel in the range [0, 255]. */
  std::uint8_t green;

  /*! \brief Blue channel in the range [0, 255]. */
  std::uint8_t blue;
};

/*! \brief Identifies a GGEMS color family. */
enum class ColorFamily : std::uint8_t {
  /*! \brief Gray family. */
  Gray = 0,

  /*! \brief Red family. */
  Red,

  /*! \brief Orange family. */
  Orange,

  /*! \brief Yellow family. */
  Yellow,

  /*! \brief Green family. */
  Green,

  /*! \brief Cyan family. */
  Cyan,

  /*! \brief Blue family. */
  Blue,

  /*! \brief Magenta family. */
  Magenta,

  /*! \brief White family. */
  White,

  /*! \brief Number of color families. */
  Count,
};

/*! \brief Selects a visual variant derived from one base shade. */
enum class ColorVariant : std::uint8_t {
  /*! \brief Base color variant. */
  Normal = 0,

  /*! \brief Brightened color variant. */
  Bright,

  /*! \brief Dimmed color variant. */
  Faint,

  /*! \brief Number of color variants. */
  Count,
};

/*! \brief Selects whether a color is applied to foreground or background. */
enum class ColorLayer : std::uint8_t {
  /*! \brief Foreground text color. */
  Foreground = 0,

  /*! \brief Background color. */
  Background,
};

/*!
 * \brief Identifies one GGEMS color entry.
 *
 * Combines family, shade, variant, and layer into one compact key.
 */
struct ColorKey {
  /*! \brief Color family. */
  ColorFamily family{};

  /*! \brief Shade index in the selected family. */
  std::uint8_t shade{};

  /*! \brief Color variant. */
  ColorVariant variant{ColorVariant::Normal};

  /*! \brief Target color layer. */
  ColorLayer layer{ColorLayer::Foreground};

  /*!
   * \brief Compares two color keys.
   *
   * \return Lexicographic order by family, shade, variant, then layer.
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
inline constexpr std::size_t color_shade_count = 64U;

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
 * \brief Builds one GGEMS color key.
 *
 * \param[in] family Color family.
 * \param[in] shade Stored shade index; no clamping occurs in this factory.
 * \param[in] variant Color variant.
 * \param[in] layer Target color layer.
 * \return Constructed color key.
 */
constexpr auto MakeColor(ColorFamily family, std::uint8_t shade,
                         ColorVariant variant = ColorVariant::Normal,
                         ColorLayer layer = ColorLayer::Foreground) noexcept
  -> ColorKey {
  return ColorKey{
    .family = family,
    .shade = shade,
    .variant = variant,
    .layer = layer,
  };
}

/// \cond
template <typename ShadeEnum> consteval auto FamilyOf(ShadeEnum) -> ColorFamily;
/// \endcond

/*!
 * \brief Builds one named GGEMS color key.
 *
 * \tparam ShadeEnum One of the nine palette shade enumeration types.
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

} // namespace ggems::render
