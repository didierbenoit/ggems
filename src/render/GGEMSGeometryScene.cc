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
 * \brief Packs compact shape and voxel-slice descriptors for visualization.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <numbers>
#include <type_traits>
#include <variant>
#include <utility>
#include <vector>
#include <set>
#include <optional>

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
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
#include "GGEMS/geometry/detail/GGEMSVolumeRecipe.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"

namespace {

using namespace ggems::geometry;
using ggems::render::GGEMSGeometryKind;
using ggems::render::GGEMSGeometryMesh;
using ggems::render::GGEMSGeometryOccurrence;
using ggems::render::GGEMSGeometryProfile;
using ggems::render::GGEMSGeometryProperty;
using ggems::render::GGEMSGeometryScene;
using ggems::render::GGEMSGeometryVoxelSlices;

/*! \brief Converts canonical picometers to render meters. */
constexpr double k_pm_to_m{1.0e-12};

/*! \brief Fixed azimuth subdivision shared with the procedural shader. */
constexpr std::uint32_t k_circle_segments{64U};

/*! \brief Fixed latitude subdivision shared with the procedural shader. */
constexpr std::uint32_t k_latitude_bands{32U};

/*!
 * \brief Checks the complete procedural draw before multiplying its factors.
 * \param[in] triangles_per_sector Number of triangles in one azimuth sector.
 * \param[in] sides Positive number of azimuth sectors.
 * \return Number of triangle vertices for one nonindexed Vulkan draw.
 */
auto VertexCount(std::uint32_t triangles_per_sector, std::uint32_t sides)
  -> std::uint32_t {
  return 3U * triangles_per_sector * sides;
}

/*!
 * \brief Counts nondegenerate triangles in one full conical end sector.
 * \param[in] outer Outer radius; zero denotes an apex.
 * \param[in] inner Inner radius; zero denotes a filled disk.
 * \return Zero, one disk triangle, or two annular triangles.
 */
auto CapTriangles(float outer, float inner) -> std::uint32_t {
  if (outer == 0.0F) {
    return 0U;
  }
  return inner > 0.0F ? 2U : 1U;
}

/*!
 * \brief Packs a box without calculating any visual corner positions.
 * \param[in] half Authored canonical half extents.
 * \return Box descriptor with six triangulated faces.
 */
auto BoxDescriptor(HalfExtent3PM half) -> GGEMSGeometryMesh {
  GGEMSGeometryMesh mesh{};
  mesh.shape.header[0] = static_cast<std::uint32_t>(GGEMSGeometryKind::Box);
  mesh.shape.parameters[0] = {
    static_cast<float>(static_cast<double>(half.x) * k_pm_to_m),
    static_cast<float>(static_cast<double>(half.y) * k_pm_to_m),
    static_cast<float>(static_cast<double>(half.z) * k_pm_to_m),
    0.0F,
  };

  mesh.vertex_count = 36U;
  return mesh;
}

/*!
 * \brief Packs a complete sphere or ellipsoid with explicit shader poles.
 * \param[in] axes Authored canonical semiaxes.
 * \param[in] kind Sphere or Ellipsoid family.
 * \return Descriptor and nondegenerate latitude-band draw count.
 */
auto EllipsoidDescriptor(HalfExtent3PM axes, GGEMSGeometryKind kind)
  -> GGEMSGeometryMesh {
  auto mesh = BoxDescriptor(axes);
  mesh.shape.header = {
    static_cast<std::uint32_t>(kind),
    k_circle_segments,
    0U,
    0U,
  };
  mesh.vertex_count =
    VertexCount((2U * k_latitude_bands) - 2U, k_circle_segments);
  return mesh;
}

/*!
 * \brief Packs radii and counts walls and caps without creating visual points.
 * \param[in] kind Tubs or Cons family.
 * \param[in] radii Outer -Z, outer +Z, inner -Z, and inner +Z radii in meters.
 * \param[in] half_length Canonical half length along local Z.
 * \return Descriptor and count excluding collapsed walls and apex caps.
 */
auto ConicalDescriptor(GGEMSGeometryKind kind, std::array<float, 4> radii,
                       DistancePM half_length) -> GGEMSGeometryMesh {
  GGEMSGeometryMesh mesh{};
  mesh.shape.header = {
    static_cast<std::uint32_t>(kind),
    k_circle_segments,
    0U,
    0U,
  };

  mesh.shape.parameters[0] = radii;
  mesh.shape.parameters[1][0] =
    static_cast<float>(static_cast<double>(half_length) * k_pm_to_m);

  std::uint32_t triangles = 0U;
  for (auto radius : radii) {
    triangles += radius > 0.0F ? 1U : 0U;
  }

  triangles += CapTriangles(radii[0], radii[2]);
  triangles += CapTriangles(radii[1], radii[3]);
  mesh.vertex_count = VertexCount(triangles, k_circle_segments);
  return mesh;
}

/*!
 * \brief Copies authored profile scalars and indexes their lateral triangles.
 * \param[in] shape Validated polygonal shape, with increasing profile Z.
 * \param[in,out] profiles Shared profile storage to extend once per shape.
 * \return Descriptor referencing the copied profile and its triangle count.
 */
auto PolyhedraDescriptor(GGEMSPolyhedra const &shape,
                         std::vector<GGEMSGeometryProfile> &profiles)
  -> GGEMSGeometryMesh {
  auto const source = shape.GetZProfilePM();
  auto const first = static_cast<std::uint32_t>(profiles.size());
  auto const count = static_cast<std::uint32_t>(source.size());

  std::uint64_t prefix = 0U;
  for (std::size_t index = 0; index < source.size(); ++index) {
    auto const &plane = source[index];
    if (index != 0U) {
      prefix += 2U + (source[index - 1U].inner_radius_pm > 0U ? 1U : 0U) +
                (plane.inner_radius_pm > 0U ? 1U : 0U);
    }

    profiles.push_back({
      .z_m = static_cast<float>(static_cast<double>(plane.z_pm) * k_pm_to_m),
      .inner_apothem_m = static_cast<float>(
        static_cast<double>(plane.inner_radius_pm) * k_pm_to_m),
      .outer_apothem_m = static_cast<float>(
        static_cast<double>(plane.outer_radius_pm) * k_pm_to_m),
      .first_triangle = static_cast<std::uint32_t>(prefix),
    });
  }

  auto const caps = CapTriangles(profiles[first].outer_apothem_m,
                                 profiles[first].inner_apothem_m) +
                    CapTriangles(profiles.back().outer_apothem_m,
                                 profiles.back().inner_apothem_m);

  GGEMSGeometryMesh mesh{};

  mesh.shape.header = {
    static_cast<std::uint32_t>(GGEMSGeometryKind::Polyhedra),
    shape.GetSideCount(),
    first,
    count,
  };

  mesh.shape.parameters[0][0] =
    static_cast<float>(ggems::units::ToRadians(shape.GetStartAngle()));

  mesh.vertex_count = VertexCount(static_cast<std::uint32_t>(prefix + caps),
                                  shape.GetSideCount());

  return mesh;
}

/*!
 * \brief Packs the two authored end faces without calculating their corners.
 * \tparam Shape GGEMSTrd or GGEMSTrap.
 * \param[in] shape Authored planar solid.
 * \return Descriptor for the common six-face shader topology.
 */
template <typename Shape>
auto TrapezoidDescriptor(Shape const &shape) -> GGEMSGeometryMesh {
  GGEMSGeometryMesh mesh{};
  auto const negative = shape.GetNegativeZHalfSizePM();
  auto const positive = shape.GetPositiveZHalfSizePM();
  auto &parameters = mesh.shape.parameters;

  if constexpr (std::is_same_v<Shape, GGEMSTrd>) {
    mesh.shape.header[0] = static_cast<std::uint32_t>(GGEMSGeometryKind::Trd);
    parameters[0] = {
      static_cast<float>(static_cast<double>(negative[1]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(negative[0]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(negative[0]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(shape.GetHalfLengthPM()) *
                         k_pm_to_m),
    };

    parameters[1] = {
      static_cast<float>(static_cast<double>(positive[1]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(positive[0]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(positive[0]) * k_pm_to_m),
      0.0F,
    };
  } else {
    mesh.shape.header[0] = static_cast<std::uint32_t>(GGEMSGeometryKind::Trap);
    parameters[0] = {
      static_cast<float>(static_cast<double>(negative[0]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(negative[1]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(negative[2]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(shape.GetHalfLengthPM()) *
                         k_pm_to_m),
    };

    parameters[1] = {
      static_cast<float>(static_cast<double>(positive[0]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(positive[1]) * k_pm_to_m),
      static_cast<float>(static_cast<double>(positive[2]) * k_pm_to_m),
      0.0F,
    };

    parameters[2] = {
      static_cast<float>(ggems::units::ToRadians(shape.GetNegativeZSkew())),
      static_cast<float>(ggems::units::ToRadians(shape.GetPositiveZSkew())),
      static_cast<float>(ggems::units::ToRadians(shape.GetInclinationTheta())),
      static_cast<float>(ggems::units::ToRadians(shape.GetInclinationPhi())),
    };
  }

  mesh.vertex_count = 36U;
  return mesh;
}

/*!
 * \brief Copies the active analytic family into its immutable GPU descriptor.
 * \param[in] volume Validated shape with canonical authored dimensions.
 * \param[in,out] profiles Shared profile scalar storage.
 * \return Descriptor and procedural draw count, without CPU visual geometry.
 */

auto ShapeDescriptor(GGEMSWorld::Volume const &volume,
                     std::vector<GGEMSGeometryProfile> &profiles)
  -> GGEMSGeometryMesh {
  return std::visit(
    [&profiles](auto const &shape) -> GGEMSGeometryMesh {
      using Shape = std::decay_t<decltype(shape)>;

      if constexpr (std::is_same_v<Shape, GGEMSBox>) {
        return BoxDescriptor(shape.GetHalfExtentPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
        auto const radius = shape.GetRadiusPM();
        return EllipsoidDescriptor({.x = radius, .y = radius, .z = radius},
                                   GGEMSGeometryKind::Sphere);
      } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
        return EllipsoidDescriptor(shape.GetSemiAxesPM(),
                                   GGEMSGeometryKind::Ellipsoid);
      } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
        auto const outer = static_cast<float>(
          static_cast<double>(shape.GetOuterRadiusPM()) * k_pm_to_m);
        auto const inner = static_cast<float>(
          static_cast<double>(shape.GetInnerRadiusPM()) * k_pm_to_m);

        return ConicalDescriptor(GGEMSGeometryKind::Tubs,
                                 {outer, outer, inner, inner},
                                 shape.GetHalfLengthPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
        return ConicalDescriptor(
          GGEMSGeometryKind::Cons,
          {
            static_cast<float>(
              static_cast<double>(shape.GetNegativeZOuterRadiusPM()) *
              k_pm_to_m),
            static_cast<float>(
              static_cast<double>(shape.GetPositiveZOuterRadiusPM()) *
              k_pm_to_m),
            static_cast<float>(
              static_cast<double>(shape.GetNegativeZInnerRadiusPM()) *
              k_pm_to_m),
            static_cast<float>(
              static_cast<double>(shape.GetPositiveZInnerRadiusPM()) *
              k_pm_to_m),
          },
          shape.GetHalfLengthPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
        return PolyhedraDescriptor(shape, profiles);
      } else {
        return TrapezoidDescriptor(shape);
      }
    },
    volume);
}

/*! \brief Selects an analytic support function for conservative bounds. */
enum class EnvelopeKind : std::uint8_t {
  /*! \brief Axis-aligned local rectangular envelope. */
  Box,

  /*! \brief Full ellipsoid support, also used for a sphere. */
  Ellipsoid,

  /*! \brief Full local-Z cylinder enclosing a circular or polygonal profile. */
  Cylinder,
};

/*! \brief Stores dimensions of an analytic envelope, never visual vertices. */
struct AnalyticEnvelope {
  /*! \brief Local half extents; a cylinder uses X for its radius. */
  std::array<double, 3> half;

  /*! \brief Local Z center, nonzero only for an asymmetric polygon profile. */
  double center_z{0.0};

  /*! \brief Support function used under the composed render transform. */
  EnvelopeKind kind{EnvelopeKind::Box};
};

/*!
 * \brief Converts canonical rectangular half extents into a double envelope.
 * \param[in] half Canonical local half extents in picometers.
 * \param[in] kind Analytic support function for these dimensions.
 * \return Analytic envelope independent of descriptor rounding.
 */
auto AxisEnvelope(HalfExtent3PM half, EnvelopeKind kind = EnvelopeKind::Box)
  -> AnalyticEnvelope {
  return {
    .half =
      {
        static_cast<double>(half.x) * k_pm_to_m,
        static_cast<double>(half.y) * k_pm_to_m,
        static_cast<double>(half.z) * k_pm_to_m,
      },
    .kind = kind,
  };
}

/*!
 * \brief Bounds skewed and inclined planar faces using canonical scalars.
 * \tparam Shape GGEMSTrd or GGEMSTrap.
 * \param[in] shape Authored planar shape.
 * \return Conservative local rectangular envelope, without visual corners.
 */
template <typename Shape>
auto TrapezoidEnvelope(Shape const &shape) -> AnalyticEnvelope {
  auto const negative = shape.GetNegativeZHalfSizePM();
  auto const positive = shape.GetPositiveZHalfSizePM();
  auto const half_length =
    static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m;

  if constexpr (std::is_same_v<Shape, GGEMSTrd>) {
    return {
      .half =
        {
          static_cast<double>(std::max(negative[0], positive[0])) * k_pm_to_m,
          static_cast<double>(std::max(negative[1], positive[1])) * k_pm_to_m,
          half_length,
        },
    };
  } else {
    auto const negative_skew =
      static_cast<double>(ggems::units::ToRadians(shape.GetNegativeZSkew()));
    auto const positive_skew =
      static_cast<double>(ggems::units::ToRadians(shape.GetPositiveZSkew()));
    auto const theta =
      static_cast<double>(ggems::units::ToRadians(shape.GetInclinationTheta()));
    auto const phi =
      static_cast<double>(ggems::units::ToRadians(shape.GetInclinationPhi()));
    auto const half_x = std::max(
      static_cast<double>(std::max(negative[1], negative[2])) +
        (static_cast<double>(negative[0]) * std::abs(std::tan(negative_skew))),
      static_cast<double>(std::max(positive[1], positive[2])) +
        (static_cast<double>(positive[0]) * std::abs(std::tan(positive_skew))));
    return {
      .half =
        {
          (half_x * k_pm_to_m) +
            (half_length * std::abs(std::tan(theta) * std::cos(phi))),
          (static_cast<double>(std::max(negative[0], positive[0])) *
           k_pm_to_m) +
            (half_length * std::abs(std::tan(theta) * std::sin(phi))),
          half_length,
        },
    };
  }
}

/*!
 * \brief Bounds a polygon profile by its full circumscribed cylinder.
 * \param[in] shape Authored profile with canonical apothems and local Z.
 * \return Conservative cylinder preserving asymmetric profile placement.
 */
auto PolyhedraEnvelope(GGEMSPolyhedra const &shape) -> AnalyticEnvelope {
  auto const profile = shape.GetZProfilePM();
  DistancePM outer = 0U;
  for (auto const &plane : profile) {
    outer = std::max(outer, plane.outer_radius_pm);
  }

  auto const radius = static_cast<double>(outer) * k_pm_to_m /
                      std::cos(std::numbers::pi / shape.GetSideCount());
  auto const first_z = static_cast<double>(profile.front().z_pm) * k_pm_to_m;
  auto const last_z = static_cast<double>(profile.back().z_pm) * k_pm_to_m;
  return {
    .half = {radius, 0.0, (last_z - first_z) / 2.0},
    .center_z = (last_z + first_z) / 2.0,
    .kind = EnvelopeKind::Cylinder,
  };
}

/*!
 * \brief Selects a conservative envelope from canonical dimensions in double.
 * \param[in] volume Validated authored analytic shape.
 * \return Presentation envelope without tessellation or descriptor rounding.
 */
auto ShapeEnvelope(GGEMSWorld::Volume const &volume) -> AnalyticEnvelope {
  return std::visit(
    [](auto const &shape) -> AnalyticEnvelope {
      using Shape = std::decay_t<decltype(shape)>;

      if constexpr (std::is_same_v<Shape, GGEMSBox>) {
        return AxisEnvelope(shape.GetHalfExtentPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
        auto const radius = shape.GetRadiusPM();
        return AxisEnvelope({.x = radius, .y = radius, .z = radius},
                            EnvelopeKind::Ellipsoid);
      } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
        return AxisEnvelope(shape.GetSemiAxesPM(), EnvelopeKind::Ellipsoid);
      } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
        return {
          .half =
            {
              static_cast<double>(shape.GetOuterRadiusPM()) * k_pm_to_m,
              0.0,
              static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m,
            },
          .kind = EnvelopeKind::Cylinder,
        };
      } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
        return {
          .half =
            {
              static_cast<double>(std::max(shape.GetNegativeZOuterRadiusPM(),
                                           shape.GetPositiveZOuterRadiusPM())) *
                k_pm_to_m,
              0.0,
              static_cast<double>(shape.GetHalfLengthPM()) * k_pm_to_m,
            },
          .kind = EnvelopeKind::Cylinder,
        };
      } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
        return PolyhedraEnvelope(shape);
      } else {
        return TrapezoidEnvelope(shape);
      }
    },
    volume);
}

/*!
 * \brief Evaluates an envelope's support along one World-coordinate row.
 * \param[in] envelope Analytic local envelope.
 * \param[in] row Three rotation coefficients from the actual render matrix.
 * \return Nonnegative World-coordinate half width.
 */
auto ProjectedHalfExtent(AnalyticEnvelope const &envelope,
                         std::span<float const, 3> row) -> double {
  auto const &half = envelope.half;

  if (envelope.kind == EnvelopeKind::Cylinder) {
    return (half[0] * std::hypot(static_cast<double>(row[0]),
                                 static_cast<double>(row[1]))) +
           (half[2] * std::abs(row[2]));
  }

  if (envelope.kind == EnvelopeKind::Ellipsoid) {
    return std::hypot(half[0] * row[0], half[1] * row[1], half[2] * row[2]);
  }

  return (half[0] * std::abs(row[0])) + (half[1] * std::abs(row[1])) +
         (half[2] * std::abs(row[2]));
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

/*!
 * \brief Formats one canonical physical parameter for immutable inspection.
 * \tparam Quantity GGEMS quantity type.
 * \param[in] name Parameter label.
 * \param[in] value Canonical parameter value.
 * \return Owned label and formatted value.
 */
template <ggems::units::QuantityType Quantity>
auto Property(std::string_view name, Quantity value) -> GGEMSGeometryProperty {
  return {
    .name = std::string{name},
    .value = ggems::units::HumanReadable(value),
  };
}

/*!
 * \brief Formats full axis lengths from canonical rectangular half extents.
 * \param[in] half Canonical half extents in picometers.
 * \return Labeled full X, Y, and Z lengths.
 */
auto BoxDimensions(HalfExtent3PM half) -> std::vector<GGEMSGeometryProperty> {
  using ggems::units::Length;
  return {
    Property("Size X", Length{2U * half.x}),
    Property("Size Y", Length{2U * half.y}),
    Property("Size Z", Length{2U * half.z}),
  };
}

/*!
 * \brief Describes the two canonical end faces of a trapezoidal solid.
 * \tparam Shape GGEMSTrd or GGEMSTrap.
 * \param[in,out] occurrence Presentation record to populate.
 * \param[in] shape Authored solid with canonical dimensions and local angles.
 */
template <typename Shape>
auto DescribeTrapezoid(GGEMSGeometryOccurrence &occurrence, Shape const &shape)
  -> void {
  using ggems::units::Length;
  auto &dimensions = occurrence.dimensions;
  dimensions = {Property("Length Z", Length{2U * shape.GetHalfLengthPM()})};

  for (std::size_t end = 0; end < 2; ++end) {
    auto const half = end == 0 ? shape.GetNegativeZHalfSizePM()
                               : shape.GetPositiveZHalfSizePM();
    auto const *face = end == 0 ? "-Z" : "+Z";

    if constexpr (std::is_same_v<Shape, GGEMSTrd>) {
      occurrence.shape_name = "Trd";
      dimensions.push_back(
        Property(std::format("Size X at {}", face), Length{2U * half[0]}));
      dimensions.push_back(
        Property(std::format("Size Y at {}", face), Length{2U * half[1]}));
    } else {
      occurrence.shape_name = "Trap";
      dimensions.push_back(
        Property(std::format("Height Y at {}", face), Length{2U * half[0]}));
      dimensions.push_back(
        Property(std::format("Width X at {}, -Y", face), Length{2U * half[1]}));
      dimensions.push_back(
        Property(std::format("Width X at {}, +Y", face), Length{2U * half[2]}));
      dimensions.push_back(Property(std::format("Skew at {}", face),
                                    end == 0 ? shape.GetNegativeZSkew()
                                             : shape.GetPositiveZSkew()));
    }
  }

  if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
    dimensions.push_back(
      Property("Inclination theta", shape.GetInclinationTheta()));
    dimensions.push_back(
      Property("Inclination phi", shape.GetInclinationPhi()));
  }
}

/*!
 * \brief Copies model color and canonical analytic inspection parameters.
 * \param[in,out] occurrence Presentation record to populate.
 * \param[in] volume Validated authored shape, borrowed during construction.
 */
auto DescribeShape(GGEMSGeometryOccurrence &occurrence,
                   GGEMSWorld::Volume const &volume) -> void {
  using ggems::units::Length;
  std::visit(
    [&occurrence](auto const &shape) -> void {
      using Shape = std::decay_t<decltype(shape)>;
      auto &dimensions = occurrence.dimensions;

      occurrence.color =
        shape.GetColorOverride().value_or(shape.GetMaterial().GetColor());

      if constexpr (std::is_same_v<Shape, GGEMSBox>) {
        occurrence.shape_name = "Box";
        dimensions = BoxDimensions(shape.GetHalfExtentPM());
      } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
        occurrence.shape_name = "Sphere";
        dimensions = {Property("Radius", Length{shape.GetRadiusPM()})};
      } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
        occurrence.shape_name = "Ellipsoid";
        auto const axes = shape.GetSemiAxesPM();
        dimensions = {
          Property("Semiaxis X", Length{axes.x}),
          Property("Semiaxis Y", Length{axes.y}),
          Property("Semiaxis Z", Length{axes.z}),
        };
      } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
        occurrence.shape_name = "Tubs";
        dimensions = {
          Property("Outer radius", Length{shape.GetOuterRadiusPM()}),
          Property("Inner radius", Length{shape.GetInnerRadiusPM()}),
          Property("Length Z", Length{2U * shape.GetHalfLengthPM()}),
        };
      } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
        occurrence.shape_name = "Cons";
        dimensions = {
          Property("Outer radius at -Z",
                   Length{shape.GetNegativeZOuterRadiusPM()}),
          Property("Inner radius at -Z",
                   Length{shape.GetNegativeZInnerRadiusPM()}),
          Property("Outer radius at +Z",
                   Length{shape.GetPositiveZOuterRadiusPM()}),
          Property("Inner radius at +Z",
                   Length{shape.GetPositiveZInnerRadiusPM()}),
          Property("Length Z", Length{2U * shape.GetHalfLengthPM()}),
        };
      } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
        occurrence.shape_name = "Polyhedra";
        dimensions = {
          {.name = "Sides", .value = std::to_string(shape.GetSideCount())},
          Property("Start angle", shape.GetStartAngle()),
        };

        auto const profile = shape.GetZProfilePM();

        for (std::size_t index = 0; index < profile.size(); ++index) {
          auto const &plane = profile[index];
          dimensions.push_back(
            Property(std::format("Plane {} Z", index),
                     ggems::units::PositionCoordinate{plane.z_pm}));
          dimensions.push_back(
            Property(std::format("Plane {} inner apothem", index),
                     Length{plane.inner_radius_pm}));
          dimensions.push_back(
            Property(std::format("Plane {} outer apothem", index),
                     Length{plane.outer_radius_pm}));
        }
      } else {
        DescribeTrapezoid(occurrence, shape);
      }
    },
    volume);
}

/*!
 * \brief Copies the composed physical pose before render rounding.
 * \param[in,out] occurrence Presentation record with its existing render pose.
 * \param[in] pose Composed host pose before float rendering conversion.
 */
auto DescribePose(GGEMSGeometryOccurrence &occurrence,
                  GGEMSWorld::Pose const &pose) -> void {
  auto const &rotation = pose.rotation;

  occurrence.orientation = {
    rotation.axis_x.x, rotation.axis_y.x, rotation.axis_z.x,
    rotation.axis_x.y, rotation.axis_y.y, rotation.axis_z.y,
    rotation.axis_x.z, rotation.axis_y.z, rotation.axis_z.z,
  };

  for (std::size_t axis = 0; axis < 3; ++axis) {
    occurrence.position_m[axis] = pose.position_pm[axis] * k_pm_to_m;
  }
}

/*!
 * \brief Copies homogeneous metadata and bounds an analytic occurrence.
 * \param[in,out] occurrence Presentation record with its existing render pose.
 * \param[in] pose Composed host pose before float rendering conversion.
 * \param[in] material Actual Material borrowed only for copying metadata.
 * \param[in] envelope Conservative analytic envelope in local meters.
 */
auto DescribePlacement(GGEMSGeometryOccurrence &occurrence,
                       GGEMSWorld::Pose const &pose,
                       ggems::core::materials::GGEMSMaterial const &material,
                       AnalyticEnvelope const &envelope) -> void {
  DescribePose(occurrence, pose);

  occurrence.material_name = material.GetName();
  occurrence.material_density = material.GetDensity();

  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const *row = occurrence.local_to_world.data() + (4U * axis);

    auto const center = row[3] + (row[2] * envelope.center_z);
    auto const half =
      ProjectedHalfExtent(envelope, std::span<float const, 3>{row, 3U});

    occurrence.minimum_m[axis] = center - half;
    occurrence.maximum_m[axis] = center + half;
  }
}

/*!
 * \brief Captures grid metadata, bounds, and all mapped central-plane cells.
 * \param[in,out] occurrence The single physical voxelized occurrence.
 * \param[in] volume Immutable imported grid and local Material palette.
 * \param[in] pose Actual composed host placement.
 * \param[in,out] visible_cells Immutable scene-wide central-cell index stream.
 * \return One procedural Box descriptor, shared by all displayed cell
 * instances.
 * \throws ggems::core::GGEMSRecoverable If a slice exceeds Vulkan's 32-bit
 * instance-count domain.
 */
auto DescribeVoxelized(GGEMSGeometryOccurrence &occurrence,
                       GGEMSVoxelizedVolume const &volume,
                       GGEMSWorld::Pose const &pose,
                       std::vector<std::uint32_t> &visible_cells)
  -> GGEMSGeometryMesh {
  auto const &dimensions = volume.GetDimensions();
  auto const &spacing = volume.GetSpacing();
  auto const &size = volume.GetSize();

  GGEMSGeometryVoxelSlices slices{};

  slices.palette_size = volume.GetMaterials().size();
  for (auto const &mapping : volume.GetDisplayMappings()) {
    slices.mappings.push_back({
      .first_label = mapping.first_label,
      .end_label = mapping.end_label,
      .material_name = mapping.material_name,
      .mode = mapping.style.mode,
      .color = mapping.style.color,
    });
  }

  // These pair products are bounded by the already allocated full cell field.
  std::array const counts{
    dimensions[0] * dimensions[1],
    dimensions[0] * dimensions[2],
    dimensions[1] * dimensions[2],
  };

  GGEMSGeometryMesh mesh{};
  mesh.shape.header[0] =
    static_cast<std::uint32_t>(GGEMSGeometryKind::Voxelized);
  mesh.vertex_count = 36U;

  occurrence.shape_name = "VoxelizedVolume";
  occurrence.name = volume.GetName();

  DescribePose(occurrence, pose);

  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (counts[axis] > std::numeric_limits<std::uint32_t>::max()) {
      throw ggems::core::GGEMSRecoverable(
        "Voxel slice exceeds the 32-bit Vulkan instance-count domain.");
    }

    slices.dimensions[axis] = static_cast<std::uint32_t>(dimensions[axis]);
    slices.instance_counts[axis] = static_cast<std::uint32_t>(counts[axis]);
    slices.spacing_m[axis] =
      static_cast<double>(spacing[axis].value) * k_pm_to_m;
    slices.size_m[axis] = static_cast<double>(size[axis].value) * k_pm_to_m;

    mesh.shape.header[axis + 1U] = slices.dimensions[axis];
    mesh.shape.parameters[0][axis] = static_cast<float>(slices.spacing_m[axis]);

    auto const axis_name = "XYZ"[axis];

    occurrence.dimensions.push_back({
      .name = std::format("Cells {}", axis_name),
      .value = std::to_string(dimensions[axis]),
    });

    occurrence.dimensions.push_back(
      Property(std::format("Spacing {}", axis_name), spacing[axis]));

    occurrence.dimensions.push_back(
      Property(std::format("Size {}", axis_name), size[axis]));
  }

  for (std::size_t row = 0; row < 3; ++row) {
    double half = 0.0;

    for (std::size_t column = 0; column < 3; ++column) {
      half += std::abs(occurrence.orientation[(3U * row) + column]) *
              slices.size_m[column] * 0.5;
    }

    occurrence.minimum_m[row] = occurrence.position_m[row] - half;
    occurrence.maximum_m[row] = occurrence.position_m[row] + half;
  }

  for (std::uint32_t plane = 0U; plane < 3U; ++plane) {
    std::vector<std::vector<std::uint32_t>> groups(slices.mappings.size());

    for (std::size_t index = 0U; index < counts[plane]; ++index) {
      auto x = dimensions[0] / 2U;
      auto y = dimensions[1] / 2U;
      auto z = dimensions[2] / 2U;

      if (plane == 0U) {
        x = index % dimensions[0];
        y = index / dimensions[0];
      } else if (plane == 1U) {
        x = index % dimensions[0];
        z = index / dimensions[0];
      } else {
        y = index % dimensions[1];
        z = index / dimensions[1];
      }

      auto const label =
        volume.GetLabels()[x + (dimensions[0] * (y + (dimensions[1] * z)))];

      groups[volume.GetDisplayMappingIndex(label)].push_back(
        static_cast<std::uint32_t>(index));
    }

    for (std::size_t mapping = 0U; mapping < groups.size(); ++mapping) {
      auto const &cells = groups[mapping];
      if (cells.empty()) {
        continue;
      }
      if (cells.size() >
          std::numeric_limits<std::uint32_t>::max() - visible_cells.size()) {
        throw ggems::core::GGEMSRecoverable(
          "Central voxel cells exceed the 32-bit Vulkan instance-index "
          "domain.");
      }

      slices.batches.push_back({
        .plane = plane,
        .first_instance = static_cast<std::uint32_t>(visible_cells.size()),
        .instance_count = static_cast<std::uint32_t>(cells.size()),
        .mode = slices.mappings[mapping].mode,
        .color = slices.mappings[mapping].color,
        .mapping_index = static_cast<std::uint32_t>(mapping),
      });

      visible_cells.insert(visible_cells.end(), cells.begin(), cells.end());
    }
  }

  occurrence.voxel_slices = std::move(slices);
  return mesh;
}

/*!
 * \brief Collects one analytic subtree once while preparing presentation.
 * \param[in] scene Immutable physical links already captured from World.
 * \param[in] volume_id Current analytic physical identity.
 * \param[out] members Ordered physical IDs controlled by the named root.
 */
auto AppendAnalyticSubtree(GGEMSGeometryScene const &scene,
                           std::uint32_t volume_id,
                           std::vector<std::uint32_t> &members) -> void {
  auto const &occurrence = scene.occurrences[volume_id];

  if (occurrence.voxel_slices) {
    return;
  }

  members.push_back(volume_id);

  for (auto const child : occurrence.children) {
    AppendAnalyticSubtree(scene, child, members);
  }
}

/*!
 * \brief Prepares named membership, reverse counts, and physical disclosures.
 * \param[in,out] scene Complete owned physical snapshot; presentation caches
 * are built once without changing physical IDs or links.
 */
auto BuildNamedDefinitions(GGEMSGeometryScene &scene) -> void {
  std::map<std::uint32_t, std::uint32_t> named_definitions;
  for (auto &occurrence : scene.occurrences) {
    std::map<std::uint32_t, std::size_t> branches;
    for (auto const child : occurrence.children) {
      auto const repeater = scene.occurrences[child].repeater_group_id;

      if (repeater == 0U) {
        occurrence.child_branches.push_back({child});
      } else {
        auto const [entry, inserted] =
          branches.try_emplace(repeater, occurrence.child_branches.size());

        if (inserted) {
          occurrence.child_branches.emplace_back();
        }

        occurrence.child_branches[entry->second].push_back(child);
      }
    }

    if (occurrence.volume_id == 0U ||
        (occurrence.name.empty() && !occurrence.voxel_slices)) {
      continue;
    }

    auto const [entry, inserted] = named_definitions.try_emplace(
      occurrence.authored_group_id,
      static_cast<std::uint32_t>(scene.groups.size()));

    if (inserted) {
      scene.groups.push_back({
        .name =
          occurrence.name.empty() ? occurrence.shape_name : occurrence.name,
        .subtree =
          occurrence.parent_volume_id == 0U && !occurrence.voxel_slices,
        .physical_leaves = true,
        .members = {},
        .physical_roots = {},
        .child_groups = {},
        .inspection = {},
      });
    }

    occurrence.named_group = entry->second;
    scene.groups[entry->second].physical_roots.push_back(occurrence.volume_id);
  }
}

/*!
 * \brief Prepares group commands and their incremental summary dependencies.
 * \param[in,out] scene Named definitions with complete physical root lists.
 */
auto BuildGroupMemberships(GGEMSGeometryScene &scene) -> void {
  for (std::uint32_t index = 0U; index < scene.groups.size(); ++index) {
    auto &group = scene.groups[index];
    for (auto const root : group.physical_roots) {
      auto const &physical = scene.occurrences[root];
      group.physical_leaves = group.physical_leaves &&
                              physical.children.empty() &&
                              !physical.voxel_slices;

      if (group.subtree) {
        AppendAnalyticSubtree(scene, root, group.members);
      } else {
        group.members.push_back(root);
      }
    }

    std::set<std::uint32_t> children;
    for (auto const member : group.members) {
      auto &occurrence = scene.occurrences[member];
      occurrence.controlling_groups.push_back(index);
      if (group.subtree && occurrence.named_group &&
          *occurrence.named_group != index) {
        children.insert(*occurrence.named_group);
      }
    }

    group.child_groups.assign(children.begin(), children.end());
  }
}

/*!
 * \brief Summarizes actual members without treating a heterogeneous root as
 * one representative shape or Material.
 * \param[in] scene Owned physical metadata prepared for every occurrence.
 * \param[in,out] group Group receiving compact counted inspection variants.
 */
auto DescribeGroupMembers(GGEMSGeometryScene const &scene,
                          ggems::render::GGEMSGeometryGroup &group) -> void {
  std::map<std::string, std::size_t> types;
  std::map<std::pair<std::string, ggems::units::Density>, std::size_t>
    materials;
  std::map<std::pair<std::string, std::string>, std::size_t> dimensions;
  std::map<std::string, std::size_t> mappings;
  for (auto const member : group.members) {
    auto const &occurrence = scene.occurrences[member];
    ++types[occurrence.shape_name];
    std::string parameters;
    for (auto const &property : occurrence.dimensions) {
      if (!parameters.empty()) {
        parameters += "; ";
      }
      parameters += property.name + ": " + property.value;
    }
    ++dimensions[{occurrence.shape_name, std::move(parameters)}];
    if (occurrence.voxel_slices) {
      for (auto const &mapping : occurrence.voxel_slices->mappings) {
        ++mappings[mapping.material_name];
      }
    } else {
      ++materials[{occurrence.material_name, occurrence.material_density}];
    }
  }
  for (auto const &[type, count] : types) {
    group.inspection.push_back({
      .name = "Type",
      .value = std::format("{} ({} occurrences)", type, count),
    });
  }
  for (auto const &[material, count] : materials) {
    group.inspection.push_back({
      .name = "Material",
      .value =
        std::format("{}; density {}; {} occurrences", material.first,
                    ggems::units::HumanReadable(material.second, 3), count),
    });
  }
  for (auto const &[material, count] : mappings) {
    group.inspection.push_back({
      .name = "Material mapping",
      .value =
        std::format("{} ({} rows); density not available", material, count),
    });
  }
  for (auto const &[shape, count] : dimensions) {
    group.inspection.push_back({
      .name = std::format("Dimensions ({})", shape.first),
      .value = std::format("{}; {} occurrences", shape.second, count),
    });
  }
}

/*!
 * \brief Copies true root-placement facts from the shared captured recipe.
 * \param[in] world Authoritative immutable host recipe owner.
 * \param[in,out] group Named definition whose roots share one authored token.
 */
auto DescribeGroupRecipe(GGEMSWorld const &world,
                         ggems::render::GGEMSGeometryGroup &group) -> void {
  auto const *recipe =
    world.GetAuthoredRecipe(group.physical_roots.front() - 1U);
  if (recipe == nullptr) {
    group.inspection.push_back(
      {.name = "Root placement", .value = "Not available"});
    return;
  }
  using Kind = detail::GGEMSRepeatKind;
  std::string kind{"Single attachment"};
  if (recipe->kind == Kind::Linear) {
    kind = "Linear";
  } else if (recipe->kind == Kind::Circular) {
    kind = "Circular";
  }
  group.inspection.push_back(
    {.name = "Root placement", .value = std::move(kind)});
  group.inspection.push_back(
    {.name = "Roots per parent", .value = std::format("{}", recipe->count)});
  if (recipe->kind == Kind::Linear) {
    group.inspection.push_back({
      .name = "Linear counts (X, Y, Z)",
      .value = std::format("({}, {}, {})", recipe->linear_count[0],
                           recipe->linear_count[1], recipe->linear_count[2]),
    });
    group.inspection.push_back({
      .name = "Linear pitch (X, Y, Z)",
      .value = std::format(
        "({}, {}, {})", ggems::units::HumanReadable(recipe->linear_pitch[0], 3),
        ggems::units::HumanReadable(recipe->linear_pitch[1], 3),
        ggems::units::HumanReadable(recipe->linear_pitch[2], 3)),
    });
  } else if (recipe->kind == Kind::Circular) {
    std::array constexpr names{"X", "Y", "Z"};
    group.inspection.push_back({
      .name = "Circular axis",
      .value = names[static_cast<std::size_t>(recipe->axis)],
    });
  }
}

/*!
 * \brief Prepares compact group facts once, independently of frame rendering.
 * \param[in] world Source of true captured placement recipes.
 * \param[in,out] scene Complete member lists and owned occurrence descriptors.
 */
auto BuildGroupInspection(GGEMSWorld const &world, GGEMSGeometryScene &scene)
  -> void {
  for (auto &group : scene.groups) {
    std::set<std::uint32_t> parents;
    for (auto const root : group.physical_roots) {
      parents.insert(scene.occurrences[root].parent_volume_id);
    }
    group.inspection = {
      {
        .name = "Controlled occurrences",
        .value = std::format("{}", group.members.size()),
      },
      {
        .name = "Authored roots (global)",
        .value = std::format("{}", group.physical_roots.size()),
      },
      {
        .name = "Physical parents",
        .value = std::format("{}", parents.size()),
      },
    };
    DescribeGroupMembers(scene, group);
    DescribeGroupRecipe(world, group);
  }
}

/*!
 * \brief Finds named controls that lack a named analytic root header.
 * \param[in] scene Prepared named definitions and physical parent links.
 * \return Supplemental group indices indexed by their top physical root.
 */
auto SupplementalGroups(GGEMSGeometryScene const &scene)
  -> std::vector<std::vector<std::uint32_t>> {
  // A named definition below an unnamed root still needs a discoverable row.
  std::vector<std::vector<std::uint32_t>> supplemental(
    scene.occurrences.size());

  for (std::uint32_t index = 0U; index < scene.groups.size(); ++index) {
    auto root = scene.groups[index].physical_roots.front();
    bool nested = false;

    while (scene.occurrences[root].parent_volume_id != 0U) {
      root = scene.occurrences[root].parent_volume_id;
      auto const parent_group = scene.occurrences[root].named_group;
      nested = nested || (parent_group && scene.groups[*parent_group].subtree);
    }

    if (!nested && root != scene.groups[index].physical_roots.front()) {
      supplemental[root].push_back(index);
    }
  }

  return supplemental;
}

/*!
 * \brief Builds stable ordered root rows without per-frame deduplication.
 * \param[in,out] scene Prepared named memberships and child disclosures.
 */
auto BuildRootEntries(GGEMSGeometryScene &scene) -> void {
  auto const supplemental = SupplementalGroups(scene);
  std::set<std::uint32_t> displayed;
  for (auto const &branch : scene.occurrences.front().child_branches) {
    auto const group = scene.occurrences[branch.front()].named_group;

    if (group) {
      if (displayed.insert(*group).second) {
        scene.root_entries.push_back({.group_index = group, .volumes = {}});
      }
    } else {
      scene.root_entries.push_back(
        {.group_index = std::nullopt, .volumes = branch});
    }

    for (auto const root : branch) {
      for (auto const extra : supplemental[root]) {
        if (displayed.insert(extra).second) {
          scene.root_entries.push_back({.group_index = extra, .volumes = {}});
        }
      }
    }
  }
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

  scene.meshes.push_back(BoxDescriptor(half));
  scene.occurrences.reserve(world.GetVolumeCount() + 1U);
  scene.occurrences.push_back({
    .volume_id = 0U,
    .parent_volume_id = 0U,
    .mesh_index = 0U,
    .local_to_world = Transform({.position_pm = {}, .rotation = {}}),
    .shape_name = "GGEMSWorld",
    .dimensions = BoxDimensions(half),
    .name = world.GetName(),
  });

  DescribePlacement(scene.occurrences.front(),
                    {.position_pm = {}, .rotation = {}}, world.GetMaterial(),
                    AxisEnvelope(half));

  auto const links = world.BuildOccurrenceRecords();

  std::vector<std::pair<std::size_t, std::uint32_t>> mesh_sources;
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    if (world.GetMeshVolume(index) != nullptr) {
      throw ggems::core::GGEMSRecoverable(
        "The geometry viewer does not support mesh occurrences yet.");
    }

    auto const pose = world.BuildVolumePose(index);
    scene.occurrences.push_back({
      .volume_id = static_cast<std::uint32_t>(index) + 1U,
      .parent_volume_id = links[index].parent_volume_id,
      .mesh_index = 0U,
      .local_to_world = Transform(pose),
      .shape_name = {},
      .dimensions = {},
      .repeater_group_id = world.GetRepeaterGroupId(index),
      .repeater_recipe_id = world.GetRepeaterRecipeId(index),
      .authored_group_id = world.GetAuthoredGroupId(index),
    });

    auto &occurrence = scene.occurrences.back();

    if (auto const *voxelized = world.GetVoxelizedVolume(index)) {
      occurrence.mesh_index = static_cast<std::uint32_t>(scene.meshes.size());
      scene.meshes.push_back(
        DescribeVoxelized(occurrence, *voxelized, pose, scene.voxel_cells));
    } else {
      auto const &shape = world.GetVolume(index);

      auto const found =
        std::ranges::find_if(mesh_sources, [&](auto source) -> bool {
          return SameShape(shape, world.GetVolume(source.first));
        });

      if (found == mesh_sources.end()) {
        occurrence.mesh_index = static_cast<std::uint32_t>(scene.meshes.size());
        scene.meshes.push_back(ShapeDescriptor(shape, scene.profiles));
        mesh_sources.emplace_back(index, occurrence.mesh_index);
      } else {
        occurrence.mesh_index = found->second;
      }

      DescribeShape(occurrence, shape);

      occurrence.name = std::visit(
        [](auto const &value) -> std::string const & {
          return value.GetName();
        },
        shape);

      DescribePlacement(occurrence, pose, *world.GetVolumeMaterial(index),
                        ShapeEnvelope(shape));
    }

    scene.occurrences[occurrence.parent_volume_id].children.push_back(
      occurrence.volume_id);
  }

  BuildNamedDefinitions(scene);
  BuildGroupMemberships(scene);
  BuildGroupInspection(world, scene);
  BuildRootEntries(scene);

  return scene;
}

} // namespace ggems::render
