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
 * \brief Validates Ellipsoid authoring dimensions and its shared rigid pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSEllipsoid.hh"

#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

auto GGEMSEllipsoid::SetSemiAxes(units::Length axis_x, units::Length axis_y,
                                 units::Length axis_z) -> GGEMSEllipsoid & {
  if (axis_x.value == 0ULL || axis_y.value == 0ULL || axis_z.value == 0ULL) {
    throw core::GGEMSRecoverable(
      "Ellipsoid semi-axes must be strictly positive.");
  }
  semi_axes_pm_ = {.x = axis_x.value, .y = axis_y.value, .z = axis_z.value};
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::SetPosition(Position3PM center) noexcept
  -> GGEMSEllipsoid & {
  center_pm_ = center;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::SetRotation(units::Angle angle_x, units::Angle angle_y,
                                 units::Angle angle_z) -> GGEMSEllipsoid & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSEllipsoid & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::Validate() const -> void {
  if (semi_axes_pm_.x == 0ULL) {
    throw core::GGEMSRecoverable("Ellipsoid dimensions are not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("Ellipsoid material is not set.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::GetSemiAxesPM() const noexcept -> HalfExtent3PM {
  return semi_axes_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSEllipsoid::GetMaterial() const
  -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Ellipsoid material is not set.");
  }

  return *material_;
}

} // namespace ggems::geometry
