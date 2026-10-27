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
 * \brief Renders diagnostic coordinate axes with a row-major clip transform.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
/*!
 * \brief Matches the renderer's four-row World-to-clip push-constant block.
 *
 * The vertex entry point generates six line endpoints in World meters; the
 * fragment entry point forwards interpolated color. The 64-byte push-constant
 * block matches the renderer's four float4 matrix rows.
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
 * \brief Carries homogeneous clip position and display color to rasterization.
 */
struct VertexOutput {
  /*! \brief Homogeneous clip-space position used by Vulkan rasterization. */
  float4 position : SV_Position;
  /*!
   * \brief Interpolated RGB axis color; the fragment shader adds unit alpha.
   */
  float3 color : COLOR0;
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
 * \brief Generates one endpoint of the diagnostic X, Y, or Z axis.
 *
 * \param[in] vertex_id Vertex index in [0, 6), arranged as three line pairs.
 * \return Transformed endpoint and its axis color.
 */
[shader("vertex")]
VertexOutput VertexMain(uint vertex_id : SV_VertexID) {
  float3 positions[6] = {
      // X axis
      float3(-1.0, 0.0, 0.0),
      float3( 1.0, 0.0, 0.0),

      // Y axis
      float3(0.0, -1.0, 0.0),
      float3(0.0,  1.0, 0.0),

      // Z axis
      float3(0.0, 0.0, -1.0),
      float3(0.0, 0.0,  1.0)
  };

  float3 colors[6] = {
      // X axis
      float3(0.95, 0.20, 0.16),
      float3(0.95, 0.20, 0.16),

      // Y axis
      float3(0.20, 0.85, 0.35),
      float3(0.20, 0.85, 0.35),

      // Z axis
      float3(0.25, 0.45, 1.00),
      float3(0.25, 0.45, 1.00)
  };

  VertexOutput output;
  output.position = TransformWorldPosition(positions[vertex_id]);
  output.color = colors[vertex_id];

  return output;
}

/*!
 * \brief Publishes the interpolated diagnostic display color.
 *
 * \param[in] input Rasterized vertex outputs.
 * \return RGBA fragment color; axis fragments use alpha one.
 */
[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  return float4(input.color, 1.0);
}
