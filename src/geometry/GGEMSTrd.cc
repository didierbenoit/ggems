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
 * \brief Validates Trd authoring and owns its dimensions, pose and Material.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSTrd.hh"

#include <array>
#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

auto GGEMSTrd::SetNegativeZFace(units::Length size_x, units::Length size_y)
  -> GGEMSTrd & {
  std::array<DistancePM, 2U> const half{
    detail::RequireEvenHalfExtentPM(size_x, "Trd", "X"),
    detail::RequireEvenHalfExtentPM(size_y, "Trd", "Y"),
  };
  negative_half_size_pm_ = half;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::SetPositiveZFace(units::Length size_x, units::Length size_y)
  -> GGEMSTrd & {
  std::array<DistancePM, 2U> const half{
    detail::RequireEvenHalfExtentPM(size_x, "Trd", "X"),
    detail::RequireEvenHalfExtentPM(size_y, "Trd", "Y"),
  };
  positive_half_size_pm_ = half;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::SetLength(units::Length length) -> GGEMSTrd & {
  half_length_pm_ = detail::RequireEvenHalfExtentPM(length, "Trd", "Z");

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::SetPosition(Position3PM center) noexcept -> GGEMSTrd & {
  center_pm_ = center;

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::SetRotation(units::Angle angle_x, units::Angle angle_y,
                           units::Angle angle_z) -> GGEMSTrd & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSTrd & {
  material_ = std::move(material);

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::Validate() const -> void {
  if (negative_half_size_pm_[0] == 0ULL || positive_half_size_pm_[0] == 0ULL ||
      half_length_pm_ == 0ULL) {
    throw core::GGEMSRecoverable("Trd dimensions are not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("Trd material is not set.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetNegativeZHalfSizePM() const noexcept
  -> std::array<DistancePM, 2U> {
  return negative_half_size_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetPositiveZHalfSizePM() const noexcept
  -> std::array<DistancePM, 2U> {
  return positive_half_size_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetHalfLengthPM() const noexcept -> DistancePM {
  return half_length_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetPositionPM() const noexcept -> Position3PM {
  return center_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

// -----------------------------------------------------------------------------

auto GGEMSTrd::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Trd material is not set.");
  }

  return *material_;
}

} // namespace ggems::geometry
