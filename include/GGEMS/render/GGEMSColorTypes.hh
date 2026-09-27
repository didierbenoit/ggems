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
#include <cstddef>
#include <cstdint>
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
  Count,    /*!< Number of color families. */
};

/*!
 * \enum ColorVariant
 * \brief Selects a visual variant derived from one base shade.
 */
enum class ColorVariant : std::uint8_t {
  Normal = 0, /*!< Base color variant. */
  Bright,     /*!< Brightened color variant. */
  Faint,      /*!< Dimmed color variant. */
  Count,      /*!< Number of color variants. */
};

/*!
 * \enum ColorLayer
 * \brief Selects whether a color is applied to foreground or background.
 */
enum class ColorLayer : std::uint8_t {
  Foreground = 0, /*!< Foreground text color. */
  Background,     /*!< Background color. */
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

} // namespace ggems::render
