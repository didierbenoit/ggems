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
 * \brief Owns Vulkan wireframe resources and records physical occurrences.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <utility>

#include "GGEMS/ui/detail/GGEMSVulkanGeometry.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"

namespace ggems::ui::detail {

auto GGEMSVulkanGeometry::Initialize(
  vk::raii::PhysicalDevice const &physical_device,
  vk::raii::Device const &device, vk::Format color_format,
  vk::Format depth_format) -> void {
  physical_device_ = &physical_device;
  device_ = &device;

  auto const shader_directory =
    std::filesystem::path{GGEMS_UI_SHADER_DIRECTORY};

  auto const vertex_code =
    ReadSPIRVFile(shader_directory / "GGEMSGeometry.vert.spv");

  auto const fragment_code =
    ReadSPIRVFile(shader_directory / "GGEMSGeometry.frag.spv");

  vk::raii::ShaderModule const vertex_module{
    device, vk::ShaderModuleCreateInfo{
              .codeSize = vertex_code.size() * sizeof(std::uint32_t),
              .pCode = vertex_code.data(),
            }};

  vk::raii::ShaderModule const fragment_module{
    device, vk::ShaderModuleCreateInfo{
              .codeSize = fragment_code.size() * sizeof(std::uint32_t),
              .pCode = fragment_code.data(),
            }};

  std::array<vk::PipelineShaderStageCreateInfo, 2U> const shader_stages{
    vk::PipelineShaderStageCreateInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = *vertex_module,
      .pName = "VertexMain",
    },
    vk::PipelineShaderStageCreateInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = *fragment_module,
      .pName = "FragmentMain",
    },
  };

  vk::VertexInputBindingDescription const vertex_binding{
    .binding = 0U,
    .stride = sizeof(render::GGEMSGeometryVertex),
    .inputRate = vk::VertexInputRate::eVertex,
  };

  vk::VertexInputAttributeDescription const vertex_attribute{
    .location = 0U,
    .binding = 0U,
    .format = vk::Format::eR32G32B32Sfloat,
    .offset = 0U,
  };

  vk::PipelineVertexInputStateCreateInfo const vertex_input{
    .vertexBindingDescriptionCount = 1U,
    .pVertexBindingDescriptions = &vertex_binding,
    .vertexAttributeDescriptionCount = 1U,
    .pVertexAttributeDescriptions = &vertex_attribute,
  };

  vk::PipelineInputAssemblyStateCreateInfo const assembly{
    .topology = vk::PrimitiveTopology::eLineList,
  };

  vk::PipelineViewportStateCreateInfo const viewport{
    .viewportCount = 1U,
    .scissorCount = 1U,
  };

  vk::PipelineRasterizationStateCreateInfo const rasterization{
    .polygonMode = vk::PolygonMode::eFill,
    .cullMode = vk::CullModeFlagBits::eNone,
    .frontFace = vk::FrontFace::eCounterClockwise,
    .lineWidth = 1.0F,
  };

  vk::PipelineMultisampleStateCreateInfo const multisample{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
  };

  vk::PipelineDepthStencilStateCreateInfo const depth{
    .depthTestEnable = vk::True,
    .depthWriteEnable = vk::True,
    .depthCompareOp = vk::CompareOp::eLessOrEqual,
  };

  vk::PipelineColorBlendAttachmentState const attachment{
    .colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
  };

  vk::PipelineColorBlendStateCreateInfo const blend{
    .attachmentCount = 1U,
    .pAttachments = &attachment,
  };

  std::array<vk::DynamicState, 2U> const dynamic_states{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
  };

  vk::PipelineDynamicStateCreateInfo const dynamic{
    .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
    .pDynamicStates = dynamic_states.data(),
  };

  vk::PushConstantRange const push_range{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
    .offset = 0U,
    .size = sizeof(GeometryPushConstants),
  };

  pipeline_layout_ =
    vk::raii::PipelineLayout{device, vk::PipelineLayoutCreateInfo{
                                       .pushConstantRangeCount = 1U,
                                       .pPushConstantRanges = &push_range,
                                     }};

  vk::PipelineRenderingCreateInfo const rendering{
    .colorAttachmentCount = 1U,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = depth_format,
  };

  vk::GraphicsPipelineCreateInfo const pipeline{
    .pNext = &rendering,
    .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
    .pStages = shader_stages.data(),
    .pVertexInputState = &vertex_input,
    .pInputAssemblyState = &assembly,
    .pViewportState = &viewport,
    .pRasterizationState = &rasterization,
    .pMultisampleState = &multisample,
    .pDepthStencilState = &depth,
    .pColorBlendState = &blend,
    .pDynamicState = &dynamic,
    .layout = *pipeline_layout_,
  };

  pipeline_ = vk::raii::Pipeline{device, nullptr, pipeline};
  color_ = ToVulkanClearColor(GetThemeColorKey(GGEMSThemeRole::PrimaryText));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::Shutdown() noexcept -> void {
  vertex_buffer_ = nullptr;
  vertex_memory_ = nullptr;
  scene_.reset();
  pipeline_ = nullptr;
  pipeline_layout_ = nullptr;
  physical_device_ = nullptr;
  device_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::SetScene(
  std::shared_ptr<render::GGEMSGeometryScene const> scene) -> void {
  vertex_buffer_ = nullptr;
  vertex_memory_ = nullptr;
  scene_ = std::move(scene);

  if (!scene_) {
    return;
  }

  auto const buffer_size = static_cast<vk::DeviceSize>(
    scene_->vertices.size() * sizeof(render::GGEMSGeometryVertex));
  vertex_buffer_ = vk::raii::Buffer{
    *device_, vk::BufferCreateInfo{
                .size = buffer_size,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer,
                .sharingMode = vk::SharingMode::eExclusive,
              }};

  auto const requirements = vertex_buffer_.getMemoryRequirements();

  vk::PhysicalDeviceMemoryProperties const memory_properties =
    physical_device_->getMemoryProperties();

  vk::MemoryPropertyFlags const required_properties =
    vk::MemoryPropertyFlagBits::eHostVisible |
    vk::MemoryPropertyFlagBits::eHostCoherent;

  std::uint32_t memory_type_index = 0U;
  for (; memory_type_index < memory_properties.memoryTypeCount;
       ++memory_type_index) {
    bool const type_supported =
      (requirements.memoryTypeBits & (1U << memory_type_index)) != 0U;
    bool const properties_supported =
      (memory_properties.memoryTypes[memory_type_index].propertyFlags &
       required_properties) == required_properties;

    if (type_supported && properties_supported) {
      break;
    }
  }

  if (memory_type_index == memory_properties.memoryTypeCount) {
    throw ggems::core::GGEMSInternal(
      "No suitable Vulkan memory type was found for the geometry vertex "
      "buffer.");
  }

  vertex_memory_ =
    vk::raii::DeviceMemory{*device_, vk::MemoryAllocateInfo{
                                       .allocationSize = requirements.size,
                                       .memoryTypeIndex = memory_type_index,
                                     }};
  vertex_buffer_.bindMemory(*vertex_memory_, 0U);

  void *mapped = vertex_memory_.mapMemory(0U, buffer_size);
  std::memcpy(mapped, scene_->vertices.data(),
              static_cast<std::size_t>(buffer_size));
  vertex_memory_.unmapMemory();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::GetScene() const noexcept
  -> render::GGEMSGeometryScene const * {
  return scene_.get();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::Record(
  vk::raii::CommandBuffer const &command_buffer,
  std::array<std::array<float, 4U>, 4U> const &world_to_clip) const -> void {
  if (!scene_) {
    return;
  }

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
  std::array<vk::Buffer, 1U> const buffers{*vertex_buffer_};
  std::array<vk::DeviceSize, 1U> const offsets{0U};
  command_buffer.bindVertexBuffers(0U, buffers, offsets);

  GeometryPushConstants constants{.color = color_};
  for (auto const &occurrence : scene_->occurrences) {
    for (std::size_t row = 0U; row < 4U; ++row) {
      for (std::size_t column = 0U; column < 4U; ++column) {
        float value = 0.0F;
        for (std::size_t inner = 0U; inner < 4U; ++inner) {
          value += world_to_clip[row][inner] *
                   occurrence.local_to_world[(inner * 4U) + column];
        }
        constants.local_to_clip[row][column] = value;
      }
    }

    command_buffer.pushConstants(
      *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
      vk::ArrayProxy<GeometryPushConstants const>{1U, &constants});
    auto const &mesh = scene_->meshes[occurrence.mesh_index];
    command_buffer.draw(mesh.vertex_count, 1U, mesh.first_vertex, 0U);
  }
}

} // namespace ggems::ui::detail
