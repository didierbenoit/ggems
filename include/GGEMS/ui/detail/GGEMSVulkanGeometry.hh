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
 * \brief Declares the Vulkan wireframe geometry renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <memory>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace ggems::ui::detail {

/*!
 * \brief Owns the geometry pipeline, immutable scene, and vertex buffer.
 *
 * Physical and logical devices are borrowed and must outlive this component.
 * The application completes GPU use before replacing the scene, shutting down,
 * or destroying the component. Shader modules exist only during initialization;
 * drawing uses the caller's scene pass and viewport.
 */
class GGEMSVulkanGeometry {
public:
  /*! \brief Creates an inactive geometry renderer. */
  GGEMSVulkanGeometry() = default;

  /*! \brief Releases owned resources after GPU use has completed. */
  ~GGEMSVulkanGeometry() = default;

  /*! \brief Disallows copying Vulkan resources. */
  GGEMSVulkanGeometry(GGEMSVulkanGeometry const &) = delete;

  /*! \brief Disallows moving Vulkan resources. */
  GGEMSVulkanGeometry(GGEMSVulkanGeometry &&) = delete;

  /*! \brief Disallows copy assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanGeometry const &) -> GGEMSVulkanGeometry & = delete;

  /*! \brief Disallows move assignment of Vulkan resources. */
  auto operator=(GGEMSVulkanGeometry &&) -> GGEMSVulkanGeometry & = delete;

  /*!
   * \brief Creates the line pipeline and resolves neutral and accent colors.
   *
   * Initialize an inactive component; call Shutdown before reinitializing it.
   *
   * \param[in] physical_device Device providing vertex-memory capabilities.
   * \param[in] device Logical device that must outlive the owned resources.
   * \param[in] color_format Existing offscreen color attachment format.
   * \param[in] depth_format Existing scene depth attachment format.
   * \throws core::GGEMSInternal If compiled geometry shaders cannot be loaded.
   * \throws vk::SystemError If Vulkan resource creation fails.
   */
  auto Initialize(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format color_format,
                  vk::Format depth_format) -> void;

  /*! \brief Releases idle resources and clears borrowed devices. */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Retains an immutable scene and uploads its shared line vertices.
   *
   * Initialize before supplying a nonnull scene. The caller must finish GPU use
   * of the previous buffer first. Occurrences, physical identities, parent
   * links, and local-to-World transforms remain in the shared scene unchanged.
   *
   * \param[in] scene Prepared scene, or null to clear the displayed geometry.
   * \throws core::GGEMSInternal If no host-visible coherent memory type exists.
   * \throws vk::SystemError If buffer allocation, binding, or mapping fails.
   */
  auto SetScene(std::shared_ptr<ggems::render::GGEMSGeometryScene const> scene)
    -> void;

  /*!
   * \brief Borrows the retained scene without extending its shared ownership.
   *
   * \return Current scene, or null when none is retained; valid until the next
   *   SetScene, Shutdown, or destruction of this component.
   */
  [[nodiscard]] auto GetScene() const noexcept
    -> ggems::render::GGEMSGeometryScene const *;

  /*!
   * \brief Records each physical occurrence with its shared wireframe mesh.
   *
   * No commands are recorded when no scene is retained. The caller has already
   * set the dynamic viewport and scissor.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] world_to_clip Rows mapping World meters to clip coordinates.
   * \param[in] selected_volume_id Occurrence to accent, or none; depth is
   * unchanged.
   */
  auto
  Record(vk::raii::CommandBuffer const &command_buffer,
         std::array<std::array<float, 4U>, 4U> const &world_to_clip,
         std::optional<std::uint32_t> selected_volume_id = std::nullopt) const
    -> void;

private:
  /*! \brief Per-occurrence clip transform and linear wireframe color. */
  struct GeometryPushConstants {
    /*! \brief Rows mapping local meters to clip coordinates. */
    std::array<std::array<float, 4U>, 4U> local_to_clip{};

    /*! \brief Linear RGBA for the current occurrence's edges. */
    std::array<float, 4U> color{};
  };

  /*! \brief Borrowed physical device used to select coherent vertex memory. */
  vk::raii::PhysicalDevice const *physical_device_{nullptr};

  /*! \brief Borrowed logical device used to allocate geometry resources. */
  vk::raii::Device const *device_{nullptr};

  /*! \brief Immutable meshes, occurrences, and physical hierarchy. */
  std::shared_ptr<ggems::render::GGEMSGeometryScene const> scene_;

  /*! \brief Linear neutral palette color prepared at initialization. */
  std::array<float, 4U> color_{};

  /*!
   * \brief Linear theme accent used only for the selected physical occurrence.
   */
  std::array<float, 4U> selected_color_{};

  /*! \brief Layout of the 80-byte transform and color vertex push constants. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Depth-tested line-list pipeline for physical geometry edges. */
  vk::raii::Pipeline pipeline_{nullptr};

  /*! \brief Host-visible coherent allocation that outlives its bound buffer. */
  vk::raii::DeviceMemory vertex_memory_{nullptr};

  /*! \brief One vertex buffer shared by every occurrence in the scene. */
  vk::raii::Buffer vertex_buffer_{nullptr};
};

} // namespace ggems::ui::detail
