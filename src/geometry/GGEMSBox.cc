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
 * \brief Validates the center-placed analytic Box authoring parameters.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <format>
#include <string_view>
#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry::detail {

auto RequireEvenHalfExtentPM(units::Length size, std::string_view volume,
                             std::string_view axis) -> DistancePM {
  if (size.value == 0ULL) {
    throw core::GGEMSRecoverable(
      std::format("{} {} size must be strictly positive.", volume, axis));
  }

  if (size.value % 2ULL != 0ULL) {
    throw core::GGEMSRecoverable(std::format(
      "{} {} size must be an even number of picometers so that the centered "
      "faces lie on the integer picometer lattice; got {} pm.",
      volume, axis, size.value));
  }

  return size.value / 2ULL;
}

} // namespace ggems::geometry::detail

// =============================================================================
// =============================================================================

namespace ggems::geometry {

auto GGEMSBox::SetSize(units::Length size_x, units::Length size_y,
                       units::Length size_z) -> GGEMSBox & {
  HalfExtent3PM const half_extent{
    .x = detail::RequireEvenHalfExtentPM(size_x, "Box", "X"),
    .y = detail::RequireEvenHalfExtentPM(size_y, "Box", "Y"),
    .z = detail::RequireEvenHalfExtentPM(size_z, "Box", "Z"),
  };

  half_extent_pm_ = half_extent;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::SetPosition(Position3PM center) noexcept -> GGEMSBox & {
  center_pm_ = center;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::SetRotation(units::Angle angle_x, units::Angle angle_y,
                           units::Angle angle_z) -> GGEMSBox & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSBox & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::Validate() const -> void {
  if (half_extent_pm_.x == 0ULL) {
    throw core::GGEMSRecoverable("Box size is not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("Box material is not set.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetHalfExtentPM() const noexcept -> HalfExtent3PM {
  return half_extent_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Box material is not set.");
  }

  return *material_;
}

} // namespace ggems::geometry
