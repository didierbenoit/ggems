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
 * \brief Validates and packs continuous full-azimuth polygonal profiles.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSPolyhedra.hh"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <numbers>
#include <span>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/detail/GGEMSPolyhedraGeometry.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

auto GGEMSPolyhedra::SetSideCount(std::uint32_t side_count)
  -> GGEMSPolyhedra & {
  if (side_count < 3U) {
    throw core::GGEMSRecoverable("Polyhedra requires at least three sides.");
  }
  side_count_ = side_count;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::SetStartAngle(units::Angle angle) -> GGEMSPolyhedra & {
  start_angle_ = angle;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::SetZProfile(
  std::span<units::PositionCoordinate const> z_planes,
  std::span<units::Length const> inner_radii,
  std::span<units::Length const> outer_radii) -> GGEMSPolyhedra & {
  if (z_planes.size() < 2U || z_planes.size() != inner_radii.size() ||
      z_planes.size() != outer_radii.size()) {
    throw core::GGEMSRecoverable(
      "Polyhedra requires equally sized Z/inner/outer arrays with at least "
      "two planes.");
  }
  if (z_planes.size() - 1U >
      (std::numeric_limits<std::uint32_t>::max() - 1U) / 4U) {
    throw core::GGEMSRecoverable(
      "Polyhedra section count exceeds the transport event index range.");
  }

  std::vector<GGEMSPolyhedraProfile> profile;
  profile.reserve(z_planes.size());
  for (std::size_t index = 0U; index < z_planes.size(); ++index) {
    if (index > 0U && z_planes[index].value <= z_planes[index - 1U].value) {
      throw core::GGEMSRecoverable(
        "Polyhedra Z planes must increase strictly; repeated-Z steps are "
        "not supported.");
    }
    if (outer_radii[index].value == 0ULL ||
        inner_radii[index].value >= outer_radii[index].value) {
      throw core::GGEMSRecoverable(
        "Polyhedra apothems must satisfy 0 <= inner < outer at every plane.");
    }
    profile.push_back({
      .z_pm = z_planes[index].value,
      .inner_radius_pm = inner_radii[index].value,
      .outer_radius_pm = outer_radii[index].value,
    });
  }
  profile_ = std::move(profile);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::SetPosition(Position3PM position) noexcept
  -> GGEMSPolyhedra & {
  position_pm_ = position;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::SetRotation(units::Angle angle_x, units::Angle angle_y,
                                 units::Angle angle_z) -> GGEMSPolyhedra & {
  rotation_ = BuildRotationXYZ(angle_x, angle_y, angle_z);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::SetMaterial(core::materials::GGEMSMaterial material)
  -> GGEMSPolyhedra & {
  material_ = std::move(material);
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::Validate() const -> void {
  if (side_count_ < 3U || profile_.size() < 2U) {
    throw core::GGEMSRecoverable("Polyhedra dimensions are not set.");
  }
  if (!material_) {
    throw core::GGEMSRecoverable("Polyhedra material is not set.");
  }
  static_cast<void>(detail::BuildPolyhedraSections(*this));
}

// -----------------------------------------------------------------------------

auto GGEMSPolyhedra::GetSideCount() const noexcept -> std::uint32_t {
  return side_count_;
}

auto GGEMSPolyhedra::GetStartAngle() const noexcept -> units::Angle {
  return start_angle_;
}

auto GGEMSPolyhedra::GetZProfilePM() const noexcept
  -> std::span<GGEMSPolyhedraProfile const> {
  return profile_;
}

auto GGEMSPolyhedra::GetPositionPM() const noexcept -> Position3PM {
  return position_pm_;
}

auto GGEMSPolyhedra::GetRotation() const noexcept -> GGEMSRotation3D const & {
  return rotation_;
}

auto GGEMSPolyhedra::GetMaterial() const
  -> core::materials::GGEMSMaterial const & {
  if (!material_) {
    throw core::GGEMSRecoverable("Polyhedra material is not set.");
  }
  return *material_;
}

} // namespace ggems::geometry

namespace ggems::geometry::detail {

auto BuildPolyhedraSection(GGEMSPolyhedraProfile const &lower,
                           GGEMSPolyhedraProfile const &upper) noexcept
  -> GGEMSPolyhedraSectionRecord {
  auto const z_lower = static_cast<float>(lower.z_pm);
  auto const z_upper = static_cast<float>(upper.z_pm);
  auto const outer_lower = static_cast<float>(lower.outer_radius_pm);
  auto const outer_upper = static_cast<float>(upper.outer_radius_pm);
  auto const inner_lower = static_cast<float>(lower.inner_radius_pm);
  auto const inner_upper = static_cast<float>(upper.inner_radius_pm);
  auto const height = static_cast<double>(z_upper) - z_lower;
  auto const outer_slope = static_cast<float>(
    (static_cast<double>(outer_upper) - outer_lower) / height);
  auto const inner_slope = static_cast<float>(
    (static_cast<double>(inner_upper) - inner_lower) / height);
  return {
    .z_lower_pm = z_lower,
    .z_upper_pm = z_upper,
    .outer_radius_pm = outer_lower,
    .outer_slope = outer_slope,
    .outer_normal_length = std::hypot(1.0F, outer_slope),
    .inner_radius_pm = inner_lower,
    .inner_slope = inner_slope,
    .inner_normal_length = std::hypot(1.0F, inner_slope),
  };
}

// -----------------------------------------------------------------------------

auto BuildPolyhedraSections(GGEMSPolyhedra const &polyhedra)
  -> std::vector<GGEMSPolyhedraSectionRecord> {
  auto const profile = polyhedra.GetZProfilePM();
  std::vector<GGEMSPolyhedraSectionRecord> sections;
  sections.reserve(profile.size() - 1U);
  for (std::size_t index = 1U; index < profile.size(); ++index) {
    auto const lower = profile[index - 1U];
    auto const upper = profile[index];
    auto const z_lower = static_cast<float>(lower.z_pm);
    auto const z_upper = static_cast<float>(upper.z_pm);
    auto const outer_lower = static_cast<float>(lower.outer_radius_pm);
    auto const outer_upper = static_cast<float>(upper.outer_radius_pm);
    auto const inner_lower = static_cast<float>(lower.inner_radius_pm);
    auto const inner_upper = static_cast<float>(upper.inner_radius_pm);
    if (z_lower >= z_upper || inner_lower >= outer_lower ||
        inner_upper >= outer_upper) {
      throw core::GGEMSRecoverable("Polyhedra section height or radial wall "
                                   "collapses after FP32 packing.");
    }
    sections.push_back(BuildPolyhedraSection(lower, upper));
  }
  return sections;
}

// -----------------------------------------------------------------------------

auto BuildPolyhedraSides(GGEMSPolyhedra const &polyhedra)
  -> std::vector<GGEMSPolyhedraSideRecord> {
  auto const count = polyhedra.GetSideCount();
  auto const start = units::ToRadians(polyhedra.GetStartAngle());
  std::vector<GGEMSPolyhedraSideRecord> sides;
  sides.reserve(count);
  for (std::uint32_t index = 0U; index < count; ++index) {
    auto const angle = start + (((2.0L * index) + 1.0L) *
                                std::numbers::pi_v<long double> / count);
    sides.push_back({
      .normal_x = static_cast<float>(std::cos(angle)),
      .normal_y = static_cast<float>(std::sin(angle)),
    });
  }
  return sides;
}

// -----------------------------------------------------------------------------

auto BuildPolyhedraData(GGEMSPolyhedra const &polyhedra)
  -> std::vector<std::byte> {
  auto const count = static_cast<std::size_t>(polyhedra.GetSideCount());
  auto const section_count = polyhedra.GetZProfilePM().size() - 1U;
  auto const max_bytes = std::numeric_limits<std::uint32_t>::max();
  if (count > (max_bytes - sizeof(GGEMSBoxPoseRecord)) /
                sizeof(GGEMSPolyhedraSideRecord) ||
      section_count > (max_bytes - sizeof(GGEMSBoxPoseRecord) -
                       (count * sizeof(GGEMSPolyhedraSideRecord))) /
                        sizeof(GGEMSPolyhedraSectionRecord)) {
    throw core::GGEMSRecoverable(
      "Polyhedra auxiliary data exceeds the 32-bit byte index range.");
  }
  auto const sides = BuildPolyhedraSides(polyhedra);
  auto const sections = BuildPolyhedraSections(polyhedra);
  auto const center = polyhedra.GetPositionPM();
  auto const &rotation = polyhedra.GetRotation();
  GGEMSBoxPoseRecord const pose{
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
  auto const side_bytes = sides.size() * sizeof(GGEMSPolyhedraSideRecord);
  auto const section_bytes =
    sections.size() * sizeof(GGEMSPolyhedraSectionRecord);
  std::vector<std::byte> data(sizeof(pose) + side_bytes + section_bytes);
  std::memcpy(data.data(), &pose, sizeof(pose));
  std::memcpy(data.data() + sizeof(pose), sides.data(), side_bytes);
  std::memcpy(data.data() + sizeof(pose) + side_bytes, sections.data(),
              section_bytes);
  return data;
}

} // namespace ggems::geometry::detail
