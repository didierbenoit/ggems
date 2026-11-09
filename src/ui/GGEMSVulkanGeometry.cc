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
 * \brief Owns shared procedural triangle resources and records physical
 * occurrences.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <utility>
#include <optional>
#include <span>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/ui/detail/GGEMSVulkanGeometry.hh"

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

  vk::PipelineVertexInputStateCreateInfo const vertex_input{};

  vk::PipelineInputAssemblyStateCreateInfo const assembly{
    .topology = vk::PrimitiveTopology::eTriangleList,
  };

  vk::PipelineViewportStateCreateInfo const viewport{
    .viewportCount = 1U,
    .scissorCount = 1U,
  };

  vk::PipelineRasterizationStateCreateInfo rasterization{
    .polygonMode = vk::PolygonMode::eLine,
    .cullMode = vk::CullModeFlagBits::eNone,
    .frontFace = vk::FrontFace::eCounterClockwise,
    .lineWidth = 1.0F,
  };

  vk::PipelineMultisampleStateCreateInfo const multisample{
    .rasterizationSamples = vk::SampleCountFlagBits::e1,
  };

  vk::StencilOpState const stencil{
    .compareMask = 1U,
    .writeMask = 1U,
    .reference = 1U,
  };

  vk::PipelineDepthStencilStateCreateInfo const depth{
    .depthTestEnable = vk::True,
    .depthWriteEnable = vk::True,
    .depthCompareOp = vk::CompareOp::eLessOrEqual,
    .stencilTestEnable = vk::True,
    .front = stencil,
    .back = stencil,
  };

  vk::PipelineColorBlendAttachmentState const attachment{
    .blendEnable = vk::True,
    .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
    .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
    .colorBlendOp = vk::BlendOp::eAdd,
    .srcAlphaBlendFactor = vk::BlendFactor::eOne,
    .dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
    .alphaBlendOp = vk::BlendOp::eAdd,
    .colorWriteMask =
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
  };

  vk::PipelineColorBlendStateCreateInfo const blend{
    .attachmentCount = 1U,
    .pAttachments = &attachment,
  };

  std::array<vk::DynamicState, 3U> const dynamic_states{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor,
    vk::DynamicState::eStencilOp,
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

  std::array<vk::DescriptorSetLayoutBinding, 2U> const bindings{
    vk::DescriptorSetLayoutBinding{
      .binding = 0U,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1U,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
    },
    vk::DescriptorSetLayoutBinding{
      .binding = 1U,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .descriptorCount = 1U,
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
    },
  };

  descriptor_layout_ = vk::raii::DescriptorSetLayout{
    device, vk::DescriptorSetLayoutCreateInfo{
              .bindingCount = static_cast<std::uint32_t>(bindings.size()),
              .pBindings = bindings.data(),
            }};
  auto const layout = *descriptor_layout_;

  pipeline_layout_ =
    vk::raii::PipelineLayout{device, vk::PipelineLayoutCreateInfo{
                                       .setLayoutCount = 1U,
                                       .pSetLayouts = &layout,
                                       .pushConstantRangeCount = 1U,
                                       .pPushConstantRanges = &push_range,
                                     }};

  vk::DescriptorPoolSize const pool_size{
    .type = vk::DescriptorType::eStorageBuffer,
    .descriptorCount = 2U,
  };

  descriptor_pool_ = vk::raii::DescriptorPool{
    device, vk::DescriptorPoolCreateInfo{
              .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
              .maxSets = 1U,
              .poolSizeCount = 1U,
              .pPoolSizes = &pool_size,
            }};

  vk::PipelineRenderingCreateInfo const rendering{
    .colorAttachmentCount = 1U,
    .pColorAttachmentFormats = &color_format,
    .depthAttachmentFormat = depth_format,
    .stencilAttachmentFormat = depth_format,
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

  wire_pipeline_ = vk::raii::Pipeline{device, nullptr, pipeline};
  rasterization.polygonMode = vk::PolygonMode::eFill;
  solid_pipeline_ = vk::raii::Pipeline{device, nullptr, pipeline};
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::Shutdown() noexcept -> void {
  descriptor_set_ = nullptr;
  descriptor_buffer_ = nullptr;
  descriptor_memory_ = nullptr;
  scene_.reset();
  solid_pipeline_ = nullptr;
  wire_pipeline_ = nullptr;
  pipeline_layout_ = nullptr;
  descriptor_pool_ = nullptr;
  descriptor_layout_ = nullptr;
  physical_device_ = nullptr;
  device_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::SetScene(
  std::shared_ptr<render::GGEMSGeometryScene const> scene) -> void {
  descriptor_set_ = nullptr;
  descriptor_buffer_ = nullptr;
  descriptor_memory_ = nullptr;
  scene_ = std::move(scene);

  if (!scene_) {
    return;
  }

  auto const shape_bytes = static_cast<vk::DeviceSize>(
    scene_->meshes.size() * sizeof(render::GGEMSGeometryShape));

  auto const limits = physical_device_->getProperties().limits;

  auto const alignment = limits.minStorageBufferOffsetAlignment;

  auto const profile_offset =
    ((shape_bytes + alignment - 1U) / alignment) * alignment;

  // Vulkan requires a nonempty descriptor range even when no Polyhedra uses it.
  auto const profile_bytes = static_cast<vk::DeviceSize>(
    std::max(scene_->profiles.size(), std::size_t{1U}) *
    sizeof(render::GGEMSGeometryProfile));

  if (shape_bytes > limits.maxStorageBufferRange ||
      profile_bytes > limits.maxStorageBufferRange) {
    throw ggems::core::GGEMSInternal("Geometry analytic descriptors exceed the "
                                     "device's maxStorageBufferRange.");
  }
  auto const buffer_size = profile_offset + profile_bytes;
  descriptor_buffer_ = vk::raii::Buffer{
    *device_, vk::BufferCreateInfo{
                .size = buffer_size,
                .usage = vk::BufferUsageFlagBits::eStorageBuffer,
                .sharingMode = vk::SharingMode::eExclusive,
              }};

  auto const requirements = descriptor_buffer_.getMemoryRequirements();

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
      "No suitable Vulkan memory type was found for the geometry analytic "
      "buffer.");
  }

  descriptor_memory_ =
    vk::raii::DeviceMemory{*device_, vk::MemoryAllocateInfo{
                                       .allocationSize = requirements.size,
                                       .memoryTypeIndex = memory_type_index,
                                     }};

  descriptor_buffer_.bindMemory(*descriptor_memory_, 0U);

  void *mapped = descriptor_memory_.mapMemory(0U, buffer_size);
  auto *bytes = static_cast<std::byte *>(mapped);

  for (std::size_t index = 0U; index < scene_->meshes.size(); ++index) {
    std::memcpy(bytes + (index * sizeof(render::GGEMSGeometryShape)),
                &scene_->meshes[index].shape,
                sizeof(render::GGEMSGeometryShape));
  }

  std::memset(bytes + profile_offset, 0,
              static_cast<std::size_t>(profile_bytes));

  if (!scene_->profiles.empty()) {
    std::memcpy(bytes + profile_offset, scene_->profiles.data(),
                scene_->profiles.size() * sizeof(render::GGEMSGeometryProfile));
  }

  descriptor_memory_.unmapMemory();

  auto const layout = *descriptor_layout_;

  descriptor_set_ = std::move(vk::raii::DescriptorSets{
    *device_, vk::DescriptorSetAllocateInfo{
                .descriptorPool = *descriptor_pool_,
                .descriptorSetCount = 1U,
                .pSetLayouts = &layout,
              }}.front());

  std::array<vk::DescriptorBufferInfo, 2U> const buffers{
    vk::DescriptorBufferInfo{
      .buffer = *descriptor_buffer_,
      .offset = 0U,
      .range = shape_bytes,
    },
    vk::DescriptorBufferInfo{
      .buffer = *descriptor_buffer_,
      .offset = profile_offset,
      .range = profile_bytes,
    },
  };

  std::array<vk::WriteDescriptorSet, 2U> writes{};

  for (std::uint32_t binding = 0U; binding < writes.size(); ++binding) {
    writes[binding] = vk::WriteDescriptorSet{
      .dstSet = *descriptor_set_,
      .dstBinding = binding,
      .descriptorCount = 1U,
      .descriptorType = vk::DescriptorType::eStorageBuffer,
      .pBufferInfo = &buffers[binding],
    };
  }

  device_->updateDescriptorSets(writes, {});
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::GetScene() const noexcept
  -> render::GGEMSGeometryScene const * {
  return scene_.get();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanGeometry::Record(
  vk::raii::CommandBuffer const &command_buffer,
  std::array<std::array<float, 4U>, 4U> const &world_to_clip,
  std::optional<std::uint32_t> selected_volume_id,
  std::span<render::GGEMSGeometryAppearance const> appearance,
  bool selected_only) const -> void {
  if (!scene_ || (selected_only && !selected_volume_id)) {
    return;
  }

  command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                    *pipeline_layout_, 0U, {*descriptor_set_},
                                    {});

  command_buffer.setStencilOp(
    vk::StencilFaceFlagBits::eFrontAndBack, vk::StencilOp::eKeep,
    selected_only ? vk::StencilOp::eReplace : vk::StencilOp::eKeep,
    vk::StencilOp::eKeep, vk::CompareOp::eAlways);

  std::optional<render::GGEMSGeometryDisplayMode> bound_mode;

  GeometryPushConstants constants;

  render::GGEMSGeometryAppearance const automatic;

  for (auto const &occurrence : scene_->occurrences) {
    if ((selected_volume_id == occurrence.volume_id) != selected_only) {
      continue;
    }

    auto const &override =
      appearance.empty() ? automatic : appearance[occurrence.volume_id];

    auto const mode = render::ResolveGeometryDisplayMode(occurrence, override);
    if (mode == render::GGEMSGeometryDisplayMode::None) {
      continue;
    }

    bool const solid = mode == render::GGEMSGeometryDisplayMode::Solid;

    if (bound_mode != mode) {
      command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                  solid ? *solid_pipeline_ : *wire_pipeline_);
      bound_mode = mode;
    }

    constants.color = ToVulkanClearColor(occurrence.color);
    constants.shape_index = occurrence.mesh_index;
    constants.shaded = solid ? 1U : 0U;

    std::copy_n(occurrence.local_to_world.data(), 3U,
                constants.normal_row_0.begin());
    std::copy_n(occurrence.local_to_world.data() + 4U, 3U,
                constants.normal_row_1.begin());
    std::copy_n(occurrence.local_to_world.data() + 8U, 3U,
                constants.normal_row_2.begin());

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

    auto const &mesh = scene_->meshes[occurrence.mesh_index];

    if (selected_only) {
      // Preserve scene color while marking the nearest selected surface.
      // Consume its stencil bit once, including coincident Wireframe edges.
      constants.color[3U] = 0.0F;
      command_buffer.pushConstants(
        *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
        vk::ArrayProxy<GeometryPushConstants const>{1U, &constants});
      command_buffer.draw(mesh.vertex_count, 1U, 0U, 0U);

      command_buffer.setStencilOp(vk::StencilFaceFlagBits::eFrontAndBack,
                                  vk::StencilOp::eKeep, vk::StencilOp::eZero,
                                  vk::StencilOp::eKeep, vk::CompareOp::eEqual);
      constants.color[3U] = 0.5F;
    }

    command_buffer.pushConstants(
      *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
      vk::ArrayProxy<GeometryPushConstants const>{1U, &constants});

    command_buffer.draw(mesh.vertex_count, 1U, 0U, 0U);
  }
}

} // namespace ggems::ui::detail
