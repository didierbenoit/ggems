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
 * \brief Renders reference axes and diagnostic traces to an offscreen image.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui {

/*!
 * \brief Owns offscreen scene targets and diagnostic drawing resources.
 *
 * Physical and logical devices are borrowed. The application completes GPU use
 *   before replacing targets or trace data and before shutdown; scene texture
 *   descriptors must be released before their image views.
 */
class GGEMSVulkanSceneRenderer {
private:
  /*! \brief Four world-to-clip rows passed to scene vertex shaders. */
  using ScenePushConstants = std::array<std::array<float, 4U>, 4U>;

  /*! \brief Shared diagnostic line-list vertex format. */
  using TraceVertex = ggems::render::GGEMSParticleTraceVertex;

  /*! \brief Shared source-grouped vertex range for trace drawing. */
  using TraceDrawRange = ggems::render::GGEMSParticleTraceDrawRange;

public:
  /*! \brief Borrows presentation inputs for one scene recording. */
  struct RenderParameters {
    /*! \brief Rows mapping world positions in meters to clip coordinates. */
    std::array<std::array<float, 4U>, 4U> world_to_clip{};

    /*! \brief Linear RGBA clear color of the scene target. */
    std::array<float, 4U> clear_color{};

    /*! \brief Whether reference axes are included in this frame. */
    bool show_axes{true};

    /*! \brief Borrowed global and per-source trace visibility. */
    ggems::render::GGEMSParticleTraceVisibility const &trace_visibility;
  };

  /*! \brief Creates an inactive UI component. */
  GGEMSVulkanSceneRenderer() = default;

  /*! \brief Releases the owned resources. */
  ~GGEMSVulkanSceneRenderer() = default;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSVulkanSceneRenderer const &)
    -> GGEMSVulkanSceneRenderer & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSVulkanSceneRenderer &&)
    -> GGEMSVulkanSceneRenderer & = delete;

  /*!
   * \brief Creates scene shaders and pipelines for the borrowed device.
   *
   * Render targets are created separately after a nonzero viewport extent is
   *   set.
   *
   * \param[in] physical_device Physical-device wrapper that must outlive this
   *   renderer.
   * \param[in] device Logical-device wrapper that must outlive owned GPU
   *   resources.
   * \param[in] color_format Offscreen color format used by the scene pipelines.
   */
  auto Initialize(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format color_format)
    -> void;

  /*!
   * \brief Releases initialized scene resources and clears the device bindings.
   *
   * Outstanding GPU use must already be complete; an uninitialized renderer is
   *   unchanged.
   */
  auto Shutdown() -> void;

  /*!
   * \brief Requests a new nonzero physical scene extent.
   *
   * A changed extent marks the targets for recreation; zero dimensions are
   *   ignored.
   *
   * \param[in] extent Requested color and depth target dimensions in pixels.
   */
  auto SetViewportExtent(vk::Extent2D const &extent) -> void;

  /*!
   * \brief Replaces color and depth targets for the requested extent.
   *
   * The caller must finish GPU use and unregister the old scene texture first.
   *
   * \throws core::GGEMSInternal If the device or nonzero extent is missing.
   */
  auto RecreateRenderTargets() -> void;

  /*!
   * \brief Reports completion of scene renderer initialization.
   *
   * \return True after shader and pipeline setup completes.
   */
  [[nodiscard]] auto IsInitialized() const noexcept -> bool;

  /*!
   * \brief Reports whether target creation or resizing is pending.
   *
   * \return True until targets matching the requested extent are created.
   */
  [[nodiscard]] auto RequiresResize() const noexcept -> bool;

  /*!
   * \brief Returns the configured offscreen color format.
   *
   * \return Scene color format, or undefined before initialization.
   */
  [[nodiscard]] auto GetColorFormat() const noexcept -> vk::Format;

  /*!
   * \brief Returns the sampled view of the current scene image.
   *
   * \return Borrowed view valid until render targets are replaced or released.
   */
  [[nodiscard]] auto GetColorImageView() const noexcept -> vk::ImageView;

  /*!
   * \brief Records scene rendering and prepares its image for sampling.
   *
   * No commands are recorded before color targets exist. A completed scene pass
   *   leaves its color image in shader-read-only layout.
   *
   * \param[in] command_buffer Recording graphics command buffer outside an
   *   active rendering pass.
   * \param[in] parameters Transform, clear color, and visibility controls.
   */
  auto RecordSceneCommands(vk::raii::CommandBuffer const &command_buffer,
                           RenderParameters const &parameters) -> void;

  /*!
   * \brief Replaces and uploads source-grouped diagnostic trace geometry.
   *
   * The caller must finish GPU use of the previous trace buffer first.
   *
   * \param[in] segments Captured line segments converted to owned draw data.
   * \throws core::GGEMSInternal If no device is bound or the vertex count
   *   exceeds the draw range.
   */
  auto SetParticleTraceSegments(
    std::span<ggems::render::GGEMSParticleTraceSegment const> segments) -> void;

  /*! \brief Clears host trace data and releases the idle vertex buffer. */
  auto ClearParticleTraces() -> void;

  /*!
   * \brief Returns the number of retained diagnostic vertices.
   *
   * \return Current vertex count, with two vertices per retained segment.
   */
  [[nodiscard]] auto GetParticleTraceVertexCount() const noexcept
    -> std::uint32_t;

private:
  /*! \brief Allocates the sampled color attachment for the requested extent. */
  auto CreateColorTarget() -> void;

  /*! \brief Releases color and depth targets and resets layout tracking. */
  auto CleanupRenderTargets() noexcept -> void;

  /*!
   * \brief Finds a memory type satisfying an allocation request.
   *
   * \param[in] type_filter Bit mask of compatible Vulkan memory types.
   * \param[in] properties Required memory-property flags.
   * \return Index of the first matching memory type.
   * \throws core::GGEMSInternal If no physical device or matching memory type
   *   is available.
   */
  [[nodiscard]] auto FindMemoryType(std::uint32_t type_filter,
                                    vk::MemoryPropertyFlags properties) const
    -> std::uint32_t;

  /*! \brief Loads the compiled reference-axis vertex and fragment shaders. */
  auto CreateAxesShaderModules() -> void;

  /*! \brief Releases axes and trace shader modules. */
  auto CleanupShaderModules() noexcept -> void;

  /*!
   * \brief Reads a nonempty word-aligned SPIR-V binary.
   *
   * \param[in] path Compiled shader file to read.
   * \return Shader words for Vulkan module creation.
   * \throws core::GGEMSInternal If the file cannot be read or has an invalid
   *   byte size.
   */
  [[nodiscard]] static auto ReadSPIRVFile(std::filesystem::path const &path)
    -> std::vector<std::uint32_t>;

  /*! \brief Creates the depth-tested axes pipeline for the scene format. */
  auto CreateAxesPipeline() -> void;

  /*! \brief Releases the axes pipeline and its layout. */
  auto CleanupAxesPipeline() noexcept -> void;

  /*!
   * \brief Records the reference-axis line draw when its pipeline exists.
   *
   * \param[in] command_buffer Recording buffer inside the scene rendering pass.
   * \param[in] push_constants World-to-clip transform rows.
   */
  auto RecordAxesCommands(vk::raii::CommandBuffer const &command_buffer,
                          ScenePushConstants const &push_constants) -> void;

  /*! \brief Loads the compiled diagnostic trace vertex and fragment shaders. */
  auto CreateTraceShaderModules() -> void;

  /*! \brief Creates the depth-tested pipeline for diagnostic line vertices. */
  auto CreateTracePipeline() -> void;

  /*! \brief Releases the trace pipeline and its layout. */
  auto CleanupTracePipeline() noexcept -> void;

  /*! \brief Releases the trace vertex buffer and cached draw data. */
  auto CleanupTraceResources() noexcept -> void;

  /*! \brief Uploads nonempty trace vertices into host-coherent memory. */
  auto CreateTraceVertexBuffer() -> void;

  /*! \brief Releases the trace vertex buffer and its allocation. */
  auto DestroyTraceVertexBuffer() noexcept -> void;

  /*!
   * \brief Records source-grouped trace draws admitted by visibility controls.
   *
   * \param[in] command_buffer Recording buffer inside the scene rendering pass.
   * \param[in] push_constants World-to-clip transform rows.
   * \param[in] visibility Global and per-source diagnostic trace visibility.
   */
  auto RecordTraceCommands(
    vk::raii::CommandBuffer const &command_buffer,
    ScenePushConstants const &push_constants,
    ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void;

  /*!
   * \brief Allocates the depth attachment for the requested scene extent.
   *
   * \throws core::GGEMSRecoverable If the selected depth format is unsupported.
   */
  auto CreateDepthTarget() -> void;

  /*!
   * \brief Checks optimal-tiling support for a depth attachment format.
   *
   * \param[in] format Candidate depth format.
   * \return True when the physical device supports depth-stencil attachment
   *   use.
   */
  [[nodiscard]] auto IsDepthFormatSupported(vk::Format format) const -> bool;

  /*! \brief Borrowed physical-device wrapper used for memory capabilities. */
  vk::raii::PhysicalDevice const *physical_device_{nullptr};

  /*! \brief Borrowed logical-device wrapper used by owned resources. */
  vk::raii::Device const *device_{nullptr};

  /*! \brief Requested scene target extent in physical pixels. */
  vk::Extent2D viewport_extent_{};

  /*! \brief Format used for the offscreen color attachment. */
  vk::Format color_format_{vk::Format::eUndefined};

  /*! \brief Owned sampled color attachment. */
  vk::raii::Image color_image_{nullptr};

  /*! \brief Device-local allocation bound to the color image. */
  vk::raii::DeviceMemory color_memory_{nullptr};

  /*! \brief Color view borrowed by the ImGui texture registration. */
  vk::raii::ImageView color_image_view_{nullptr};

  /*! \brief Tracked layout of the offscreen color image. */
  vk::ImageLayout color_image_layout_{vk::ImageLayout::eUndefined};

  /*! \brief Depth attachment format used by scene pipelines. */
  vk::Format depth_format_{vk::Format::eD32Sfloat};

  /*! \brief Owned depth attachment. */
  vk::raii::Image depth_image_{nullptr};

  /*! \brief Device-local allocation bound to the depth image. */
  vk::raii::DeviceMemory depth_memory_{nullptr};

  /*! \brief Owned depth-attachment view. */
  vk::raii::ImageView depth_image_view_{nullptr};

  /*! \brief Tracked layout of the depth image. */
  vk::ImageLayout depth_image_layout_{vk::ImageLayout::eUndefined};

  /*! \brief Whether scene shader and pipeline initialization completed. */
  bool initialized_{false};

  /*! \brief Whether render targets must be created or resized. */
  bool requires_resize_{false};

  /*! \brief Vertex shader generating the reference-axis endpoints. */
  vk::raii::ShaderModule axes_vertex_shader_module_{nullptr};

  /*! \brief Fragment shader shading the reference axes. */
  vk::raii::ShaderModule axes_fragment_shader_module_{nullptr};

  /*! \brief Vertex shader transforming diagnostic trace vertices. */
  vk::raii::ShaderModule trace_vertex_shader_module_{nullptr};

  /*! \brief Fragment shader shading diagnostic traces. */
  vk::raii::ShaderModule trace_fragment_shader_module_{nullptr};

  /*! \brief Axes pipeline layout with world-to-clip push constants. */
  vk::raii::PipelineLayout axes_pipeline_layout_{nullptr};

  /*! \brief Line-list graphics pipeline for the reference axes. */
  vk::raii::Pipeline axes_pipeline_{nullptr};

  /*! \brief Trace pipeline layout with world-to-clip push constants. */
  vk::raii::PipelineLayout trace_pipeline_layout_{nullptr};

  /*! \brief Line-list graphics pipeline for diagnostic traces. */
  vk::raii::Pipeline trace_pipeline_{nullptr};

  /*! \brief Owned GPU-readable trace vertex buffer. */
  vk::raii::Buffer trace_vertex_buffer_{nullptr};

  /*! \brief Host-visible coherent allocation for trace vertices. */
  vk::raii::DeviceMemory trace_vertex_memory_{nullptr};

  /*! \brief Owned source-grouped vertices retained on the host. */
  std::vector<TraceVertex> trace_vertices_;

  /*! \brief Source-specific ranges used for visibility filtering. */
  std::vector<TraceDrawRange> trace_draw_ranges_;
};

} // namespace ggems::ui
