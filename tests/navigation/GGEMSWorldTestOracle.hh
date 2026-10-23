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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <limits>

#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMSFP32TestOracle.hh"

namespace ggems::test {

struct WorldRayReference {
  std::array<fp32::Scalar, 3> point{};
  std::uint32_t status{0};
  std::uint32_t faces{0};
};

// Binary64 ray/plane intersections on the actual uploaded FP32 scene.
// The spatial budget propagates one plane subtraction, one full-profile
// division (2.5 ULP), one multiply and one addition. Input quantization is
// already represented; it is not added again as a free error allowance.
// Exact ties are retained for symmetric represented fixtures. General
// competing planes must have distinguishable parameters.
inline auto WorldReference(geometry::GGEMSWorldRecord const &world,
                           std::array<float, 3> const &position,
                           std::array<float, 3> const &direction)
  -> WorldRayReference {
  WorldRayReference result;
  std::array<double, 3> const half{
    world.half_extent_x_pm,
    world.half_extent_y_pm,
    world.half_extent_z_pm,
  };
  bool moving{false};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result.point[axis] = fp32::FromFloat(position[axis]);
    moving |= direction[axis] != 0;
    if (std::abs(static_cast<double>(position[axis])) > half[axis]) {
      result.status = 1;
    }
  }
  if (result.status != 0) {
    return result;
  }
  if (!moving) {
    result.status = 2;
    return result;
  }
  double distance = std::numeric_limits<double>::infinity();
  fp32::Scalar parameter{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (direction[axis] == 0) {
      continue;
    }
    double const plane = std::copysign(half[axis], direction[axis]);
    double const candidate =
      (plane - static_cast<double>(position[axis])) / direction[axis];
    auto const face = 1U << ((2U * axis) + (direction[axis] > 0 ? 1U : 0U));
    if (candidate < distance) {
      distance = candidate;
      result.faces = face;
      parameter = fp32::Divide(fp32::Subtract({.value = plane, .error = 0},
                                              fp32::FromFloat(position[axis])),
                               fp32::FromFloat(direction[axis]));
    } else if (candidate == distance) {
      result.faces |= face;
    }
  }
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result.point[axis] =
      fp32::Add(fp32::FromFloat(position[axis]),
                fp32::Multiply(parameter, fp32::FromFloat(direction[axis])));
  }
  return result;
}

} // namespace ggems::test
