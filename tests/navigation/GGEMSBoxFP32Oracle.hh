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

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMSFP32TestOracle.hh"

namespace ggems::test::box_oracle {

using fp32::Scalar;
using RayPoint = std::array<Scalar, 3>;
using Direction = std::array<float, 3>;

inline auto PackedPoint(auto const &position) -> RayPoint {
  return {
    fp32::FromFloat(static_cast<float>(position[0])),
    fp32::FromFloat(static_cast<float>(position[1])),
    fp32::FromFloat(static_cast<float>(position[2])),
  };
}

inline auto Dot(RayPoint const &left, Direction const &right) -> Scalar {
  // Binary64 center value is the independent real dot product of the uploaded
  // floats. The bound covers three products and two sums, including possible
  // FMA contraction. This qualifies the scalar dot lowering used by the three
  // current devices; it is not a universal bound for every OpenCL dot lowering.
  auto result = fp32::Multiply(left[0], fp32::FromFloat(right[0]));
  for (std::size_t axis = 1; axis < 3; ++axis) {
    result = fp32::Add(
      result, fp32::Multiply(left[axis], fp32::FromFloat(right[axis])));
  }
  return result;
}

inline auto HalfExtents(geometry::GGEMSBoxRecord const &box) -> Direction {
  return {box.half_extent_x_pm, box.half_extent_y_pm, box.half_extent_z_pm};
}

inline auto HalfExtents(geometry::GGEMSWorldRecord const &world) -> Direction {
  return {
    world.half_extent_x_pm,
    world.half_extent_y_pm,
    world.half_extent_z_pm,
  };
}

inline auto Center(geometry::GGEMSBoxPoseRecord const &pose) -> Direction {
  return {pose.center_x_pm, pose.center_y_pm, pose.center_z_pm};
}

inline auto Normal(geometry::GGEMSBoxPoseRecord const &pose, std::size_t axis)
  -> Direction {
  std::array<Direction, 3> const axes{
    {
      {pose.axis_x_x, pose.axis_x_y, pose.axis_x_z},
      {pose.axis_y_x, pose.axis_y_y, pose.axis_y_z},
      {pose.axis_z_x, pose.axis_z_y, pose.axis_z_z},
    },
  };
  return axes[axis];
}

// The face is prescribed by the independent fixture's physical topology. For
// a true tie, any one of its planes gives the same analytic intersection; the
// entire mask is still compared exactly with the device result by the caller.
inline auto Parameter(geometry::GGEMSWorldRecord const &world,
                      geometry::GGEMSBoxRecord const &box,
                      geometry::GGEMSBoxPoseRecord const &pose,
                      RayPoint const &position, Direction const &direction,
                      std::uint32_t faces, bool world_exit) -> Scalar {
  auto const face = static_cast<unsigned>(std::countr_zero(faces));
  auto const axis = static_cast<std::size_t>(face / 2U);
  float const sign = face % 2U == 0U ? -1.0F : 1.0F;
  if (world_exit) {
    return fp32::Divide(
      fp32::Subtract(fp32::FromFloat(sign * HalfExtents(world)[axis]),
                     position[axis]),
      fp32::FromFloat(direction[axis]));
  }
  auto const center = Center(pose);
  RayPoint offset;
  for (std::size_t component = 0; component < 3; ++component) {
    offset[component] =
      fp32::Subtract(position[component], fp32::FromFloat(center[component]));
  }
  auto const normal = Normal(pose, axis);
  return fp32::Divide(
    fp32::Subtract(fp32::FromFloat(sign * HalfExtents(box)[axis]),
                   Dot(offset, normal)),
    Dot(PackedPoint(direction), normal));
}

inline auto Advance(RayPoint const &position, Direction const &direction,
                    Scalar parameter) -> RayPoint {
  RayPoint result;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] =
      fp32::Add(position[axis],
                fp32::Multiply(parameter, fp32::FromFloat(direction[axis])));
  }
  return result;
}

// An untraced stream only exports its final point. Independently clip its
// original represented line against the six Box planes, then propagate the
// arithmetic budget through the prescribed entry/exit and World planes. This
// is not used as an owner/mask/count oracle; those remain exact independent
// requirements in the tests and the traced companion cases.
inline auto WorldExit(geometry::GGEMSWorldRecord const &world,
                      geometry::GGEMSBoxRecord const *box,
                      geometry::GGEMSBoxPoseRecord const *pose,
                      RayPoint position, Direction const &direction)
  -> RayPoint {
  geometry::GGEMSBoxRecord const unused_box{};
  geometry::GGEMSBoxPoseRecord const unused_pose{};
  auto const &shape = box != nullptr ? *box : unused_box;
  auto const &placement = pose != nullptr ? *pose : unused_pose;
  double world_parameter = std::numeric_limits<double>::infinity();
  std::uint32_t world_face = 0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (direction[axis] == 0.0F) {
      continue;
    }
    auto const face = 1U << ((2U * axis) + (direction[axis] > 0.0F ? 1U : 0U));
    auto const t =
      Parameter(world, shape, placement, position, direction, face, true).value;
    if (t < world_parameter) {
      world_parameter = t;
      world_face = face;
    }
  }
  if (box != nullptr && pose != nullptr) {
    double entry = -std::numeric_limits<double>::infinity();
    double exit = std::numeric_limits<double>::infinity();
    std::uint32_t entry_face = 0;
    std::uint32_t exit_face = 0;
    bool intersects = true;
    auto const center = Center(placement);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      auto const normal = Normal(placement, axis);
      double local = 0;
      double slope = 0;
      for (std::size_t component = 0; component < 3; ++component) {
        local +=
          (position[component].value - center[component]) * normal[component];
        slope += static_cast<double>(direction[component]) * normal[component];
      }
      if (slope == 0.0) {
        intersects = intersects && std::abs(local) < HalfExtents(shape)[axis];
        continue;
      }
      auto near = (-HalfExtents(shape)[axis] - local) / slope;
      auto far = (HalfExtents(shape)[axis] - local) / slope;
      auto near_face = 1U << (2U * axis);
      auto far_face = 1U << ((2U * axis) + 1U);
      if (near > far) {
        std::swap(near, far);
        std::swap(near_face, far_face);
      }
      if (near > entry) {
        entry = near;
        entry_face = near_face;
      }
      if (far < exit) {
        exit = far;
        exit_face = far_face;
      }
    }
    if (intersects && entry < exit && exit > 0.0) {
      if (entry > 0.0 && entry < world_parameter) {
        position = Advance(position, direction,
                           Parameter(world, shape, placement, position,
                                     direction, entry_face, false));
      }
      if (exit < world_parameter) {
        position = Advance(position, direction,
                           Parameter(world, shape, placement, position,
                                     direction, exit_face, false));
      }
    }
  }
  return Advance(
    position, direction,
    Parameter(world, shape, placement, position, direction, world_face, true));
}

} // namespace ggems::test::box_oracle
