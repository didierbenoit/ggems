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
 * \brief Declares the Vulkan procedural triangle geometry renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <memory>
#include <span>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"

namespace ggems::ui::detail {

/*!
 * \brief Owns the shared triangle pipelines, immutable scene, and analytic
 * buffers.
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
   * \brief Creates shared wire/solid triangle pipelines with alpha blending.
   *
   * Initialize an inactive component; call Shutdown before reinitializing it.
   *
   * \param[in] physical_device Device providing analytic-buffer memory
   * capabilities.
   * \param[in] device Logical device that must outlive the owned resources.
   * \param[in] color_format Existing offscreen color attachment format.
   * \param[in] depth_format Existing combined depth/stencil attachment format.
   * \throws core::GGEMSInternal If compiled geometry shaders cannot be loaded.
   * \throws vk::SystemError If Vulkan resource creation fails.
   */
  auto Initialize(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format color_format,
                  vk::Format depth_format) -> void;

  /*! \brief Releases idle resources and clears borrowed devices. */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Retains an immutable scene and uploads only shared analytic shape
   * and profile descriptors.
   *
   * Initialize before supplying a nonnull scene. The caller must finish GPU use
   * of the previous buffer first. Occurrences, physical identities, parent
   * links, and local-to-World transforms remain in the shared scene unchanged.
   *
   * \param[in] scene Prepared scene, or null to clear the displayed geometry.
   * \throws core::GGEMSInternal If descriptor ranges exceed the device limit or
   * no host-visible coherent memory type exists.
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
   * \brief Records visible occurrences using the same procedural triangles in
   * both modes.
   *
   * No commands are recorded when no scene is retained. The caller has already
   * set the dynamic viewport and scissor. Record opaque occurrences first,
   * then depth-writing scene contents, then the selection before overlays in
   * the same rendering pass.
   * Selection resolves its nearest depth without changing color, then consumes
   * one stencil bit to blend that surface exactly once at alpha 0.5. Farther
   * faces and coincident Wireframe edges do not compound opacity. The caller
   * clears stencil to zero at the start of the scene.
   *
   * \param[in] command_buffer Recording buffer inside dynamic rendering.
   * \param[in] world_to_clip Rows mapping World meters to clip coordinates.
   * \param[in] selected_volume_id Occurrence to make translucent, or none.
   * \param[in] appearance Per-VolumeID mode overrides covering all occurrences,
   * or explicitly empty for hierarchical defaults.
   * \param[in] selected_only False for opaque occurrences excluding selection;
   * true for the selected occurrence depth and color draws.
   */
  auto Record(vk::raii::CommandBuffer const &command_buffer,
              std::array<std::array<float, 4U>, 4U> const &world_to_clip,
              std::optional<std::uint32_t> selected_volume_id,
              std::span<render::GGEMSGeometryAppearance const> appearance,
              bool selected_only) const -> void;

private:
  /*!
   * \brief 128-byte vertex push contract: transform, color, rigid rotation,
   * and draw indices.
   */
  struct GeometryPushConstants {
    /*! \brief Rows mapping local meters to clip coordinates. */
    std::array<std::array<float, 4U>, 4U> local_to_clip{};

    /*! \brief Linear RGBA for the current occurrence. */
    std::array<float, 4U> color{};

    /*!
     * \brief First rigid rotation row, followed by the descriptor index at
     * byte 92.
     */
    std::array<float, 3U> normal_row_0{};

    /*! \brief Index of the shared analytic shape descriptor. */
    std::uint32_t shape_index{};

    /*! \brief Second rigid rotation row at byte 96. */
    std::array<float, 3U> normal_row_1{};

    /*! \brief One for solid lighting, zero for unlit triangle edges. */
    std::uint32_t shaded{};

    /*! \brief Third rigid rotation row at byte 112. */
    std::array<float, 3U> normal_row_2{};

    /*! \brief Reserved scalar completing the 128-byte push range. */
    std::uint32_t padding{};
  };

  /*!
   * \brief Borrowed physical device used to select coherent descriptor memory.
   */
  vk::raii::PhysicalDevice const *physical_device_{nullptr};

  /*! \brief Borrowed logical device used to allocate geometry resources. */
  vk::raii::Device const *device_{nullptr};

  /*! \brief Immutable meshes, occurrences, and physical hierarchy. */
  std::shared_ptr<ggems::render::GGEMSGeometryScene const> scene_;

  /*! \brief Read-only analytic shape and Polyhedra profile storage bindings. */
  vk::raii::DescriptorSetLayout descriptor_layout_{nullptr};

  /*! \brief Shared descriptor set and 128-byte vertex push layout. */
  vk::raii::PipelineLayout pipeline_layout_{nullptr};

  /*! \brief Depth-tested TriangleList rasterized as edges. */
  vk::raii::Pipeline wire_pipeline_{nullptr};

  /*! \brief Identical TriangleList rasterized as filled surfaces. */
  vk::raii::Pipeline solid_pipeline_{nullptr};

  /*! \brief Pool for the current scene's two analytic storage bindings. */
  vk::raii::DescriptorPool descriptor_pool_{nullptr};

  /*! \brief Host-visible coherent allocation that outlives its bound buffer. */
  vk::raii::DeviceMemory descriptor_memory_{nullptr};

  /*!
   * \brief Shared analytic parameters only; contains no positions or normals.
   */
  vk::raii::Buffer descriptor_buffer_{nullptr};

  /*!
   * \brief Current immutable scene descriptors, released before their buffer.
   */
  vk::raii::DescriptorSet descriptor_set_{nullptr};
};

} // namespace ggems::ui::detail
