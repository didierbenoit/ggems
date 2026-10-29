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
 * \brief Authors full-azimuth solid and hollow conical frustums.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <optional>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Owns a full-azimuth solid or hollow conical frustum.
 *
 * The local axis is Z, with planar caps at +/- half length. Inner and outer
 * radii vary linearly between their independently authored negative-Z and
 * positive-Z values. SetLength() takes the full cap-to-cap length. Both inner
 * radii zero give a filled solid; a positive inner radius at either end gives
 * a bore, which may close to an inner apex at the opposite end.
 *
 * A zero outer radius is admitted at one end only for a solid cone with both
 * inner radii zero. Two zero outer radii and any hollow outer-apex case are
 * rejected by Validate(). End radii otherwise satisfy inner < outer.
 *
 * The default rigid pose is centered at the parent origin with local axes
 * aligned to parent axes. GGEMSWorld copies the configured value and separately
 * admits its FP32 dimensions, pose, automatic surface half-band, and
 * direct-child World clearance. Nonzero end features and wall thickness must
 * remain resolvable; a true apex is classified directionally within its surface
 * band. Pose coordinates and fixed rotation axes belong to the parent selected
 * by GGEMSWorld::AddVolume; parent zero selects the World frame.
 */
class GGEMSCons {
public:
  /*! \brief Creates an unconfigured frustum with an identity pose. */
  GGEMSCons() = default;

  /*!
   * \brief Sets the inner and outer radii at the negative Z cap.
   * \param[in] inner_radius Nonnegative bore radius.
   * \param[in] outer_radius Outer radius, or zero with zero inner radius.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the radial ordering is invalid.
   */
  auto SetNegativeZRadii(units::Length inner_radius, units::Length outer_radius)
    -> GGEMSCons &;

  /*!
   * \brief Sets the inner and outer radii at the positive Z cap.
   * \param[in] inner_radius Nonnegative bore radius.
   * \param[in] outer_radius Outer radius, or zero with zero inner radius.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the radial ordering is invalid.
   */
  auto SetPositiveZRadii(units::Length inner_radius, units::Length outer_radius)
    -> GGEMSCons &;

  /*!
   * \brief Sets the full cap-to-cap length.
   * \param[in] length Positive even integer-pm length.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the length is zero or odd.
   */
  auto SetLength(units::Length length) -> GGEMSCons &;

  /*!
   * \brief Sets the Parent-frame position of the local cap-midpoint origin.
   * \param[in] center Signed integer-pm parent-frame position.
   * \return This solid for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSCons &;

  /*!
   * \brief Replaces the absolute local-to-parent rotation Rz * Ry * Rx.
   * \param[in] angle_x Right-handed rotation about fixed X.
   * \param[in] angle_y Right-handed rotation about fixed Y.
   * \param[in] angle_z Right-handed rotation about fixed Z.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If a proper basis cannot be formed.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSCons &;

  /*!
   * \brief Sets the owned medium between the inner and outer surfaces.
   * \param[in] material Explicit medium copied or moved into this solid.
   * \return This solid for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSCons &;

  /*!
   * \brief Checks dimensions, apex compatibility, and the owned Material.
   *
   * FP32 feature and World-clearance admission belongs to GGEMSWorld.
   * \throws core::GGEMSRecoverable If dimensions or Material are absent,
   * both outer radii are zero, or an outer apex accompanies a bore.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the outer radius at the local negative-Z cap.
   * \return Radius in integer pm; zero for an apex or an unconfigured end.
   */
  [[nodiscard]] auto GetNegativeZOuterRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the outer radius at the local positive-Z cap.
   * \return Radius in integer pm; zero for an apex or an unconfigured end.
   */
  [[nodiscard]] auto GetPositiveZOuterRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the bore radius at the local negative-Z cap.
   * \return Radius in integer pm; zero closes the bore at this end.
   */
  [[nodiscard]] auto GetNegativeZInnerRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the bore radius at the local positive-Z cap.
   * \return Radius in integer pm; zero closes the bore at this end.
   */
  [[nodiscard]] auto GetPositiveZInnerRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the distance from the local origin to either cap.
   * \return Half length in integer pm; zero before SetLength().
   */
  [[nodiscard]] auto GetHalfLengthPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the local cap-midpoint origin in the parent frame.
   * \return Signed integer-pm position; the parent origin by default.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Borrows the owned local-to-parent rotation.
   * \return Proper rotation, initially identity; it must not outlive this
   * solid.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Borrows the owned frustum medium.
   * \return Material reference; it must not outlive this solid.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Parent-frame position of the local cap-midpoint origin in pm. */
  Position3PM center_pm_{};

  /*!
   * \brief Outer radius at the negative-Z cap in pm; zero admits a solid tip.
   */
  DistancePM negative_z_outer_radius_pm_{0};

  /*!
   * \brief Outer radius at the positive-Z cap in pm; zero admits a solid tip.
   */
  DistancePM positive_z_outer_radius_pm_{0};

  /*! \brief Negative-Z bore radius in pm; zero closes this end. */
  DistancePM negative_z_inner_radius_pm_{0};

  /*! \brief Positive-Z bore radius in pm; zero closes this end. */
  DistancePM positive_z_inner_radius_pm_{0};

  /*! \brief Center-to-cap length in pm; zero until configured. */
  DistancePM half_length_pm_{0};

  /*! \brief Distinguishes an authored negative-Z apex from an unset end. */
  bool negative_z_set_{false};

  /*! \brief Distinguishes an authored positive-Z apex from an unset end. */
  bool positive_z_set_{false};

  /*! \brief Proper local-to-parent rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Explicit owned medium, absent until SetMaterial(). */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
