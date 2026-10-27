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
 * \brief Owns a full-azimuth frustum with independent inner and outer end
 * radii.
 *
 * The local axis is Z, with caps at +/- half length. Radii vary linearly
 * between caps. Two zero inner radii disable the bore. A zero outer endpoint
 * is a solid tip; it is not silently replaced by a finite radius.
 */
class GGEMSCons {
public:
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

  auto SetPosition(Position3PM center) noexcept -> GGEMSCons &;

  /*! \brief Assigns the absolute local-to-global rotation Rz * Ry * Rx. */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSCons &;

  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSCons &;

  /*! \brief Rejects incomplete dimensions, a vanishing solid, or no Material.
   */
  auto Validate() const -> void;

  [[nodiscard]] auto GetNegativeZOuterRadiusPM() const noexcept -> DistancePM;
  [[nodiscard]] auto GetPositiveZOuterRadiusPM() const noexcept -> DistancePM;
  [[nodiscard]] auto GetNegativeZInnerRadiusPM() const noexcept -> DistancePM;
  [[nodiscard]] auto GetPositiveZInnerRadiusPM() const noexcept -> DistancePM;
  [[nodiscard]] auto GetHalfLengthPM() const noexcept -> DistancePM;
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  Position3PM center_pm_{};
  DistancePM negative_z_outer_radius_pm_{0};
  DistancePM positive_z_outer_radius_pm_{0};
  DistancePM negative_z_inner_radius_pm_{0};
  DistancePM positive_z_inner_radius_pm_{0};
  DistancePM half_length_pm_{0};
  bool negative_z_set_{false};
  bool positive_z_set_{false};
  GGEMSRotation3D rotation_{};
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
