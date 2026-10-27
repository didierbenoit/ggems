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
 * \brief Transforms diagnostic trace vertices and forwards display colors.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
/*!
 * \brief Matches the renderer's four-row World-to-clip push-constant block.
 *
 * Vertex positions use World meters, distinct from persistent particle
 * coordinates in picometers. The 64-byte push-constant block supplies four
 * float4 rows of the World-to-clip transform. Fragment output preserves the
 * interpolated input RGBA value.
 */
struct ScenePushConstants {
  /*! \brief Row 0 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_0;
  /*! \brief Row 1 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_1;
  /*! \brief Row 2 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_2;
  /*! \brief Row 3 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_3;
};

/*!
 * \brief Reads one host-packed trace vertex with World position and RGBA
 * color.
 */
struct VertexInput {
  /*! \brief World position in meters prepared by the host trace renderer. */
  float3 position : POSITION0;
  /*! \brief RGBA display color, including alpha. */
  float4 color : COLOR0;
};

/*!
 * \brief Carries homogeneous clip position and display color to rasterization.
 */
struct VertexOutput {
  /*! \brief Homogeneous clip-space position used by Vulkan rasterization. */
  float4 position : SV_Position;
  /*! \brief RGBA display color, including alpha. */
  float4 color : COLOR0;
};

/*! \brief Borrows the host-provided per-draw World-to-clip matrix. */
[[vk::push_constant]]
ConstantBuffer<ScenePushConstants> g_scene;

/*!
 * \brief Transforms a World position using the four uploaded matrix rows.
 *
 * \param[in] world_position World position in meters.
 * \return Homogeneous clip-space position.
 */
float4 TransformWorldPosition(float3 world_position) {
  float4 position = float4(world_position, 1.0);

  return float4(dot(g_scene.world_to_clip_row_0, position),
                dot(g_scene.world_to_clip_row_1, position),
                dot(g_scene.world_to_clip_row_2, position),
                dot(g_scene.world_to_clip_row_3, position));
}

/*!
 * \brief Transforms a trace vertex while preserving its display color.
 *
 * \param[in] input Host-packed World-meter position and RGBA color.
 * \return Clip-space vertex with the unchanged RGBA value.
 */
[shader("vertex")]
VertexOutput VertexMain(VertexInput input) {
  VertexOutput output;
  output.position = TransformWorldPosition(input.position);
  output.color = input.color;
  return output;
}

/*!
 * \brief Publishes the interpolated diagnostic display color.
 *
 * \param[in] input Rasterized vertex outputs.
 * \return Interpolated RGBA fragment color.
 */
[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  return input.color;
}
