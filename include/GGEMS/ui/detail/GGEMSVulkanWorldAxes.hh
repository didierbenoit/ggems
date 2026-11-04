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
 * \brief Declares the focused procedural World-reference renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <span>

#include <vulkan/vulkan_raii.hpp>

namespace ggems::ui::detail {

/*!
 * \brief Owns one depth-tested pipeline for three arrows and the origin sphere.
 *
 * The application finishes GPU use before shutdown or destruction. Shader
 * modules exist only during initialization; all vertices are generated in
 * Slang.
 */
class GGEMSVulkanWorldAxes {
public:
  /*! \brief Creates an inactive reference renderer. */
  GGEMSVulkanWorldAxes() = default;

  /*! \brief Releases the owned pipeline and layout. */
  ~GGEMSVulkanWorldAxes() = default;

  /*! \brief Disallows copying Vulkan resources. */
  GGEMSVulkanWorldAxes(GGEMSVulkanWorldAxes const &) = delete;

  /*! \brief Disallows moving Vulkan resources. */
  GGEMSVulkanWorldAxes(GGEMSVulkanWorldAxes &&) = delete;

  /*! \brief Disallows copy assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanWorldAxes const &)
    -> GGEMSVulkanWorldAxes & = delete;

  /*! \brief Disallows move assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanWorldAxes &&) -> GGEMSVulkanWorldAxes & = delete;

  /*!
   * \brief Creates the pipeline and resolves its four palette colors once.
   *
   * \param[in] device Device that must outlive the owned resources.
   * \param[in] color_format Existing offscreen color attachment format.
   * \param[in] depth_format Existing scene depth attachment format.
   */
  auto Initialize(vk::raii::Device const &device, vk::Format color_format,
                  vk::Format depth_format) -> void;

  /*! \brief Releases the pipeline and layout after GPU use has completed. */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Records seven procedural draws inside the existing scene pass.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] extent Current nonzero scene viewport extent.
   * \param[in] world_to_clip Rows mapping World meters to clip coordinates.
   * \param[in] world_half_extent_m Positive half extents of the displayed
   *   World.
   */
  auto Record(vk::raii::CommandBuffer const &command_buffer,
              vk::Extent2D const &extent,
              std::array<std::array<float, 4U>, 4U> const &world_to_clip,
              std::array<double, 3U> const &world_half_extent_m) const -> void;

private:
  /*! \brief Layout of the 104-byte vertex-stage push constants. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Triangle-list pipeline with no vertex-input bindings. */
  vk::raii::Pipeline pipeline_{nullptr};

  /*! \brief Linear palette colors for X, Y, Z and the origin sphere. */
  std::array<std::array<float, 4U>, 4U> colors_{};
};

} // namespace ggems::ui::detail
