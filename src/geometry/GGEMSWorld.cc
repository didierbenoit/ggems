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
 * \brief Validates World sizes and classifies positions against the World.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSWorld.hh"

#include <format>
#include <string_view>
#include <utility>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

/*!
 * \brief Converts one validated full World size into its half extent.
 *
 * \param[in] size Full extent along the axis.
 * \param[in] axis Axis label used in diagnostics.
 * \return Half extent in picometers.
 * \throws ggems::core::GGEMSRecoverable If the size is zero, odd, or larger
 * than ggems::geometry::k_max_world_size_pm.
 */
[[nodiscard]] auto RequireHalfExtent(ggems::units::Length size,
                                     std::string_view axis)
  -> ggems::geometry::DistancePM {
  if (size.value == 0ULL) {
    throw ggems::core::GGEMSRecoverable(
      std::format("World {} size must be strictly positive.", axis));
  }

  if (size.value % 2ULL != 0ULL) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "World {} size must be an even number of picometers so that the "
      "origin-centered faces lie on the integer picometer lattice; got {} pm.",
      axis, size.value));
  }

  if (size.value > ggems::geometry::k_max_world_size_pm) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "World {} size {} pm exceeds the largest admitted size {} pm.", axis,
      size.value, ggems::geometry::k_max_world_size_pm));
  }

  return size.value / 2ULL;
}

} // namespace

// =============================================================================
// =============================================================================

namespace ggems::geometry {

GGEMSWorld::GGEMSWorld(units::Length size_x, units::Length size_y,
                       units::Length size_z,
                       core::materials::GGEMSMaterial material)
    : half_extent_pm_{
        .x = RequireHalfExtent(size_x, "X"),
        .y = RequireHalfExtent(size_y, "Y"),
        .z = RequireHalfExtent(size_z, "Z"),
      },
      material_{std::move(material)} {}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetHalfExtentPM() const noexcept -> HalfExtent3PM {
  return half_extent_pm_;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetLowerCornerPM() const noexcept -> Position3PM {
  return Position3PM{
    .x = -static_cast<CoordinatePM>(half_extent_pm_.x),
    .y = -static_cast<CoordinatePM>(half_extent_pm_.y),
    .z = -static_cast<CoordinatePM>(half_extent_pm_.z),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetUpperCornerPM() const noexcept -> Position3PM {
  return Position3PM{
    .x = static_cast<CoordinatePM>(half_extent_pm_.x),
    .y = static_cast<CoordinatePM>(half_extent_pm_.y),
    .z = static_cast<CoordinatePM>(half_extent_pm_.z),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::Classify(Position3PM position) const noexcept
  -> GGEMSWorldRegion {
  Position3PM const lower = GetLowerCornerPM();
  Position3PM const upper = GetUpperCornerPM();

  bool const outside = position.x < lower.x || position.x > upper.x ||
                       position.y < lower.y || position.y > upper.y ||
                       position.z < lower.z || position.z > upper.z;

  if (outside) {
    return GGEMSWorldRegion::Outside;
  }

  bool const on_face = position.x == lower.x || position.x == upper.x ||
                       position.y == lower.y || position.y == upper.y ||
                       position.z == lower.z || position.z == upper.z;

  return on_face ? GGEMSWorldRegion::Boundary : GGEMSWorldRegion::Inside;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::Contains(Position3PM position) const noexcept -> bool {
  return Classify(position) != GGEMSWorldRegion::Outside;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetMaterial() const noexcept
  -> core::materials::GGEMSMaterial const & {
  return material_;
}

} // namespace ggems::geometry
