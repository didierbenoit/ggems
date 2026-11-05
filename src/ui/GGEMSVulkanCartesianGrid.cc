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
 * \brief Owns the bounded procedural Cartesian grid pipeline and its draws.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <filesystem>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/ui/detail/GGEMSVulkanCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

/*!
 * \brief Matches the shader's World transform, palette, extent, and spacing.
 */
struct GridPushConstants {
  /*! \brief Rows mapping World meters to clip coordinates. */
  std::array<std::array<float, 4U>, 4U> world_to_clip{};

  /*! \brief Linear-light minor line color. */
  std::array<float, 4U> minor_color{};

  /*! \brief Linear-light major line color. */
  std::array<float, 4U> major_color{};

  /*! \brief World half extents and shared minor spacing, all in meters. */
  std::array<float, 4U> dimensions{};

  /*!
   * \brief Converts physical pixel coordinates to normalized viewport space.
   */
  std::array<float, 2U> inverse_viewport{};

  /*! \brief XY = 0, XZ = 1, YZ = 2. */
  std::uint32_t plane{};
};

} // namespace

namespace ggems::ui::detail {

auto GGEMSVulkanCartesianGrid::Initialize(vk::raii::Device const &device,
                                          vk::Format color_format,
                                          vk::Format depth_format) -> void {
  std::filesystem::path const shader_directory{GGEMS_UI_SHADER_DIRECTORY};

  auto const vertex_code =
    ReadSPIRVFile(shader_directory / "GGEMSCartesianGrid.vert.spv");

  auto const fragment_code =
    ReadSPIRVFile(shader_directory / "GGEMSCartesianGrid.frag.spv");

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
    .depthWriteEnable = vk::False,
    .depthCompareOp = vk::CompareOp::eLessOrEqual,
    .depthBoundsTestEnable = vk::False,
    .stencilTestEnable = vk::False,
    .minDepthBounds = 0.0F,
    .maxDepthBounds = 1.0F,
  };

  vk::PushConstantRange grid_push_constant_range{
    .stageFlags =
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
    .offset = 0U,
    .size = sizeof(GridPushConstants),
  };

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
    .pushConstantRangeCount = 1U,
    .pPushConstantRanges = &grid_push_constant_range,
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
    ToVulkanClearColor(GetThemeColorKey(GGEMSThemeRole::Separator)),
    ToVulkanClearColor(GetThemeColorKey(GGEMSThemeRole::MutedText)),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCartesianGrid::Shutdown() noexcept -> void {
  pipeline_ = nullptr;
  pipeline_layout_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCartesianGrid::Record(
  vk::raii::CommandBuffer const &command_buffer,
  std::array<std::array<float, 4U>, 4U> const &world_to_clip,
  vk::Extent2D viewport_extent,
  std::array<double, 3U> const &world_half_extent_m,
  GGEMSCartesianGrid const &grid) const -> void {
  if (grid.visible == std::array<bool, 3U>{} || grid.GetMinorStepM() <= 0.0F) {
    return;
  }

  GridPushConstants constants{
    .world_to_clip = world_to_clip,
    .minor_color = colors_[0],
    .major_color = colors_[1],
    .dimensions =
      {
        static_cast<float>(world_half_extent_m[0]),
        static_cast<float>(world_half_extent_m[1]),
        static_cast<float>(world_half_extent_m[2]),
        grid.GetMinorStepM(),
      },
    .inverse_viewport =
      {
        2.0F / static_cast<float>(viewport_extent.width),
        2.0F / static_cast<float>(viewport_extent.height),
      },
  };

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);

  for (std::uint32_t plane = 0U; plane < grid.visible.size(); ++plane) {
    if (!grid.visible[plane]) {
      continue;
    }
    constants.plane = plane;
    command_buffer.pushConstants(
      *pipeline_layout_,
      vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0U,
      vk::ArrayProxy<GridPushConstants const>{1U, &constants});
    command_buffer.draw(6U, 1U, 0U, 0U);
  }
}

} // namespace ggems::ui::detail
