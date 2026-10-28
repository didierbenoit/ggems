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
 * \brief Validates the authored World scene and compiles its device records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numbers>
#include <limits>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>
#include <type_traits>

#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/detail/GGEMSPolyhedraGeometry.hh"
#include "GGEMS/geometry/detail/GGEMSConsGeometry.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/detail/GGEMSPlanarGeometry.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using ggems::geometry::CoordinatePM;
using ggems::geometry::DistancePM;

/*! \brief Immutable FP32 surface envelope used only during host admission. */
struct SurfaceScale {
  /*! \brief Absolute posed surface envelope along World X/Y/Z, in pm. */
  std::array<double, 3U> envelope_pm{};
  /*! \brief World-axis envelope expansion per unit local surface half-band. */
  std::array<double, 3U> band_projection{1.0, 1.0, 1.0};
  /*! \brief Smallest required interior, bore, or wall half-width in pm. */
  double minimum_half_extent_pm{0.0};
  /*! \brief Largest local coordinate scale used for FP32 spacing, in pm. */
  float local_scale_pm{0.0F};
};

/*!
 * \brief Returns the upward binary32 spacing at a nonnegative scene scale.
 * \param[in] magnitude Represented coordinate or extent magnitude in pm.
 * \return Spacing in pm; zero for an unconfigured zero scale.
 */
[[nodiscard]] auto SpacingPM(float magnitude) noexcept -> float {
  return magnitude == 0.0F
           ? 0.0F
           : std::nextafter(magnitude, std::numeric_limits<float>::infinity()) -
               magnitude;
}

/*!
 * \brief Returns the largest represented World coordinate spacing in pm.
 * \param[in] half World half extents in canonical pm.
 * \return Binary32 spacing at the largest uploaded half extent.
 */
[[nodiscard]] auto WorldSpacingPM(ggems::geometry::HalfExtent3PM half) noexcept
  -> float {
  return SpacingPM(static_cast<float>(std::max({half.x, half.y, half.z})));
}

/*!
 * \brief Bounds a Box using its actual uploaded center, dimensions and axes.
 * \param[in] box Complete authored Box.
 * \return Global surface envelope and local feature scale in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSBox const &box) noexcept
  -> SurfaceScale {
  auto const half = box.GetHalfExtentPM();
  auto const center = box.GetPositionPM();
  auto const &rotation = box.GetRotation();

  std::array<double, 3U> const extent{
    static_cast<float>(half.x),
    static_cast<float>(half.y),
    static_cast<float>(half.z),
  };

  std::array<double, 3U> const position{
    static_cast<float>(center.x),
    static_cast<float>(center.y),
    static_cast<float>(center.z),
  };

  std::array const rows{
    std::array{
      static_cast<float>(rotation.axis_x.x),
      static_cast<float>(rotation.axis_y.x),
      static_cast<float>(rotation.axis_z.x),
    },
    std::array{
      static_cast<float>(rotation.axis_x.y),
      static_cast<float>(rotation.axis_y.y),
      static_cast<float>(rotation.axis_z.y),
    },
    std::array{
      static_cast<float>(rotation.axis_x.z),
      static_cast<float>(rotation.axis_y.z),
      static_cast<float>(rotation.axis_z.z),
    },
  };

  SurfaceScale scale{
    .minimum_half_extent_pm = std::min({extent[0], extent[1], extent[2]}),
    .local_scale_pm =
      static_cast<float>(std::max({extent[0], extent[1], extent[2]})),
  };

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    scale.envelope_pm[axis] = std::abs(position[axis]);
    scale.band_projection[axis] = 0.0;
    for (std::size_t column = 0U; column < 3U; ++column) {
      scale.envelope_pm[axis] += std::abs(rows[axis][column]) * extent[column];
      scale.band_projection[axis] += std::abs(rows[axis][column]);
    }
  }

  return scale;
}

/*!
 * \brief Bounds a Sphere using its actual uploaded center and radius.
 * \param[in] sphere Complete authored Sphere.
 * \return Global surface envelope and local radius in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSSphere const &sphere) noexcept
  -> SurfaceScale {
  auto const center = sphere.GetPositionPM();
  auto const radius = static_cast<float>(sphere.GetRadiusPM());

  return {
    .envelope_pm =
      {
        std::abs(static_cast<double>(static_cast<float>(center.x))) + radius,
        std::abs(static_cast<double>(static_cast<float>(center.y))) + radius,
        std::abs(static_cast<double>(static_cast<float>(center.z))) + radius,
      },
    .minimum_half_extent_pm = radius,
    .local_scale_pm = radius,
  };
}

/*!
 * \brief Bounds a posed cylinder or ellipsoid from uploaded dimensions.
 * \param[in] center World position of the local frame origin in pm.
 * \param[in] rotation Shared proper local-to-global rotation.
 * \param[in] extent Uploaded local enclosing radii/half-length or semi-axes in
 * pm.
 * \param[in] cylinder True for a cylindrical bound; false for an ellipsoid.
 * \return Surface envelope and projected band expansion for host admission.
 */
[[nodiscard]] auto
BuildCurvedSurfaceScale(ggems::geometry::Position3PM center,
                        ggems::geometry::GGEMSRotation3D const &rotation,
                        std::array<double, 3U> extent, bool cylinder) noexcept
  -> SurfaceScale {
  std::array<double, 3U> const position{
    static_cast<float>(center.x),
    static_cast<float>(center.y),
    static_cast<float>(center.z),
  };

  std::array const rows{
    std::array{
      static_cast<float>(rotation.axis_x.x),
      static_cast<float>(rotation.axis_y.x),
      static_cast<float>(rotation.axis_z.x),
    },
    std::array{
      static_cast<float>(rotation.axis_x.y),
      static_cast<float>(rotation.axis_y.y),
      static_cast<float>(rotation.axis_z.y),
    },
    std::array{
      static_cast<float>(rotation.axis_x.z),
      static_cast<float>(rotation.axis_y.z),
      static_cast<float>(rotation.axis_z.z),
    },
  };

  SurfaceScale scale{
    .minimum_half_extent_pm = std::min({extent[0], extent[1], extent[2]}),
    .local_scale_pm =
      static_cast<float>(std::max({extent[0], extent[1], extent[2]})),
  };

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const x = static_cast<double>(rows[axis][0]);
    auto const y = static_cast<double>(rows[axis][1]);
    auto const z = static_cast<double>(rows[axis][2]);
    auto const radius =
      cylinder
        ? (extent[0] * std::sqrt((x * x) + (y * y))) + (extent[2] * std::abs(z))
        : std::sqrt(((x * extent[0]) * (x * extent[0])) +
                    ((y * extent[1]) * (y * extent[1])) +
                    ((z * extent[2]) * (z * extent[2])));
    scale.envelope_pm[axis] = std::abs(position[axis]) + radius;
    scale.band_projection[axis] = cylinder
                                    ? std::sqrt((x * x) + (y * y)) + std::abs(z)
                                    : std::abs(x) + std::abs(y) + std::abs(z);
  }

  return scale;
}

/*!
 * \brief Bounds a Tubs and its bore/material feature widths after FP32 packing.
 * \param[in] tubs Complete authored tube.
 * \return Global surface envelope and narrowest resolvable half-width.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTubs const &tubs) noexcept
  -> SurfaceScale {
  auto const outer = static_cast<float>(tubs.GetOuterRadiusPM());
  auto const inner = static_cast<float>(tubs.GetInnerRadiusPM());
  auto scale = BuildCurvedSurfaceScale(
    tubs.GetPositionPM(), tubs.GetRotation(),
    {outer, outer, static_cast<float>(tubs.GetHalfLengthPM())}, true);

  if (inner > 0.0F) {
    scale.minimum_half_extent_pm = std::min({
      scale.minimum_half_extent_pm,
      static_cast<double>(inner),
      (static_cast<double>(outer) - inner) * 0.5,
    });
  }

  return scale;
}

/*!
 * \brief Bounds a frustum and its nonzero normal feature widths.
 * \param[in] cons Complete authored solid or hollow frustum.
 * \return Posed envelope, band projection, and required feature half-width.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSCons const &cons) noexcept
  -> SurfaceScale {
  auto const record = ggems::geometry::detail::BuildConsRecord(cons);
  auto const outer = static_cast<float>(std::max(
    cons.GetNegativeZOuterRadiusPM(), cons.GetPositiveZOuterRadiusPM()));
  auto scale =
    BuildCurvedSurfaceScale(cons.GetPositionPM(), cons.GetRotation(),
                            {outer, outer, record.half_length_pm}, true);
  scale.minimum_half_extent_pm = std::min(
    scale.minimum_half_extent_pm,
    static_cast<double>(record.outer_radius_pm / record.outer_normal_scale));
  // A true tip has no finite-width feature at the cap. Its surface band is
  // resolved directionally; every nonzero end feature must remain separated.
  for (auto const radii : {
         std::array{
           cons.GetNegativeZInnerRadiusPM(),
           cons.GetNegativeZOuterRadiusPM(),
         },
         std::array{
           cons.GetPositiveZInnerRadiusPM(),
           cons.GetPositiveZOuterRadiusPM(),
         },
       }) {
    auto const inner = static_cast<float>(radii[0]);
    auto const radius = static_cast<float>(radii[1]);
    if (radius > 0.0F) {
      scale.minimum_half_extent_pm =
        std::min(scale.minimum_half_extent_pm,
                 static_cast<double>(radius / record.outer_normal_scale));
    }
    if (inner > 0.0F) {
      scale.minimum_half_extent_pm = std::min({
        scale.minimum_half_extent_pm,
        static_cast<double>(inner / record.inner_normal_scale),
        static_cast<double>((radius - inner) / (record.outer_normal_scale +
                                                record.inner_normal_scale)),
      });
    }
  }
  for (auto &projection : scale.band_projection) {
    projection *= record.outer_normal_scale + std::abs(record.outer_slope);
  }
  return scale;
}

/*!
 * \brief Bounds a continuous polygonal profile and its required features.
 * \param[in] solid Complete profile with noncollapsed FP32 sections.
 * \return Posed envelope and section, side, bore, and wall feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSPolyhedra const &solid) noexcept
  -> SurfaceScale {
  auto const profile = solid.GetZProfilePM();
  auto const cosine = std::cos(std::numbers::pi / solid.GetSideCount());
  double outer = 0.0;
  double axial = 0.0;
  for (auto const &plane : profile) {
    outer = std::max(
      outer,
      static_cast<double>(static_cast<float>(plane.outer_radius_pm)) / cosine);
    axial = std::max(
      axial, std::abs(static_cast<double>(static_cast<float>(plane.z_pm))));
  }
  auto scale = BuildCurvedSurfaceScale(
    solid.GetPositionPM(), solid.GetRotation(), {outer, outer, axial}, true);
  double expansion = 1.0;
  for (std::size_t index = 0; index + 1U < profile.size(); ++index) {
    auto const section = ggems::geometry::detail::BuildPolyhedraSection(
      profile[index], profile[index + 1U]);
    scale.minimum_half_extent_pm = std::min(
      scale.minimum_half_extent_pm,
      0.5 * (static_cast<double>(section.z_upper_pm) - section.z_lower_pm));
    for (auto const endpoint : {index, index + 1U}) {
      auto const radius = static_cast<float>(profile[endpoint].outer_radius_pm);
      auto const inner = static_cast<float>(profile[endpoint].inner_radius_pm);
      scale.minimum_half_extent_pm = std::min({
        scale.minimum_half_extent_pm,
        static_cast<double>(radius / section.outer_normal_length),
        static_cast<double>((radius - inner) / (section.outer_normal_length +
                                                section.inner_normal_length)),
        radius * std::tan(std::numbers::pi / solid.GetSideCount()),
      });
      if (inner > 0.0F) {
        scale.minimum_half_extent_pm =
          std::min(scale.minimum_half_extent_pm,
                   static_cast<double>(inner / section.inner_normal_length));
      }
    }
    expansion = std::max(
      expansion,
      (section.outer_normal_length + std::abs(section.outer_slope)) / cosine);
  }
  for (auto &projection : scale.band_projection) {
    projection *= expansion;
  }
  return scale;
}

/*!
 * \brief Bounds an Ellipsoid using its uploaded semi-axes and rigid pose.
 * \param[in] ellipsoid Complete authored ellipsoid.
 * \return Global surface envelope and smallest semi-axis in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSEllipsoid const &ellipsoid) noexcept
  -> SurfaceScale {
  auto const axes = ellipsoid.GetSemiAxesPM();
  return BuildCurvedSurfaceScale(ellipsoid.GetPositionPM(),
                                 ellipsoid.GetRotation(),
                                 {
                                   static_cast<float>(axes.x),
                                   static_cast<float>(axes.y),
                                   static_cast<float>(axes.z),
                                 },
                                 false);
}

/*!
 * \brief Bounds the actual packed trapezoid and its normal-band expansion.
 * \param[in] center Canonical global frame origin.
 * \param[in] rotation Shared proper local-to-global pose.
 * \param[in] record Four admitted side planes and implicit Z caps.
 * \return Posed corner envelope, band projection and narrowest normal feature.
 */
[[nodiscard]] auto BuildPlanarSurfaceScale(
  ggems::geometry::Position3PM center,
  ggems::geometry::GGEMSRotation3D const &rotation,
  ggems::geometry::GGEMSPlanarRecord const &record) noexcept -> SurfaceScale {
  std::array const position{
    static_cast<float>(center.x),
    static_cast<float>(center.y),
    static_cast<float>(center.z),
  };

  std::array const rows{
    std::array{
      static_cast<float>(rotation.axis_x.x),
      static_cast<float>(rotation.axis_y.x),
      static_cast<float>(rotation.axis_z.x),
    },
    std::array{
      static_cast<float>(rotation.axis_x.y),
      static_cast<float>(rotation.axis_y.y),
      static_cast<float>(rotation.axis_z.y),
    },
    std::array{
      static_cast<float>(rotation.axis_x.z),
      static_cast<float>(rotation.axis_y.z),
      static_cast<float>(rotation.axis_z.z),
    },
  };

  SurfaceScale scale{
    .band_projection = {},
    .minimum_half_extent_pm = record.half_length_pm,
    .local_scale_pm = record.half_length_pm,
  };

  for (auto const sign_z : {-1.0F, 1.0F}) {
    auto const z = sign_z * record.half_length_pm;
    auto const &lower_y = record.sides[0];
    auto const &upper_y = record.sides[1];
    auto const center_y =
      (((lower_y.offset_pm - (lower_y.normal_z * z)) / lower_y.normal_y) +
       ((upper_y.offset_pm - (upper_y.normal_z * z)) / upper_y.normal_y)) *
      0.5F;

    auto const &lower_x = record.sides[2];
    auto const &upper_x = record.sides[3];
    auto const center_x = (((lower_x.offset_pm - (lower_x.normal_y * center_y) -
                             (lower_x.normal_z * z)) /
                            lower_x.normal_x) +
                           ((upper_x.offset_pm - (upper_x.normal_y * center_y) -
                             (upper_x.normal_z * z)) /
                            upper_x.normal_x)) *
                          0.5F;

    for (auto const &plane : record.sides) {
      auto const margin = plane.offset_pm - (plane.normal_x * center_x) -
                          (plane.normal_y * center_y) - (plane.normal_z * z);
      scale.minimum_half_extent_pm =
        std::min(scale.minimum_half_extent_pm, static_cast<double>(margin));
    }

    for (std::size_t side_y = 0U; side_y < 2U; ++side_y) {
      auto const &plane_y = record.sides[side_y];
      auto const y =
        (plane_y.offset_pm - (plane_y.normal_z * z)) / plane_y.normal_y;
      auto const band_y =
        (1.0F - (plane_y.normal_z * sign_z)) / plane_y.normal_y;
      auto const edge_left =
        (lower_x.offset_pm - (lower_x.normal_y * y) - (lower_x.normal_z * z)) /
        lower_x.normal_x;
      auto const edge_right =
        (upper_x.offset_pm - (upper_x.normal_y * y) - (upper_x.normal_z * z)) /
        upper_x.normal_x;
      auto const edge_margin =
        (edge_right - edge_left) * 0.5F *
        std::min(std::abs(lower_x.normal_x), std::abs(upper_x.normal_x));
      scale.minimum_half_extent_pm = std::min(scale.minimum_half_extent_pm,
                                              static_cast<double>(edge_margin));
      for (std::size_t side_x = 2U; side_x < 4U; ++side_x) {
        auto const &plane_x = record.sides[side_x];
        auto const x = (plane_x.offset_pm - (plane_x.normal_y * y) -
                        (plane_x.normal_z * z)) /
                       plane_x.normal_x;
        auto const band_x =
          (1.0F - (plane_x.normal_y * band_y) - (plane_x.normal_z * sign_z)) /
          plane_x.normal_x;
        scale.local_scale_pm =
          std::max({scale.local_scale_pm, std::abs(x), std::abs(y)});

        for (std::size_t axis = 0U; axis < 3U; ++axis) {
          auto const &row = rows[axis];
          auto const coordinate =
            position[axis] + (row[0] * x) + (row[1] * y) + (row[2] * z);
          auto const expansion =
            (row[0] * band_x) + (row[1] * band_y) + (row[2] * sign_z);
          scale.envelope_pm[axis] = std::max(
            scale.envelope_pm[axis], static_cast<double>(std::abs(coordinate)));
          scale.band_projection[axis] =
            std::max(scale.band_projection[axis],
                     static_cast<double>(std::abs(expansion)));
        }
      }
    }
  }
  return scale;
}

/*!
 * \brief Bounds an authored Trd through its packed planar half-spaces.
 * \param[in] trd Complete positive-size solid.
 * \return FP32 surface envelope and normal-band feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTrd const &trd) noexcept
  -> SurfaceScale {
  return BuildPlanarSurfaceScale(
    trd.GetPositionPM(), trd.GetRotation(),
    ggems::geometry::detail::BuildPlanarRecord(trd));
}

/*!
 * \brief Bounds an authored Trap through its admitted packed half-spaces.
 * \param[in] trap Complete planar solid.
 * \return FP32 surface envelope and normal-band feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTrap const &trap) noexcept
  -> SurfaceScale {
  return BuildPlanarSurfaceScale(
    trap.GetPositionPM(), trap.GetRotation(),
    ggems::geometry::detail::BuildPlanarRecord(trap));
}

/*!
 * \brief Returns the representability scale of the solid's surface operations.
 * \param[in] scale Uploaded geometry's static envelope.
 * \return Largest local or global surface spacing in pm.
 */
[[nodiscard]] auto SurfaceSpacingPM(SurfaceScale const &scale) noexcept
  -> float {
  return SpacingPM(std::max(
    scale.local_scale_pm,
    static_cast<float>(std::max(
      {scale.envelope_pm[0], scale.envelope_pm[1], scale.envelope_pm[2]}))));
}

/*!
 * \brief Derives a dimensionless-margin FP32 surface half-band on the host.
 * \param[in] scale Uploaded geometry's static envelope.
 * \param[in] world_spacing Largest World coordinate spacing in pm.
 * \return Half-band in pm, without a fixed physical-distance floor.
 */
[[nodiscard]] auto RequiredTolerancePM(SurfaceScale const &scale,
                                       float world_spacing) noexcept -> float {
  // Sixteen surface spacings cover upload, subtraction and local predicate
  // arithmetic. Every query additionally reserves eight World spacings for
  // remote ray parameters and movement. These engineering margins are qualified
  // by the device matrix; they are not exact-sign or continuum error proofs.
  return std::max(16.0F * SurfaceSpacingPM(scale), 8.0F * world_spacing);
}

/*!
 * \brief Rejects an unusable half-band below the derived FP32 requirement.
 * \param[in] tolerance Selected half-band in pm.
 * \param[in] required Minimum admitted half-band in pm.
 * \param[in] boundary Boundary label for the user diagnostic.
 * \throws ggems::core::GGEMSRecoverable If the half-band is not usable.
 */
auto RequireTolerance(float tolerance, float required,
                      std::string_view boundary) -> void {
  if (!std::isnormal(tolerance) || tolerance < required) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "{} navigation tolerance {} pm is below the required FP32 half-band "
      "{} pm; increase the tolerance or reduce the coordinate envelope.",
      boundary, tolerance, required));
  }
}

/*!
 * \brief Requires distinct interior and World-clearance tolerance bands.
 * \param[in] world_half Uploaded World half extents' authoring values.
 * \param[in] scale Uploaded solid envelope and minimum feature half-width.
 * \param[in] world_tolerance Effective World half-band in pm.
 * \param[in] tolerance Effective solid half-band in pm.
 * \throws ggems::core::GGEMSRecoverable If a feature or gap is unresolved.
 */
auto RequireSeparatedBands(ggems::geometry::HalfExtent3PM world_half,
                           SurfaceScale const &scale, float world_tolerance,
                           float tolerance) -> void {
  auto const spacing = SurfaceSpacingPM(scale);

  if (!(scale.minimum_half_extent_pm >
        static_cast<double>(tolerance) + spacing)) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "The smallest solid half-width/radius {} pm cannot separate navigation "
      "half-band {} pm and FP32 spacing {} pm; enlarge the feature or reduce "
      "the coordinate envelope.",
      scale.minimum_half_extent_pm, tolerance, spacing));
  }

  std::array<float, 3U> const half{
    static_cast<float>(world_half.x),
    static_cast<float>(world_half.y),
    static_cast<float>(world_half.z),
  };

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const gap = static_cast<double>(half[axis]) - scale.envelope_pm[axis];
    auto const required = static_cast<double>(world_tolerance) +
                          (tolerance * scale.band_projection[axis]) +
                          std::max(SpacingPM(half[axis]), spacing);

    if (!(gap > required)) {
      throw ggems::core::GGEMSRecoverable(std::format(
        "World/solid gap on axis {} is {} pm after FP32 packing; it must "
        "exceed {} pm to separate both navigation bands and one spacing.",
        axis, gap, required));
    }
  }
}

/*!
 * \brief Converts one World full size into its half extent.
 *
 * \param[in] size Full extent along the axis.
 * \param[in] axis Axis label used in diagnostics.
 * \return Half extent in picometers, at most 2^62 - 1.
 * \throws ggems::core::GGEMSRecoverable If the size is zero, odd, or larger
 * than ggems::geometry::k_max_world_size_pm.
 */
[[nodiscard]] auto RequireWorldHalfExtentPM(ggems::units::Length size,
                                            std::string_view axis)
  -> DistancePM {
  if (size.value > ggems::geometry::k_max_world_size_pm) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "World {} size {} pm exceeds the largest admitted size {} pm.", axis,
      size.value, ggems::geometry::k_max_world_size_pm));
  }

  return ggems::geometry::detail::RequireEvenHalfExtentPM(size, "World", axis);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks conservative rotated extrema against exact World margins.
 * \param[in] world_half Canonical World half extents.
 * \param[in] box Box with a nonidentity proper rotation.
 * \return True only for strict containment including a FP64 roundoff allowance.
 */
[[nodiscard]] auto
IsRotatedStrictlyInside(ggems::geometry::HalfExtent3PM world_half,
                        ggems::geometry::GGEMSBox const &box) noexcept -> bool {
  auto const half = box.GetHalfExtentPM();
  auto const center = box.GetPositionPM();
  auto const &rotation = box.GetRotation();

  std::array<DistancePM, 3U> const sizes{half.x, half.y, half.z};
  std::array<DistancePM, 3U> const world{world_half.x, world_half.y,
                                         world_half.z};

  std::array<CoordinatePM, 3U> const position{center.x, center.y, center.z};

  std::array<std::array<double, 3U>, 3U> const rows{{
    {rotation.axis_x.x, rotation.axis_y.x, rotation.axis_z.x},
    {rotation.axis_x.y, rotation.axis_y.y, rotation.axis_z.y},
    {rotation.axis_x.z, rotation.axis_y.z, rotation.axis_z.z},
  }};

  auto const infinity = std::numeric_limits<long double>::infinity();

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const world_radius = static_cast<CoordinatePM>(world[axis]);
    if (position[axis] <= -world_radius || position[axis] >= world_radius) {
      return false;
    }

    CoordinatePM const magnitude =
      position[axis] < 0 ? -position[axis] : position[axis];

    auto const margin = static_cast<long double>(world_radius - magnitude);

    long double radius = 0.0L;

    for (std::size_t column = 0U; column < 3U; ++column) {
      long double const extent =
        std::nextafter(static_cast<long double>(sizes[column]), infinity);

      long double const term = std::nextafter(
        std::abs(static_cast<long double>(rows[axis][column])) * extent,
        infinity);

      radius = std::nextafter(radius + term, infinity);
    }

    // Includes roundoff from the binary64 basis, never a physical gap.
    radius = std::nextafter(
      radius * (1.0L + (16.0L * std::numeric_limits<double>::epsilon())),
      infinity);

    if (!(radius < std::nextafter(margin, -infinity))) {
      return false;
    }
  }

  return true;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks that a centered Box lies strictly inside the World.
 *
 * The World half extents are at most 2^62 - 1 pm. Each margin is formed only
 * after the Box half extent is known to be smaller, so it is a positive signed
 * value and the center is compared against it directly; no face sum is
 * evaluated, so the check cannot overflow for any signed center.
 *
 * \param[in] world_half World half extents.
 * \param[in] box Complete Box.
 * \return True when every Box face is strictly between the World faces.
 */
[[nodiscard]] auto
IsStrictlyInside(ggems::geometry::HalfExtent3PM world_half,
                 ggems::geometry::GGEMSBox const &box) noexcept -> bool {
  if (!box.GetRotation().IsIdentity()) {
    return IsRotatedStrictlyInside(world_half, box);
  }

  ggems::geometry::HalfExtent3PM const half = box.GetHalfExtentPM();
  if (half.x >= world_half.x || half.y >= world_half.y ||
      half.z >= world_half.z) {
    return false;
  }

  auto const margin_x = static_cast<CoordinatePM>(world_half.x - half.x);
  auto const margin_y = static_cast<CoordinatePM>(world_half.y - half.y);
  auto const margin_z = static_cast<CoordinatePM>(world_half.z - half.z);
  ggems::geometry::Position3PM const center = box.GetPositionPM();
  return center.x > -margin_x && center.x < margin_x && center.y > -margin_y &&
         center.y < margin_y && center.z > -margin_z && center.z < margin_z;
}

} // namespace

// =============================================================================
// =============================================================================

namespace ggems::geometry {

auto GGEMSWorld::SetSize(units::Length size_x, units::Length size_y,
                         units::Length size_z) -> GGEMSWorld & {
  HalfExtent3PM const half_extent{
    .x = RequireWorldHalfExtentPM(size_x, "X"),
    .y = RequireWorldHalfExtentPM(size_y, "Y"),
    .z = RequireWorldHalfExtentPM(size_z, "Z"),
  };

  half_extent_pm_ = half_extent;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetNavigationTolerancePM() const noexcept -> float {
  if (half_extent_pm_.x == 0ULL) {
    return 0.0F;
  }

  return navigation_tolerance_
           ? static_cast<float>(navigation_tolerance_->value)
           : 8.0F * WorldSpacingPM(half_extent_pm_);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetVolumeNavigationTolerancePM(
  std::size_t index) const noexcept -> float {
  if (index >= volumes_.size()) {
    return 0.0F;
  }

  auto const &volume = volumes_[index];

  if (auto const *box = std::get_if<GGEMSBox>(&volume)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*box),
                                   WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*sphere),
                                   WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*solid),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *cons = std::get_if<GGEMSCons>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*cons),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *tubs = std::get_if<GGEMSTubs>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*tubs),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *ellipsoid = std::get_if<GGEMSEllipsoid>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*ellipsoid),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *trd = std::get_if<GGEMSTrd>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*trd),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *trap = std::get_if<GGEMSTrap>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*trap),
                               WorldSpacingPM(half_extent_pm_));
  }

  return 0.0F;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSWorld & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSBox box) -> GGEMSWorld & {
  box.Validate();
  volumes_.emplace_back(std::move(box));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSSphere sphere) -> GGEMSWorld & {
  sphere.Validate();
  volumes_.emplace_back(std::move(sphere));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTubs tubs) -> GGEMSWorld & {
  tubs.Validate();
  volumes_.emplace_back(std::move(tubs));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSEllipsoid ellipsoid) -> GGEMSWorld & {
  ellipsoid.Validate();
  volumes_.emplace_back(std::move(ellipsoid));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTrd trd) -> GGEMSWorld & {
  trd.Validate();
  volumes_.emplace_back(std::move(trd));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTrap trap) -> GGEMSWorld & {
  trap.Validate();
  volumes_.emplace_back(std::move(trap));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSCons cons) -> GGEMSWorld & {
  cons.Validate();
  volumes_.emplace_back(std::move(cons));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSPolyhedra polyhedra) -> GGEMSWorld & {
  polyhedra.Validate();
  volumes_.emplace_back(std::move(polyhedra));

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::Validate() const -> void {
  if (half_extent_pm_.x == 0ULL) {
    throw core::GGEMSRecoverable("World size is not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("World material is not set.");
  }

  auto const world_spacing = WorldSpacingPM(half_extent_pm_);
  auto const world_tolerance = GetNavigationTolerancePM();
  RequireTolerance(world_tolerance, 8.0F * world_spacing, "World");

  if (!(static_cast<float>(
          std::min({half_extent_pm_.x, half_extent_pm_.y, half_extent_pm_.z})) >
        world_tolerance + world_spacing)) {
    throw core::GGEMSRecoverable(
      "World half extents must separate both navigation surface bands.");
  }

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    ValidateVolume(index, world_spacing, world_tolerance);
  }

  if (volumes_.size() > std::numeric_limits<std::uint32_t>::max() ||
      CountBoundaryEvents() > std::numeric_limits<std::uint32_t>::max()) {
    throw core::GGEMSRecoverable("Analytical occurrence count or crossing "
                                 "bound exceeds the device domain.");
  }
  static_cast<void>(BuildAuxiliaryLayout());

  for (std::size_t first = 0U; first < volumes_.size(); ++first) {
    for (std::size_t second = first + 1U; second < volumes_.size(); ++second) {
      ValidateSimplePair(first, second);
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::ValidateVolume(std::size_t index, float world_spacing,
                                float world_tolerance) const -> void {
  auto const &volume = volumes_[index];
  for (auto const &box : GetBoxes(index)) {
    if (!IsStrictlyInside(half_extent_pm_, box)) {
      throw core::GGEMSRecoverable(
        "Every Box must lie strictly inside the World; coincident or "
        "crossing faces are not supported.");
    }

    auto const scale = BuildSurfaceScale(box);
    auto const tolerance = GetVolumeNavigationTolerancePM(index);

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Box");

    RequireSeparatedBands(half_extent_pm_, scale, world_tolerance, tolerance);
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    auto const radius = sphere->GetRadiusPM();
    auto const center = sphere->GetPositionPM();

    // Compare margins before subtracting signed centers or adding the radius.
    if (radius >= half_extent_pm_.x || radius >= half_extent_pm_.y ||
        radius >= half_extent_pm_.z) {
      throw core::GGEMSRecoverable(
        "Sphere must lie strictly inside the World.");
    }

    auto const scale = BuildSurfaceScale(*sphere);
    auto const tolerance = GetVolumeNavigationTolerancePM(index);

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Sphere");

    RequireSeparatedBands(half_extent_pm_, scale, world_tolerance, tolerance);

    auto const margin_x = static_cast<CoordinatePM>(half_extent_pm_.x - radius);
    auto const margin_y = static_cast<CoordinatePM>(half_extent_pm_.y - radius);
    auto const margin_z = static_cast<CoordinatePM>(half_extent_pm_.z - radius);

    if (center.x <= -margin_x || center.x >= margin_x ||
        center.y <= -margin_y || center.y >= margin_y ||
        center.z <= -margin_z || center.z >= margin_z) {
      throw core::GGEMSRecoverable(
        "Sphere must lie strictly inside the World.");
    }
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*solid),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *cons = std::get_if<GGEMSCons>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*cons),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *tubs = std::get_if<GGEMSTubs>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*tubs),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *ellipsoid = std::get_if<GGEMSEllipsoid>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*ellipsoid),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *trd = std::get_if<GGEMSTrd>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*trd),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *trap = std::get_if<GGEMSTrap>(&volume)) {
    RequireSeparatedBands(half_extent_pm_, BuildSurfaceScale(*trap),
                          world_tolerance,
                          GetVolumeNavigationTolerancePM(index));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::ValidateSimplePair(std::size_t first, std::size_t second) const
  -> void {
  // Exact integer checks reject identity-axis Boxes and coaxial Spheres only.
  // Other shape pairs retain the explicit disjoint-interior authoring contract.
  auto const *left_box = std::get_if<GGEMSBox>(&volumes_[first]);
  auto const *right_box = std::get_if<GGEMSBox>(&volumes_[second]);
  bool overlap = false;

  if (left_box != nullptr && right_box != nullptr &&
      left_box->GetRotation().IsIdentity() &&
      right_box->GetRotation().IsIdentity()) {
    auto const left = left_box->GetPositionPM();
    auto const right = right_box->GetPositionPM();
    auto const left_half = left_box->GetHalfExtentPM();
    auto const right_half = right_box->GetHalfExtentPM();

    overlap = static_cast<DistancePM>(std::abs(left.x - right.x)) <
                left_half.x + right_half.x &&
              static_cast<DistancePM>(std::abs(left.y - right.y)) <
                left_half.y + right_half.y &&
              static_cast<DistancePM>(std::abs(left.z - right.z)) <
                left_half.z + right_half.z;
  }

  auto const *left_sphere = std::get_if<GGEMSSphere>(&volumes_[first]);
  auto const *right_sphere = std::get_if<GGEMSSphere>(&volumes_[second]);

  if (left_sphere != nullptr && right_sphere != nullptr) {
    auto const left = left_sphere->GetPositionPM();
    auto const right = right_sphere->GetPositionPM();
    auto const radius =
      left_sphere->GetRadiusPM() + right_sphere->GetRadiusPM();

    if (left.y == right.y && left.z == right.z) {
      overlap = std::cmp_less(std::abs(left.x - right.x), radius);
    } else if (left.x == right.x && left.z == right.z) {
      overlap = std::cmp_less(std::abs(left.y - right.y), radius);
    } else if (left.x == right.x && left.y == right.y) {
      overlap = std::cmp_less(std::abs(left.z - right.z), radius);
    }
  }

  if (overlap) {
    throw core::GGEMSRecoverable(std::format(
      "Analytical occurrences {} and {} have overlapping interiors.",
      first + 1U, second + 1U));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxRecords(std::span<std::uint32_t const> material_ids,
                                 std::size_t index) const
  -> std::vector<GGEMSBoxRecord> {
  auto const boxes = GetBoxes(index);

  if (material_ids.size() != boxes.size()) {
    throw core::GGEMSRecoverable(
      "Box Material identity count is inconsistent.");
  }

  if (boxes.empty()) {
    return {};
  }

  auto const half = boxes.front().GetHalfExtentPM();
  return {
    GGEMSBoxRecord{
      .half_extent_x_pm = static_cast<float>(half.x),
      .half_extent_y_pm = static_cast<float>(half.y),
      .half_extent_z_pm = static_cast<float>(half.z),
      .volume_id = static_cast<std::uint32_t>(index) + 1U,
      .material_id = material_ids.front(),
      .tolerance_pm = GetVolumeNavigationTolerancePM(index),
    },
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxPoseRecord(std::size_t index) const
  -> std::optional<GGEMSBoxPoseRecord> {
  if (index >= volumes_.size()) {
    return std::nullopt;
  }

  return std::visit(
    [](auto const &volume) -> std::optional<GGEMSBoxPoseRecord> {
      using Shape = std::decay_t<decltype(volume)>;
      if constexpr (std::is_same_v<Shape, GGEMSBox> ||
                    std::is_same_v<Shape, GGEMSTubs> ||
                    std::is_same_v<Shape, GGEMSEllipsoid> ||
                    std::is_same_v<Shape, GGEMSTrd> ||
                    std::is_same_v<Shape, GGEMSTrap> ||
                    std::is_same_v<Shape, GGEMSCons> ||
                    std::is_same_v<Shape, GGEMSPolyhedra>) {
        auto const center = volume.GetPositionPM();
        auto const &rotation = volume.GetRotation();

        return GGEMSBoxPoseRecord{
          .center_x_pm = static_cast<float>(center.x),
          .center_y_pm = static_cast<float>(center.y),
          .center_z_pm = static_cast<float>(center.z),
          .axis_x_x = static_cast<float>(rotation.axis_x.x),
          .axis_x_y = static_cast<float>(rotation.axis_x.y),
          .axis_x_z = static_cast<float>(rotation.axis_x.z),
          .axis_y_x = static_cast<float>(rotation.axis_y.x),
          .axis_y_y = static_cast<float>(rotation.axis_y.y),
          .axis_y_z = static_cast<float>(rotation.axis_y.z),
          .axis_z_x = static_cast<float>(rotation.axis_z.x),
          .axis_z_y = static_cast<float>(rotation.axis_z.y),
          .axis_z_z = static_cast<float>(rotation.axis_z.z),
        };
      } else {
        return std::nullopt;
      }
    },

    volumes_[index]);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticAuxiliaryData(std::size_t index) const
  -> std::vector<std::byte> {
  if (index >= volumes_.size()) {
    return {};
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volumes_[index])) {
    return detail::BuildPolyhedraData(*solid);
  }

  auto const pose = BuildBoxPoseRecord(index);

  if (!pose) {
    return {};
  }

  std::vector<std::byte> data(sizeof(GGEMSBoxPoseRecord));
  std::memcpy(data.data(), &*pose, sizeof(GGEMSBoxPoseRecord));

  return data;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetVolumeCount() const noexcept -> std::size_t {
  return volumes_.size();
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::CountBoundaryEvents() const noexcept -> std::uint64_t {
  std::uint64_t count = 1ULL;

  for (auto const &volume : volumes_) {
    auto const crossings = std::visit(
      [](auto const &shape) -> std::uint64_t {
        using Shape = std::decay_t<decltype(shape)>;
        if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
          return 4ULL *
                 static_cast<std::uint64_t>(shape.GetZProfilePM().size() - 1U);
        } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
          return 4ULL;
        } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
          return shape.GetInnerRadiusPM() > 0ULL ? 4ULL : 2ULL;
        } else {
          return 2ULL;
        }
      },
      volume);

    if (count > std::numeric_limits<std::uint64_t>::max() - crossings) {
      return std::numeric_limits<std::uint64_t>::max();
    }

    count += crossings;
  }

  return count;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAuxiliaryLayout() const -> AuxiliaryLayout {
  AuxiliaryLayout layout;
  auto const checked_append = [&layout](std::size_t byte_count) -> void {
    if (byte_count >
        std::numeric_limits<std::uint32_t>::max() - layout.byte_count) {
      throw core::GGEMSRecoverable(
        "Analytical auxiliary data exceeds the 32-bit byte-offset domain.");
    }
    layout.byte_count += byte_count;
  };

  if (volumes_.size() > std::numeric_limits<std::uint32_t>::max()) {
    throw core::GGEMSRecoverable(
      "Analytical occurrence count exceeds the device domain.");
  }

  if (volumes_.size() > 1U) {
    checked_append(volumes_.size() * sizeof(GGEMSAnalyticOccurrenceRecord));
  }

  layout.occurrences.reserve(volumes_.size());
  layout.profile_offsets.resize(volumes_.size());

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    GGEMSAnalyticOccurrenceRecord occurrence{
      .record_index = static_cast<std::uint32_t>(index),
    };

    if (!std::holds_alternative<GGEMSSphere>(volumes_[index])) {
      occurrence.auxiliary_offset =
        static_cast<std::uint32_t>(layout.byte_count);
      checked_append(sizeof(GGEMSBoxPoseRecord));
    }
    layout.occurrences.push_back(occurrence);
  }

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    auto const *shape = std::get_if<GGEMSPolyhedra>(&volumes_[index]);

    if (shape == nullptr) {
      continue;
    }

    for (auto const source : layout.profile_sources) {
      auto const &candidate = std::get<GGEMSPolyhedra>(volumes_[source]);

      if (shape->GetSideCount() == candidate.GetSideCount() &&
          shape->GetStartAngle().value == candidate.GetStartAngle().value &&
          std::ranges::equal(shape->GetZProfilePM(), candidate.GetZProfilePM(),
                             [](GGEMSPolyhedraProfile const &left,
                                GGEMSPolyhedraProfile const &right) -> bool {
                               return left.z_pm == right.z_pm &&
                                      left.inner_radius_pm ==
                                        right.inner_radius_pm &&
                                      left.outer_radius_pm ==
                                        right.outer_radius_pm;
                             })) {
        layout.profile_offsets[index] = layout.profile_offsets[source];
        break;
      }
    }

    if (layout.profile_offsets[index] != 0U) {
      continue;
    }

    layout.profile_offsets[index] =
      static_cast<std::uint32_t>(layout.byte_count);
    checked_append(static_cast<std::size_t>(shape->GetSideCount()) *
                   sizeof(GGEMSPolyhedraSideRecord));
    checked_append((shape->GetZProfilePM().size() - 1U) *
                   sizeof(GGEMSPolyhedraSectionRecord));
    layout.profile_sources.push_back(index);
  }

  return layout;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildOccurrenceRecords() const
  -> std::vector<GGEMSAnalyticOccurrenceRecord> {
  return BuildAuxiliaryLayout().occurrences;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticDeviceRecords(
  std::span<std::uint32_t const> material_ids) const
  -> std::vector<GGEMSAnalyticVolumeRecord> {
  if (material_ids.size() != volumes_.size()) {
    throw core::GGEMSRecoverable(
      "Analytical Material identity count is inconsistent.");
  }

  if (volumes_.empty()) {
    return {GGEMSAnalyticVolumeRecord{}};
  }

  auto const layout = BuildAuxiliaryLayout();
  std::vector<GGEMSAnalyticVolumeRecord> records;
  records.reserve(volumes_.size());

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    auto record = BuildAnalyticVolumeRecord(material_ids[index], index);
    if (record.kind == GGEMSAnalyticKind::Polyhedra) {
      record.parameters.polyhedra.profile_offset_bytes =
        layout.profile_offsets[index] -
        layout.occurrences[index].auxiliary_offset;
    }
    records.push_back(record);
  }

  return records;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticAuxiliaryBuffer() const
  -> std::vector<std::byte> {

  if (volumes_.size() <= 1U) {
    return BuildAnalyticAuxiliaryData();
  }

  auto const layout = BuildAuxiliaryLayout();
  std::vector<std::byte> data(layout.byte_count);
  std::memcpy(data.data(), layout.occurrences.data(),
              layout.occurrences.size() *
                sizeof(GGEMSAnalyticOccurrenceRecord));

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    if (auto const pose = BuildBoxPoseRecord(index)) {
      std::memcpy(data.data() + layout.occurrences[index].auxiliary_offset,
                  &*pose, sizeof(GGEMSBoxPoseRecord));
    }
  }

  for (auto const source : layout.profile_sources) {
    auto const profile =
      detail::BuildPolyhedraData(std::get<GGEMSPolyhedra>(volumes_[source]));
    std::memcpy(data.data() + layout.profile_offsets[source],
                profile.data() + sizeof(GGEMSBoxPoseRecord),
                profile.size() - sizeof(GGEMSBoxPoseRecord));
  }

  return data;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetHalfExtentPM() const noexcept -> HalfExtent3PM {
  return half_extent_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetLowerCornerPM() const noexcept -> Position3PM {
  return Position3PM{
    .x = -static_cast<CoordinatePM>(half_extent_pm_.x),
    .y = -static_cast<CoordinatePM>(half_extent_pm_.y),
    .z = -static_cast<CoordinatePM>(half_extent_pm_.z),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetUpperCornerPM() const noexcept -> Position3PM {
  return Position3PM{
    .x = static_cast<CoordinatePM>(half_extent_pm_.x),
    .y = static_cast<CoordinatePM>(half_extent_pm_.y),
    .z = static_cast<CoordinatePM>(half_extent_pm_.z),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::Contains(Position3PM position) const noexcept -> bool {
  Position3PM const lower = GetLowerCornerPM();
  Position3PM const upper = GetUpperCornerPM();

  return position.x >= lower.x && position.x <= upper.x &&
         position.y >= lower.y && position.y <= upper.y &&
         position.z >= lower.z && position.z <= upper.z;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildRecord() const noexcept -> GGEMSWorldRecord {
  return {
    .half_extent_x_pm = static_cast<float>(half_extent_pm_.x),
    .half_extent_y_pm = static_cast<float>(half_extent_pm_.y),
    .half_extent_z_pm = static_cast<float>(half_extent_pm_.z),
    .tolerance_pm = GetNavigationTolerancePM(),
    .occurrence_count = static_cast<std::uint32_t>(volumes_.size()),
    .max_boundary_events = static_cast<std::uint32_t>(CountBoundaryEvents()),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("World material is not set.");
  }

  return *material_;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetBoxes(std::size_t index) const noexcept
  -> std::span<GGEMSBox const> {
  if (index >= volumes_.size()) {
    return {};
  }

  auto const *box = std::get_if<GGEMSBox>(&volumes_[index]);

  return box != nullptr ? std::span<GGEMSBox const>{box, 1U}
                        : std::span<GGEMSBox const>{};
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetVolumeMaterial(std::size_t index) const
  -> core::materials::GGEMSMaterial const * {
  if (index >= volumes_.size()) {
    return nullptr;
  }

  return std::visit(
    [](auto const &volume) -> core::materials::GGEMSMaterial const * {
      return &volume.GetMaterial();
    },
    volumes_[index]);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticVolumeRecord(std::uint32_t material_id,
                                           std::size_t index) const
  -> GGEMSAnalyticVolumeRecord {
  if (index >= volumes_.size()) {
    return {};
  }

  auto const &volume = volumes_[index];
  auto const volume_id = static_cast<std::uint32_t>(index) + 1U;

  if (std::holds_alternative<GGEMSBox>(volume)) {
    std::array const material_ids{material_id};
    return {
      .kind = GGEMSAnalyticKind::Box,
      .parameters = {.box = BuildBoxRecords(material_ids, index).front()},
    };
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    auto const center = sphere->GetPositionPM();
    return {
      .kind = GGEMSAnalyticKind::Sphere,
      .parameters =
        {
          .sphere =
            {
              .center_x_pm = static_cast<float>(center.x),
              .center_y_pm = static_cast<float>(center.y),
              .center_z_pm = static_cast<float>(center.z),
              .radius_pm = static_cast<float>(sphere->GetRadiusPM()),
              .volume_id = volume_id,
              .material_id = material_id,
              .tolerance_pm = GetVolumeNavigationTolerancePM(index),
            },
        },
    };
  }

  if (auto const *tubs = std::get_if<GGEMSTubs>(&volume)) {
    return {
      .kind = GGEMSAnalyticKind::Tubs,
      .parameters =
        {
          .tubs =
            {
              .outer_radius_pm = static_cast<float>(tubs->GetOuterRadiusPM()),
              .inner_radius_pm = static_cast<float>(tubs->GetInnerRadiusPM()),
              .half_length_pm = static_cast<float>(tubs->GetHalfLengthPM()),
              .volume_id = volume_id,
              .material_id = material_id,
              .tolerance_pm = GetVolumeNavigationTolerancePM(index),
            },
        },
    };
  }

  if (auto const *ellipsoid = std::get_if<GGEMSEllipsoid>(&volume)) {
    auto const axes = ellipsoid->GetSemiAxesPM();
    return {
      .kind = GGEMSAnalyticKind::Ellipsoid,
      .parameters =
        {
          .ellipsoid =
            {
              .semi_axis_x_pm = static_cast<float>(axes.x),
              .semi_axis_y_pm = static_cast<float>(axes.y),
              .semi_axis_z_pm = static_cast<float>(axes.z),
              .volume_id = volume_id,
              .material_id = material_id,
              .tolerance_pm = GetVolumeNavigationTolerancePM(index),
            },
        },
    };
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volume)) {
    return {
      .kind = GGEMSAnalyticKind::Polyhedra,
      .parameters =
        {
          .polyhedra =
            {
              .side_count = solid->GetSideCount(),
              .section_count =
                static_cast<std::uint32_t>(solid->GetZProfilePM().size() - 1U),
              .volume_id = volume_id,
              .material_id = material_id,
              .tolerance_pm = GetVolumeNavigationTolerancePM(index),
            },
        },
    };
  }

  if (auto const *cons = std::get_if<GGEMSCons>(&volume)) {
    auto record = detail::BuildConsRecord(*cons);
    record.volume_id = volume_id;
    record.material_id = material_id;
    record.tolerance_pm = GetVolumeNavigationTolerancePM(index);
    return {.kind = GGEMSAnalyticKind::Cons, .parameters = {.cons = record}};
  }

  if (auto const *trd = std::get_if<GGEMSTrd>(&volume)) {
    auto record = detail::BuildPlanarRecord(*trd);
    record.volume_id = volume_id;
    record.material_id = material_id;
    record.tolerance_pm = GetVolumeNavigationTolerancePM(index);
    return {.kind = GGEMSAnalyticKind::Trd, .parameters = {.planar = record}};
  }

  if (auto const *trap = std::get_if<GGEMSTrap>(&volume)) {
    auto record = detail::BuildPlanarRecord(*trap);
    record.volume_id = volume_id;
    record.material_id = material_id;
    record.tolerance_pm = GetVolumeNavigationTolerancePM(index);
    return {.kind = GGEMSAnalyticKind::Trap, .parameters = {.planar = record}};
  }

  return {};
}

} // namespace ggems::geometry
