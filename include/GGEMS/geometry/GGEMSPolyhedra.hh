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
 * \brief Declares a full-azimuth polygonal solid with continuous axial
 * profiles.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*! \brief Stores one authored local Z plane and its polygon apothems in pm. */
struct GGEMSPolyhedraProfile {
  /*! \brief Signed absolute coordinate along the local Z axis. */
  CoordinatePM z_pm{0};

  /*! \brief Inner apothem; zero closes the bore at this plane. */
  DistancePM inner_radius_pm{0};

  /*! \brief Strictly positive outer apothem. */
  DistancePM outer_radius_pm{0};
};

/*!
 * \brief Owns a regular polygon swept through continuous axial radial profiles.
 *
 * Radii measure distance to polygon sides, not vertices. The start angle points
 * to the first vertex; side normals lie halfway between neighboring vertices.
 * Z coordinates are absolute local coordinates and need not be centered.
 * Adjacent planes interpolate both apothems linearly, with no radial jump
 * at a shared plane. Axial profiles may be concave: one ray can cross multiple
 * disjoint material intervals, all with the same physical volume and Material.
 * Zero inner apothems throughout give a filled solid; positive values form a
 * polygonal bore that may close where the inner profile reaches zero.
 *
 * At least three sides and two strictly increasing local Z planes are needed.
 * Repeated-Z steps, partial-azimuth sectors, zero outer apothems, arbitrary RZ
 * polygons, and arbitrary meshes are unsupported. This API is a continuous
 * regular-polygon profile and does not provide full G4Polyhedra equivalence.
 *
 * Validate() checks section and wall survival after FP32 packing. GGEMSWorld
 * additionally checks the automatically derived surface half-band against
 * section height, side width, nonzero bore, wall thickness, and direct-child
 * World clearance. Pose, side normals, and axial coefficients are uploaded once
 * as immutable auxiliary data; all per-particle queries execute in OpenCL. Pose
 * coordinates and fixed rotation axes belong to the parent selected by
 * GGEMSWorld::AddVolume; parent zero selects the World frame.
 */
class GGEMSPolyhedra {
public:
  /*! \brief Creates an unconfigured solid with an identity pose. */
  GGEMSPolyhedra() = default;

  /*!
   * \brief Sets the number of sides of every regular polygon cross-section.
   * \param[in] side_count At least three; no fixed hexagonal specialization.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If fewer than three sides are requested.
   */
  auto SetSideCount(std::uint32_t side_count) -> GGEMSPolyhedra &;

  /*!
   * \brief Sets the angle from local +X to the first polygon vertex.
   * \param[in] angle Canonical angle, positive from +X toward +Y about +Z.
   * \return This solid for chained configuration.
   */
  auto SetStartAngle(units::Angle angle) -> GGEMSPolyhedra &;

  /*!
   * \brief Copies a continuous axial profile from canonical typed quantities.
   * \param[in] z_planes Strictly increasing absolute local Z coordinates.
   * \param[in] inner_radii Nonnegative inner apothems; zero closes the bore.
   * \param[in] outer_radii Positive outer apothems, each greater than the
   * inner.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If array sizes differ, fewer than two planes
   * are supplied, Z is not strictly increasing, radii are invalid, or the
   * section count exceeds the transport event-index range. The old profile
   * remains unchanged on rejection.
   */
  auto SetZProfile(std::span<units::PositionCoordinate const> z_planes,
                   std::span<units::Length const> inner_radii,
                   std::span<units::Length const> outer_radii)
    -> GGEMSPolyhedra &;

  /*!
   * \brief Places the local frame origin in the parent frame.
   * \param[in] position Signed integer-pm origin; not the profile midpoint.
   * \return This solid for chained configuration.
   */
  auto SetPosition(Position3PM position) noexcept -> GGEMSPolyhedra &;

  /*!
   * \brief Replaces the active local-to-parent rotation by Rz * Ry * Rx.
   * \param[in] angle_x Rotation about the fixed X axis.
   * \param[in] angle_y Rotation about the fixed Y axis.
   * \param[in] angle_z Rotation about the fixed Z axis.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the rotation is invalid.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSPolyhedra &;

  /*!
   * \brief Sets the one owned medium for the entire profile.
   * \param[in] material Explicit owned medium.
   * \return This solid for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSPolyhedra &;

  /*!
   * \brief Checks complete authoring state and noncollapsed FP32 sections.
   * \throws core::GGEMSRecoverable If side count, profile, or Material is
   * absent, or FP32 conversion collapses a section height or radial wall.
   * World-clearance and surface-band admission belongs to GGEMSWorld.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the configured regular-polygon side count.
   * \return At least three after SetSideCount(), or zero before configuration.
   */
  [[nodiscard]] auto GetSideCount() const noexcept -> std::uint32_t;

  /*!
   * \brief Returns the authored first-vertex angle.
   * \return Angle in integer picoradians, measured from local +X toward +Y.
   */
  [[nodiscard]] auto GetStartAngle() const noexcept -> units::Angle;

  /*!
   * \brief Borrows the authored axial profile.
   * \return Increasing local-Z planes in integer pm; SetZProfile() or
   * destruction invalidates the view.
   */
  [[nodiscard]] auto GetZProfilePM() const noexcept
    -> std::span<GGEMSPolyhedraProfile const>;

  /*!
   * \brief Returns the local frame origin in the parent frame.
   * \return Signed integer-pm position, which need not be the profile midpoint.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Borrows the proper local-to-parent rotation.
   * \return Owned rotation, initially identity; it must not outlive this solid.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Borrows the owned material.
   * \return Material valid for this solid's lifetime.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Global local-frame origin. */
  Position3PM position_pm_{};

  /*! \brief Regular polygon side count; zero until configuration. */
  std::uint32_t side_count_{0};

  /*! \brief First-vertex orientation in canonical angular units. */
  units::Angle start_angle_{};

  /*! \brief Validated continuous local axial profile. */
  std::vector<GGEMSPolyhedraProfile> profile_;

  /*! \brief Proper local-to-parent rotation. */
  GGEMSRotation3D rotation_{};

  /*! \brief One owned medium for all sections. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
