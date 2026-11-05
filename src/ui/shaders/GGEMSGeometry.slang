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
 * \brief Transforms shared geometry edges with each physical occurrence pose.
 *
 * Documentation guards around the fragment semantic support Doxygen's C++
 * parser. The shader and source listing retain the complete signature.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/*! \brief Matches the host's 80-byte geometry push-constant block. */
struct GeometryPushConstants {
  /*! \brief Row 0 of the occurrence's local-meter to clip transform. */
  float4 local_to_clip_row_0;

  /*! \brief Row 1 of the occurrence's local-meter to clip transform. */
  float4 local_to_clip_row_1;

  /*! \brief Row 2 of the occurrence's local-meter to clip transform. */
  float4 local_to_clip_row_2;

  /*! \brief Row 3 of the occurrence's local-meter to clip transform. */
  float4 local_to_clip_row_3;

  /*! \brief Linear-light RGBA applied to every edge of this occurrence. */
  float4 color;
};

/*! \brief Reads one local position in meters from the shared edge buffer. */
struct VertexInput {
  /*! \brief One shared wireframe endpoint in occurrence-local meters. */
  float3 position : POSITION0;
};

/*! \brief Carries clip position and linear RGBA to the fragment stage. */
struct VertexOutput {
  /*! \brief Homogeneous clip position used by Vulkan rasterization. */
  float4 position : SV_Position;

  /*! \brief Linear-light RGBA forwarded to the fragment stage. */
  float4 color : COLOR0;
};

/*! \brief Borrows the occurrence's local-to-clip matrix and display color. */
[[vk::push_constant]]
ConstantBuffer<GeometryPushConstants> g_geometry;

/*!
 * \brief Transforms one shared edge endpoint using the occurrence placement.
 *
 * \param[in] input Wireframe endpoint in occurrence-local meters.
 * \return Homogeneous clip position and the occurrence's linear-light RGBA.
 */
[shader("vertex")] VertexOutput VertexMain(VertexInput input) {
  float4 position = float4(input.position, 1.0);
  VertexOutput output;
  output.position = float4(dot(g_geometry.local_to_clip_row_0, position),
                           dot(g_geometry.local_to_clip_row_1, position),
                           dot(g_geometry.local_to_clip_row_2, position),
                           dot(g_geometry.local_to_clip_row_3, position));
  output.color = g_geometry.color;
  return output;
}

  /*!
   * \brief Publishes the interpolated geometry display color.
   *
   * \param[in] input Rasterized clip position and linear-light RGBA.
   * \return Interpolated RGBA for the common scene color attachment.
   */
  [shader("fragment")] float4 FragmentMain(VertexOutput input) /*! \cond */
    : SV_Target /*! \endcond */ {
  return input.color;
}
