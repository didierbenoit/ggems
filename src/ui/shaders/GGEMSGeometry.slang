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
 * \brief Generates one analytic triangle topology for solid and wire geometry.
 *
 * Read-only shape parameters are shared by equal physical shapes. PolygonMode
 * selects filled triangles or their edges; no host position/normal mesh exists.
 * Curved shapes use 64 azimuth sectors and 32 latitude bands for visualization.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/*! \brief Matches the 64-byte immutable GGEMSGeometryShape descriptor. */
struct GeometryShape {
  /*! \brief Kind, azimuth side count, first profile, and profile count. */
  uint4 header;

  /*! \brief Analytic dimensions in meters and authored angles in radians. */
  float4 parameters[3];
};

/*! \brief Matches one 16-byte authored Polyhedra profile and draw prefix. */
struct GeometryProfile {
  /*! \brief Local axial coordinate in meters. */
  float z_m;

  /*! \brief Bore apothem in meters; zero closes the bore at this plane. */
  float inner_apothem_m;

  /*! \brief Positive outer apothem in meters. */
  float outer_apothem_m;

  /*! \brief Lateral triangles per sector preceding this plane. */
  uint first_triangle;
};

/*! \brief Matches the host's 128-byte occurrence push-constant block. */
struct GeometryPushConstants {
  /*! \brief Row 0 mapping local meters to clip coordinates. */
  float4 local_to_clip_row_0;

  /*! \brief Row 1 mapping local meters to clip coordinates. */
  float4 local_to_clip_row_1;

  /*! \brief Row 2 mapping local meters to clip coordinates. */
  float4 local_to_clip_row_2;

  /*! \brief Row 3 mapping local meters to clip coordinates. */
  float4 local_to_clip_row_3;

  /*! \brief Linear-light named or transient selection color. */
  float4 color;

  /*! \brief Row 0 rotating an outward local normal into World. */
  float3 normal_to_world_row_0;

  /*! \brief Shared shape descriptor index for this physical occurrence. */
  uint shape_index;

  /*! \brief Row 1 rotating an outward local normal into World. */
  float3 normal_to_world_row_1;

  /*! \brief Nonzero for opaque filled surfaces; wire colors remain unlit. */
  uint shaded;

  /*! \brief Row 2 rotating an outward local normal into World. */
  float3 normal_to_world_row_2;

  /*! \brief Reserved word keeping the host block at 128 bytes. */
  uint padding;
};

/*! \brief One consistently outward triangle in occurrence-local meters. */
struct GeometryTriangle {
  /*! \brief First oriented corner. */
  float3 first;

  /*! \brief Second oriented corner. */
  float3 second;

  /*! \brief Third oriented corner. */
  float3 third;
};

/*! \brief Carries the generated clip position and opaque lit or unlit color. */
struct GeometryVertexOutput {
  /*! \brief Homogeneous clip position for Vulkan rasterization. */
  float4 position : SV_Position;

  /*! \brief Linear-light display color shared by the triangle's vertices. */
  float4 color : COLOR0;
};

/*! \brief Immutable shared analytic shape parameters. */
[[vk::binding(0, 0)]]
StructuredBuffer<GeometryShape> g_geometry_shapes;

/*! \brief Immutable Polyhedra profiles, addressed only by polygon descriptors.
 */
[[vk::binding(1, 0)]]
StructuredBuffer<GeometryProfile> g_geometry_profiles;

/*! \brief Borrows transforms, color, and shared shape identity for one draw. */
[[vk::push_constant]]
ConstantBuffer<GeometryPushConstants> g_geometry;

/*!
 * \brief Splits an oriented quad along its first-to-third diagonal.
 * \param[in] first First corner in boundary order.
 * \param[in] second Second corner in boundary order.
 * \param[in] third Third corner in boundary order.
 * \param[in] fourth Fourth corner in boundary order.
 * \param[in] triangle Zero or one within the quad.
 * \return One outward triangle.
 */
GeometryTriangle GeometryQuad(float3 first, float3 second, float3 third,
                              float3 fourth, uint triangle) {
  GeometryTriangle result = {first, triangle == 0 ? second : third,
                             triangle == 0 ? third : fourth};
  return result;
}

/*!
 * \brief Evaluates one of eight Box, Trd, or Trap corners.
 * \param[in] shape Analytic dimensions and authored angles.
 * \param[in] corner X sign varies fastest, followed by Y and Z.
 * \return Local-meter corner without host-side vertex construction.
 */
float3 GeometryPlanarCorner(GeometryShape shape, uint corner) {
  float3 signs =
    float3((corner & 1) != 0 ? 1.0 : -1.0, (corner & 2) != 0 ? 1.0 : -1.0,
           (corner & 4) != 0 ? 1.0 : -1.0);
  if (shape.header.x == 0) {
    return signs * shape.parameters[0].xyz;
  }
  uint end = corner / 4;
  float4 face = shape.parameters[end];
  float z = signs.z * shape.parameters[0].w;
  float y = signs.y * face.x;
  float width = signs.y < 0.0 ? face.y : face.z;
  float4 angles = shape.parameters[2];
  float2 tilt = tan(angles.z) * float2(cos(angles.w), sin(angles.w));
  return float3(z * tilt.x + y * tan(angles[end]) + signs.x * width,
                z * tilt.y + y, z);
}

/*!
 * \brief Generates one of twelve triangles enclosing a planar solid.
 * \param[in] shape Box, Trd, or validated planar Trap descriptor.
 * \param[in] triangle Triangle index in the six oriented faces.
 * \return Outward face triangle.
 */
GeometryTriangle GeometryPlanarTriangle(GeometryShape shape, uint triangle) {
  static const uint4 faces[6] = {
    uint4(0, 2, 3, 1), uint4(4, 5, 7, 6), uint4(0, 1, 5, 4),
    uint4(2, 6, 7, 3), uint4(0, 4, 6, 2), uint4(1, 3, 7, 5),
  };
  uint4 face = faces[triangle / 2];
  return GeometryQuad(GeometryPlanarCorner(shape, face.x),
                      GeometryPlanarCorner(shape, face.y),
                      GeometryPlanarCorner(shape, face.z),
                      GeometryPlanarCorner(shape, face.w), triangle % 2);
}

/*!
 * \brief Evaluates one azimuth sample with an exact integer seam wrap.
 * \param[in] radius Local circle or polygon vertex radius in meters.
 * \param[in] z Axial coordinate in meters.
 * \param[in] sector Sample index; the side count wraps to zero.
 * \param[in] sides Azimuth side count.
 * \param[in] start Authored first-vertex angle in radians.
 * \return One local-meter ring point.
 */
float3 GeometryRingPoint(float radius, float z, uint sector, uint sides,
                         float start) {
  float angle = start + 6.283185307179586 * (sector % sides) / sides;
  return float3(radius * cos(angle), radius * sin(angle), z);
}

/*!
 * \brief Joins two rings, emitting one triangle when an end is an apex.
 * \param[in] radii Negative- and positive-end radii in meters.
 * \param[in] axial Matching strictly increasing axial coordinates.
 * \param[in] sector Current azimuth sector.
 * \param[in] sides Azimuth side count.
 * \param[in] start First-vertex angle in radians.
 * \param[in] triangle Index within the one or two nondegenerate triangles.
 * \param[in] inward Whether the face bounds a bore rather than the exterior.
 * \return Oriented wall triangle.
 */
GeometryTriangle GeometryWall(float2 radii, float2 axial, uint sector,
                              uint sides, float start, uint triangle,
                              bool inward) {
  float3 first = GeometryRingPoint(radii.x, axial.x, sector, sides, start);
  float3 second = GeometryRingPoint(radii.x, axial.x, sector + 1, sides, start);
  float3 third = GeometryRingPoint(radii.y, axial.y, sector + 1, sides, start);
  float3 fourth = GeometryRingPoint(radii.y, axial.y, sector, sides, start);
  GeometryTriangle result =
    GeometryQuad(first, second, third, fourth, radii.x == 0.0 ? 1 : triangle);
  if (inward) {
    float3 swap = result.second;
    result.second = result.third;
    result.third = swap;
  }
  return result;
}

/*!
 * \brief Closes one end with an annular pair or a filled fan triangle.
 * \param[in] inner Bore radius in meters; zero selects a filled cap.
 * \param[in] outer Positive exterior radius in meters.
 * \param[in] z Cap's axial coordinate in meters.
 * \param[in] sector Azimuth sector.
 * \param[in] sides Azimuth side count.
 * \param[in] start First-vertex angle in radians.
 * \param[in] triangle Index within the cap's one or two triangles.
 * \param[in] negative Whether the outward normal points toward negative Z.
 * \return Outward cap triangle.
 */
GeometryTriangle GeometryCap(float inner, float outer, float z, uint sector,
                             uint sides, float start, uint triangle,
                             bool negative) {
  GeometryTriangle result = GeometryQuad(
    GeometryRingPoint(inner, z, sector, sides, start),
    GeometryRingPoint(outer, z, sector, sides, start),
    GeometryRingPoint(outer, z, sector + 1, sides, start),
    GeometryRingPoint(inner, z, sector + 1, sides, start), triangle);
  if (negative) {
    float3 swap = result.second;
    result.second = result.third;
    result.third = swap;
  }
  return result;
}

/*!
 * \brief Counts one filled or annular cap, excluding an exterior apex.
 * \param[in] inner Bore radius or apothem.
 * \param[in] outer Exterior radius or apothem.
 * \return Number of cap triangles per sector.
 */
uint GeometryCapCount(float inner, float outer) {
  return outer > 0.0 ? (inner > 0.0 ? 2 : 1) : 0;
}

/*!
 * \brief Generates closed Tubs or Cons walls, bores, and end caps.
 * \param[in] shape Shared circular radii and half length.
 * \param[in] triangle Sector-major nondegenerate triangle index.
 * \return Outward local triangle.
 */
GeometryTriangle GeometryConicalTriangle(GeometryShape shape, uint triangle) {
  float4 radii = shape.parameters[0];
  float2 axial = float2(-shape.parameters[1].x, shape.parameters[1].x);
  uint outer_count = uint(radii.x > 0.0) + uint(radii.y > 0.0);
  uint inner_count = uint(radii.z > 0.0) + uint(radii.w > 0.0);
  uint lower_count = GeometryCapCount(radii.z, radii.x);
  uint count = outer_count + inner_count + lower_count +
               GeometryCapCount(radii.w, radii.y);
  uint sector = triangle / count;
  triangle %= count;
  if (triangle < outer_count) {
    return GeometryWall(radii.xy, axial, sector, shape.header.y, 0.0, triangle,
                        false);
  }
  triangle -= outer_count;
  if (triangle < inner_count) {
    return GeometryWall(radii.zw, axial, sector, shape.header.y, 0.0, triangle,
                        true);
  }
  triangle -= inner_count;
  bool negative = triangle < lower_count;
  return GeometryCap(negative ? radii.z : radii.w, negative ? radii.x : radii.y,
                     negative ? axial.x : axial.y, sector, shape.header.y, 0.0,
                     negative ? triangle : triangle - lower_count, negative);
}

/*!
 * \brief Evaluates a full ellipsoid latitude sample with exact poles.
 * \param[in] axes Positive local semiaxes in meters.
 * \param[in] latitude Ring index from zero through 32.
 * \param[in] sector Azimuth sample index.
 * \param[in] sides Azimuth side count.
 * \return Local-meter point, with no repeated approximate pole ring.
 */
float3 GeometryEllipsoidPoint(float3 axes, uint latitude, uint sector,
                              uint sides) {
  if (latitude == 0 || latitude == 32) {
    return float3(0.0, 0.0, latitude == 0 ? -axes.z : axes.z);
  }
  float angle = -1.5707963267948966 + 3.141592653589793 * latitude / 32;
  float3 point = GeometryRingPoint(cos(angle), sin(angle), sector, sides, 0.0);
  return axes * point;
}

/*!
 * \brief Generates one closed Sphere or Ellipsoid triangle.
 * \param[in] shape Positive semiaxes and azimuth side count.
 * \param[in] triangle Sector-major index with single-triangle polar fans.
 * \return Outward triangle with no degenerate pole facets.
 */
GeometryTriangle GeometryEllipsoidTriangle(GeometryShape shape, uint triangle) {
  uint sector = triangle / 62;
  uint local = triangle % 62;
  uint latitude = local == 0 ? 0 : (local == 61 ? 31 : 1 + (local - 1) / 2);
  uint half = local == 0 ? 1 : (local == 61 ? 0 : (local - 1) % 2);
  float3 axes = shape.parameters[0].xyz;
  return GeometryQuad(
    GeometryEllipsoidPoint(axes, latitude, sector, shape.header.y),
    GeometryEllipsoidPoint(axes, latitude, sector + 1, shape.header.y),
    GeometryEllipsoidPoint(axes, latitude + 1, sector + 1, shape.header.y),
    GeometryEllipsoidPoint(axes, latitude + 1, sector, shape.header.y), half);
}

/*!
 * \brief Generates authored polygon-profile walls and annular end caps.
 * \param[in] shape Polygon side count, profile range, and start angle.
 * \param[in] triangle Sector-major index in the complete closed topology.
 * \return Outward triangle; an interval lookup is logarithmic in profile size.
 */
GeometryTriangle GeometryPolyhedraTriangle(GeometryShape shape, uint triangle) {
  uint first = shape.header.z;
  uint last = first + shape.header.w - 1;
  GeometryProfile lower = g_geometry_profiles[first];
  GeometryProfile upper = g_geometry_profiles[last];
  uint lateral = upper.first_triangle;
  uint lower_cap =
    GeometryCapCount(lower.inner_apothem_m, lower.outer_apothem_m);
  uint count = lateral + lower_cap +
               GeometryCapCount(upper.inner_apothem_m, upper.outer_apothem_m);
  uint sector = triangle / count;
  triangle %= count;
  float radius_scale = 1.0 / cos(3.141592653589793 / shape.header.y);
  float start = shape.parameters[0].x;
  if (triangle >= lateral) {
    bool negative = triangle < lateral + lower_cap;
    GeometryProfile cap = negative ? lower : upper;
    return GeometryCap(
      cap.inner_apothem_m * radius_scale, cap.outer_apothem_m * radius_scale,
      cap.z_m, sector, shape.header.y, start,
      triangle - lateral - (negative ? 0 : lower_cap), negative);
  }
  while (last - first > 1) {
    uint middle = first + (last - first) / 2;
    if (g_geometry_profiles[middle].first_triangle <= triangle) {
      first = middle;
    } else {
      last = middle;
    }
  }
  lower = g_geometry_profiles[first];
  upper = g_geometry_profiles[first + 1];
  triangle -= lower.first_triangle;
  bool inward = triangle >= 2;
  float2 radii = inward ? float2(lower.inner_apothem_m, upper.inner_apothem_m)
                        : float2(lower.outer_apothem_m, upper.outer_apothem_m);
  return GeometryWall(radii * radius_scale, float2(lower.z_m, upper.z_m),
                      sector, shape.header.y, start,
                      inward ? triangle - 2 : triangle, inward);
}

/*!
 * \brief Selects the procedural topology for the eight admitted shape kinds.
 * \param[in] shape Immutable analytic descriptor.
 * \param[in] triangle Index bounded by the host's derived draw count.
 * \return One occurrence-local outward triangle.
 */
GeometryTriangle GeometryShapeTriangle(GeometryShape shape, uint triangle) {
  uint kind = shape.header.x;
  if (kind == 0 || kind == 6 || kind == 7) {
    return GeometryPlanarTriangle(shape, triangle);
  }
  if (kind == 1 || kind == 5) {
    return GeometryEllipsoidTriangle(shape, triangle);
  }
  if (kind == 4) {
    return GeometryPolyhedraTriangle(shape, triangle);
  }
  return GeometryConicalTriangle(shape, triangle);
}

/*!
 * \brief Generates one analytic vertex and applies the occurrence transform.
 * \param[in] vertex_id Vertex index within the shared procedural topology.
 * \return Clip position, authored RGB with optional light, and supplied alpha.
 */
[shader("vertex")] GeometryVertexOutput
VertexMain(uint vertex_id /*! \cond */ : SV_VertexID
           /*! \endcond */) {
  GeometryTriangle triangle = GeometryShapeTriangle(
    g_geometry_shapes[g_geometry.shape_index], vertex_id / 3);
  uint corner = vertex_id % 3;
  float3 point = corner == 0 ? triangle.first
                             : (corner == 1 ? triangle.second : triangle.third);
  float4 position = float4(point, 1.0);
  GeometryVertexOutput output;
  output.position = float4(dot(g_geometry.local_to_clip_row_0, position),
                           dot(g_geometry.local_to_clip_row_1, position),
                           dot(g_geometry.local_to_clip_row_2, position),
                           dot(g_geometry.local_to_clip_row_3, position));
  float shade = 1.0;
  if (g_geometry.shaded != 0) {
    float3 normal =
      cross(triangle.second - triangle.first, triangle.third - triangle.first);
    float scale = max(max(abs(normal.x), abs(normal.y)), abs(normal.z));
    // FP32-collapsed visual facets have no filled coverage; keep their color
    // finite. Scaling before normalization preserves picometer-scale normals.
    normal = scale > 0.0 ? normalize(normal / scale) : float3(0.0, 0.0, 1.0);
    float3 world_normal = float3(dot(g_geometry.normal_to_world_row_0, normal),
                                 dot(g_geometry.normal_to_world_row_1, normal),
                                 dot(g_geometry.normal_to_world_row_2, normal));
    float3 light = normalize(float3(0.5, -0.3, 0.8));
    shade = 0.3 + 0.7 * max(dot(normalize(world_normal), light), 0.0);
  }
  output.color = float4(g_geometry.color.rgb * shade, g_geometry.color.a);
  return output;
}

  /*!
   * \brief Publishes the generated face or wire color and supplied alpha.
   * \param[in] input Rasterized procedural geometry output.
   * \return Linear-light RGBA for blending into the shared scene attachment.
   */
  [shader("fragment")] float4
  FragmentMain(GeometryVertexOutput input) /*! \cond */
    : SV_Target
/*! \endcond */ {
  return input.color;
}
