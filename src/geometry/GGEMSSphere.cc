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
 * \brief Implements full solid Sphere authoring.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSSphere.hh"

#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

auto GGEMSSphere::SetRadius(units::Length radius) -> GGEMSSphere & {
  if (radius.value == 0) {
    throw core::GGEMSRecoverable("Sphere radius must be strictly positive.");
  }
  radius_pm_ = radius.value;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::SetPosition(Position3PM center) noexcept -> GGEMSSphere & {
  center_pm_ = center;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::SetRotation(units::Angle angle_x, units::Angle angle_y,
                              units::Angle angle_z) -> GGEMSSphere & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSSphere & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::Validate() const -> void {
  if (radius_pm_ == 0) {
    throw core::GGEMSRecoverable("Sphere radius is not set.");
  }
  if (!material_) {
    throw core::GGEMSRecoverable("Sphere material is not set.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::GetRadiusPM() const noexcept -> DistancePM {
  return radius_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSSphere::GetMaterial() const
  -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Sphere material is not set.");
  }
  return *material_;
}

} // namespace ggems::geometry
