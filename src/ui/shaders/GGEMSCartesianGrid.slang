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
 * \brief Generates bounded, origin-anchored Cartesian grid segments.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/*! \brief Matches the host's 128-byte grid vertex push-constant block. */
struct GridPushConstants {
  /*! \brief Row 0 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_0;
  /*! \brief Row 1 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_1;
  /*! \brief Row 2 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_2;
  /*! \brief Row 3 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_3;
  /*! \brief Opaque linear-light minor line color. */
  float4 minor_color;
  /*! \brief Opaque linear-light major and center line color. */
  float4 major_color;
  /*! \brief Along-line minimum, maximum, lattice step, and NDC depth allowance. */
  float4 dimensions;
  /*! \brief First signed index in the constant-coordinate direction. */
  int first_index;
  /*! \brief XY = 0, XZ = 1, YZ = 2. */
  uint plane;
  /*! \brief Constant first in-plane coordinate = 0, second = 1. */
  uint family;
  /*! \brief Nonzero suppresses a center line already emitted by another plane.
   */
  uint skip_origin;
};

/*! \brief Carries homogeneous clip position and opaque linear color. */
struct VertexOutput {
  /*! \brief Homogeneous position for the common positive-height viewport. */
  float4 position : SV_Position;
  /*! \brief Same line color at both endpoints. */
  float4 color : COLOR0;
};

/*! \brief Borrows one family of the bounded physical grid description. */
[[vk::push_constant]]
ConstantBuffer<GridPushConstants> g_grid;

/*!
 * \brief Generates one endpoint at an integer multiple of the physical step.
 *
 * \param[in] vertex_id Sequential endpoint index, two vertices per line.
 * \return World-anchored clip position and minor or major display color.
 */
[shader("vertex")] VertexOutput
VertexMain(uint vertex_id /*! \cond */ : SV_VertexID /*! \endcond */) {
  int index = g_grid.first_index + int(vertex_id / 2);
  VertexOutput output;
  output.color = index % 5 == 0 ? g_grid.major_color : g_grid.minor_color;
  if (index == 0 && g_grid.skip_origin != 0) {
    output.position = float4(0.0, 0.0, 2.0, 1.0);
    return output;
  }

  float coordinate = float(index) * g_grid.dimensions.z;
  float along = vertex_id % 2 == 0 ? g_grid.dimensions.x : g_grid.dimensions.y;
  float2 in_plane =
    g_grid.family == 0 ? float2(coordinate, along) : float2(along, coordinate);
  float3 world;
  if (g_grid.plane == 0) {
    world = float3(in_plane.x, in_plane.y, 0.0);
  } else if (g_grid.plane == 1) {
    world = float3(in_plane.x, 0.0, in_plane.y);
  } else {
    world = float3(0.0, in_plane.x, in_plane.y);
  }
  float4 position = float4(world, 1.0);
  output.position = float4(dot(g_grid.world_to_clip_row_0, position),
                           dot(g_grid.world_to_clip_row_1, position),
                           dot(g_grid.world_to_clip_row_2, position),
                           dot(g_grid.world_to_clip_row_3, position));
  output.position.z += g_grid.dimensions.w * output.position.w;
  return output;
}

/*!
 * \brief Publishes the line color without overlays, labels, or depth overrides.
 *
 * \param[in] input Rasterized grid position and color.
 * \return Opaque linear RGBA in the common scene attachment.
 */
[shader("fragment")] float4 FragmentMain(VertexOutput input) /*! \cond */
  : SV_Target /*! \endcond */ {
  return input.color;
}
