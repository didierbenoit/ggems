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
 * \brief Generates the World-origin sphere and positive arrows procedurally.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/*! \brief Matches the host's 104-byte procedural axes constants. */
struct AxesPushConstants {
  /*! \brief Row 0 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_0;
  /*! \brief Row 1 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_1;
  /*! \brief Row 2 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_2;
  /*! \brief Row 3 of the World-meter to clip-coordinate transform. */
  float4 world_to_clip_row_3;
  /*! \brief Linear-light RGB color and unit alpha, at byte 64. */
  float4 color;
  /*! \brief Axis length in meters, relative radius, height, and base Z. */
  float4 dimensions;
  /*! \brief Cylinder 0, cone 1, or sphere 2, at byte 96. */
  uint primitive;
  /*! \brief Global positive X 0, Y 1, or Z 2, at byte 100. */
  uint axis;
};

/*! \brief One procedural surface sample in normalized local coordinates. */
struct SurfaceVertex {
  /*! \brief Local position in units of the reference-axis length. */
  float3 position;
  /*! \brief Outward local surface normal. */
  float3 normal;
};

/*! \brief Carries the surface attributes required for fixed diffuse shading. */
struct VertexOutput {
  /*! \brief Homogeneous clip position used by Vulkan rasterization. */
  float4 position : SV_Position;
  /*! \brief World surface normal, normalized after interpolation. */
  float3 normal : NORMAL0;
  /*! \brief Linear-light surface color. */
  float4 color : COLOR0;
};

/*! \brief Borrows the per-draw camera, palette color, and placement. */
[[vk::push_constant]]
ConstantBuffer<AxesPushConstants> g_axes;

static const uint k_radial_segments = 32;
static const uint k_latitude_segments = 16;
static const float k_pi = 3.14159265358979323846;

/*! \brief Two consistently wound triangles covering one parametric square. */
static const uint2 k_quad_corners[6] = {
  uint2(0, 0), uint2(1, 0), uint2(0, 1), uint2(1, 0), uint2(1, 1), uint2(0, 1),
};

/*! \brief Samples one ring, including half-step cone-tip normals. */
float2 RingDirection(float segment) {
  float angle = (2.0 * k_pi * segment) / float(k_radial_segments);
  return float2(cos(angle), sin(angle));
}

/*! \brief Emits 32 shaft quads; the sphere and cone conceal the open ends. */
SurfaceVertex CylinderVertex(uint vertex_id, float radius, float height,
                             float base_z) {
  uint2 corner = k_quad_corners[vertex_id % 6];
  uint segment = ((vertex_id / 6) + corner.x) % k_radial_segments;
  float2 radial = RingDirection(float(segment));

  SurfaceVertex vertex;

  vertex.position = float3(radius * radial, base_z + height * float(corner.y));
  vertex.normal = float3(radial, 0.0);

  return vertex;
}

/*! \brief Emits 32 cone side triangles and 32 downward cap triangles. */
SurfaceVertex ConeVertex(uint vertex_id, float radius, float height,
                         float base_z) {
  uint triangle = vertex_id / 3;
  uint corner = vertex_id % 3;
  uint segment = triangle % k_radial_segments;

  SurfaceVertex vertex;

  if (triangle < k_radial_segments) {
    bool tip = corner == 2;
    float ring_sample = tip ? float(segment) + 0.5
                            : float((segment + corner) % k_radial_segments);
    float2 radial = RingDirection(ring_sample);
    vertex.position = float3(tip ? float2(0.0) : radius * radial,
                             base_z + (tip ? height : 0.0));
    vertex.normal = normalize(float3(radial, radius / height));
  } else {
    uint rim = (segment + (corner == 1 ? 1 : 0)) % k_radial_segments;
    float2 radial = RingDirection(float(rim));
    vertex.position =
      float3(corner == 0 ? float2(0.0) : radius * radial, base_z);
    vertex.normal = float3(0.0, 0.0, -1.0);
  }

  return vertex;
}

/*! \brief Samples a unit sphere with exact poles and a wrapped azimuth seam. */
float3 SphereDirection(uint latitude, uint segment) {
  if (latitude == 0) {
    return float3(0.0, 0.0, 1.0);
  }

  if (latitude == k_latitude_segments) {
    return float3(0.0, 0.0, -1.0);
  }

  float theta = (k_pi * float(latitude)) / float(k_latitude_segments);

  return float3(sin(theta) * RingDirection(float(segment % k_radial_segments)),
                cos(theta));
}

/*! \brief Emits two pole fans and 14 latitude strips without degeneracy. */
SurfaceVertex SphereVertex(uint vertex_id, float radius) {
  const uint cap_vertices = 3 * k_radial_segments;
  const uint strip_vertices = 6 * k_radial_segments * (k_latitude_segments - 2);
  uint corner = vertex_id % 3;
  uint latitude;
  uint segment;
  if (vertex_id < cap_vertices) {
    latitude = corner == 0 ? 0 : 1;
    segment = (vertex_id / 3) + (corner == 2 ? 1 : 0);
  } else if (vertex_id < cap_vertices + strip_vertices) {
    uint strip_vertex = vertex_id - cap_vertices;
    uint quad = strip_vertex / 6;
    uint2 offset = k_quad_corners[strip_vertex % 6];
    latitude = 1 + (quad / k_radial_segments) + offset.x;
    segment = (quad % k_radial_segments) + offset.y;
  } else {
    latitude = corner == 1 ? k_latitude_segments : k_latitude_segments - 1;
    segment =
      ((vertex_id - cap_vertices - strip_vertices) / 3) + (corner == 2 ? 1 : 0);
  }

  SurfaceVertex vertex;
  vertex.normal = SphereDirection(latitude, segment);
  vertex.position = radius * vertex.normal;

  return vertex;
}

/*! \brief Applies a proper rotation from local +Z to global +X, +Y, or +Z. */
float3 RotateAxis(float3 direction, uint axis) {
  if (axis == 0) {
    return float3(direction.z, direction.y, -direction.x);
  }

  if (axis == 1) {
    return float3(direction.x, direction.z, -direction.y);
  }

  return direction;
}

/*!
 * \brief Generates one triangle-list vertex without vertex or index buffers.
 *
 * \param[in] vertex_id Draw-local index in [0, 192) or [0, 2880) for a sphere.
 * \return Clip position, outward World normal, and linear-light surface color.
 */
[shader("vertex")]
VertexOutput VertexMain(uint vertex_id: SV_VertexID) {
  SurfaceVertex vertex;

  if (g_axes.primitive == 0) {
    vertex = CylinderVertex(vertex_id, g_axes.dimensions.y, g_axes.dimensions.z,
                            g_axes.dimensions.w);
  } else if (g_axes.primitive == 1) {
    vertex = ConeVertex(vertex_id, g_axes.dimensions.y, g_axes.dimensions.z,
                        g_axes.dimensions.w);
  } else {
    vertex = SphereVertex(vertex_id, g_axes.dimensions.y);
  }

  float4 world =
    float4(g_axes.dimensions.x * RotateAxis(vertex.position, g_axes.axis), 1.0);

  VertexOutput output;
  output.position = float4(dot(g_axes.world_to_clip_row_0, world),
                           dot(g_axes.world_to_clip_row_1, world),
                           dot(g_axes.world_to_clip_row_2, world),
                           dot(g_axes.world_to_clip_row_3, world));
  output.normal = RotateAxis(vertex.normal, g_axes.axis);
  output.color = g_axes.color;

  return output;
}

/*!
 * \brief Preserves the mild sphere and arrow depth cues in linear light.
 *
 * \param[in] input Rasterized position, normal, and linear-light color.
 * \return Opaque shaded RGBA color for the existing scene target.
 */
[shader("fragment")]
float4 FragmentMain(VertexOutput input) : SV_Target {
  float3 normal = normalize(input.normal);
  float3 light = normalize(float3(0.45, 0.70, 1.0));
  float shade = 0.42 + (0.58 * saturate(dot(normal, light)));
  return float4(input.color.rgb * shade, input.color.a);
}
