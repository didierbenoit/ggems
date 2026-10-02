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
 * \brief Converts GGEMS palette colors to Vulkan clear colors.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::ui::detail {

/*!
 * \brief Converts a palette color to opaque linear RGBA.
 *
 * \param[in] color Palette family, shade, and variant.
 * \return Linear RGB channels with alpha equal to one.
 */
[[nodiscard]] inline auto
ToVulkanClearColor(render::ColorKey const &color) noexcept
  -> std::array<float, 4U> {
  render::RGB const rgb =
    render::GetColorRGB(color.family, color.shade, color.variant);

  return {
    render::SRGBChannelToLinear(rgb.red),
    render::SRGBChannelToLinear(rgb.green),
    render::SRGBChannelToLinear(rgb.blue),
    1.0F,
  };
}
} // namespace ggems::ui::detail
