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
 * \brief Constructs absolute XYZ Euler rotations in the shared frame
 * convention.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <limits>
#include <numbers>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace {

/*!
 * \brief Evaluates an angle once, preserving recognized quarter turns.
 * \param[in] angle Canonical angle in radians.
 * \return Sine and cosine in host working precision.
 * \throws ggems::core::GGEMSRecoverable If the angle is nonfinite.
 */
auto SinCos(ggems::units::Angle angle) -> std::array<long double, 2U> {
  if (!std::isfinite(angle.value)) {
    throw ggems::core::GGEMSRecoverable("Box rotation angles must be finite.");
  }

  long double const half_pi = std::numbers::pi_v<long double> / 2.0L;
  long double const reduced = std::remainder(angle.value, 4.0L * half_pi);
  long double const quarter_turns = reduced / half_pi;

  if (quarter_turns == std::trunc(quarter_turns)) {
    int const quadrant = static_cast<int>(quarter_turns);
    if (quadrant == 0) {
      return {0.0L, 1.0L};
    }
    if (quadrant == 1 || quadrant == -1) {
      return {quadrant > 0 ? 1.0L : -1.0L, 0.0L};
    }
    return {0.0L, -1.0L};
  }

  // Preserve libm argument reduction for noncardinal large angles.
  return {std::sin(angle.value), std::cos(angle.value)};
}

} // namespace

// =============================================================================
// =============================================================================

namespace ggems::geometry {

auto GGEMSRotation3D::IsIdentity() const noexcept -> bool {
  return axis_x.x == 1.0 && axis_x.y == 0.0 && axis_x.z == 0.0 &&
         axis_y.x == 0.0 && axis_y.y == 1.0 && axis_y.z == 0.0 &&
         axis_z.x == 0.0 && axis_z.y == 0.0 && axis_z.z == 1.0;
}

// -----------------------------------------------------------------------------

auto BuildRotationXYZ(units::Angle angle_x, units::Angle angle_y,
                      units::Angle angle_z) -> GGEMSRotation3D {
  auto const [sx, cx] = SinCos(angle_x);
  auto const [sy, cy] = SinCos(angle_y);
  auto const [sz, cz] = SinCos(angle_z);

  GGEMSRotation3D const rotation{
    .axis_x = {.x = static_cast<double>(cz * cy),
               .y = static_cast<double>(sz * cy),
               .z = static_cast<double>(-sy)},
    .axis_y = {.x = static_cast<double>((cz * sy * sx) - (sz * cx)),
               .y = static_cast<double>((sz * sy * sx) + (cz * cx)),
               .z = static_cast<double>(cy * sx)},
    .axis_z = {.x = static_cast<double>((cz * sy * cx) + (sz * sx)),
               .y = static_cast<double>((sz * sy * cx) - (cz * sx)),
               .z = static_cast<double>(cy * cx)},
  };

  auto const &axis_x = rotation.axis_x;
  auto const &axis_y = rotation.axis_y;
  auto const &axis_z = rotation.axis_z;

  auto const cross = detail::Cross3D(axis_x, axis_y);

  double const handedness =
    (cross[0] * axis_z.x) + (cross[1] * axis_z.y) + (cross[2] * axis_z.z);

  double constexpr tolerance = 16.0 * std::numeric_limits<double>::epsilon();

  bool const valid =
    (std::abs(detail::Dot3D(axis_x, axis_x) - 1.0) <= tolerance &&
     std::abs(detail::Dot3D(axis_y, axis_y) - 1.0) <= tolerance &&
     std::abs(detail::Dot3D(axis_z, axis_z) - 1.0) <= tolerance &&
     std::abs(detail::Dot3D(axis_x, axis_y)) <= tolerance &&
     std::abs(detail::Dot3D(axis_x, axis_z)) <= tolerance &&
     std::abs(detail::Dot3D(axis_y, axis_z)) <= tolerance &&
     std::abs(handedness - 1.0) <= tolerance);

  if (!valid) {
    throw core::GGEMSRecoverable(
      "Box rotation must form a proper finite basis.");
  }

  return rotation;
}

} // namespace ggems::geometry
