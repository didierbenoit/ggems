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
 * \brief Owns the procedural World-reference pipeline and its seven draws.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>

#include "GGEMS/ui/detail/GGEMSVulkanWorldAxes.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

/*! \brief Matches the 104-byte GGEMSAxes.slang vertex push block. */
struct AxesPushConstants {
  /*! \brief Rows mapping World meters to clip coordinates. */
  std::array<std::array<float, 4U>, 4U> world_to_clip{};

  /*! \brief Opaque linear-light surface color. */
  std::array<float, 4U> color{};

  /*! \brief Axis length in meters, relative radius, height, and base Z. */
  std::array<float, 4U> dimensions{};

  /*! \brief Cylinder = 0, cone = 1, sphere = 2. */
  std::uint32_t primitive{};

  /*! \brief Proper rotation from local +Z to global X = 0, Y = 1, Z = 2. */
  std::uint32_t axis{};
};

} // namespace

namespace ggems::ui::detail {

auto GGEMSVulkanWorldAxes::Initialize(vk::raii::Device const &device,
                                      vk::Format color_format,
                                      vk::Format depth_format) -> void {
  std::filesystem::path const shader_directory{GGEMS_UI_SHADER_DIRECTORY};

  auto const vertex_code =
    ReadSPIRVFile(shader_directory / "GGEMSAxes.vert.spv");

  auto const fragment_code =
    ReadSPIRVFile(shader_directory / "GGEMSAxes.frag.spv");

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

  vk::PipelineVertexInputStateCreateInfo const vertex_input_state{};

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{
    .topology = vk::PrimitiveTopology::eTriangleList,
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

  vk::PushConstantRange axes_push_constant_range{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
    .offset = 0U,
    .size = sizeof(AxesPushConstants),
  };

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
    .pushConstantRangeCount = 1U,
    .pPushConstantRanges = &axes_push_constant_range,
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

  colors_ = {
    ToVulkanClearColor(render::RED_Vermilion),
    ToVulkanClearColor(render::GREEN_MediumSeaGreen),
    ToVulkanClearColor(render::BLUE_CornflowerBlue),
    ToVulkanClearColor(render::GRAY_Silver),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanWorldAxes::Shutdown() noexcept -> void {
  pipeline_ = nullptr;
  pipeline_layout_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanWorldAxes::Record(
  vk::raii::CommandBuffer const &command_buffer, vk::Extent2D const &extent,
  std::array<std::array<float, 4U>, 4U> const &world_to_clip,
  std::array<double, 3U> const &world_half_extent_m) const -> void {
  command_buffer.setViewport(0U, vk::Viewport{
                                   .x = 0.0F,
                                   .y = 0.0F,
                                   .width = static_cast<float>(extent.width),
                                   .height = static_cast<float>(extent.height),
                                   .minDepth = 0.0F,
                                   .maxDepth = 1.0F,
                                 });

  command_buffer.setScissor(0U, vk::Rect2D{
                                  .offset = vk::Offset2D{.x = 0, .y = 0},
                                  .extent = extent,
                                });

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);

  float const length =
    0.55F * static_cast<float>(std::ranges::min(world_half_extent_m));

  std::array<std::array<float, 4U>, 2U> const arrow_dimensions{
    {
      {length, 0.014F, 0.82F, 0.0F},
      {length, 0.055F, 0.18F, 0.82F},
    },
  };

  AxesPushConstants constants{.world_to_clip = world_to_clip};

  for (std::uint32_t axis = 0U; axis < 3U; ++axis) {
    constants.axis = axis;
    constants.color = colors_[axis];

    for (std::uint32_t primitive = 0U; primitive < 2U; ++primitive) {
      constants.primitive = primitive;
      constants.dimensions = arrow_dimensions[primitive];
      command_buffer.pushConstants(
        *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
        vk::ArrayProxy<AxesPushConstants const>{1U, &constants});
      // GGEMSAxes.slang: 32 shaft quads or 32 cone sides and base triangles.
      command_buffer.draw(192U, 1U, 0U, 0U);
    }
  }

  constants.color = colors_[3];
  constants.dimensions = {length, 0.04F, 0.0F, 0.0F};
  constants.primitive = 2U;
  constants.axis = 2U;

  command_buffer.pushConstants(
    *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
    vk::ArrayProxy<AxesPushConstants const>{1U, &constants});

  // GGEMSAxes.slang: 32 longitudes and 16 latitude divisions, including poles.
  command_buffer.draw(2880U, 1U, 0U, 0U);
}

} // namespace ggems::ui::detail
