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
 * \brief Validates Cons authoring dimensions and its shared rigid pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSCons.hh"

#include <cmath>
#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/geometry/detail/GGEMSConsGeometry.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

/*!
 * \brief Checks one end's radial ordering before authoring state changes.
 * \param[in] inner Canonical bore radius in pm.
 * \param[in] outer Canonical outer radius in pm.
 * \throws ggems::core::GGEMSRecoverable Unless inner < outer or both are zero.
 */
auto RequireRadii(ggems::units::Length inner, ggems::units::Length outer)
  -> void {
  if ((outer.value == 0ULL && inner.value != 0ULL) ||
      (outer.value != 0ULL && inner.value >= outer.value)) {
    throw ggems::core::GGEMSRecoverable(
      "Cons end radii must satisfy 0 <= inner < outer, or both be zero.");
  }
}

} // namespace

namespace ggems::geometry {

auto GGEMSCons::SetNegativeZRadii(units::Length inner_radius,
                                  units::Length outer_radius) -> GGEMSCons & {
  RequireRadii(inner_radius, outer_radius);
  negative_z_inner_radius_pm_ = inner_radius.value;
  negative_z_outer_radius_pm_ = outer_radius.value;
  negative_z_set_ = true;
  return *this;
}

auto GGEMSCons::SetPositiveZRadii(units::Length inner_radius,
                                  units::Length outer_radius) -> GGEMSCons & {
  RequireRadii(inner_radius, outer_radius);
  positive_z_inner_radius_pm_ = inner_radius.value;
  positive_z_outer_radius_pm_ = outer_radius.value;
  positive_z_set_ = true;
  return *this;
}

auto GGEMSCons::SetLength(units::Length length) -> GGEMSCons & {
  half_length_pm_ = detail::RequireEvenHalfExtentPM(length, "Cons", "Z");
  return *this;
}

auto GGEMSCons::SetPosition(Position3PM center) noexcept -> GGEMSCons & {
  center_pm_ = center;
  return *this;
}

auto GGEMSCons::SetRotation(units::Angle angle_x, units::Angle angle_y,
                            units::Angle angle_z) -> GGEMSCons & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

auto GGEMSCons::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSCons & {
  material_ = std::move(material);
  return *this;
}

auto GGEMSCons::Validate() const -> void {
  if (!negative_z_set_ || !positive_z_set_ || half_length_pm_ == 0ULL ||
      (negative_z_outer_radius_pm_ == 0ULL &&
       positive_z_outer_radius_pm_ == 0ULL)) {
    throw core::GGEMSRecoverable(
      "Cons dimensions are not set or have no volume.");
  }
  if ((negative_z_outer_radius_pm_ == 0ULL ||
       positive_z_outer_radius_pm_ == 0ULL) &&
      (negative_z_inner_radius_pm_ != 0ULL ||
       positive_z_inner_radius_pm_ != 0ULL)) {
    throw core::GGEMSRecoverable(
      "A zero Cons outer endpoint requires a solid cone.");
  }
  if (!material_) {
    throw core::GGEMSRecoverable("Cons material is not set.");
  }
}

auto GGEMSCons::GetNegativeZOuterRadiusPM() const noexcept -> DistancePM {
  return negative_z_outer_radius_pm_;
}

auto GGEMSCons::GetPositiveZOuterRadiusPM() const noexcept -> DistancePM {
  return positive_z_outer_radius_pm_;
}

auto GGEMSCons::GetNegativeZInnerRadiusPM() const noexcept -> DistancePM {
  return negative_z_inner_radius_pm_;
}

auto GGEMSCons::GetPositiveZInnerRadiusPM() const noexcept -> DistancePM {
  return positive_z_inner_radius_pm_;
}

auto GGEMSCons::GetHalfLengthPM() const noexcept -> DistancePM {
  return half_length_pm_;
}

auto GGEMSCons::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

auto GGEMSCons::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

auto GGEMSCons::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Cons material is not set.");
  }
  return *material_;
}

namespace detail {

auto BuildConsRecord(GGEMSCons const &solid) noexcept -> GGEMSConsRecord {
  auto const outer_negative =
    static_cast<float>(solid.GetNegativeZOuterRadiusPM());
  auto const outer_positive =
    static_cast<float>(solid.GetPositiveZOuterRadiusPM());
  auto const inner_negative =
    static_cast<float>(solid.GetNegativeZInnerRadiusPM());
  auto const inner_positive =
    static_cast<float>(solid.GetPositiveZInnerRadiusPM());
  auto const half_length = static_cast<float>(solid.GetHalfLengthPM());
  auto const outer_slope =
    (outer_positive - outer_negative) / (2.0F * half_length);
  auto const inner_slope =
    (inner_positive - inner_negative) / (2.0F * half_length);
  return {
    .outer_radius_pm = (0.5F * outer_negative) + (0.5F * outer_positive),
    .inner_radius_pm = (0.5F * inner_negative) + (0.5F * inner_positive),
    .outer_slope = outer_slope,
    .inner_slope = inner_slope,
    .half_length_pm = half_length,
    .outer_normal_scale = std::hypot(1.0F, outer_slope),
    .inner_normal_scale = std::hypot(1.0F, inner_slope),
  };
}

} // namespace detail
} // namespace ggems::geometry
