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
 * \brief Declares the Vulkan particle-trace renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui::detail {

/*!
 * \brief Owns the Vulkan line pipeline and source-grouped trace data.
 *
 * Physical and logical devices are borrowed and must outlive this component.
 * The application completes GPU use before replacing or clearing traces,
 * shutting down, or destroying the component. Shader modules exist only during
 * initialization; drawing uses the caller's scene pass and viewport.
 */
class GGEMSVulkanParticleTraces {
public:
  /*! \brief Creates an inactive trace renderer. */
  GGEMSVulkanParticleTraces() = default;

  /*! \brief Releases owned resources after GPU use has completed. */
  ~GGEMSVulkanParticleTraces() = default;

  /*! \brief Disallows copying Vulkan resources. */
  GGEMSVulkanParticleTraces(GGEMSVulkanParticleTraces const &) = delete;

  /*! \brief Disallows moving Vulkan resources. */
  GGEMSVulkanParticleTraces(GGEMSVulkanParticleTraces &&) = delete;

  /*! \brief Disallows copy assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanParticleTraces const &)
    -> GGEMSVulkanParticleTraces & = delete;

  /*! \brief Disallows move assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanParticleTraces &&)
    -> GGEMSVulkanParticleTraces & = delete;

  /*!
   * \brief Borrows the devices and creates the depth-tested line pipeline.
   *
   * Initialize an inactive component; call Shutdown before reinitializing it.
   *
   * \param[in] physical_device Device providing vertex-memory capabilities.
   * \param[in] device Logical device that must outlive the owned resources.
   * \param[in] color_format Existing offscreen color attachment format.
   * \param[in] depth_format Existing scene depth attachment format.
   * \throws core::GGEMSInternal If the compiled trace shaders cannot be loaded.
   * \throws vk::SystemError If Vulkan resource creation fails.
   */
  auto Initialize(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format color_format,
                  vk::Format depth_format) -> void;

  /*! \brief Releases idle trace resources and clears the borrowed devices. */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Replaces and uploads source-grouped particle-colored line vertices.
   *
   * GPU use of the previous buffer must be complete. The existing render-layer
   * draw-data builder supplies the vertices and source ranges unchanged.
   *
   * \param[in] segments Captured diagnostic line segments to display.
   * \throws core::GGEMSInternal If no device is bound, the vertex count exceeds
   *   the Vulkan draw range, or no host-visible coherent memory type exists.
   * \throws vk::SystemError If buffer allocation, binding, or mapping fails.
   */
  auto SetSegments(
    std::span<ggems::render::GGEMSParticleTraceSegment const> segments) -> void;

  /*! \brief Clears retained draw data and releases the idle vertex buffer. */
  auto Clear() -> void;

  /*!
   * \brief Returns the number of retained diagnostic vertices.
   *
   * \return Current vertex count, with two vertices per retained segment.
   */
  [[nodiscard]] auto GetVertexCount() const noexcept -> std::uint32_t;

  /*!
   * \brief Records the visible source ranges inside the existing scene pass.
   *
   * No commands are recorded when globally hidden or when no trace data exists.
   * The caller has already set the dynamic viewport and scissor.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] world_to_clip Rows mapping World meters to clip coordinates.
   * \param[in] visibility Borrowed global and per-source visibility controls.
   */
  auto
  Record(vk::raii::CommandBuffer const &command_buffer,
         std::array<std::array<float, 4U>, 4U> const &world_to_clip,
         ggems::render::GGEMSParticleTraceVisibility const &visibility) const
    -> void;

private:
  /*! \brief Four World-to-clip rows passed to the trace vertex shader. */
  using TracePushConstants = std::array<std::array<float, 4U>, 4U>;

  /*! \brief Shared position and color layout for line-list vertices. */
  using TraceVertex = ggems::render::GGEMSParticleTraceVertex;

  /*! \brief Source-grouped range within the retained vertex array. */
  using TraceDrawRange = ggems::render::GGEMSParticleTraceDrawRange;

  /*! \brief Releases idle vertex storage and clears its host draw data. */
  auto ReleaseData() noexcept -> void;

  /*! \brief Borrowed physical device used to select coherent vertex memory. */
  vk::raii::PhysicalDevice const *physical_device_{nullptr};

  /*! \brief Borrowed logical device used to allocate trace resources. */
  vk::raii::Device const *device_{nullptr};

  /*! \brief Layout of the 64-byte World-to-clip vertex push constants. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Depth-tested line-list pipeline with position and color inputs. */
  vk::raii::Pipeline pipeline_{nullptr};

  /*! \brief Host-visible coherent allocation that outlives its bound buffer. */
  vk::raii::DeviceMemory vertex_memory_{nullptr};

  /*! \brief Owned GPU-readable buffer released before its allocation. */
  vk::raii::Buffer vertex_buffer_{nullptr};

  /*! \brief Owned particle-colored vertices grouped by source slot. */
  std::vector<TraceVertex> vertices_;

  /*! \brief Source identifiers and ranges retained for visibility filtering. */
  std::vector<TraceDrawRange> draw_ranges_;
};

} // namespace ggems::ui::detail
