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
 * \brief Declares the procedural Cartesian and polar World-plane renderer.
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
 * \brief Owns one Cartesian/polar grid pipeline without vertex buffers.
 *
 * The application completes GPU use before shutdown or destruction. Grid
 * draws follow scene objects and blend only where the enabled World planes
 * are visible. Both patterns on one plane share a draw and coverage union.
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
   * \brief Records up to three World-plane rectangles in the scene pass.
   *
   * The caller sets viewport and scissor. Hidden grids emit no commands.
   * Slang owns the pattern, clipping, filtering, and subpixel depth allowance.
   * Read-only depth and alpha blending preserve antialiased line coverage.
   * The positive depth allowance gives coplanar scene objects display priority.
   * Different planes retain the existing XY, XZ, YZ blending order.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] world_to_clip Shared transform from World meters to clip space.
   * \param[in] viewport_extent Physical render-target dimensions.
   * \param[in] world_half_extent_m Positive World half extents in meters.
   * \param[in] grid Independent Cartesian/polar visibility and physical step.
   */
  auto Record(vk::raii::CommandBuffer const &command_buffer,
              std::array<std::array<float, 4U>, 4U> const &world_to_clip,
              vk::Extent2D viewport_extent,
              std::array<double, 3U> const &world_half_extent_m,
              GGEMSCartesianGrid const &grid) const -> void;

private:
  /*! \brief Layout of the shared vertex and fragment push constants. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Depth-tested triangle pipeline with no vertex inputs. */
  vk::raii::Pipeline pipeline_{nullptr};

  /*! \brief Linear minor and major colors from existing semantic roles. */
  std::array<std::array<float, 4U>, 2U> colors_{};
};

} // namespace ggems::ui::detail
