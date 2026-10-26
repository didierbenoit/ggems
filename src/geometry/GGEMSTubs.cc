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
 * \brief Validates Tubs authoring dimensions and its shared rigid pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSTubs.hh"

#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"

namespace ggems::geometry {

auto GGEMSTubs::SetRadii(units::Length outer_radius, units::Length inner_radius)
  -> GGEMSTubs & {
  if (outer_radius.value == 0ULL || inner_radius.value >= outer_radius.value) {
    throw core::GGEMSRecoverable("Tubs radii must satisfy 0 <= inner < outer.");
  }
  outer_radius_pm_ = outer_radius.value;
  inner_radius_pm_ = inner_radius.value;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::SetLength(units::Length length) -> GGEMSTubs & {
  half_length_pm_ = detail::RequireEvenHalfExtentPM(length, "Tubs", "Z");

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::SetPosition(Position3PM center) noexcept -> GGEMSTubs & {
  center_pm_ = center;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::SetRotation(units::Angle angle_x, units::Angle angle_y,
                            units::Angle angle_z) -> GGEMSTubs & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSTubs & {
  material_ = std::move(material);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::Validate() const -> void {
  if (outer_radius_pm_ == 0ULL || half_length_pm_ == 0ULL) {
    throw core::GGEMSRecoverable("Tubs dimensions are not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("Tubs material is not set.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetOuterRadiusPM() const noexcept -> DistancePM {
  return outer_radius_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetInnerRadiusPM() const noexcept -> DistancePM {
  return inner_radius_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetHalfLengthPM() const noexcept -> DistancePM {
  return half_length_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSTubs::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Tubs material is not set.");
  }

  return *material_;
}

} // namespace ggems::geometry
