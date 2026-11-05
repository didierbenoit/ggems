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
 * \brief Draws a filtered physical lattice on one finite World reference plane.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/*! \brief Matches the host transform, palette, World bounds, and spacing. */
struct GridPushConstants {
  /*! \brief Row 0 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_0;

  /*! \brief Row 1 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_1;

  /*! \brief Row 2 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_2;

  /*! \brief Row 3 mapping World meters to clip coordinates. */
  float4 world_to_clip_row_3;

  /*! \brief Linear-light minor line color. */
  float4 minor_color;

  /*! \brief Linear-light major line color. */
  float4 major_color;

  /*! \brief World half extents and shared minor spacing, all in meters. */
  float4 dimensions;

  /*! \brief Converts physical pixel coordinates to normalized viewport space.
   */
  float2 inverse_viewport;

  /*! \brief XY = 0, XZ = 1, YZ = 2. */
  uint plane;
};

/*! \brief Carries the projected World plane into rasterization. */
struct VertexOutput {
  /*! \brief Clip position, becoming physical pixel position in the fragment. */
  float4 position : SV_Position;
};

/*! \brief Blends filtered line coverage using the existing scene depth. */
struct FragmentOutput {
  /*! \brief Linear RGB and antialiased coverage. */
  float4 color : SV_Target;

  /*! \brief Plane depth with a small positive display-priority allowance. */
  float depth : SV_DepthGreaterEqual;
};

/*! \brief Borrows the active reference plane's display parameters. */
[[vk::push_constant]]
ConstantBuffer<GridPushConstants> g_grid;

/*!
 * \brief Generates two triangles spanning the finite World reference plane.
 *
 * \param[in] vertex_id Sequential corner index in the six-vertex draw.
 * \return Shared-camera clip position of one World rectangle corner.
 */
[shader("vertex")] VertexOutput
VertexMain(uint vertex_id /*! \cond */ : SV_VertexID /*! \endcond */) {
  float2 corners[6] = {float2(-1.0, -1.0), float2(1.0, -1.0),
                       float2(1.0, 1.0),   float2(-1.0, -1.0),
                       float2(1.0, 1.0),   float2(-1.0, 1.0)};
  float2 corner = corners[vertex_id];
  float3 world;
  float2 in_plane;

  if (g_grid.plane == 0) {
    in_plane = corner * g_grid.dimensions.xy;
    world = float3(in_plane.x, in_plane.y, 0.0);
  } else if (g_grid.plane == 1) {
    in_plane = corner * g_grid.dimensions.xz;
    world = float3(in_plane.x, 0.0, in_plane.y);
  } else {
    in_plane = corner * g_grid.dimensions.yz;
    world = float3(0.0, in_plane.x, in_plane.y);
  }

  float4 position = float4(world, 1.0);
  VertexOutput output;
  output.position = float4(dot(g_grid.world_to_clip_row_0, position),
                           dot(g_grid.world_to_clip_row_1, position),
                           dot(g_grid.world_to_clip_row_2, position),
                           dot(g_grid.world_to_clip_row_3, position));

  return output;
}

/*!
 * \brief Filters one lattice without changing its physical spacing.
 *
 * \param[in] coordinate Origin-relative coordinates in lattice intervals.
 * \param[in] footprint Intervals per physical pixel in each line-normal
 * direction.
 * \return One-pixel line coverage, faded when lines become too dense.
 */
float GridCoverage(float2 coordinate, float2 footprint) {
  float2 distance = abs(frac(coordinate + 0.5) - 0.5);

  float2 coverage =
    1.0 - smoothstep(0.0, 1.0, distance / max(footprint, 1.0e-6));
  coverage *= 1.0 - smoothstep(0.125, 0.25, footprint);

  return max(coverage.x, coverage.y);
}

/*!
 * \brief Filters minor/major lines and fades unresolved or grazing projections.
 *
 * \param[in] input Physical pixel position and rasterized plane depth.
 * \return Line coverage and depth for alpha blending after scene objects.
 */
[shader("fragment")] FragmentOutput FragmentMain(VertexOutput input) {
  float2 ndc = input.position.xy * g_grid.inverse_viewport - 1.0;
  uint2 axes = g_grid.plane == 0   ? uint2(0, 1)
               : g_grid.plane == 1 ? uint2(0, 2)
                                   : uint2(1, 2);
  float3 homogeneous =
    float3(g_grid.world_to_clip_row_3[axes.x],
           g_grid.world_to_clip_row_3[axes.y], g_grid.world_to_clip_row_3.w);

  float3 horizontal_row =
    float3(g_grid.world_to_clip_row_0[axes.x],
           g_grid.world_to_clip_row_0[axes.y], g_grid.world_to_clip_row_0.w) -
    ndc.x * homogeneous;

  float3 vertical_row =
    float3(g_grid.world_to_clip_row_1[axes.x],
           g_grid.world_to_clip_row_1[axes.y], g_grid.world_to_clip_row_1.w) -
    ndc.y * homogeneous;
  // Solve the two projected plane equations directly at the pixel. This avoids
  // interpolating huge lattice coordinates from clipped World corners at high
  // zoom.
  horizontal_row /= max(abs(horizontal_row.x), abs(horizontal_row.y));
  vertical_row /= max(abs(vertical_row.x), abs(vertical_row.y));

  float determinant =
    horizontal_row.x * vertical_row.y - horizontal_row.y * vertical_row.x;

  float2 coordinate =
    float2(
      horizontal_row.y * vertical_row.z - horizontal_row.z * vertical_row.y,
      horizontal_row.z * vertical_row.x - horizontal_row.x * vertical_row.z) /
    determinant;

  coordinate /= g_grid.dimensions.w;

  float2 horizontal = ddx(coordinate);
  float2 vertical = ddy(coordinate);
  float2 footprint = sqrt(horizontal * horizontal + vertical * vertical);
  float area = abs(horizontal.x * vertical.y - horizontal.y * vertical.x);
  float compression =
    area /
    max(max(dot(horizontal, horizontal), dot(vertical, vertical)), 1.0e-12);
  float grazing = saturate((compression - 0.01) / 0.09);
  float depth_allowance = 9.5367431640625e-7 + 0.5 * fwidth(input.position.z);
  float minor = GridCoverage(coordinate, footprint);
  float major = GridCoverage(coordinate / 5.0, footprint / 5.0);
  float coverage = max(minor, major);
  // Fade when a lattice coordinate's FP32 spacing approaches a physical pixel.
  float2 precision =
    abs(coordinate) * 1.1920928955078125e-7 / max(footprint, 1.0e-6);
  float resolved = 1.0 - smoothstep(0.25, 1.0, max(precision.x, precision.y));
  float opacity = coverage * grazing * resolved;

  if (opacity <= 0.0) {
    discard;
  }

  FragmentOutput output;
  output.color = float4(
    lerp(g_grid.minor_color.rgb, g_grid.major_color.rgb, major / coverage),
    opacity);
  // A wire can interpolate at its line center within this covered half-pixel.
  // The band gives coplanar objects display priority, not a physical tolerance.
  output.depth = input.position.z + depth_allowance;
  return output;
}
