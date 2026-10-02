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
 * \brief Converts UI scaling and logical extents to framebuffer sizes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ggems::ui::detail {

/*! \brief Default user multiplier for UI metrics and fonts. */
inline constexpr float k_default_user_ui_scale{1.0F};

/*! \brief Minimum admitted window content scale. */
inline constexpr float k_min_content_scale{1.0F};

/*! \brief Maximum admitted window content scale. */
inline constexpr float k_max_content_scale{2.5F};

/*! \brief Stores an image extent in the caller-selected pixel space. */
struct GGEMSPixelExtent {
  /*! \brief Horizontal extent in pixels. */
  std::uint32_t width{0U};

  /*! \brief Vertical extent in pixels. */
  std::uint32_t height{0U};

  /*!
   * \brief Compares both pixel dimensions.
   *
   * \return True when both pixel dimensions are equal.
   */
  [[nodiscard]] auto operator==(GGEMSPixelExtent const &) const noexcept
    -> bool = default;
};

/*!
 * \brief Clamps content scale to the supported UI range.
 *
 * \param[in] content_scale Finite window content scale to clamp.
 * \return Scale clamped to [1.0, 2.5].
 */
[[nodiscard]] constexpr auto NormalizeContentScale(float content_scale) noexcept
  -> float {
  return std::clamp(content_scale, k_min_content_scale, k_max_content_scale);
}

/*!
 * \brief Converts logical dimensions to physical scene pixels.
 *
 * Densities and products must be finite and representable in the target types.
 *
 * \param[in] logical_extent Viewport size in logical UI pixels.
 * \param[in] density_x Physical pixels per logical pixel on the horizontal
 *   axis.
 * \param[in] density_y Physical pixels per logical pixel on the vertical axis.
 * \return Rounded physical extent, with each axis at least one pixel.
 */
[[nodiscard]] inline auto
ComputeSceneTargetExtent(GGEMSPixelExtent const &logical_extent,
                         float density_x, float density_y) noexcept
  -> GGEMSPixelExtent {
  auto const ScaleAxis = [](std::uint32_t logical,
                            float density) noexcept -> std::uint32_t {
    long const pixels =
      std::lround(static_cast<double>(logical) * static_cast<double>(density));
    return static_cast<std::uint32_t>(std::max(pixels, 1L));
  };

  return GGEMSPixelExtent{
    .width = ScaleAxis(logical_extent.width, density_x),
    .height = ScaleAxis(logical_extent.height, density_y),
  };
}

} // namespace ggems::ui::detail
