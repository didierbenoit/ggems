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
 * \brief Owns Vulkan trace resources and records source-filtered line draws.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <span>
#include <utility>

#include "GGEMS/ui/detail/GGEMSVulkanParticleTraces.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"

namespace ggems::ui::detail {

auto GGEMSVulkanParticleTraces::Initialize(
  vk::raii::PhysicalDevice const &physical_device,
  vk::raii::Device const &device, vk::Format color_format,
  vk::Format depth_format) -> void {
  physical_device_ = &physical_device;
  device_ = &device;

  std::filesystem::path const shader_directory{GGEMS_UI_SHADER_DIRECTORY};
  auto const vertex_code =
    ReadSPIRVFile(shader_directory / "GGEMSTraces.vert.spv");
  auto const fragment_code =
    ReadSPIRVFile(shader_directory / "GGEMSTraces.frag.spv");

  vk::raii::ShaderModule const vertex_shader{
    device, vk::ShaderModuleCreateInfo{
              .codeSize = vertex_code.size() * sizeof(std::uint32_t),
              .pCode = vertex_code.data(),
            }};

  vk::raii::ShaderModule const fragment_shader{
    device, vk::ShaderModuleCreateInfo{
              .codeSize = fragment_code.size() * sizeof(std::uint32_t),
              .pCode = fragment_code.data(),
            }};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene trace shader modules created from '{}'.",
               shader_directory.string());

  vk::PipelineShaderStageCreateInfo vertex_stage{
    .stage = vk::ShaderStageFlagBits::eVertex,
    .module = *vertex_shader,
    .pName = "VertexMain",
  };

  vk::PipelineShaderStageCreateInfo fragment_stage{
    .stage = vk::ShaderStageFlagBits::eFragment,
    .module = *fragment_shader,
    .pName = "FragmentMain",
  };

  std::array<vk::PipelineShaderStageCreateInfo, 2U> shader_stages{
    vertex_stage,
    fragment_stage,
  };

  vk::VertexInputBindingDescription vertex_binding_description{
    .binding = 0U,
    .stride = sizeof(TraceVertex),
    .inputRate = vk::VertexInputRate::eVertex,
  };

  std::array<vk::VertexInputAttributeDescription, 2U>
    vertex_attribute_descriptions{
      {
        vk::VertexInputAttributeDescription{
          .location = 0U,
          .binding = 0U,
          .format = vk::Format::eR32G32B32Sfloat,
          .offset = offsetof(TraceVertex, position),
        },
        vk::VertexInputAttributeDescription{
          .location = 1U,
          .binding = 0U,
          .format = vk::Format::eR32G32B32A32Sfloat,
          .offset = offsetof(TraceVertex, color),
        },
      },
  };

  vk::PipelineVertexInputStateCreateInfo vertex_input_state{
    .vertexBindingDescriptionCount = 1U,
    .pVertexBindingDescriptions = &vertex_binding_description,
    .vertexAttributeDescriptionCount =
      static_cast<std::uint32_t>(vertex_attribute_descriptions.size()),
    .pVertexAttributeDescriptions = vertex_attribute_descriptions.data(),
  };

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{
    .topology = vk::PrimitiveTopology::eLineList,
    .primitiveRestartEnable = vk::False,
  };

  vk::PipelineViewportStateCreateInfo viewport_state{
    .viewportCount = 1U,
    .scissorCount = 1U,
  };

  vk::PipelineRasterizationStateCreateInfo rasterization_state{
    .depthClampEnable = vk::False,
    .rasterizerDiscardEnable = vk::False,
    .polygonMode = vk::PolygonMode::eFill,
    .cullMode = vk::CullModeFlagBits::eNone,
    .frontFace = vk::FrontFace::eCounterClockwise,
    .depthBiasEnable = vk::False,
    .lineWidth = 1.0F,
  };

  vk::PipelineMultisampleStateCreateInfo multisample_state{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
    .sampleShadingEnable = vk::False,
  };

  vk::PipelineColorBlendAttachmentState color_blend_attachment{
    .blendEnable = vk::False,
    .colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
  };

  vk::PipelineColorBlendStateCreateInfo color_blend_state{
    .logicOpEnable = vk::False,
    .attachmentCount = 1U,
    .pAttachments = &color_blend_attachment,
  };

  std::array<vk::DynamicState, 2U> dynamic_states{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
  };

  vk::PipelineDynamicStateCreateInfo dynamic_state{
    .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
    .pDynamicStates = dynamic_states.data(),
  };

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{
    .depthTestEnable = vk::True,
    .depthWriteEnable = vk::True,
    .depthCompareOp = vk::CompareOp::eLessOrEqual,
    .depthBoundsTestEnable = vk::False,
    .stencilTestEnable = vk::False,
    .minDepthBounds = 0.0F,
    .maxDepthBounds = 1.0F,
  };

  vk::PushConstantRange trace_push_constant_range{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
    .offset = 0U,
    .size = sizeof(TracePushConstants),
  };

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
    .pushConstantRangeCount = 1U,
    .pPushConstantRanges = &trace_push_constant_range,
  };

  pipeline_layout_ =
    vk::raii::PipelineLayout{device, pipeline_layout_create_info};

  vk::PipelineRenderingCreateInfo rendering_create_info{
    .colorAttachmentCount = 1U,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = depth_format,
  };

  vk::GraphicsPipelineCreateInfo pipeline_create_info{
    .pNext = &rendering_create_info,
    .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
    .pStages = shader_stages.data(),
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState = &multisample_state,
    .pDepthStencilState = &depth_stencil_state,
    .pColorBlendState = &color_blend_state,
    .pDynamicState = &dynamic_state,
    .layout = *pipeline_layout_,
    .renderPass = nullptr,
    .subpass = 0U,
  };

  pipeline_ = vk::raii::Pipeline{device, nullptr, pipeline_create_info};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene trace pipeline created.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::Shutdown() noexcept -> void {
  ReleaseData();
  pipeline_ = nullptr;
  pipeline_layout_ = nullptr;
  physical_device_ = nullptr;
  device_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::SetSegments(
  std::span<ggems::render::GGEMSParticleTraceSegment const> segments) -> void {
  if (device_ == nullptr) {
    throw ggems::core::GGEMSInternal(
      "A Vulkan device is required before setting particle trace segments.");
  }

  auto draw_data = ggems::render::BuildParticleTraceDrawData(segments);

  if (draw_data.vertices.size() >
      static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
    throw ggems::core::GGEMSInternal(
      "The particle trace vertex count exceeds the Vulkan uint32_t draw "
      "range.");
  }

  ReleaseData();
  vertices_ = std::move(draw_data.vertices);
  draw_ranges_ = std::move(draw_data.draw_ranges);

  if (!vertices_.empty()) {
    auto const buffer_size =
      static_cast<vk::DeviceSize>(vertices_.size() * sizeof(TraceVertex));

    vk::BufferCreateInfo const buffer_create_info{
      .size = buffer_size,
      .usage = vk::BufferUsageFlagBits::eVertexBuffer,
      .sharingMode = vk::SharingMode::eExclusive,
    };

    vertex_buffer_ = vk::raii::Buffer{*device_, buffer_create_info};

    vk::MemoryRequirements const requirements =
      vertex_buffer_.getMemoryRequirements();

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
        "No suitable Vulkan memory type was found for the trace vertex "
        "buffer.");
    }

    vk::MemoryAllocateInfo const memory_allocate_info{
      .allocationSize = requirements.size,
      .memoryTypeIndex = memory_type_index,
    };

    vertex_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
    vertex_buffer_.bindMemory(*vertex_memory_, 0U);
    void *mapped_memory = vertex_memory_.mapMemory(0, buffer_size);
    std::memcpy(mapped_memory, vertices_.data(),
                static_cast<std::size_t>(buffer_size));
    vertex_memory_.unmapMemory();

    GGEMS_INFOEX("Vulkan", 2,
                 "Vulkan trace vertex buffer "
                 "uploaded: {} vertex/vertices.",
                 vertices_.size());
  }

  GGEMS_INFOEX(
    "Vulkan", 1,
    "Loaded {} particle trace segment(s) into Vulkan: {} vertex/vertices.",
    segments.size(), vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::Clear() -> void {
  ReleaseData();
  GGEMS_INFOEX("Vulkan", 1, "Cleared Vulkan particle traces.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::GetVertexCount() const noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::Record(
  vk::raii::CommandBuffer const &command_buffer,
  std::array<std::array<float, 4U>, 4U> const &world_to_clip,
  ggems::render::GGEMSParticleTraceVisibility const &visibility) const -> void {

  if (!visibility.IsGlobalVisible()) {
    return;
  }

  if (*pipeline_ == vk::Pipeline{} || *vertex_buffer_ == vk::Buffer{} ||
      vertices_.empty() || draw_ranges_.empty()) {
    return;
  }

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);

  std::array<vk::Buffer, 1U> const vertex_buffers{*vertex_buffer_};
  std::array<vk::DeviceSize, 1U> const vertex_offsets{0U};
  command_buffer.bindVertexBuffers(0U, vertex_buffers, vertex_offsets);

  command_buffer.pushConstants(
    *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
    vk::ArrayProxy<TracePushConstants const>{1U, &world_to_clip});

  for (TraceDrawRange const &draw_range : draw_ranges_) {
    if (!visibility.ShouldDraw(draw_range.source_index)) {
      continue;
    }

    command_buffer.draw(static_cast<std::uint32_t>(draw_range.vertex_count), 1U,
                        static_cast<std::uint32_t>(draw_range.first_vertex),
                        0U);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanParticleTraces::ReleaseData() noexcept -> void {
  vertex_buffer_ = nullptr;
  vertex_memory_ = nullptr;
  vertices_.clear();
  draw_ranges_.clear();
}

} // namespace ggems::ui::detail
