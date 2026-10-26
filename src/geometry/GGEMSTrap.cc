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
 * \brief Validates Trap authoring and owns its dimensions, pose and Material.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSTrap.hh"

#include <array>
#include <utility>
#include <cmath>
#include <numbers>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/geometry/detail/GGEMSPlanarGeometry.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

/*!
 * \brief Rejects tangent-angle authoring outside the FP32 finite-slope domain.
 * \param[in] angle Canonical angle.
 * \throws ggems::core::GGEMSRecoverable Unless its FP32 radians are in
 * (-pi/2,pi/2).
 */
auto RequireSlopeAngle(ggems::units::Angle angle) -> void {
  if (!(std::abs(static_cast<float>(ggems::units::ToRadians(angle))) <
        std::numbers::pi_v<float> * 0.5F)) {
    throw ggems::core::GGEMSRecoverable(
      "Trap inclination and skew must be strictly between -90 and +90 "
      "degrees.");
  }
}

} // namespace

namespace ggems::geometry {

auto GGEMSTrap::SetNegativeZFace(units::Length height_y,
                                 units::Length width_at_negative_y,
                                 units::Length width_at_positive_y,
                                 units::Angle skew) -> GGEMSTrap & {
  RequireSlopeAngle(skew);
  std::array<DistancePM, 3U> const half{
    detail::RequireEvenHalfExtentPM(height_y, "Trap", "Y"),
    detail::RequireEvenHalfExtentPM(width_at_negative_y, "Trap", "-Y X"),
    detail::RequireEvenHalfExtentPM(width_at_positive_y, "Trap", "+Y X"),
  };
  negative_half_size_pm_ = half;
  negative_z_skew_ = skew;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetPositiveZFace(units::Length height_y,
                                 units::Length width_at_negative_y,
                                 units::Length width_at_positive_y,
                                 units::Angle skew) -> GGEMSTrap & {
  RequireSlopeAngle(skew);
  std::array<DistancePM, 3U> const half{
    detail::RequireEvenHalfExtentPM(height_y, "Trap", "Y"),
    detail::RequireEvenHalfExtentPM(width_at_negative_y, "Trap", "-Y X"),
    detail::RequireEvenHalfExtentPM(width_at_positive_y, "Trap", "+Y X"),
  };
  positive_half_size_pm_ = half;
  positive_z_skew_ = skew;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetLength(units::Length length) -> GGEMSTrap & {
  half_length_pm_ = detail::RequireEvenHalfExtentPM(length, "Trap", "Z");

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetInclination(units::Angle theta, units::Angle phi)
  -> GGEMSTrap & {
  RequireSlopeAngle(theta);
  inclination_theta_ = theta;
  inclination_phi_ = phi;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetPosition(Position3PM center) noexcept -> GGEMSTrap & {
  center_pm_ = center;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetRotation(units::Angle angle_x, units::Angle angle_y,
                            units::Angle angle_z) -> GGEMSTrap & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSTrap & {
  material_ = std::move(material);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::Validate() const -> void {
  if (negative_half_size_pm_[0] == 0ULL || positive_half_size_pm_[0] == 0ULL ||
      half_length_pm_ == 0ULL) {
    throw core::GGEMSRecoverable("Trap dimensions are not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("Trap material is not set.");
  }

  detail::RequirePlanarTrap(*this);
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetNegativeZHalfSizePM() const noexcept
  -> std::array<DistancePM, 3U> {
  return negative_half_size_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetPositiveZHalfSizePM() const noexcept
  -> std::array<DistancePM, 3U> {
  return positive_half_size_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetNegativeZSkew() const noexcept -> units::Angle {
  return negative_z_skew_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetPositiveZSkew() const noexcept -> units::Angle {
  return positive_z_skew_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetInclinationTheta() const noexcept -> units::Angle {
  return inclination_theta_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetInclinationPhi() const noexcept -> units::Angle {
  return inclination_phi_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetHalfLengthPM() const noexcept -> DistancePM {
  return half_length_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrap::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Trap material is not set.");
  }

  return *material_;
}

} // namespace ggems::geometry
