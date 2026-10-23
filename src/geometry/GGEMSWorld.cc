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
#include <algorithm>
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
#include <variant>

#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"

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

/*! \brief Immutable FP32 surface envelope used only during host admission. */
struct SurfaceScale {
  std::array<double, 3U> envelope_pm{};
  std::array<double, 3U> band_projection{1.0, 1.0, 1.0};
  double minimum_half_extent_pm{0.0};
  float local_scale_pm{0.0F};
};

/*!
 * \brief Returns the upward binary32 spacing at a nonnegative scene scale.
 * \param[in] magnitude Represented coordinate or extent magnitude in pm.
 * \return Spacing in pm; zero for an unconfigured zero scale.
 */
[[nodiscard]] auto SpacingPM(float magnitude) noexcept -> float {
  return magnitude == 0.0F
           ? 0.0F
           : std::nextafter(magnitude, std::numeric_limits<float>::infinity()) -
               magnitude;
}

/*!
 * \brief Returns the largest represented World coordinate spacing in pm.
 * \param[in] half World half extents in canonical pm.
 * \return Binary32 spacing at the largest uploaded half extent.
 */
[[nodiscard]] auto WorldSpacingPM(ggems::geometry::HalfExtent3PM half) noexcept
  -> float {
  return SpacingPM(static_cast<float>(std::max({half.x, half.y, half.z})));
}

/*!
 * \brief Bounds a Box using its actual uploaded center, dimensions and axes.
 * \param[in] box Complete authored Box.
 * \return Global surface envelope and local feature scale in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSBox const &box) noexcept
  -> SurfaceScale {
  auto const half = box.GetHalfExtentPM();
  auto const center = box.GetPositionPM();
  auto const &rotation = box.GetRotation();

  std::array<double, 3U> const extent{
    static_cast<float>(half.x),
    static_cast<float>(half.y),
    static_cast<float>(half.z),
  };

  std::array<double, 3U> const position{
    static_cast<float>(center.x),
    static_cast<float>(center.y),
    static_cast<float>(center.z),
  };

  std::array const rows{
    std::array{
      static_cast<float>(rotation.axis_x.x),
      static_cast<float>(rotation.axis_y.x),
      static_cast<float>(rotation.axis_z.x),
    },
    std::array{
      static_cast<float>(rotation.axis_x.y),
      static_cast<float>(rotation.axis_y.y),
      static_cast<float>(rotation.axis_z.y),
    },
    std::array{
      static_cast<float>(rotation.axis_x.z),
      static_cast<float>(rotation.axis_y.z),
      static_cast<float>(rotation.axis_z.z),
    },
  };

  SurfaceScale scale{
    .minimum_half_extent_pm = std::min({extent[0], extent[1], extent[2]}),
    .local_scale_pm =
      static_cast<float>(std::max({extent[0], extent[1], extent[2]})),
  };

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    scale.envelope_pm[axis] = std::abs(position[axis]);
    scale.band_projection[axis] = 0.0;
    for (std::size_t column = 0U; column < 3U; ++column) {
      scale.envelope_pm[axis] += std::abs(rows[axis][column]) * extent[column];
      scale.band_projection[axis] += std::abs(rows[axis][column]);
    }
  }

  return scale;
}

/*!
 * \brief Bounds a Sphere using its actual uploaded center and radius.
 * \param[in] sphere Complete authored Sphere.
 * \return Global surface envelope and local radius in pm.
 */
[[nodiscard]] auto
BuildSurfaceScale(ggems::geometry::GGEMSSphere const &sphere) noexcept
  -> SurfaceScale {
  auto const center = sphere.GetPositionPM();
  auto const radius = static_cast<float>(sphere.GetRadiusPM());

  return {
    .envelope_pm =
      {
        std::abs(static_cast<double>(static_cast<float>(center.x))) + radius,
        std::abs(static_cast<double>(static_cast<float>(center.y))) + radius,
        std::abs(static_cast<double>(static_cast<float>(center.z))) + radius,
      },
    .minimum_half_extent_pm = radius,
    .local_scale_pm = radius,
  };
}

/*!
 * \brief Returns the representability scale of the solid's surface operations.
 * \param[in] scale Uploaded geometry's static envelope.
 * \return Largest local or global surface spacing in pm.
 */
[[nodiscard]] auto SurfaceSpacingPM(SurfaceScale const &scale) noexcept
  -> float {
  return SpacingPM(std::max(
    scale.local_scale_pm,
    static_cast<float>(std::max(
      {scale.envelope_pm[0], scale.envelope_pm[1], scale.envelope_pm[2]}))));
}

/*!
 * \brief Derives a dimensionless-margin FP32 surface half-band on the host.
+ * \param[in] scale Uploaded geometry's static envelope.
 * \param[in] world_spacing Largest World coordinate spacing in pm.
 * \return Half-band in pm, without a fixed physical-distance floor.
 */
[[nodiscard]] auto RequiredTolerancePM(SurfaceScale const &scale,
                                       float world_spacing) noexcept -> float {
  // Sixteen surface spacings cover upload, subtraction and local predicate
  // arithmetic. Every query additionally reserves eight World spacings for
  // remote ray parameters and movement. These engineering margins are qualified
  // by the device matrix; they are not exact-sign or continuum error proofs.
  return std::max(16.0F * SurfaceSpacingPM(scale), 8.0F * world_spacing);
}

/*!
 * \brief Rejects an explicit half-band below the admitted FP32 floor.
 * \param[in] tolerance Selected half-band in pm.
 * \param[in] required Minimum admitted half-band in pm.
 * \param[in] boundary Boundary label for the user diagnostic.
 * \throws ggems::core::GGEMSRecoverable If the half-band is not usable.
 */
auto RequireTolerance(float tolerance, float required,
                      std::string_view boundary) -> void {
  if (!std::isnormal(tolerance) || tolerance < required) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "{} navigation tolerance {} pm is below the required FP32 half-band "
      "{} pm; increase the tolerance or reduce the coordinate envelope.",
      boundary, tolerance, required));
  }
}

/*!
 * \brief Requires distinct interior and World-clearance tolerance bands.
 * \param[in] world_half Uploaded World half extents' authoring values.
 * \param[in] scale Uploaded solid envelope and minimum feature half-width.
 * \param[in] world_tolerance Effective World half-band in pm.
 * \param[in] tolerance Effective solid half-band in pm.
 * \throws ggems::core::GGEMSRecoverable If a feature or gap is unresolved.
 */
auto RequireSeparatedBands(ggems::geometry::HalfExtent3PM world_half,
                           SurfaceScale const &scale, float world_tolerance,
                           float tolerance) -> void {
  auto const spacing = SurfaceSpacingPM(scale);

  if (!(scale.minimum_half_extent_pm >
        static_cast<double>(tolerance) + spacing)) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "The smallest solid half-width/radius {} pm cannot separate navigation "
      "half-band {} pm and FP32 spacing {} pm; enlarge the feature or reduce "
      "the coordinate envelope.",
      scale.minimum_half_extent_pm, tolerance, spacing));
  }

  std::array<float, 3U> const half{
    static_cast<float>(world_half.x),
    static_cast<float>(world_half.y),
    static_cast<float>(world_half.z),
  };

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const gap = static_cast<double>(half[axis]) - scale.envelope_pm[axis];
    auto const required = static_cast<double>(world_tolerance) +
                          (tolerance * scale.band_projection[axis]) +
                          std::max(SpacingPM(half[axis]), spacing);

    if (!(gap > required)) {
      throw ggems::core::GGEMSRecoverable(std::format(
        "World/solid gap on axis {} is {} pm after FP32 packing; it must "
        "exceed {} pm to separate both navigation bands and one spacing.",
        axis, gap, required));
    }
  }
}

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

auto GGEMSWorld::GetNavigationTolerancePM() const noexcept -> float {
  if (half_extent_pm_.x == 0ULL) {
    return 0.0F;
  }

  return navigation_tolerance_
           ? static_cast<float>(navigation_tolerance_->value)
           : 8.0F * WorldSpacingPM(half_extent_pm_);
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetVolumeNavigationTolerancePM() const noexcept -> float {
  if (auto const *box = std::get_if<GGEMSBox>(&volume_)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*box),
                                   WorldSpacingPM(half_extent_pm_));
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume_)) {
    return navigation_tolerance_
             ? static_cast<float>(navigation_tolerance_->value)
             : RequiredTolerancePM(BuildSurfaceScale(*sphere),
                                   WorldSpacingPM(half_extent_pm_));
  }
  return 0.0F;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSWorld & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSBox box) -> GGEMSWorld & {
  if (!std::holds_alternative<std::monostate>(volume_)) {
    throw core::GGEMSRecoverable(
      "The current slice admits at most one Box or one Sphere.");
  }

  box.Validate();
  volume_ = std::move(box);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::AddVolume(GGEMSSphere sphere) -> GGEMSWorld & {
  if (!std::holds_alternative<std::monostate>(volume_)) {
    throw core::GGEMSRecoverable(
      "The current slice admits at most one Box or one Sphere.");
  }
  sphere.Validate();
  volume_ = std::move(sphere);
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

  auto const world_spacing = WorldSpacingPM(half_extent_pm_);
  auto const world_tolerance = GetNavigationTolerancePM();
  RequireTolerance(world_tolerance, 8.0F * world_spacing, "World");
  if (!(static_cast<float>(
          std::min({half_extent_pm_.x, half_extent_pm_.y, half_extent_pm_.z})) >
        world_tolerance + world_spacing)) {
    throw core::GGEMSRecoverable(
      "World half extents must separate both navigation surface bands.");
  }

  for (auto const &box : GetBoxes()) {
    if (!IsStrictlyInside(half_extent_pm_, box)) {
      throw core::GGEMSRecoverable(
        "Every Box must lie strictly inside the World; coincident or "
        "crossing faces are not supported.");
    }

    auto const scale = BuildSurfaceScale(box);
    auto const tolerance = GetVolumeNavigationTolerancePM();

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Box");

    RequireSeparatedBands(half_extent_pm_, scale, world_tolerance, tolerance);
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume_)) {
    auto const radius = sphere->GetRadiusPM();
    auto const center = sphere->GetPositionPM();

    // Compare margins before subtracting signed centers or adding the radius.
    if (radius >= half_extent_pm_.x || radius >= half_extent_pm_.y ||
        radius >= half_extent_pm_.z) {
      throw core::GGEMSRecoverable(
        "Sphere must lie strictly inside the World.");
    }

    auto const scale = BuildSurfaceScale(*sphere);
    auto const tolerance = GetVolumeNavigationTolerancePM();

    RequireTolerance(tolerance, RequiredTolerancePM(scale, world_spacing),
                     "Sphere");

    RequireSeparatedBands(half_extent_pm_, scale, world_tolerance, tolerance);

    auto const margin_x = static_cast<CoordinatePM>(half_extent_pm_.x - radius);
    auto const margin_y = static_cast<CoordinatePM>(half_extent_pm_.y - radius);
    auto const margin_z = static_cast<CoordinatePM>(half_extent_pm_.z - radius);

    if (center.x <= -margin_x || center.x >= margin_x ||
        center.y <= -margin_y || center.y >= margin_y ||
        center.z <= -margin_z || center.z >= margin_z) {
      throw core::GGEMSRecoverable(
        "Sphere must lie strictly inside the World.");
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxRecords(std::span<std::uint32_t const> material_ids)
  const -> std::vector<GGEMSBoxRecord> {
  auto const boxes = GetBoxes();
  std::vector<GGEMSBoxRecord> records;
  records.reserve(boxes.size());

  for (std::size_t index = 0U; index < boxes.size(); ++index) {
    HalfExtent3PM const half = boxes[index].GetHalfExtentPM();

    records.push_back(GGEMSBoxRecord{
      .half_extent_x_pm = static_cast<float>(half.x),
      .half_extent_y_pm = static_cast<float>(half.y),
      .half_extent_z_pm = static_cast<float>(half.z),
      .volume_id = static_cast<std::uint32_t>(index) + 1U,
      .material_id = material_ids[index],
      .tolerance_pm = GetVolumeNavigationTolerancePM(),
    });
  }

  return records;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildBoxPoseRecord() const
  -> std::optional<GGEMSBoxPoseRecord> {
  auto const boxes = GetBoxes();
  if (boxes.empty()) {
    return std::nullopt;
  }

  auto const center = boxes.front().GetPositionPM();
  auto const &rotation = boxes.front().GetRotation();

  return GGEMSBoxPoseRecord{
    .center_x_pm = static_cast<float>(center.x),
    .center_y_pm = static_cast<float>(center.y),
    .center_z_pm = static_cast<float>(center.z),
    .axis_x_x = static_cast<float>(rotation.axis_x.x),
    .axis_x_y = static_cast<float>(rotation.axis_x.y),
    .axis_x_z = static_cast<float>(rotation.axis_x.z),
    .axis_y_x = static_cast<float>(rotation.axis_y.x),
    .axis_y_y = static_cast<float>(rotation.axis_y.y),
    .axis_y_z = static_cast<float>(rotation.axis_y.z),
    .axis_z_x = static_cast<float>(rotation.axis_z.x),
    .axis_z_y = static_cast<float>(rotation.axis_z.y),
    .axis_z_z = static_cast<float>(rotation.axis_z.z),
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
  return {
    .half_extent_x_pm = static_cast<float>(half_extent_pm_.x),
    .half_extent_y_pm = static_cast<float>(half_extent_pm_.y),
    .half_extent_z_pm = static_cast<float>(half_extent_pm_.z),
    .tolerance_pm = GetNavigationTolerancePM(),
  };
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
  auto const *box = std::get_if<GGEMSBox>(&volume_);
  return box != nullptr ? std::span<GGEMSBox const>{box, 1U}
                        : std::span<GGEMSBox const>{};
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::GetVolumeMaterial() const
  -> core::materials::GGEMSMaterial const * {
  if (auto const *box = std::get_if<GGEMSBox>(&volume_)) {
    return &box->GetMaterial();
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume_)) {
    return &sphere->GetMaterial();
  }

  return nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSWorld::BuildAnalyticVolumeRecord(std::uint32_t material_id) const
  -> GGEMSAnalyticVolumeRecord {
  if (std::holds_alternative<GGEMSBox>(volume_)) {
    std::array const material_ids{material_id};
    return {
      .kind = GGEMSAnalyticKind::Box,
      .parameters = {.box = BuildBoxRecords(material_ids).front()},
    };
  }

  if (auto const *sphere = std::get_if<GGEMSSphere>(&volume_)) {
    auto const center = sphere->GetPositionPM();
    return {
      .kind = GGEMSAnalyticKind::Sphere,
      .parameters =
        {
          .sphere =
            {
              .center_x_pm = static_cast<float>(center.x),
              .center_y_pm = static_cast<float>(center.y),
              .center_z_pm = static_cast<float>(center.z),
              .radius_pm = static_cast<float>(sphere->GetRadiusPM()),
              .volume_id = 1U,
              .material_id = material_id,
              .tolerance_pm = GetVolumeNavigationTolerancePM(),
            },
        },
    };
  }

  return {};
}

} // namespace ggems::geometry
