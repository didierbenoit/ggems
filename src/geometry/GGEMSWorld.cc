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
 * \brief Validates the authored World scene and compiles its device records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/detail/GGEMSGeometrySize.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using ggems::geometry::CoordinatePM;
using ggems::geometry::DistancePM;

/*!
 * \brief Converts one World full size into its half extent.
 *
 * \param[in] size Full extent along the axis.
 * \param[in] axis Axis label used in diagnostics.
 * \return Half extent in picometers, at most 2^62 - 1.
 * \throws ggems::core::GGEMSRecoverable If the size is zero, odd, or larger
 * than ggems::geometry::k_max_world_size_pm.
 */
[[nodiscard]] auto RequireWorldHalfExtentPM(ggems::units::Length size,
                                            std::string_view axis)
  -> DistancePM {
  if (size.value > ggems::geometry::k_max_world_size_pm) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "World {} size {} pm exceeds the largest admitted size {} pm.", axis,
      size.value, ggems::geometry::k_max_world_size_pm));
  }

  return ggems::geometry::detail::RequireEvenHalfExtentPM(size, "World", axis);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks conservative rotated extrema against exact World margins.
 * \param[in] world_half Canonical World half extents.
 * \param[in] box Box with a nonidentity proper rotation.
 * \return True only for strict containment including a FP64 roundoff allowance.
 */
[[nodiscard]] auto
IsRotatedStrictlyInside(ggems::geometry::HalfExtent3PM world_half,
                        ggems::geometry::GGEMSBox const &box) noexcept -> bool {
  auto const half = box.GetHalfExtentPM();
  auto const center = box.GetPositionPM();
  auto const &rotation = box.GetRotation();

  std::array<DistancePM, 3U> const sizes{half.x, half.y, half.z};
  std::array<DistancePM, 3U> const world{world_half.x, world_half.y,
                                         world_half.z};

  std::array<CoordinatePM, 3U> const position{center.x, center.y, center.z};

  std::array<std::array<double, 3U>, 3U> const rows{{
    {rotation.axis_x.x, rotation.axis_y.x, rotation.axis_z.x},
    {rotation.axis_x.y, rotation.axis_y.y, rotation.axis_z.y},
    {rotation.axis_x.z, rotation.axis_y.z, rotation.axis_z.z},
  }};

  auto const infinity = std::numeric_limits<long double>::infinity();

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const world_radius = static_cast<CoordinatePM>(world[axis]);
    if (position[axis] <= -world_radius || position[axis] >= world_radius) {
      return false;
    }

    CoordinatePM const magnitude =
      position[axis] < 0 ? -position[axis] : position[axis];

    auto const margin = static_cast<long double>(world_radius - magnitude);

    long double radius = 0.0L;

    for (std::size_t column = 0U; column < 3U; ++column) {
      long double const extent =
        std::nextafter(static_cast<long double>(sizes[column]), infinity);

      long double const term = std::nextafter(
        std::abs(static_cast<long double>(rows[axis][column])) * extent,
        infinity);

      radius = std::nextafter(radius + term, infinity);
    }

    // Includes binary64 transpose/query roundoff, never a physical gap.
    radius = std::nextafter(
      radius * (1.0L + (16.0L * std::numeric_limits<double>::epsilon())),
      infinity);

    if (!(radius < std::nextafter(margin, -infinity))) {
      return false;
    }
  }

  return true;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks that a centered Box lies strictly inside the World.
 *
 * The World half extents are at most 2^62 - 1 pm. Each margin is formed only
 * after the Box half extent is known to be smaller, so it is a positive signed
 * value and the center is compared against it directly; no face sum is
 * evaluated, so the check cannot overflow for any signed center.
 *
 * \param[in] world_half World half extents.
 * \param[in] box Complete Box.
 * \return True when every Box face is strictly between the World faces.
 */
[[nodiscard]] auto
IsStrictlyInside(ggems::geometry::HalfExtent3PM world_half,
                 ggems::geometry::GGEMSBox const &box) noexcept -> bool {
  if (!box.GetRotation().IsIdentity()) {
    return IsRotatedStrictlyInside(world_half, box);
  }

  ggems::geometry::HalfExtent3PM const half = box.GetHalfExtentPM();
  if (half.x >= world_half.x || half.y >= world_half.y ||
      half.z >= world_half.z) {
    return false;
  }

  auto const margin_x = static_cast<CoordinatePM>(world_half.x - half.x);
  auto const margin_y = static_cast<CoordinatePM>(world_half.y - half.y);
  auto const margin_z = static_cast<CoordinatePM>(world_half.z - half.z);
  ggems::geometry::Position3PM const center = box.GetPositionPM();
  return center.x > -margin_x && center.x < margin_x && center.y > -margin_y &&
         center.y < margin_y && center.z > -margin_z && center.z < margin_z;
}

} // namespace

// =============================================================================
// =============================================================================

namespace ggems::geometry {

auto GGEMSWorld::SetSize(units::Length size_x, units::Length size_y,
                         units::Length size_z) -> GGEMSWorld & {
  HalfExtent3PM const half_extent{
    .x = RequireWorldHalfExtentPM(size_x, "X"),
    .y = RequireWorldHalfExtentPM(size_y, "Y"),
    .z = RequireWorldHalfExtentPM(size_z, "Z"),
  };

  half_extent_pm_ = half_extent;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSWorld & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSBox box) -> GGEMSWorld & {
  if (!boxes_.empty()) {
    throw core::GGEMSRecoverable("The current slice admits exactly one Box.");
  }

  box.Validate();
  boxes_.push_back(std::move(box));
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::Validate() const -> void {
  if (half_extent_pm_.x == 0ULL) {
    throw core::GGEMSRecoverable("World size is not set.");
  }

  if (!material_) {
    throw core::GGEMSRecoverable("World material is not set.");
  }

  for (auto const &box : boxes_) {
    if (!IsStrictlyInside(half_extent_pm_, box)) {
      throw core::GGEMSRecoverable(
        "Every Box must lie strictly inside the World; coincident or "
        "crossing faces are not supported.");
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxRecords(std::span<std::uint32_t const> material_ids)
  const -> std::vector<GGEMSBoxRecord> {
  std::vector<GGEMSBoxRecord> records;
  records.reserve(boxes_.size());

  for (std::size_t index = 0U; index < boxes_.size(); ++index) {
    HalfExtent3PM const half = boxes_[index].GetHalfExtentPM();

    Position3PM const center = boxes_[index].GetRotation().IsIdentity()
                                 ? boxes_[index].GetPositionPM()
                                 : Position3PM{};

    records.push_back(GGEMSBoxRecord{
      .lower_x_pm = center.x - static_cast<CoordinatePM>(half.x),
      .lower_y_pm = center.y - static_cast<CoordinatePM>(half.y),
      .lower_z_pm = center.z - static_cast<CoordinatePM>(half.z),
      .upper_x_pm = center.x + static_cast<CoordinatePM>(half.x),
      .upper_y_pm = center.y + static_cast<CoordinatePM>(half.y),
      .upper_z_pm = center.z + static_cast<CoordinatePM>(half.z),
      .volume_id = static_cast<std::uint32_t>(index) + 1U,
      .material_id = material_ids[index],
    });
  }

  return records;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxPoseRecord() const
  -> std::optional<GGEMSBoxPoseRecord> {
  if (boxes_.empty() || boxes_.front().GetRotation().IsIdentity()) {
    return std::nullopt;
  }

  auto const center = boxes_.front().GetPositionPM();
  auto const &rotation = boxes_.front().GetRotation();

  return GGEMSBoxPoseRecord{
    .center_x_pm = center.x,
    .center_y_pm = center.y,
    .center_z_pm = center.z,
    .axis_x_x = rotation.axis_x.x,
    .axis_x_y = rotation.axis_x.y,
    .axis_x_z = rotation.axis_x.z,
    .axis_y_x = rotation.axis_y.x,
    .axis_y_y = rotation.axis_y.y,
    .axis_y_z = rotation.axis_y.z,
    .axis_z_x = rotation.axis_z.x,
    .axis_z_y = rotation.axis_z.y,
    .axis_z_z = rotation.axis_z.z,
  };
}

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

auto GGEMSWorld::Contains(Position3PM position) const noexcept -> bool {
  Position3PM const lower = GetLowerCornerPM();
  Position3PM const upper = GetUpperCornerPM();

  return position.x >= lower.x && position.x <= upper.x &&
         position.y >= lower.y && position.y <= upper.y &&
         position.z >= lower.z && position.z <= upper.z;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildRecord() const noexcept -> GGEMSWorldRecord {
  return {.half_extent_x_pm = static_cast<CoordinatePM>(half_extent_pm_.x),
          .half_extent_y_pm = static_cast<CoordinatePM>(half_extent_pm_.y),
          .half_extent_z_pm = static_cast<CoordinatePM>(half_extent_pm_.z)};
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetMaterial() const -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("World material is not set.");
  }

  return *material_;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetBoxes() const noexcept -> std::span<GGEMSBox const> {
  return boxes_;
}

} // namespace ggems::geometry
