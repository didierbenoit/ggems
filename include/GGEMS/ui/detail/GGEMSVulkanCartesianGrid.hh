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
 * \brief Declares the bounded procedural Cartesian grid renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"

namespace ggems::ui::detail {

/*!
 * \brief Owns a depth-tested procedural line pipeline without vertex buffers.
 *
 * The application completes GPU use before shutdown or destruction. Grid
 * draws precede scene objects so equal-depth geometry remains authoritative.
 */
class GGEMSVulkanCartesianGrid {
public:
  /*! \brief Creates an inactive renderer. */
  GGEMSVulkanCartesianGrid() = default;

  /*! \brief Releases the owned pipeline and layout. */
  ~GGEMSVulkanCartesianGrid() = default;

  /*! \brief Disallows copying Vulkan resources. */
  GGEMSVulkanCartesianGrid(GGEMSVulkanCartesianGrid const &) = delete;

  /*! \brief Disallows moving Vulkan resources. */
  GGEMSVulkanCartesianGrid(GGEMSVulkanCartesianGrid &&) = delete;

  /*! \brief Disallows copy assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanCartesianGrid const &)
    -> GGEMSVulkanCartesianGrid & = delete;

  /*! \brief Disallows move assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanCartesianGrid &&)
    -> GGEMSVulkanCartesianGrid & = delete;

  /*!
   * \brief Creates the pipeline and resolves the existing grid palette roles.
   *
   * \param[in] device Device that outlives the owned resources.
   * \param[in] color_format Existing scene color attachment format.
   * \param[in] depth_format Existing scene depth attachment format.
   */
  auto Initialize(vk::raii::Device const &device, vk::Format color_format,
                  vk::Format depth_format) -> void;

  /*! \brief Releases resources after their GPU use has completed. */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Records at most six bounded procedural draws in the scene pass.
   *
   * The caller sets viewport and scissor. Invisible grids emit no commands.
   * Center lines use the major color and are shared between intersecting
   * planes. The existing World axes retain the emphasized origin marker.
   * A subpixel depth allowance gives coplanar objects stable priority.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] world_to_clip Shared transform from World meters to clip space.
   * \param[in] viewport_extent Physical render-target dimensions.
   * \param[in] grid Updated physical intervals and independent visibility.
   * \param[in] background Linear clear color used for grazing-angle fading.
   */
  auto Record(vk::raii::CommandBuffer const &command_buffer,
              std::array<std::array<float, 4U>, 4U> const &world_to_clip,
              vk::Extent2D viewport_extent, GGEMSCartesianGrid const &grid,
              std::array<float, 4U> const &background) const -> void;

private:
  /*! \brief Layout of the 128-byte vertex-stage push constants. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Depth-tested line-list pipeline with no vertex inputs. */
  vk::raii::Pipeline pipeline_{nullptr};

  /*! \brief Linear minor and major colors from existing semantic roles. */
  std::array<std::array<float, 4U>, 2U> colors_{};
};

} // namespace ggems::ui::detail
