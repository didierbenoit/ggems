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
 * \brief Implements scene projections, exact views, and pixel-based controls.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace ggems::ui::detail {

namespace {
/*! \brief Maximum absolute orbit pitch, in radians. */
constexpr float k_max_pitch_radians =
  static_cast<float>(units::ToRadians(units::MakeDegrees(90.0L)));

/*! \brief Tangent of half the maximum 45-degree perspective vertical FOV. */
constexpr float k_perspective_half_fov_tangent{0.414213562373095F};

/*! \brief Orbit sensitivity in degrees per logical drag pixel. */
constexpr float k_orbit_degrees_per_pixel{0.20F};
} // namespace

// =============================================================================
// =============================================================================

GGEMSSceneCamera::GGEMSSceneCamera() noexcept { Reset(); }

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::SetProjection(Projection projection) noexcept -> void {
  projection_ = projection;
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::SetView(ViewPlane plane) noexcept -> void {
  Reset();
  switch (plane) {
  case ViewPlane::XY:
    SetOrbitAngles(270.0F, 0.0F);
    break;
  case ViewPlane::XZ:
    SetOrbitAngles(270.0F, 90.0F);
    break;
  case ViewPlane::YZ:
    SetOrbitAngles(0.0F, 0.0F);
    break;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::SetViewportSize(std::uint32_t width,
                                       std::uint32_t height) noexcept -> void {
  if (width == 0U || height == 0U) {
    return;
  }

  if (world_radius_m_ > 0.0F) {
    float const old_aspect = static_cast<float>(viewport_width_) /
                             static_cast<float>(viewport_height_);
    float const new_aspect =
      static_cast<float>(width) / static_cast<float>(height);

    zoom_ *= std::min(1.0F, new_aspect) / std::min(1.0F, old_aspect);
  }

  viewport_width_ = width;
  viewport_height_ = height;
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::SetOrbitAngles(float yaw_degrees, float pitch_degrees)
  -> void {
  auto const yaw = static_cast<float>(units::ToRadians(
    units::MakeDegrees(static_cast<long double>(yaw_degrees))));

  auto const pitch = static_cast<float>(units::ToRadians(
    units::MakeDegrees(static_cast<long double>(pitch_degrees))));

  yaw_radians_ = yaw;
  pitch_radians_ = std::clamp(pitch, -k_max_pitch_radians, k_max_pitch_radians);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::SetZoom(float zoom) noexcept -> void {
  float const aspect =
    static_cast<float>(viewport_width_) / static_cast<float>(viewport_height_);

  float const minimum_zoom =
    world_radius_m_ > 0.0F ? 0.05F * std::min(1.0F, aspect) / world_radius_m_
                           : 0.05F;

  zoom_ = std::max(minimum_zoom, zoom);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::FrameWorld(
  std::array<double, 3> const &half_extent_m) noexcept -> void {
  world_radius_m_ = static_cast<float>(
    std::hypot(half_extent_m[0], half_extent_m[1], half_extent_m[2]));

  depth_scale_ = 0.45F / world_radius_m_;

  Reset();
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Orbit(float delta_yaw_degrees, float delta_pitch_degrees)
  -> void {
  auto const delta_yaw = static_cast<float>(units::ToRadians(
    units::MakeDegrees(static_cast<long double>(delta_yaw_degrees))));

  auto const delta_pitch = static_cast<float>(units::ToRadians(
    units::MakeDegrees(static_cast<long double>(delta_pitch_degrees))));

  yaw_radians_ += delta_yaw;

  pitch_radians_ = std::clamp(pitch_radians_ + delta_pitch,
                              -k_max_pitch_radians, k_max_pitch_radians);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::OrbitByPixels(float delta_x_pixels, float delta_y_pixels)
  -> void {
  if (delta_x_pixels == 0.0F && delta_y_pixels == 0.0F) {
    return;
  }

  Orbit(-delta_x_pixels * k_orbit_degrees_per_pixel,
        -delta_y_pixels * k_orbit_degrees_per_pixel);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Pan(float delta_x_pixels, float delta_y_pixels) noexcept
  -> void {
  if (delta_x_pixels == 0.0F && delta_y_pixels == 0.0F) {
    return;
  }

  if (viewport_height_ == 0U || zoom_ <= 0.0F) {
    return;
  }

  CameraBasis basis = BuildCameraBasis();

  float world_units_per_pixel =
    2.0F / (zoom_ * static_cast<float>(viewport_height_));

  float delta_x_world = delta_x_pixels * world_units_per_pixel;
  float delta_y_world = delta_y_pixels * world_units_per_pixel;

  target_m_.x +=
    (-basis.right.x * delta_x_world) - (basis.up.x * delta_y_world);
  target_m_.y +=
    (-basis.right.y * delta_x_world) - (basis.up.y * delta_y_world);
  target_m_.z +=
    (-basis.right.z * delta_x_world) - (basis.up.z * delta_y_world);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::ZoomBy(float wheel_delta) noexcept -> void {
  if (std::abs(wheel_delta) <= 1.0e-6F) {
    return;
  }

  float zoom_factor = std::pow(1.12F, wheel_delta);
  SetZoom(zoom_ * zoom_factor);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Reset() noexcept -> void {
  target_m_ = Vector3{.x = 0.0F, .y = 0.0F, .z = 0.0F};

  SetOrbitAngles(300.0F, -25.0F);

  float const aspect =
    static_cast<float>(viewport_width_) / static_cast<float>(viewport_height_);

  SetZoom(world_radius_m_ > 0.0F
            ? 0.85F * std::min(1.0F, aspect) / world_radius_m_
            : 0.8F);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::BuildWorldToClipMatrix() const noexcept
  -> GGEMSSceneCamera::Matrix4Rows {
  float inverse_aspect_ratio =
    static_cast<float>(viewport_height_) / static_cast<float>(viewport_width_);

  CameraBasis basis = BuildCameraBasis();

  Vector3 const &right = basis.right;
  Vector3 const &up_reference = basis.up;
  Vector3 const &forward = basis.forward;

  float horizontal_scale = zoom_ * inverse_aspect_ratio;
  float vertical_scale = zoom_;

  PerspectiveDepth const perspective = projection_ == Projection::Perspective
                                         ? BuildPerspectiveDepth(basis)
                                         : PerspectiveDepth{};

  if (projection_ == Projection::Perspective) {
    horizontal_scale *= perspective.distance_m;
    vertical_scale *= perspective.distance_m;
  }

  Matrix4Rows matrix{};

  matrix.row_0[0] = right.x * horizontal_scale;
  matrix.row_0[1] = right.y * horizontal_scale;
  matrix.row_0[2] = right.z * horizontal_scale;
  matrix.row_0[3] = -Dot(right, target_m_) * horizontal_scale;

  matrix.row_1[0] = up_reference.x * vertical_scale;
  matrix.row_1[1] = up_reference.y * vertical_scale;
  matrix.row_1[2] = up_reference.z * vertical_scale;
  matrix.row_1[3] = -Dot(up_reference, target_m_) * vertical_scale;

  matrix.row_2[0] = forward.x * depth_scale_;
  matrix.row_2[1] = forward.y * depth_scale_;
  matrix.row_2[2] = forward.z * depth_scale_;
  matrix.row_2[3] = world_radius_m_ > 0.0F
                      ? 0.5F
                      : 0.5F - (Dot(forward, target_m_) * depth_scale_);

  matrix.row_3[0] = 0.0F;
  matrix.row_3[1] = 0.0F;
  matrix.row_3[2] = 0.0F;
  matrix.row_3[3] = 1.0F;

  if (projection_ == Projection::Perspective) {
    float const eye_depth = perspective.distance_m - Dot(forward, target_m_);
    float const depth_multiplier =
      perspective.far_m / (perspective.far_m - perspective.near_m);

    matrix.row_2 = {
      forward.x * depth_multiplier,
      forward.y * depth_multiplier,
      forward.z * depth_multiplier,
      (eye_depth - perspective.near_m) * depth_multiplier,
    };

    matrix.row_3 = {forward.x, forward.y, forward.z, eye_depth};
  }

  return matrix;
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::GetDiagnostics() const noexcept -> Diagnostics {
  CameraBasis const basis = BuildCameraBasis();

  PerspectiveDepth const perspective = projection_ == Projection::Perspective
                                         ? BuildPerspectiveDepth(basis)
                                         : PerspectiveDepth{};

  return Diagnostics{
    .yaw_degrees = yaw_radians_ * (180.0F / std::numbers::pi_v<float>),
    .pitch_degrees = pitch_radians_ * (180.0F / std::numbers::pi_v<float>),
    .target_m = {target_m_.x, target_m_.y, target_m_.z},
    .projection = projection_,
    .zoom = zoom_,
    .vertical_fov_degrees =
      projection_ == Projection::Perspective
        ? 2.0F * std::atan(1.0F / (zoom_ * perspective.distance_m)) *
            (180.0F / std::numbers::pi_v<float>)
        : 0.0F,
    .distance_m = perspective.distance_m,
    .near_m = perspective.near_m,
    .far_m = perspective.far_m,
    .viewport_width = viewport_width_,
    .viewport_height = viewport_height_,
    .right = {basis.right.x, basis.right.y, basis.right.z},
    .up = {basis.up.x, basis.up.y, basis.up.z},
    .forward = {basis.forward.x, basis.forward.y, basis.forward.z},
  };
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::BuildCameraBasis() const noexcept
  -> GGEMSSceneCamera::CameraBasis {
  float const cos_pitch = std::abs(pitch_radians_) == k_max_pitch_radians
                            ? 0.0F
                            : std::cos(pitch_radians_);

  Vector3 camera_offset{
    .x = std::cos(-yaw_radians_) * cos_pitch,
    .y = std::sin(pitch_radians_),
    .z = std::sin(-yaw_radians_) * cos_pitch,
  };

  Vector3 forward = Normalize(Vector3{
    .x = -camera_offset.x,
    .y = -camera_offset.y,
    .z = -camera_offset.z,
  });

  // Yaw defines a continuous screen frame even at the two exact Y poles.
  Vector3 right = Normalize(Vector3{
    .x = -std::sin(yaw_radians_),
    .y = 0.0F,
    .z = -std::cos(yaw_radians_),
  });

  Vector3 up_reference = Cross(right, forward);

  return CameraBasis{.right = right, .up = up_reference, .forward = forward};
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::BuildPerspectiveDepth(
  CameraBasis const &basis) const noexcept -> PerspectiveDepth {
  float const radius = world_radius_m_ > 0.0F ? world_radius_m_ : 1.0F;
  float const target_depth = Dot(basis.forward, target_m_);
  float const distance =
    std::max(1.0F / (zoom_ * k_perspective_half_fov_tangent),
             target_depth + (1.1F * radius));
  float const world_depth = distance - target_depth;

  return PerspectiveDepth{
    .distance_m = distance,
    .near_m = (world_depth - radius) * 0.5F,
    .far_m = world_depth + (1.05F * radius),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Dot(Vector3 const &first, Vector3 const &second) noexcept
  -> float {
  return (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Cross(Vector3 const &first,
                             Vector3 const &second) noexcept
  -> GGEMSSceneCamera::Vector3 {
  return Vector3{
    .x = (first.y * second.z) - (first.z * second.y),
    .y = (first.z * second.x) - (first.x * second.z),
    .z = (first.x * second.y) - (first.y * second.x),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSSceneCamera::Normalize(Vector3 const &vec) noexcept
  -> GGEMSSceneCamera::Vector3 {
  float length = std::sqrt(Dot(vec, vec));

  if (length <= 1.0e-6F) {
    return Vector3{};
  }

  float inverse_length = 1.0F / length;

  return Vector3{
    .x = vec.x * inverse_length,
    .y = vec.y * inverse_length,
    .z = vec.z * inverse_length,
  };
}
} // namespace ggems::ui::detail
