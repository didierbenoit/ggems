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
 * \brief Validates and packs immutable analytic Box parameters.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <format>
#include <limits>
#include <string_view>
#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

/*!
 * \brief Computes one upper face from a lower face and a validated size.
 *
 * \param[in] lower Lower face coordinate.
 * \param[in] size Full extent along the axis.
 * \param[in] axis Axis label used in diagnostics.
 * \return Upper face coordinate.
 * \throws ggems::core::GGEMSRecoverable If the size is zero or the upper face
 * does not fit the picometer lattice.
 */
auto RequireUpperFace(ggems::geometry::CoordinatePM lower,
                      ggems::units::Length size, std::string_view axis)
  -> ggems::geometry::CoordinatePM {
  if (size.value == 0ULL) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Box {} size must be strictly positive.", axis));
  }

  return lower + static_cast<ggems::geometry::CoordinatePM>(size.value);
}

} // namespace

// =============================================================================
// =============================================================================

namespace ggems::geometry {

GGEMSBox::GGEMSBox(units::Length size_x, units::Length size_y,
                   units::Length size_z, Position3PM lower_corner,
                   core::materials::GGEMSMaterial material)
    : lower_corner_pm_{lower_corner},
      upper_corner_pm_{
        .x = RequireUpperFace(lower_corner.x, size_x, "X"),
        .y = RequireUpperFace(lower_corner.y, size_y, "Y"),
        .z = RequireUpperFace(lower_corner.z, size_z, "Z"),
      },
      material_{std::move(material)} {}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetLowerCornerPM() const noexcept -> Position3PM {
  return lower_corner_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetUpperCornerPM() const noexcept -> Position3PM {
  return upper_corner_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::IsStrictlyInside(GGEMSWorld const &world) const noexcept
  -> bool {
  Position3PM const world_lower = world.GetLowerCornerPM();
  Position3PM const world_upper = world.GetUpperCornerPM();

  return lower_corner_pm_.x > world_lower.x &&
         upper_corner_pm_.x < world_upper.x &&
         lower_corner_pm_.y > world_lower.y &&
         upper_corner_pm_.y < world_upper.y &&
         lower_corner_pm_.z > world_lower.z &&
         upper_corner_pm_.z < world_upper.z;
}

// -----------------------------------------------------------------------------

auto GGEMSBox::BuildRecord(std::uint32_t volume_id,
                           std::uint32_t material_id) const noexcept
  -> GGEMSBoxRecord {
  return GGEMSBoxRecord{
    .lower_x_pm = lower_corner_pm_.x,
    .lower_y_pm = lower_corner_pm_.y,
    .lower_z_pm = lower_corner_pm_.z,
    .upper_x_pm = upper_corner_pm_.x,
    .upper_y_pm = upper_corner_pm_.y,
    .upper_z_pm = upper_corner_pm_.z,
    .volume_id = volume_id,
    .material_id = material_id,
  };
}

// -----------------------------------------------------------------------------

auto GGEMSBox::GetMaterial() const noexcept
  -> core::materials::GGEMSMaterial const & {
  return material_;
}

} // namespace ggems::geometry
