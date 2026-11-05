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
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/ui/detail/GGEMSVulkanCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"

namespace {

/*! \brief Matches the 128-byte GGEMSCartesianGrid.slang vertex push block. */
struct GridPushConstants {
  /*! \brief Rows mapping World meters to clip coordinates. */
  std::array<std::array<float, 4U>, 4U> world_to_clip{};

  /*! \brief Opaque linear-light minor line color. */
  std::array<float, 4U> minor_color{};

  /*! \brief Opaque linear-light major and center line color. */
  std::array<float, 4U> major_color{};

  /*! \brief Along-line minimum, maximum, lattice step, and NDC depth allowance.
   */
  std::array<float, 4U> dimensions{};

  /*! \brief First signed lattice index in the constant-coordinate direction. */
  std::int32_t first_index{};
  /*! \brief XY = 0, XZ = 1, YZ = 2. */
  std::uint32_t plane{};
  /*! \brief Constant first in-plane coordinate = 0, second = 1. */
  std::uint32_t family{};
  /*! \brief Whether another plane already draws this shared center line. */
  std::uint32_t skip_origin{};
};

static_assert(sizeof(GridPushConstants) == 128U);
static_assert(offsetof(GridPushConstants, first_index) == 112U);

/*!
 * \brief Gives coplanar geometry a small, resolution-aware display priority.
 *
 * On a projected plane, NDC depth is affine in NDC x and y. The restricted
 * matrix rows give its two slopes without camera-specific projection math.
 * One 1/256-pixel allowance per screen axis plus a float roundoff floor
 * stabilizes coincident lines. This is a visual priority band, not a bound
 * on implementation-dependent Vulkan line interpolation error.
 *
 * \param[in] matrix Shared World-to-clip transform.
 * \param[in] axes Two World axes spanning the grid plane.
 * \param[in] extent Physical render-target dimensions.
 * \return Positive NDC allowance, or zero for an exactly edge-on plane.
 */
auto GridDepthAllowance(std::array<std::array<float, 4U>, 4U> const &matrix,
                        std::array<std::uint32_t, 2U> const &axes,
                        vk::Extent2D extent) -> float {
  std::array<std::array<double, 3U>, 4U> rows{};
  for (std::size_t row = 0U; row < rows.size(); ++row) {
    rows[row] = {matrix[row][axes[0]], matrix[row][axes[1]], matrix[row][3]};
  }
  auto const determinant = [](std::array<double, 3U> const &first,
                              std::array<double, 3U> const &second,
                              std::array<double, 3U> const &third) -> double {
    return (first[0] * ((second[1] * third[2]) - (second[2] * third[1]))) -
           (first[1] * ((second[0] * third[2]) - (second[2] * third[0]))) +
           (first[2] * ((second[0] * third[1]) - (second[1] * third[0])));
  };
  double const divisor = determinant(rows[0], rows[1], rows[3]);
  if (divisor == 0.0 || extent.width == 0U || extent.height == 0U) {
    return 0.0F;
  }
  double const horizontal = determinant(rows[2], rows[1], rows[3]) / divisor;
  double const vertical = determinant(rows[0], rows[2], rows[3]) / divisor;
  double constexpr subpixel_allowance = 1.0 / 256.0;
  double constexpr roundoff_allowance =
    8.0 * std::numeric_limits<float>::epsilon();
  return static_cast<float>(
    roundoff_allowance +
    (subpixel_allowance * 2.0 *
     ((std::abs(horizontal) / static_cast<double>(extent.width)) +
      (std::abs(vertical) / static_cast<double>(extent.height)))));
}

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

  vk::PushConstantRange grid_push_constant_range{
    .stageFlags = vk::ShaderStageFlagBits::eVertex,
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
  vk::Extent2D viewport_extent, GGEMSCartesianGrid const &grid,
  std::array<float, 4U> const &background) const -> void {
  auto const &planes = grid.GetPlanes();
  std::array<bool, 3U> origin_drawn{};
  bool pipeline_bound{};
  GridPushConstants constants{.world_to_clip = world_to_clip};
  std::array<std::array<std::uint32_t, 2U>, 3U> constexpr plane_axes{
    {
      {0U, 1U},
      {0U, 2U},
      {1U, 2U},
    },
  };

  for (std::uint32_t plane = 0U; plane < planes.size(); ++plane) {
    auto const &lines = planes[plane];
    if (!grid.visible[plane] || lines.opacity <= 0.0F) {
      continue;
    }
    constants.plane = plane;
    float const depth_allowance =
      GridDepthAllowance(world_to_clip, plane_axes[plane], viewport_extent);
    constants.minor_color = colors_[0];
    constants.major_color = colors_[1];
    for (std::uint32_t channel = 0U; channel < 3U; ++channel) {
      constants.minor_color[channel] =
        std::lerp(background[channel], colors_[0][channel], lines.opacity);
      constants.major_color[channel] =
        std::lerp(background[channel], colors_[1][channel], lines.opacity);
    }

    for (std::uint32_t family = 0U; family < 2U; ++family) {
      auto const count = lines.line_count[family];
      if (count == 0U) {
        continue;
      }
      if (!pipeline_bound) {
        command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                    *pipeline_);
        pipeline_bound = true;
      }
      auto const along = 1U - family;
      auto const world_axis = plane_axes[plane][along];
      constants.family = family;
      constants.first_index = lines.first_index[family];
      constants.skip_origin = origin_drawn[world_axis] ? 1U : 0U;
      constants.dimensions = {
        static_cast<float>(lines.minimum_m[along]),
        static_cast<float>(lines.maximum_m[along]),
        static_cast<float>(lines.minor_step_m),
        depth_allowance,
      };
      command_buffer.pushConstants(
        *pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
        vk::ArrayProxy<GridPushConstants const>{1U, &constants});
      command_buffer.draw(2U * count, 1U, 0U, 0U);
      if (constants.first_index <= 0 &&
          constants.first_index + static_cast<std::int32_t>(count) > 0) {
        origin_drawn[world_axis] = true;
      }
    }
  }
}

} // namespace ggems::ui::detail
