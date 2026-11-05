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
 * \brief Tessellates analytical outlines once for read-only visualization.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <variant>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace {

using namespace ggems::geometry;
using ggems::render::GGEMSGeometryVertex;

/*! \brief Packed line-list endpoints in local meters. */
using Vertices = std::vector<GGEMSGeometryVertex>;

/*! \brief One local Cartesian position in meters before float conversion. */
using Point = std::array<double, 3>;

/*! \brief Converts canonical picometers to render meters. */
constexpr double k_pm_to_m{1.0e-12};

/*! \brief Number of segments in a visualization circle. */
constexpr std::uint32_t k_circle_segments{64U};

/*!
 * \brief Appends two local-meter endpoints without introducing diagonals.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] first First endpoint in local meters.
 * \param[in] second Second endpoint in local meters.
 */
auto Line(Vertices &vertices, Point first, Point second) -> void {
  vertices.push_back({
    .x = static_cast<float>(first[0]),
    .y = static_cast<float>(first[1]),
    .z = static_cast<float>(first[2]),
  });

  vertices.push_back({
    .x = static_cast<float>(second[0]),
    .y = static_cast<float>(second[1]),
    .z = static_cast<float>(second[2]),
  });
}

/*!
 * \brief Connects the corners of two rectangular or trapezoidal ends.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] corners Local-meter corners: four per end, X varying before Y.
 */
auto PlanarEdges(Vertices &vertices, std::array<Point, 8> const &corners)
  -> void {
  for (std::size_t end = 0; end < 2; ++end) {
    auto const base = 4 * end;
    Line(vertices, corners[base], corners[base + 1]);
    Line(vertices, corners[base + 1], corners[base + 3]);
    Line(vertices, corners[base + 3], corners[base + 2]);
    Line(vertices, corners[base + 2], corners[base]);
  }

  for (std::size_t corner = 0; corner < 4; ++corner) {
    Line(vertices, corners[corner], corners[corner + 4]);
  }
}

/*!
 * \brief Builds the twelve physical edges of a centered rectangular solid.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] half Half extents along the local axes in picometers.
 */
auto BoxEdges(Vertices &vertices, HalfExtent3PM half) -> void {
  std::array<Point, 8> corners{};

  for (std::size_t index = 0; index < corners.size(); ++index) {
    corners[index] = {
      (index % 2 == 0 ? -1.0 : 1.0) * static_cast<double>(half.x) * k_pm_to_m,
      (index % 4 < 2 ? -1.0 : 1.0) * static_cast<double>(half.y) * k_pm_to_m,
      (index < 4 ? -1.0 : 1.0) * static_cast<double>(half.z) * k_pm_to_m,
    };
  }

  PlanarEdges(vertices, corners);
}

/*!
 * \brief Returns one circle point in local meters.
 *
 * \param[in] radius Circle radius in meters.
 * \param[in] z Local Z coordinate of the circle in meters.
 * \param[in] angle Azimuth from local +X toward +Y in radians.
 * \return Local-meter position on the circle.
 */
auto RingPoint(double radius, double z, double angle) -> Point {
  return {radius * std::cos(angle), radius * std::sin(angle), z};
}

/*!
 * \brief Tessellates one ring; zero-radius endpoints are apexes, not rings.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] radius Ring radius in meters; zero appends no edges.
 * \param[in] z Local Z coordinate in meters.
 * \param[in] sides Number of evenly spaced edges, at least three.
 * \param[in] start Azimuth of the first vertex in radians.
 */
auto Ring(Vertices &vertices, double radius, double z,
          std::uint32_t sides = k_circle_segments, double start = 0.0) -> void {
  if (radius == 0.0) {
    return;
  }

  auto const step = 2.0 * std::numbers::pi / sides;

  for (std::uint32_t side = 0; side < sides; ++side) {
    Line(vertices, RingPoint(radius, z, start + (side * step)),
         RingPoint(radius, z, start + (((side + 1U) % sides) * step)));
  }
}

/*!
 * \brief Draws axial generatrices, including a possible closing apex.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] first_radius First ring radius in meters; zero denotes an apex.
 * \param[in] first_z First ring's local Z coordinate in meters.
 * \param[in] second_radius Second ring radius in meters; zero denotes an apex.
 * \param[in] second_z Second ring's local Z coordinate in meters.
 * \param[in] sides Number of matching azimuth samples, at least three.
 * \param[in] start Azimuth of the first sample in radians.
 */
auto ConnectRings(Vertices &vertices, double first_radius, double first_z,
                  double second_radius, double second_z, std::uint32_t sides,
                  double start = 0.0) -> void {
  if (first_radius == 0.0 && second_radius == 0.0) {
    return;
  }

  auto const step = 2.0 * std::numbers::pi / sides;

  for (std::uint32_t side = 0; side < sides; ++side) {
    auto const angle = start + (side * step);
    Line(vertices, RingPoint(first_radius, first_z, angle),
         RingPoint(second_radius, second_z, angle));
  }
}

/*!
 * \brief Draws latitude rings and meridians of a complete ellipsoid.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] axes Local X, Y, and Z semiaxes in picometers.
 */
auto EllipsoidEdges(Vertices &vertices, HalfExtent3PM axes) -> void {
  auto const point = [axes](double latitude, double longitude) -> Point {
    return {
      static_cast<double>(axes.x) * k_pm_to_m * std::cos(latitude) *
        std::cos(longitude),
      static_cast<double>(axes.y) * k_pm_to_m * std::cos(latitude) *
        std::sin(longitude),
      static_cast<double>(axes.z) * k_pm_to_m * std::sin(latitude),
    };
  };

  auto const step = 2.0 * std::numbers::pi / k_circle_segments;
  for (int ring = -2; ring <= 2; ++ring) {
    auto const latitude = ring * std::numbers::pi / 6.0;

    for (std::uint32_t side = 0; side < k_circle_segments; ++side) {
      Line(vertices, point(latitude, side * step),
           point(latitude, ((side + 1U) % k_circle_segments) * step));
    }
  }

  for (std::uint32_t meridian = 0; meridian < 8U; ++meridian) {
    auto const longitude = meridian * std::numbers::pi / 4.0;

    for (std::uint32_t side = 0; side < k_circle_segments / 2U; ++side) {
      auto const latitude = (-std::numbers::pi / 2.0) + (side * step);
      Line(vertices, point(latitude, longitude),
           point(latitude + step, longitude));
    }
  }
}

/*!
 * \brief Draws both walls and end rims of a tube or conical frustum.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] half_length Half length along local Z in meters.
 * \param[in] outer Outer radii in meters at negative and positive Z.
 * \param[in] inner Bore radii in meters at negative and positive Z; zero
 * closes.
 */
auto ConicalEdges(Vertices &vertices, double half_length,
                  std::array<double, 2> outer, std::array<double, 2> inner)
  -> void {
  for (auto const &radii : {outer, inner}) {
    Ring(vertices, radii[0], -half_length);
    Ring(vertices, radii[1], half_length);
    ConnectRings(vertices, radii[0], -half_length, radii[1], half_length, 8U);
  }
}

/*!
 * \brief Draws actual polygon profile rings and longitudinal polygon edges.
 *
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] shape Authored polygon profile; picometer apothems are converted
 * to vertex radii, and the start angle locates each ring's first vertex.
 */
auto PolyhedraEdges(Vertices &vertices, GGEMSPolyhedra const &shape) -> void {
  auto const sides = shape.GetSideCount();

  auto const start =
    static_cast<double>(ggems::units::ToRadians(shape.GetStartAngle()));

  auto const vertex_scale = k_pm_to_m / std::cos(std::numbers::pi / sides);

  auto const profile = shape.GetZProfilePM();

  for (std::size_t plane = 0; plane < profile.size(); ++plane) {
    auto const &current = profile[plane];
    auto const z = static_cast<double>(current.z_pm) * k_pm_to_m;

    auto const outer =
      static_cast<double>(current.outer_radius_pm) * vertex_scale;

    auto const inner =
      static_cast<double>(current.inner_radius_pm) * vertex_scale;

    Ring(vertices, outer, z, sides, start);
    Ring(vertices, inner, z, sides, start);

    if (plane != 0U) {
      auto const &previous = profile[plane - 1U];
      auto const previous_z = static_cast<double>(previous.z_pm) * k_pm_to_m;

      ConnectRings(vertices,
                   static_cast<double>(previous.outer_radius_pm) * vertex_scale,
                   previous_z, outer, z, sides, start);

      ConnectRings(vertices,
                   static_cast<double>(previous.inner_radius_pm) * vertex_scale,
                   previous_z, inner, z, sides, start);
    }
  }
}

/*!
 * \brief Builds exact planar corners using canonical dimensions and angles.
 *
 * \tparam Shape GGEMSTrd or GGEMSTrap.
 * \param[in,out] vertices Line-list endpoints to extend.
 * \param[in] shape Authored solid with picometer dimensions and local angles.
 */
template <typename Shape>
auto TrapezoidEdges(Vertices &vertices, Shape const &shape) -> void {
  std::array<Point, 8> corners{};

  auto const half_length =
    static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m;

  double tilt_x = 0.0;
  double tilt_y = 0.0;

  if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
    auto const theta =
      static_cast<double>(ggems::units::ToRadians(shape.GetInclinationTheta()));
    auto const phi =
      static_cast<double>(ggems::units::ToRadians(shape.GetInclinationPhi()));
    tilt_x = std::tan(theta) * std::cos(phi);
    tilt_y = std::tan(theta) * std::sin(phi);
  }

  for (std::size_t end = 0; end < 2; ++end) {
    auto const half = end == 0 ? shape.GetNegativeZHalfSizePM()
                               : shape.GetPositiveZHalfSizePM();
    auto const z = end == 0 ? -half_length : half_length;
    double skew = 0.0;

    if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
      skew = std::tan(static_cast<double>(ggems::units::ToRadians(
        end == 0 ? shape.GetNegativeZSkew() : shape.GetPositiveZSkew())));
    }

    for (std::size_t side_y = 0; side_y < 2; ++side_y) {
      auto const height = std::is_same_v<Shape, GGEMSTrap> ? half[0] : half[1];

      auto const local_y =
        (side_y == 0 ? -1.0 : 1.0) * static_cast<double>(height) * k_pm_to_m;

      auto const width = [&] -> DistancePM {
        if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
          return half[1 + side_y];
        } else {
          return half[0];
        }
      }();

      for (std::size_t side_x = 0; side_x < 2; ++side_x) {
        corners[(4 * end) + (2 * side_y) + side_x] = {
          (z * tilt_x) + (local_y * skew) +
            ((side_x == 0 ? -1.0 : 1.0) * static_cast<double>(width) *
             k_pm_to_m),
          (z * tilt_y) + local_y,
          z,
        };
      }
    }
  }

  PlanarEdges(vertices, corners);
}

/*!
 * \brief Appends only the selected analytical family's wire representation.
 *
 * \param[in,out] vertices Line-list endpoints to extend in local meters.
 * \param[in] volume Validated shape with canonical authored dimensions.
 */
auto ShapeEdges(Vertices &vertices, GGEMSWorld::Volume const &volume) -> void {
  std::visit(
    [&vertices](auto const &shape) -> void {
      using Shape = std::decay_t<decltype(shape)>;

      if constexpr (std::is_same_v<Shape, GGEMSBox>) {
        BoxEdges(vertices, shape.GetHalfExtentPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
        auto const radius = shape.GetRadiusPM();
        EllipsoidEdges(vertices, {.x = radius, .y = radius, .z = radius});
      } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
        EllipsoidEdges(vertices, shape.GetSemiAxesPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
        auto const outer =
          static_cast<double>(shape.GetOuterRadiusPM()) * k_pm_to_m;

        auto const inner =
          static_cast<double>(shape.GetInnerRadiusPM()) * k_pm_to_m;

        ConicalEdges(vertices,
                     static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m,
                     {outer, outer}, {inner, inner});
      } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
        ConicalEdges(
          vertices, static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m,
          {
            static_cast<double>(shape.GetNegativeZOuterRadiusPM()) * k_pm_to_m,
            static_cast<double>(shape.GetPositiveZOuterRadiusPM()) * k_pm_to_m,
          },
          {
            static_cast<double>(shape.GetNegativeZInnerRadiusPM()) * k_pm_to_m,
            static_cast<double>(shape.GetPositiveZInnerRadiusPM()) * k_pm_to_m,
          });
      } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
        PolyhedraEdges(vertices, shape);
      } else {
        TrapezoidEdges(vertices, shape);
      }
    },
    volume);
}

/*!
 * \brief Compares exact dimensions only, excluding Material and placement.
 *
 * \param[in] first First authored shape to compare.
 * \param[in] second Second authored shape to compare.
 * \return True for the same shape family and equal geometric parameters.
 */
auto SameShape(GGEMSWorld::Volume const &first,
               GGEMSWorld::Volume const &second) -> bool {
  return std::visit(
    [&second](auto const &shape) -> bool {
      using Shape = std::decay_t<decltype(shape)>;
      auto const *other = std::get_if<Shape>(&second);
      if (other == nullptr) {
        return false;
      }
      if constexpr (std::is_same_v<Shape, GGEMSBox>) {
        return shape.GetHalfExtentPM() == other->GetHalfExtentPM();
      } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
        return shape.GetRadiusPM() == other->GetRadiusPM();
      } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
        return shape.GetSemiAxesPM() == other->GetSemiAxesPM();
      } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
        return shape.GetOuterRadiusPM() == other->GetOuterRadiusPM() &&
               shape.GetInnerRadiusPM() == other->GetInnerRadiusPM() &&
               shape.GetHalfLengthPM() == other->GetHalfLengthPM();
      } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
        return shape.GetNegativeZOuterRadiusPM() ==
                 other->GetNegativeZOuterRadiusPM() &&
               shape.GetPositiveZOuterRadiusPM() ==
                 other->GetPositiveZOuterRadiusPM() &&
               shape.GetNegativeZInnerRadiusPM() ==
                 other->GetNegativeZInnerRadiusPM() &&
               shape.GetPositiveZInnerRadiusPM() ==
                 other->GetPositiveZInnerRadiusPM() &&
               shape.GetHalfLengthPM() == other->GetHalfLengthPM();
      } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
        return shape.GetSideCount() == other->GetSideCount() &&
               shape.GetStartAngle() == other->GetStartAngle() &&
               std::ranges::equal(
                 shape.GetZProfilePM(), other->GetZProfilePM(),
                 [](auto const &left, auto const &right) -> bool {
                   return left.z_pm == right.z_pm &&
                          left.inner_radius_pm == right.inner_radius_pm &&
                          left.outer_radius_pm == right.outer_radius_pm;
                 });
      } else {
        auto const dimensions_match =
          shape.GetNegativeZHalfSizePM() == other->GetNegativeZHalfSizePM() &&
          shape.GetPositiveZHalfSizePM() == other->GetPositiveZHalfSizePM() &&
          shape.GetHalfLengthPM() == other->GetHalfLengthPM();
        if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
          return dimensions_match &&
                 shape.GetNegativeZSkew() == other->GetNegativeZSkew() &&
                 shape.GetPositiveZSkew() == other->GetPositiveZSkew() &&
                 shape.GetInclinationTheta() == other->GetInclinationTheta() &&
                 shape.GetInclinationPhi() == other->GetInclinationPhi();
        } else {
          return dimensions_match;
        }
      }
    },
    first);
}

/*!
 * \brief Converts a composed host pose into a row-major meter transform.
 *
 * \param[in] pose Local-to-World rotation and World translation in picometers.
 * \return Row-major homogeneous local-to-World matrix for meter coordinates.
 */
auto Transform(GGEMSWorld::Pose const &pose) -> std::array<float, 16> {
  auto const &rotation = pose.rotation;
  return {
    static_cast<float>(rotation.axis_x.x),
    static_cast<float>(rotation.axis_y.x),
    static_cast<float>(rotation.axis_z.x),
    static_cast<float>(pose.position_pm[0] * k_pm_to_m),
    static_cast<float>(rotation.axis_x.y),
    static_cast<float>(rotation.axis_y.y),
    static_cast<float>(rotation.axis_z.y),
    static_cast<float>(pose.position_pm[1] * k_pm_to_m),
    static_cast<float>(rotation.axis_x.z),
    static_cast<float>(rotation.axis_y.z),
    static_cast<float>(rotation.axis_z.z),
    static_cast<float>(pose.position_pm[2] * k_pm_to_m),
    0.0F,
    0.0F,
    0.0F,
    1.0F,
  };
}

} // namespace

namespace ggems::render {

auto BuildGeometryScene(geometry::GGEMSWorld const &world)
  -> GGEMSGeometryScene {
  GGEMSGeometryScene scene;
  auto const half = world.GetHalfExtentPM();
  scene.world_half_extent_m = {
    static_cast<double>(half.x) * k_pm_to_m,
    static_cast<double>(half.y) * k_pm_to_m,
    static_cast<double>(half.z) * k_pm_to_m,
  };
  BoxEdges(scene.vertices, half);
  scene.meshes.push_back({.first_vertex = 0U, .vertex_count = 24U});
  scene.occurrences.reserve(world.GetVolumeCount() + 1U);
  scene.occurrences.push_back({
    .volume_id = 0U,
    .parent_volume_id = 0U,
    .mesh_index = 0U,
    .local_to_world = Transform({.position_pm = {}, .rotation = {}}),
  });

  auto const links = world.BuildOccurrenceRecords();
  std::vector<std::size_t> mesh_sources;
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    auto const &shape = world.GetVolume(index);
    auto const found =
      std::ranges::find_if(mesh_sources, [&](std::size_t source) -> bool {
        return SameShape(shape, world.GetVolume(source));
      });
    auto const mesh_index =
      static_cast<std::uint32_t>(found - mesh_sources.begin()) + 1U;
    if (found == mesh_sources.end()) {
      auto const first = scene.vertices.size();
      ShapeEdges(scene.vertices, shape);
      if (scene.vertices.size() > std::numeric_limits<std::uint32_t>::max()) {
        throw core::GGEMSRecoverable(
          "Geometry wireframe exceeds the Vulkan vertex range.");
      }
      scene.meshes.push_back({
        .first_vertex = static_cast<std::uint32_t>(first),
        .vertex_count =
          static_cast<std::uint32_t>(scene.vertices.size() - first),
      });
      mesh_sources.push_back(index);
    }
    scene.occurrences.push_back({
      .volume_id = static_cast<std::uint32_t>(index) + 1U,
      .parent_volume_id = links[index].parent_volume_id,
      .mesh_index = mesh_index,
      .local_to_world = Transform(world.BuildVolumePose(index)),
    });
  }
  return scene;
}

} // namespace ggems::render
