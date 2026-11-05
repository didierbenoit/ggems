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
 * \brief Records offscreen grids, geometry, axes, and source-filtered traces.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <format>
#include <cstdint>
#include <utility>
#include <span>
#include <memory>

#include "GGEMS/ui/detail/GGEMSVulkanSceneRenderer.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GGEMSVulkanSceneRenderer::Initialize(
  vk::raii::PhysicalDevice const &physical_device,
  vk::raii::Device const &device, vk::Format color_format) -> void {
  physical_device_ = &physical_device;
  device_ = &device;
  color_format_ = color_format;

  world_axes_.Initialize(device, color_format_, depth_format_);
  cartesian_grid_.Initialize(device, color_format_, depth_format_);

  particle_traces_.Initialize(physical_device, device, color_format_,
                              depth_format_);

  geometry_.Initialize(physical_device, device, color_format_, depth_format_);

  initialized_ = true;
  requires_resize_ = true;

  GGEMS_INFOEX("Vulkan", 1, "Vulkan scene renderer initialized.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::Shutdown() -> void {
  if (!initialized_) {
    return;
  }

  CleanupRenderTargets();
  particle_traces_.Shutdown();

  world_axes_.Shutdown();
  cartesian_grid_.Shutdown();

  geometry_.Shutdown();

  physical_device_ = nullptr;
  device_ = nullptr;
  color_format_ = vk::Format::eUndefined;
  initialized_ = false;
  requires_resize_ = false;
  viewport_extent_ = vk::Extent2D{};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene renderer shut down.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetViewportExtent(vk::Extent2D const &extent)
  -> void {
  if (extent.width == 0U || extent.height == 0U) {
    return;
  }

  if (viewport_extent_.width == extent.width &&
      viewport_extent_.height == extent.height) {
    return;
  }

  viewport_extent_ = extent;
  requires_resize_ = true;

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene viewport resized to {}x{}",
               viewport_extent_.width, viewport_extent_.height);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecreateRenderTargets() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan device is required before recreating scene render targets.");
  }

  if (!(viewport_extent_.width > 0U && viewport_extent_.height > 0U)) {
    throw ggems::core::GGEMSInternal(
      "A valid scene viewport extent is required before "
      "recreating scene render targets.");
  }

  CleanupRenderTargets();
  CreateColorTarget();
  CreateDepthTarget();

  requires_resize_ = false;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::IsInitialized() const noexcept -> bool {
  return initialized_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RequiresResize() const noexcept -> bool {
  return requires_resize_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetColorFormat() const noexcept -> vk::Format {
  return color_format_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetColorImageView() const noexcept
  -> vk::ImageView {
  return *color_image_view_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::IsDepthFormatSupported(vk::Format format) const
  -> bool {
  if (!(physical_device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan physical device is required before checking "
      "depth format support.");
  }

  vk::FormatProperties properties =
    physical_device_->getFormatProperties(format);

  return (properties.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eDepthStencilAttachment) ==
         vk::FormatFeatureFlagBits::eDepthStencilAttachment;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateColorTarget() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan device is required before creating a scene "
      "color target.");
  }

  vk::ImageCreateInfo image_create_info{
    .imageType = vk::ImageType::e2D,
    .format = color_format_,
    .extent =
      vk::Extent3D{
        .width = viewport_extent_.width,
        .height = viewport_extent_.height,
        .depth = 1U,
      },
    .mipLevels = 1U,
    .arrayLayers = 1U,
    .samples = vk::SampleCountFlagBits::e1,
    .tiling = vk::ImageTiling::eOptimal,
    .usage = vk::ImageUsageFlagBits::eColorAttachment |
             vk::ImageUsageFlagBits::eSampled,
    .sharingMode = vk::SharingMode::eExclusive,
    .initialLayout = vk::ImageLayout::eUndefined,
  };

  color_image_ = vk::raii::Image{*device_, image_create_info};

  vk::MemoryRequirements memory_requirements =
    color_image_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex = FindMemoryType(memory_requirements.memoryTypeBits,
                                      vk::MemoryPropertyFlagBits::eDeviceLocal),
  };

  color_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  color_image_.bindMemory(*color_memory_, 0U);

  vk::ImageViewCreateInfo image_view_create_info{
    .image = *color_image_,
    .viewType = vk::ImageViewType::e2D,
    .format = color_format_,
    .subresourceRange =
      vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0U,
        .levelCount = 1U,
        .baseArrayLayer = 0U,
        .layerCount = 1U,
      },
  };

  color_image_view_ = vk::raii::ImageView{*device_, image_view_create_info};

  color_image_layout_ = vk::ImageLayout::eUndefined;

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene color target created: {}x{}, format={}.",
               viewport_extent_.width, viewport_extent_.height,
               vk::to_string(color_format_));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateDepthTarget() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan device is required before creating a scene "
      "depth target.");
  }

  if (!IsDepthFormatSupported(depth_format_)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Vulkan depth format '{}' is not supported as a depth "
                  "attachment.",
                  vk::to_string(depth_format_)));
  }

  vk::ImageCreateInfo depth_image_create_info{
    .imageType = vk::ImageType::e2D,
    .format = depth_format_,
    .extent =
      vk::Extent3D{
        .width = viewport_extent_.width,
        .height = viewport_extent_.height,
        .depth = 1U,
      },
    .mipLevels = 1U,
    .arrayLayers = 1U,
    .samples = vk::SampleCountFlagBits::e1,
    .tiling = vk::ImageTiling::eOptimal,
    .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
    .sharingMode = vk::SharingMode::eExclusive,
    .initialLayout = vk::ImageLayout::eUndefined,
  };

  depth_image_ = vk::raii::Image{*device_, depth_image_create_info};

  vk::MemoryRequirements memory_requirements =
    depth_image_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
    .allocationSize = memory_requirements.size,
    .memoryTypeIndex =
      FindMemoryType(memory_requirements.memoryTypeBits,
                     vk::MemoryPropertyFlagBits::eDeviceLocal)};

  depth_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  depth_image_.bindMemory(*depth_memory_, 0U);

  vk::ImageViewCreateInfo depth_view_create_info{
    .image = *depth_image_,
    .viewType = vk::ImageViewType::e2D,
    .format = depth_format_,
    .subresourceRange =
      vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eDepth,
        .baseMipLevel = 0U,
        .levelCount = 1U,
        .baseArrayLayer = 0U,
        .layerCount = 1U,
      },
  };

  depth_image_view_ = vk::raii::ImageView{*device_, depth_view_create_info};

  depth_image_layout_ = vk::ImageLayout::eUndefined;

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene depth target created: {}x{}, format={}.",
               viewport_extent_.width, viewport_extent_.height,
               vk::to_string(depth_format_));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupRenderTargets() noexcept -> void {
  color_image_view_ = nullptr;
  color_memory_ = nullptr;
  color_image_ = nullptr;
  color_image_layout_ = vk::ImageLayout::eUndefined;

  depth_image_view_ = nullptr;
  depth_memory_ = nullptr;
  depth_image_ = nullptr;
  depth_image_layout_ = vk::ImageLayout::eUndefined;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::FindMemoryType(
  std::uint32_t type_filter, vk::MemoryPropertyFlags properties) const
  -> std::uint32_t {
  if (!(physical_device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan physical device is required before selecting "
      "a memory type.");
  }

  vk::PhysicalDeviceMemoryProperties memory_properties =
    physical_device_->getMemoryProperties();

  for (std::uint32_t i = 0U; i < memory_properties.memoryTypeCount; ++i) {
    bool type_supported = (type_filter & (1U << i)) != 0U;
    bool properties_supported =
      (memory_properties.memoryTypes[i].propertyFlags & properties) ==
      properties;

    if (type_supported && properties_supported) {
      return i;
    }
  }

  if (!(false)) {
    throw ggems::core::GGEMSInternal(
      "No suitable Vulkan memory type was found for the scene "
      "renderer color target.");
  }

  return 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecordSceneCommands(
  vk::raii::CommandBuffer const &command_buffer,
  RenderParameters const &parameters) -> void {
  if (*color_image_ == vk::Image{} || *color_image_view_ == vk::ImageView{}) {
    return;
  }

  vk::ImageSubresourceRange color_range{
    .aspectMask = vk::ImageAspectFlagBits::eColor,
    .baseMipLevel = 0U,
    .levelCount = 1U,
    .baseArrayLayer = 0U,
    .layerCount = 1U,
  };

  vk::PipelineStageFlags2 src_stage =
    color_image_layout_ == vk::ImageLayout::eUndefined
      ? vk::PipelineStageFlagBits2::eNone
      : vk::PipelineStageFlagBits2::eFragmentShader;

  vk::AccessFlags2 src_access =
    color_image_layout_ == vk::ImageLayout::eUndefined
      ? vk::AccessFlagBits2::eNone
      : vk::AccessFlagBits2::eShaderSampledRead;

  vk::ImageMemoryBarrier2 to_color_attachment{
    .srcStageMask = src_stage,
    .srcAccessMask = src_access,
    .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
    .oldLayout = color_image_layout_,
    .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = *color_image_,
    .subresourceRange = color_range,
  };

  vk::DependencyInfo to_color_attachment_dependency{
    .imageMemoryBarrierCount = 1U,
    .pImageMemoryBarriers = &to_color_attachment,
  };

  command_buffer.pipelineBarrier2(to_color_attachment_dependency);

  vk::ClearValue clear_value{vk::ClearColorValue{parameters.clear_color}};

  vk::RenderingAttachmentInfo color_attachment{
    .imageView = *color_image_view_,
    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp = vk::AttachmentLoadOp::eClear,
    .storeOp = vk::AttachmentStoreOp::eStore,
    .clearValue = clear_value,
  };

  vk::ImageSubresourceRange depth_range{
    .aspectMask = vk::ImageAspectFlagBits::eDepth,
    .baseMipLevel = 0U,
    .levelCount = 1U,
    .baseArrayLayer = 0U,
    .layerCount = 1U,
  };

  vk::PipelineStageFlags2 depth_src_stage =
    depth_image_layout_ == vk::ImageLayout::eUndefined
      ? vk::PipelineStageFlagBits2::eNone
      : vk::PipelineStageFlagBits2::eLateFragmentTests;

  vk::AccessFlags2 depth_src_access =
    depth_image_layout_ == vk::ImageLayout::eUndefined
      ? vk::AccessFlagBits2::eNone
      : vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

  vk::ImageMemoryBarrier2 to_depth_attachment{
    .srcStageMask = depth_src_stage,
    .srcAccessMask = depth_src_access,
    .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                    vk::PipelineStageFlagBits2::eLateFragmentTests,
    .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead |
                     vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
    .oldLayout = depth_image_layout_,
    .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = *depth_image_,
    .subresourceRange = depth_range,
  };

  vk::DependencyInfo to_depth_attachment_dependency{
    .imageMemoryBarrierCount = 1U,
    .pImageMemoryBarriers = &to_depth_attachment,
  };

  command_buffer.pipelineBarrier2(to_depth_attachment_dependency);

  depth_image_layout_ = vk::ImageLayout::eDepthAttachmentOptimal;

  vk::ClearValue depth_clear_value{};
  depth_clear_value.depthStencil =
    vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0U};

  vk::RenderingAttachmentInfo depth_attachment{
    .imageView = *depth_image_view_,
    .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
    .loadOp = vk::AttachmentLoadOp::eClear,
    .storeOp = vk::AttachmentStoreOp::eDontCare,
    .clearValue = depth_clear_value,
  };

  vk::RenderingInfo const rendering_info{
    .renderArea =
      vk::Rect2D{
        .offset = vk::Offset2D{.x = 0, .y = 0},
        .extent = viewport_extent_,
      },
    .layerCount = 1U,
    .colorAttachmentCount = 1U,
    .pColorAttachments = &color_attachment,
    .pDepthAttachment = &depth_attachment,
  };

  command_buffer.beginRendering(rendering_info);

  vk::Viewport const viewport{
    .x = 0.0F,
    .y = 0.0F,
    .width = static_cast<float>(viewport_extent_.width),
    .height = static_cast<float>(viewport_extent_.height),
    .minDepth = 0.0F,
    .maxDepth = 1.0F,
  };

  vk::Rect2D const scissor{
    .offset = vk::Offset2D{.x = 0, .y = 0},
    .extent = viewport_extent_,
  };

  command_buffer.setViewport(0U, viewport);
  command_buffer.setScissor(0U, scissor);

  if (parameters.cartesian_grid != nullptr) {
    cartesian_grid_.Record(command_buffer, parameters.world_to_clip,
                           viewport_extent_, *parameters.cartesian_grid,
                           parameters.clear_color);
  }

  geometry_.Record(command_buffer, parameters.world_to_clip);

  auto const *scene = geometry_.GetScene();
  if (parameters.show_axes && scene != nullptr) {
    world_axes_.Record(command_buffer, viewport_extent_,
                       parameters.world_to_clip, scene->world_half_extent_m);
  }

  particle_traces_.Record(command_buffer, parameters.world_to_clip,
                          parameters.trace_visibility);

  command_buffer.endRendering();

  vk::ImageMemoryBarrier2 to_shader_read{
    .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
    .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
    .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
    .dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
    .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = *color_image_,
    .subresourceRange = color_range,
  };

  vk::DependencyInfo const to_shader_read_dependency{
    .imageMemoryBarrierCount = 1U,
    .pImageMemoryBarriers = &to_shader_read,
  };

  command_buffer.pipelineBarrier2(to_shader_read_dependency);

  color_image_layout_ = vk::ImageLayout::eShaderReadOnlyOptimal;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetParticleTraceVertexCount() const noexcept
  -> std::uint32_t {
  return particle_traces_.GetVertexCount();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetParticleTraceSegments(
  std::span<ggems::render::GGEMSParticleTraceSegment const> segments) -> void {
  particle_traces_.SetSegments(segments);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ClearParticleTraces() -> void {
  particle_traces_.Clear();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetGeometryScene(
  std::shared_ptr<render::GGEMSGeometryScene const> scene) -> void {
  geometry_.SetScene(std::move(scene));
}

} // namespace ggems::ui
