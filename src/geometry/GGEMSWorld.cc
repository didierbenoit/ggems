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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] box Complete authored Box.
 * \return Global surface envelope and local feature scale in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSBox const &box,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  auto const half = box.GetHalfExtentPM();

  std::array<double, 3U> const extent{
    static_cast<float>(half.x),
    static_cast<float>(half.y),
    static_cast<float>(half.z),
  };

  std::array<double, 3U> const position{
    pose.center_x_pm,
    pose.center_y_pm,
    pose.center_z_pm,
  };

  std::array const rows{
    std::array{
      pose.axis_x_x,
      pose.axis_y_x,
      pose.axis_z_x,
    },
    std::array{
      pose.axis_x_y,
      pose.axis_y_y,
      pose.axis_z_y,
    },
    std::array{
      pose.axis_x_z,
      pose.axis_y_z,
      pose.axis_z_z,
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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] sphere Complete authored Sphere.
 * \return Global surface envelope and local radius in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSSphere const &sphere,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  auto const radius = static_cast<float>(sphere.GetRadiusPM());

  return {
    .envelope_pm =
      {
        std::abs(static_cast<double>(pose.center_x_pm)) + radius,
        std::abs(static_cast<double>(pose.center_y_pm)) + radius,
        std::abs(static_cast<double>(pose.center_z_pm)) + radius,
      },
    .minimum_half_extent_pm = radius,
    .local_scale_pm = radius,
  };
}

/*!
 * \brief Bounds a posed cylinder or ellipsoid from uploaded dimensions.
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] extent Uploaded local enclosing radii/half-length or semi-axes in
 * pm.
 * \param[in] cylinder True for a cylindrical bound; false for an ellipsoid.
 * \return Surface envelope and projected band expansion for host admission.
 */
[[nodiscard]] auto
BuildCurvedSurfaceScale(ggems::geometry::GGEMSBoxPoseRecord const &pose,
                        std::array<double, 3U> extent, bool cylinder) noexcept
  -> SurfaceScale {
  std::array<double, 3U> const position{
    pose.center_x_pm,
    pose.center_y_pm,
    pose.center_z_pm,
  };

  std::array const rows{
    std::array{
      pose.axis_x_x,
      pose.axis_y_x,
      pose.axis_z_x,
    },
    std::array{
      pose.axis_x_y,
      pose.axis_y_y,
      pose.axis_z_y,
    },
    std::array{
      pose.axis_x_z,
      pose.axis_y_z,
      pose.axis_z_z,
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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] tubs Complete authored tube.
 * \return Global surface envelope and narrowest resolvable half-width.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTubs const &tubs,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  auto const outer = static_cast<float>(tubs.GetOuterRadiusPM());
  auto const inner = static_cast<float>(tubs.GetInnerRadiusPM());
  auto scale = BuildCurvedSurfaceScale(
    pose, {outer, outer, static_cast<float>(tubs.GetHalfLengthPM())}, true);

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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] cons Complete authored solid or hollow frustum.
 * \return Posed envelope, band projection, and required feature half-width.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSCons const &cons,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  auto const record = ggems::geometry::detail::BuildConsRecord(cons);
  auto const outer = static_cast<float>(std::max(
    cons.GetNegativeZOuterRadiusPM(), cons.GetPositiveZOuterRadiusPM()));
  auto scale =
    BuildCurvedSurfaceScale(pose, {outer, outer, record.half_length_pm}, true);
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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] solid Complete profile with noncollapsed FP32 sections.
 * \return Posed envelope and section, side, bore, and wall feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSPolyhedra const &solid,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
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
  auto scale = BuildCurvedSurfaceScale(pose, {outer, outer, axial}, true);
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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] ellipsoid Complete authored ellipsoid.
 * \return Global surface envelope and smallest semi-axis in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSEllipsoid const &ellipsoid,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  auto const axes = ellipsoid.GetSemiAxesPM();
  return BuildCurvedSurfaceScale(pose,
                                 {
                                   static_cast<float>(axes.x),
                                   static_cast<float>(axes.y),
                                   static_cast<float>(axes.z),
                                 },
                                 false);
}

/*!
 * \brief Bounds the actual packed trapezoid and its normal-band expansion.
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] record Four admitted side planes and implicit Z caps.
 * \return Posed corner envelope, band projection and narrowest normal feature.
 */
[[nodiscard]] auto BuildPlanarSurfaceScale(
  ggems::geometry::GGEMSBoxPoseRecord const &pose,
  ggems::geometry::GGEMSPlanarRecord const &record) noexcept -> SurfaceScale {
  std::array const position{
    pose.center_x_pm,
    pose.center_y_pm,
    pose.center_z_pm,
  };

  std::array const rows{
    std::array{
      pose.axis_x_x,
      pose.axis_y_x,
      pose.axis_z_x,
    },
    std::array{
      pose.axis_x_y,
      pose.axis_y_y,
      pose.axis_z_y,
    },
    std::array{
      pose.axis_x_z,
      pose.axis_y_z,
      pose.axis_z_z,
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
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] trd Complete positive-size solid.
 * \return FP32 surface envelope and normal-band feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTrd const &trd,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  return BuildPlanarSurfaceScale(
    pose, ggems::geometry::detail::BuildPlanarRecord(trd));
}

/*!
 * \brief Bounds an authored Trap through its admitted packed half-spaces.
 * \param[in] pose Final uploaded global center and local-to-global axes.
 * \param[in] trap Complete planar solid.
 * \return FP32 surface envelope and normal-band feature margins.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSTrap const &trap,
                  ggems::geometry::GGEMSBoxPoseRecord const &pose) noexcept
  -> SurfaceScale {
  return BuildPlanarSurfaceScale(
    pose, ggems::geometry::detail::BuildPlanarRecord(trap));
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
 * \brief Requires the existing automatic band to leave a resolved feature.
 * \param[in] scale Uploaded solid envelope and minimum feature half-width.
 * \param[in] tolerance Effective solid half-band in pm.
 * \throws ggems::core::GGEMSRecoverable If a nominal feature is unresolved.
 */
auto RequireFeatureBands(SurfaceScale const &scale, float tolerance) -> void {
  auto const spacing = SurfaceSpacingPM(scale);
  if (!(scale.minimum_half_extent_pm >
        static_cast<double>(tolerance) + spacing)) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "The smallest solid half-width/radius {} pm cannot separate navigation "
      "half-band {} pm and FP32 spacing {} pm; enlarge the feature or reduce "
      "the coordinate envelope.",
      scale.minimum_half_extent_pm, tolerance, spacing));
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
 * \brief Checks exact identity-axis Box containment, allowing contact.
 * \param[in] parent_half Canonical parent Box or World half extents.
 * \param[in] box Child Box in the parent frame with identity rotation.
 * \return True when all faces are within the closed parent bounds.
 */
[[nodiscard]] auto
IsAxisAlignedBoxInside(ggems::geometry::HalfExtent3PM parent_half,
                       ggems::geometry::GGEMSBox const &box) noexcept -> bool {
  auto const half = box.GetHalfExtentPM();
  if (half.x > parent_half.x || half.y > parent_half.y ||
      half.z > parent_half.z) {
    return false;
  }
  auto const margin_x = static_cast<CoordinatePM>(parent_half.x - half.x);
  auto const margin_y = static_cast<CoordinatePM>(parent_half.y - half.y);
  auto const margin_z = static_cast<CoordinatePM>(parent_half.z - half.z);
  auto const center = box.GetPositionPM();
  return center.x >= -margin_x && center.x <= margin_x &&
         center.y >= -margin_y && center.y <= margin_y &&
         center.z >= -margin_z && center.z <= margin_z;
}

/*! \brief Global or local diagnostic coordinates in pm, never track state. */
using AdmissionPoint = std::array<double, 3U>;

/*! \brief One bounded diagnostic point on or inside physical material. */
struct AdmissionSample {
  /*! \brief Global position in pm. */
  AdmissionPoint position;

  /*! \brief True only on an actual outer or bore boundary. */
  bool surface;
};

/*! \brief Temporary preparation data, never uploaded to a device. */
struct AdmissionSolid {
  /*! \brief Existing packed analytical coefficients. */
  ggems::geometry::GGEMSAnalyticVolumeRecord record;

  /*! \brief Existing composed local-to-World pose. */
  ggems::geometry::GGEMSBoxPoseRecord pose;

  /*! \brief Conservative global-axis radii around the posed origin. */
  AdmissionPoint radius;

  /*! \brief Existing automatic surface half-band in pm. */
  double tolerance;

  /*! \brief Polygon side normals, empty for other families. */
  std::vector<ggems::geometry::GGEMSPolyhedraSideRecord> sides;

  /*! \brief Polygon axial sections, empty for other families. */
  std::vector<ggems::geometry::GGEMSPolyhedraSectionRecord> sections;

  /*! \brief Deterministic surface and interior diagnostic samples. */
  std::vector<AdmissionSample> samples;
};

/*!
 * \brief Classifies a diagnostic point against expanded physical support.
 * \param[in] solid Packed static geometry with its composed pose.
 * \param[in] point Global diagnostic position in pm.
 * \param[in] band Signed existing half-band: negative contracts the support.
 * \return True in expanded closed or contracted strict support, excluding
 * bores.
 */
auto AdmissionContains(AdmissionSolid const &solid, AdmissionPoint point,
                       double band) -> bool {
  using ggems::geometry::GGEMSAnalyticKind;
  auto const &pose = solid.pose;
  auto const dx = point[0] - pose.center_x_pm;
  auto const dy = point[1] - pose.center_y_pm;
  auto const dz = point[2] - pose.center_z_pm;
  auto const x =
    (dx * pose.axis_x_x) + (dy * pose.axis_x_y) + (dz * pose.axis_x_z);
  auto const y =
    (dx * pose.axis_y_x) + (dy * pose.axis_y_y) + (dz * pose.axis_y_z);
  auto const z =
    (dx * pose.axis_z_x) + (dy * pose.axis_z_y) + (dz * pose.axis_z_z);
  auto const &parameters = solid.record.parameters;
  auto const within = [band](double value, double limit) -> bool {
    return band < 0.0 ? value < limit : value <= limit;
  };
  switch (solid.record.kind) {
  case GGEMSAnalyticKind::Box:
    return within(std::abs(x), parameters.box.half_extent_x_pm + band) &&
           within(std::abs(y), parameters.box.half_extent_y_pm + band) &&
           within(std::abs(z), parameters.box.half_extent_z_pm + band);
  case GGEMSAnalyticKind::Sphere:
    return within(std::hypot(dx, dy, dz), parameters.sphere.radius_pm + band);
  case GGEMSAnalyticKind::Ellipsoid: {
    auto const &axes = parameters.ellipsoid;
    if (std::min(
          {axes.semi_axis_x_pm, axes.semi_axis_y_pm, axes.semi_axis_z_pm}) +
          band <=
        0.0) {
      return false;
    }
    return within(std::hypot(x / (axes.semi_axis_x_pm + band),
                             y / (axes.semi_axis_y_pm + band),
                             z / (axes.semi_axis_z_pm + band)),
                  1.0);
  }
  case GGEMSAnalyticKind::Trd:
  case GGEMSAnalyticKind::Trap:
    if (!within(std::abs(z), parameters.planar.half_length_pm + band)) {
      return false;
    }
    return std::ranges::all_of(
      parameters.planar.sides, [&](auto const &side) -> bool {
        return within((side.normal_x * x) + (side.normal_y * y) +
                        (side.normal_z * z),
                      side.offset_pm + band);
      });
  case GGEMSAnalyticKind::Tubs: {
    auto const &tube = parameters.tubs;
    auto const radius = std::hypot(x, y);
    return within(std::abs(z), tube.half_length_pm + band) &&
           within(radius, tube.outer_radius_pm + band) &&
           (tube.inner_radius_pm == 0.0F ||
            within(-radius, -tube.inner_radius_pm + band));
  }
  case GGEMSAnalyticKind::Cons: {
    auto const &cone = parameters.cons;
    auto const radius = std::hypot(x, y);
    return within(std::abs(z), cone.half_length_pm + band) &&
           within(radius, cone.outer_radius_pm + (cone.outer_slope * z) +
                            (band * cone.outer_normal_scale)) &&
           (cone.inner_radius_pm == 0.0F ||
            within(-radius, -cone.inner_radius_pm - (cone.inner_slope * z) +
                              (band * cone.inner_normal_scale)));
  }
  case GGEMSAnalyticKind::Polyhedra: {
    if (!within(-z, -solid.sections.front().z_lower_pm + band) ||
        !within(z, solid.sections.back().z_upper_pm + band)) {
      return false;
    }
    double projection = -std::numeric_limits<double>::infinity();
    for (auto const &side : solid.sides) {
      projection =
        std::max(projection, (side.normal_x * x) + (side.normal_y * y));
    }
    auto section =
      std::ranges::find_if(solid.sections, [&](auto const &part) -> bool {
        return z <= part.z_upper_pm;
      });
    if (section == solid.sections.end()) {
      section = solid.sections.end() - 1;
    }
    auto const offset = z - section->z_lower_pm;
    return within(projection, section->outer_radius_pm +
                                (section->outer_slope * offset) +
                                (band * section->outer_normal_length)) &&
           ((section->inner_radius_pm == 0.0F &&
             section->inner_slope == 0.0F) ||
            within(-projection, -section->inner_radius_pm -
                                  (section->inner_slope * offset) +
                                  (band * section->inner_normal_length)));
  }
  case GGEMSAnalyticKind::None:
    return false;
  }
  return false;
}

/*!
 * \brief Samples physical surfaces and interior points without a proof claim.
 * \param[in,out] solid Packed geometry receiving at most 1152 static samples.
 *
 * Curves use 16 azimuths; polygon profiles use at most eight sections.
 * Internal profile joins are not physical surfaces. Narrow defects between
 * samples can be missed, including unsampled profile sections.
 */
auto SampleAdmissionSolid(AdmissionSolid &solid) -> void {
  using ggems::geometry::GGEMSAnalyticKind;
  auto const &pose = solid.pose;
  auto const add = [&](double x, double y, double z, bool surface) -> void {
    solid.samples.push_back({
      .position =
        {
          pose.center_x_pm + (x * pose.axis_x_x) + (y * pose.axis_y_x) +
            (z * pose.axis_z_x),
          pose.center_y_pm + (x * pose.axis_x_y) + (y * pose.axis_y_y) +
            (z * pose.axis_z_y),
          pose.center_z_pm + (x * pose.axis_x_z) + (y * pose.axis_y_z) +
            (z * pose.axis_z_z),
        },
      .surface = surface,
    });
  };
  auto const &parameters = solid.record.parameters;
  auto const kind = solid.record.kind;
  if (kind == GGEMSAnalyticKind::Box || kind == GGEMSAnalyticKind::Sphere ||
      kind == GGEMSAnalyticKind::Ellipsoid) {
    AdmissionPoint extent{};
    if (kind == GGEMSAnalyticKind::Box) {
      extent = {
        parameters.box.half_extent_x_pm,
        parameters.box.half_extent_y_pm,
        parameters.box.half_extent_z_pm,
      };
    } else if (kind == GGEMSAnalyticKind::Sphere) {
      extent.fill(parameters.sphere.radius_pm);
    } else {
      extent = {
        parameters.ellipsoid.semi_axis_x_pm,
        parameters.ellipsoid.semi_axis_y_pm,
        parameters.ellipsoid.semi_axis_z_pm,
      };
    }
    for (auto x : {-1.0, 0.0, 1.0}) {
      for (auto y : {-1.0, 0.0, 1.0}) {
        for (auto z : {-1.0, 0.0, 1.0}) {
          auto const norm = std::hypot(x, y, z);
          auto const divisor =
            kind == GGEMSAnalyticKind::Box || norm == 0.0 ? 1.0 : norm;
          add(x * extent[0] / divisor, y * extent[1] / divisor,
              z * extent[2] / divisor, norm != 0.0);
        }
      }
    }
    return;
  }
  if (kind == GGEMSAnalyticKind::Trd || kind == GGEMSAnalyticKind::Trap) {
    auto const &planar = parameters.planar;
    for (auto axial : {-1.0, 0.0, 1.0}) {
      auto const z = axial * planar.half_length_pm;
      auto const lower_y =
        (planar.sides[0].offset_pm - (planar.sides[0].normal_z * z)) /
        planar.sides[0].normal_y;
      auto const upper_y =
        (planar.sides[1].offset_pm - (planar.sides[1].normal_z * z)) /
        planar.sides[1].normal_y;
      for (auto across_y : {0.0, 0.5, 1.0}) {
        auto const y = std::lerp(lower_y, upper_y, across_y);
        auto const lower_x =
          (planar.sides[2].offset_pm - (planar.sides[2].normal_y * y) -
           (planar.sides[2].normal_z * z)) /
          planar.sides[2].normal_x;
        auto const upper_x =
          (planar.sides[3].offset_pm - (planar.sides[3].normal_y * y) -
           (planar.sides[3].normal_z * z)) /
          planar.sides[3].normal_x;
        for (auto across_x : {0.0, 0.5, 1.0}) {
          add(std::lerp(lower_x, upper_x, across_x), y, z,
              axial != 0.0 || across_x != 0.5 || across_y != 0.5);
        }
      }
    }
    return;
  }
  auto const sections = kind == GGEMSAnalyticKind::Polyhedra
                          ? std::min(solid.sections.size(), std::size_t{8U})
                          : 1U;
  for (std::size_t sample = 0U; sample < sections; ++sample) {
    auto const index =
      sections <= 1U ? 0U
                     : sample * (solid.sections.size() - 1U) / (sections - 1U);
    for (auto axial : {0.0, 0.5, 1.0}) {
      double z;
      double inner;
      double outer;
      bool cap = axial != 0.5;
      if (kind == GGEMSAnalyticKind::Polyhedra) {
        auto const &section = solid.sections[index];
        z = std::lerp(static_cast<double>(section.z_lower_pm),
                      static_cast<double>(section.z_upper_pm), axial);
        inner = section.inner_radius_pm +
                (section.inner_slope * (z - section.z_lower_pm));
        outer = section.outer_radius_pm +
                (section.outer_slope * (z - section.z_lower_pm));
        cap = (axial == 0.0 && index == 0U) ||
              (axial == 1.0 && index + 1U == solid.sections.size());
      } else if (kind == GGEMSAnalyticKind::Cons) {
        auto const &cone = parameters.cons;
        z = ((2.0 * axial) - 1.0) * cone.half_length_pm;
        inner = cone.inner_radius_pm + (cone.inner_slope * z);
        outer = cone.outer_radius_pm + (cone.outer_slope * z);
      } else {
        auto const &tube = parameters.tubs;
        z = ((2.0 * axial) - 1.0) * tube.half_length_pm;
        inner = tube.inner_radius_pm;
        outer = tube.outer_radius_pm;
      }
      for (unsigned azimuth = 0U; azimuth < 16U; ++azimuth) {
        auto const angle = 2.0 * std::numbers::pi * azimuth / 16.0;
        auto const x = std::cos(angle);
        auto const y = std::sin(angle);
        double projection = 1.0;
        if (kind == GGEMSAnalyticKind::Polyhedra) {
          projection = 0.0;
          for (auto const &side : solid.sides) {
            projection =
              std::max(projection, (side.normal_x * x) + (side.normal_y * y));
          }
        }
        for (auto radial : {0.0, 0.5, 1.0}) {
          auto const radius = std::lerp(inner, outer, radial) / projection;
          add(radius * x, radius * y, z,
              cap || radial == 1.0 || (radial == 0.0 && inner > 0.0));
        }
      }
    }
  }
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

  auto const &volume = volumes_[index].volume;
  auto const pose = BuildOccurrencePose(index);

  if (auto const *box = std::get_if<GGEMSBox>(&volume)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*box, pose),
                                   WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*sphere, pose),
                                   WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*solid, pose),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *cons = std::get_if<GGEMSCons>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*cons, pose),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *tubs = std::get_if<GGEMSTubs>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*tubs, pose),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *ellipsoid = std::get_if<GGEMSEllipsoid>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*ellipsoid, pose),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *trd = std::get_if<GGEMSTrd>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*trd, pose),
                               WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *trap = std::get_if<GGEMSTrap>(&volume)) {
    return RequiredTolerancePM(BuildSurfaceScale(*trap, pose),
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

auto GGEMSWorld::AddVolume(GGEMSBox box, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(box), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSSphere sphere, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(sphere), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTubs tubs, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(tubs), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSEllipsoid ellipsoid,
                           std::uint32_t parent_volume_id) -> GGEMSWorld & {
  return AddOccurrence(std::move(ellipsoid), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTrd trd, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(trd), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSTrap trap, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(trap), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSCons cons, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  return AddOccurrence(std::move(cons), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSPolyhedra polyhedra,
                           std::uint32_t parent_volume_id) -> GGEMSWorld & {
  return AddOccurrence(std::move(polyhedra), parent_volume_id);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddOccurrence(Volume volume, std::uint32_t parent_volume_id)
  -> GGEMSWorld & {
  if (parent_volume_id > volumes_.size()) {
    throw core::GGEMSRecoverable(
      "Physical parent must already exist in this World.");
  }
  if (GetOccurrenceDepth(parent_volume_id) >= 3U) {
    throw core::GGEMSRecoverable(
      "Physical hierarchy depth cannot exceed three below World.");
  }
  if (volumes_.size() >= std::numeric_limits<std::uint32_t>::max() - 1U) {
    throw core::GGEMSRecoverable("Physical VolumeID domain is exhausted.");
  }
  std::visit([](auto const &shape) -> void { shape.Validate(); }, volume);
  volumes_.push_back(
    {.volume = std::move(volume), .parent_volume_id = parent_volume_id});
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetOccurrenceDepth(std::uint32_t volume_id) const noexcept
  -> std::uint32_t {
  std::uint32_t depth = 0U;
  while (volume_id != 0U) {
    ++depth;
    volume_id = volumes_[volume_id - 1U].parent_volume_id;
  }
  return depth;
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
    ValidateVolume(index, world_spacing);
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
  ValidateHierarchy();
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::ValidateVolume(std::size_t index, float world_spacing) const
  -> void {
  auto const &volume = volumes_[index].volume;
  auto const direct_child = volumes_[index].parent_volume_id == 0U;
  auto const pose = BuildOccurrencePose(index);
  for (auto const &box : GetBoxes(index)) {
    if (direct_child && box.GetRotation().IsIdentity() &&
        !IsAxisAlignedBoxInside(half_extent_pm_, box)) {
      throw core::GGEMSRecoverable("Every Box must fit inside the World; "
                                   "crossing faces are not supported.");
    }

    auto const scale = BuildSurfaceScale(box, pose);
    auto const tolerance = GetVolumeNavigationTolerancePM(index);

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Box");

    RequireFeatureBands(scale, tolerance);
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    auto const radius = sphere->GetRadiusPM();
    auto const center = sphere->GetPositionPM();

    // Compare margins before subtracting signed centers or adding the radius.
    if (direct_child &&
        (radius > half_extent_pm_.x || radius > half_extent_pm_.y ||
         radius > half_extent_pm_.z)) {
      throw core::GGEMSRecoverable("Sphere must fit inside the World.");
    }

    auto const scale = BuildSurfaceScale(*sphere, pose);
    auto const tolerance = GetVolumeNavigationTolerancePM(index);

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Sphere");

    RequireFeatureBands(scale, tolerance);

    if (direct_child) {
      auto const margin_x =
        static_cast<CoordinatePM>(half_extent_pm_.x - radius);
      auto const margin_y =
        static_cast<CoordinatePM>(half_extent_pm_.y - radius);
      auto const margin_z =
        static_cast<CoordinatePM>(half_extent_pm_.z - radius);

      if (center.x < -margin_x || center.x > margin_x || center.y < -margin_y ||
          center.y > margin_y || center.z < -margin_z || center.z > margin_z) {
        throw core::GGEMSRecoverable("Sphere must fit inside the World.");
      }
    }
  }

  if (auto const *solid = std::get_if<GGEMSPolyhedra>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*solid, pose),
                        GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *cons = std::get_if<GGEMSCons>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*cons, pose),
                        GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *tubs = std::get_if<GGEMSTubs>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*tubs, pose),
                        GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *ellipsoid = std::get_if<GGEMSEllipsoid>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*ellipsoid, pose),
                        GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *trd = std::get_if<GGEMSTrd>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*trd, pose),
                        GetVolumeNavigationTolerancePM(index));
  }

  if (auto const *trap = std::get_if<GGEMSTrap>(&volume)) {
    RequireFeatureBands(BuildSurfaceScale(*trap, pose),
                        GetVolumeNavigationTolerancePM(index));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::ValidateSimplePair(std::size_t first, std::size_t second) const
  -> void {
  if (volumes_[first].parent_volume_id != 0U ||
      volumes_[second].parent_volume_id != 0U) {
    return;
  }
  // Exact integer checks reject identity-axis Boxes and coaxial Spheres only.
  // Remaining pairs also receive bounded physical-support diagnostics.
  auto const *left_box = std::get_if<GGEMSBox>(&volumes_[first].volume);
  auto const *right_box = std::get_if<GGEMSBox>(&volumes_[second].volume);
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

  auto const *left_sphere = std::get_if<GGEMSSphere>(&volumes_[first].volume);
  auto const *right_sphere = std::get_if<GGEMSSphere>(&volumes_[second].volume);

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

auto GGEMSWorld::ValidateHierarchy() const -> void {
  std::vector<AdmissionSolid> solids;
  solids.reserve(volumes_.size());
  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    auto const pose = BuildOccurrencePose(index);
    auto centered_pose = pose;
    centered_pose.center_x_pm = 0.0F;
    centered_pose.center_y_pm = 0.0F;
    centered_pose.center_z_pm = 0.0F;
    AdmissionSolid solid{
      .record = BuildAnalyticVolumeRecord(0U, index),
      .pose = pose,
      .radius = std::visit(
        [&](auto const &shape) -> AdmissionPoint {
          return BuildSurfaceScale(shape, centered_pose).envelope_pm;
        },
        volumes_[index].volume),
      .tolerance = GetVolumeNavigationTolerancePM(index),
      .sides = {},
      .sections = {},
      .samples = {},
    };
    if (auto const *polygon =
          std::get_if<GGEMSPolyhedra>(&volumes_[index].volume)) {
      solid.sides = detail::BuildPolyhedraSides(*polygon);
      solid.sections = detail::BuildPolyhedraSections(*polygon);
    }
    SampleAdmissionSolid(solid);
    solids.push_back(std::move(solid));
  }

  AdmissionPoint const world_half{
    static_cast<float>(half_extent_pm_.x),
    static_cast<float>(half_extent_pm_.y),
    static_cast<float>(half_extent_pm_.z),
  };
  for (std::size_t index = 0U; index < solids.size(); ++index) {
    auto const &child = solids[index];
    auto const parent_id = volumes_[index].parent_volume_id;
    bool outside = false;
    if (parent_id == 0U) {
      auto const band = child.tolerance + GetNavigationTolerancePM();
      // These curved envelopes are actual supports, not conservative cylinders.
      if (child.record.kind == GGEMSAnalyticKind::Tubs ||
          child.record.kind == GGEMSAnalyticKind::Ellipsoid) {
        AdmissionPoint const center{
          child.pose.center_x_pm,
          child.pose.center_y_pm,
          child.pose.center_z_pm,
        };
        for (std::size_t axis = 0U; axis < 3U; ++axis) {
          outside = outside || std::abs(center[axis]) + child.radius[axis] >
                                 world_half[axis] + band;
        }
      }
      outside =
        outside ||
        std::ranges::any_of(child.samples, [&](auto const &sample) -> bool {
          return std::abs(sample.position[0]) > world_half[0] + band ||
                 std::abs(sample.position[1]) > world_half[1] + band ||
                 std::abs(sample.position[2]) > world_half[2] + band;
        });
    } else {
      auto const &parent = solids[parent_id - 1U];
      auto const band = parent.tolerance + child.tolerance;
      auto const *parent_box =
        std::get_if<GGEMSBox>(&volumes_[parent_id - 1U].volume);
      auto const *child_box = std::get_if<GGEMSBox>(&volumes_[index].volume);
      if (parent_box != nullptr && child_box != nullptr &&
          child_box->GetRotation().IsIdentity()) {
        outside =
          !IsAxisAlignedBoxInside(parent_box->GetHalfExtentPM(), *child_box);
      }
      outside =
        outside ||
        std::ranges::any_of(child.samples,
                            [&](auto const &sample) -> bool {
                              return !AdmissionContains(parent, sample.position,
                                                        band);
                            }) ||
        std::ranges::any_of(parent.samples, [&](auto const &sample) -> bool {
          // A bore boundary strictly inside the child detects filled
          // cavities that the child's own outer surface can miss.
          return sample.surface &&
                 AdmissionContains(child, sample.position, -band);
        });
    }
    if (outside) {
      throw core::GGEMSRecoverable(std::format(
        "Analytical occurrence {} protrudes outside the physical support of "
        "parent {} (containment diagnostic).",
        index + 1U, parent_id));
    }
  }

  for (std::size_t first = 0U; first < solids.size(); ++first) {
    for (std::size_t second = first + 1U; second < solids.size(); ++second) {
      if (volumes_[first].parent_volume_id !=
          volumes_[second].parent_volume_id) {
        continue;
      }
      auto const &left = solids[first];
      auto const &right = solids[second];
      AdmissionPoint const separation{
        std::abs(static_cast<double>(left.pose.center_x_pm) -
                 right.pose.center_x_pm),
        std::abs(static_cast<double>(left.pose.center_y_pm) -
                 right.pose.center_y_pm),
        std::abs(static_cast<double>(left.pose.center_z_pm) -
                 right.pose.center_z_pm),
      };
      if (separation[0] >= left.radius[0] + right.radius[0] ||
          separation[1] >= left.radius[1] + right.radius[1] ||
          separation[2] >= left.radius[2] + right.radius[2]) {
        continue;
      }
      auto const band = left.tolerance + right.tolerance;
      if (std::ranges::any_of(left.samples,
                              [&](auto const &sample) -> bool {
                                return AdmissionContains(right, sample.position,
                                                         -band);
                              }) ||
          std::ranges::any_of(right.samples, [&](auto const &sample) -> bool {
            return AdmissionContains(left, sample.position, -band);
          })) {
        throw core::GGEMSRecoverable(std::format(
          "Analytical occurrences {} and {} have overlapping interiors "
          "(bounded sampling diagnostic).",
          first + 1U, second + 1U));
      }
    }
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

auto GGEMSWorld::BuildOccurrencePose(std::size_t index) const noexcept
  -> GGEMSBoxPoseRecord {
  auto const local_pose = [this](std::size_t local_index)
    -> std::pair<std::array<double, 3U>, GGEMSRotation3D> {
    return std::visit(
      [](auto const &shape)
        -> std::pair<std::array<double, 3U>, GGEMSRotation3D> {
        auto const center = shape.GetPositionPM();
        auto const &rotation = shape.GetRotation();
        return std::pair{
          std::array{
            static_cast<double>(center.x),
            static_cast<double>(center.y),
            static_cast<double>(center.z),
          },
          rotation,
        };
      },
      volumes_[local_index].volume);
  };
  auto [center, rotation] = local_pose(index);
  auto const authored_center = std::visit(
    [](auto const &shape) -> Position3PM { return shape.GetPositionPM(); },
    volumes_[index].volume);
  auto const direct_child = volumes_[index].parent_volume_id == 0U;
  auto parent = volumes_[index].parent_volume_id;
  while (parent != 0U) {
    auto const [parent_center, parent_rotation] = local_pose(parent - 1U);
    auto const rotate =
      [&parent_rotation](
        std::array<double, 3U> vector) -> std::array<double, 3U> {
      return std::array{
        (parent_rotation.axis_x.x * vector[0]) +
          (parent_rotation.axis_y.x * vector[1]) +
          (parent_rotation.axis_z.x * vector[2]),
        (parent_rotation.axis_x.y * vector[0]) +
          (parent_rotation.axis_y.y * vector[1]) +
          (parent_rotation.axis_z.y * vector[2]),
        (parent_rotation.axis_x.z * vector[0]) +
          (parent_rotation.axis_y.z * vector[1]) +
          (parent_rotation.axis_z.z * vector[2]),
      };
    };
    center = rotate(center);
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      center[axis] += parent_center[axis];
    }
    auto const axis_x =
      rotate({rotation.axis_x.x, rotation.axis_x.y, rotation.axis_x.z});
    auto const axis_y =
      rotate({rotation.axis_y.x, rotation.axis_y.y, rotation.axis_y.z});
    auto const axis_z =
      rotate({rotation.axis_z.x, rotation.axis_z.y, rotation.axis_z.z});
    rotation.axis_x = {.x = axis_x[0], .y = axis_x[1], .z = axis_x[2]};
    rotation.axis_y = {.x = axis_y[0], .y = axis_y[1], .z = axis_y[2]};
    rotation.axis_z = {.x = axis_z[0], .y = axis_z[1], .z = axis_z[2]};
    parent = volumes_[parent - 1U].parent_volume_id;
  }
  return {
    .center_x_pm = direct_child ? static_cast<float>(authored_center.x)
                                : static_cast<float>(center[0]),
    .center_y_pm = direct_child ? static_cast<float>(authored_center.y)
                                : static_cast<float>(center[1]),
    .center_z_pm = direct_child ? static_cast<float>(authored_center.z)
                                : static_cast<float>(center[2]),
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
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxPoseRecord(std::size_t index) const
  -> std::optional<GGEMSBoxPoseRecord> {
  if (index >= volumes_.size() ||
      std::holds_alternative<GGEMSSphere>(volumes_[index].volume)) {
    return std::nullopt;
  }
  return BuildOccurrencePose(index);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticAuxiliaryData(std::size_t index) const
  -> std::vector<std::byte> {
  if (index >= volumes_.size()) {
    return {};
  }

  if (auto const *solid =
        std::get_if<GGEMSPolyhedra>(&volumes_[index].volume)) {
    auto data = detail::BuildPolyhedraData(*solid);
    auto const pose = BuildOccurrencePose(index);
    std::memcpy(data.data(), &pose, sizeof(pose));
    return data;
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
      volume.volume);

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
    if (volumes_.size() > std::numeric_limits<std::uint32_t>::max() /
                            sizeof(GGEMSAnalyticOccurrenceRecord)) {
      throw core::GGEMSRecoverable(
        "Analytical descriptors exceed the 32-bit byte-offset domain.");
    }
    checked_append(volumes_.size() * sizeof(GGEMSAnalyticOccurrenceRecord));
  }

  layout.occurrences.reserve(volumes_.size());
  layout.profile_offsets.resize(volumes_.size());

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    GGEMSAnalyticOccurrenceRecord occurrence{
      .record_index = static_cast<std::uint32_t>(index),
      .parent_volume_id = volumes_[index].parent_volume_id,
    };

    if (!std::holds_alternative<GGEMSSphere>(volumes_[index].volume)) {
      occurrence.auxiliary_offset =
        static_cast<std::uint32_t>(layout.byte_count);
      checked_append(sizeof(GGEMSBoxPoseRecord));
    }
    layout.occurrences.push_back(occurrence);
  }

  std::uint32_t first_world_child = 0U;
  for (std::size_t reverse = volumes_.size(); reverse > 0U; --reverse) {
    auto &occurrence = layout.occurrences[reverse - 1U];
    auto &first_child =
      occurrence.parent_volume_id == 0U
        ? first_world_child
        : layout.occurrences[occurrence.parent_volume_id - 1U].first_child_id;
    occurrence.next_sibling_id = first_child;
    first_child = static_cast<std::uint32_t>(reverse);
  }

  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    auto const *shape = std::get_if<GGEMSPolyhedra>(&volumes_[index].volume);

    if (shape == nullptr) {
      continue;
    }

    for (auto const source : layout.profile_sources) {
      auto const &candidate = std::get<GGEMSPolyhedra>(volumes_[source].volume);

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
    auto const profile = detail::BuildPolyhedraData(
      std::get<GGEMSPolyhedra>(volumes_[source].volume));
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
  std::uint32_t depth = 0U;
  for (std::size_t index = 0U; index < volumes_.size(); ++index) {
    depth = std::max(
      depth, GetOccurrenceDepth(static_cast<std::uint32_t>(index) + 1U));
  }
  return {
    .half_extent_x_pm = static_cast<float>(half_extent_pm_.x),
    .half_extent_y_pm = static_cast<float>(half_extent_pm_.y),
    .half_extent_z_pm = static_cast<float>(half_extent_pm_.z),
    .tolerance_pm = GetNavigationTolerancePM(),
    .occurrence_count = static_cast<std::uint32_t>(volumes_.size()),
    .max_boundary_events = static_cast<std::uint32_t>(CountBoundaryEvents()),
    .hierarchy_depth = depth,
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

  auto const *box = std::get_if<GGEMSBox>(&volumes_[index].volume);

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
    volumes_[index].volume);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticVolumeRecord(std::uint32_t material_id,
                                           std::size_t index) const
  -> GGEMSAnalyticVolumeRecord {
  if (index >= volumes_.size()) {
    return {};
  }

  auto const &volume = volumes_[index].volume;
  auto const volume_id = static_cast<std::uint32_t>(index) + 1U;

  if (std::holds_alternative<GGEMSBox>(volume)) {
    std::array const material_ids{material_id};
    return {
      .kind = GGEMSAnalyticKind::Box,
      .parameters = {.box = BuildBoxRecords(material_ids, index).front()},
    };
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume)) {
    auto const pose = BuildOccurrencePose(index);
    return {
      .kind = GGEMSAnalyticKind::Sphere,
      .parameters =
        {
          .sphere =
            {
              .center_x_pm = pose.center_x_pm,
              .center_y_pm = pose.center_y_pm,
              .center_z_pm = pose.center_z_pm,
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
