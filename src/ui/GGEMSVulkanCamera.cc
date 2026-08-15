#include <algorithm>
#include <cmath>

#include <vulkan/vulkan.hpp>

#include "GGEMS/ui/GGEMSVulkanCamera.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace ggems::ui {

namespace {
constexpr float k_max_pitch_radians =
    static_cast<float>(units::ToRadians(units::MakeDegrees(85.0L)));
} // namespace

// =============================================================================
// =============================================================================

GGEMSVulkanCamera::GGEMSVulkanCamera() noexcept { Reset(); }

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::SetViewportExtent(vk::Extent2D const &extent) noexcept
    -> void {
  if (extent.width == 0U || extent.height == 0U) {
    return;
  }

  viewport_extent_ = extent;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::SetOrbitAngles(float yaw_degrees,
                                       float pitch_degrees) noexcept -> void {
  yaw_radians_ = static_cast<float>(units::ToRadians(
      units::MakeDegrees(static_cast<long double>(yaw_degrees))));

  pitch_radians_ =
      std::clamp(static_cast<float>(units::ToRadians(units::MakeDegrees(
                     static_cast<long double>(pitch_degrees)))),
                 -k_max_pitch_radians, k_max_pitch_radians);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::SetZoom(float zoom) noexcept -> void {
  zoom_ = std::max(0.05F, zoom);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Orbit(float delta_yaw_degrees,
                              float delta_pitch_degrees) noexcept -> void {
  yaw_radians_ += static_cast<float>(units::ToRadians(
      units::MakeDegrees(static_cast<long double>(delta_yaw_degrees))));

  pitch_radians_ = std::clamp(
      pitch_radians_ + static_cast<float>(units::ToRadians(units::MakeDegrees(
                           static_cast<long double>(delta_pitch_degrees)))),
      -k_max_pitch_radians, k_max_pitch_radians);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Pan(float delta_x_pixels, float delta_y_pixels) noexcept
    -> void {
  if (delta_x_pixels == 0.0F && delta_y_pixels == 0.0F) {
    return;
  }

  if (viewport_extent_.height == 0U || zoom_ <= 1.0e-6F) {
    return;
  }

  CameraBasis basis = BuildCameraBasis();

  float world_units_per_pixel =
      2.0F / (zoom_ * static_cast<float>(viewport_extent_.height));

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

auto GGEMSVulkanCamera::ZoomBy(float wheel_delta) noexcept -> void {
  if (std::abs(wheel_delta) <= 1.0e-6F) {
    return;
  }

  float zoom_factor = std::pow(1.12F, wheel_delta);
  SetZoom(zoom_ * zoom_factor);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Reset() noexcept -> void {
  target_m_ = Vector3{.x = 0.0F, .y = 0.0F, .z = 0.0F};
  SetOrbitAngles(315.0F, 45.0F);
  SetZoom(0.8F);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::BuildWorldToClipMatrix() const noexcept
    -> GGEMSVulkanCamera::Matrix4Rows {
  float inverse_aspect_ratio = static_cast<float>(viewport_extent_.height) /
                               static_cast<float>(viewport_extent_.width);

  CameraBasis basis = BuildCameraBasis();

  Vector3 const &right = basis.right;
  Vector3 const &up_reference = basis.up;
  Vector3 const &forward = basis.forward;

  float horizontal_scale = zoom_ * inverse_aspect_ratio;
  float vertical_scale = zoom_;

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
  matrix.row_2[3] = 0.5F - (Dot(forward, target_m_) * depth_scale_);

  matrix.row_3[0] = 0.0F;
  matrix.row_3[1] = 0.0F;
  matrix.row_3[2] = 0.0F;
  matrix.row_3[3] = 1.0F;

  return matrix;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::BuildCameraBasis() const noexcept
    -> GGEMSVulkanCamera::CameraBasis {
  float cos_pitch = std::cos(pitch_radians_);

  Vector3 camera_offset{.x = std::cos(-yaw_radians_) * cos_pitch,
                        .y = std::sin(pitch_radians_),
                        .z = std::sin(-yaw_radians_) * cos_pitch};

  Vector3 forward = Normalize(Vector3{
      .x = -camera_offset.x, .y = -camera_offset.y, .z = -camera_offset.z});

  Vector3 world_up{.x = 0.0F, .y = 1.0F, .z = 0.0F};

  Vector3 right = Normalize(Cross(forward, world_up));

  if (std::abs(right.x) < 1.0e-6F && std::abs(right.y) < 1.0e-6F &&
      std::abs(right.z) < 1.0e-6F) {
    right = Vector3{.x = 1.0F, .y = 0.0F, .z = 0.0F};
  }

  Vector3 up_reference = Cross(right, forward);

  return CameraBasis{.right = right, .up = up_reference, .forward = forward};
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Dot(Vector3 const &first,
                            Vector3 const &second) noexcept -> float {
  return (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Cross(Vector3 const &first,
                              Vector3 const &second) noexcept
    -> GGEMSVulkanCamera::Vector3 {
  return Vector3{.x = (first.y * second.z) - (first.z * second.y),
                 .y = (first.z * second.x) - (first.x * second.z),
                 .z = (first.x * second.y) - (first.y * second.x)};
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanCamera::Normalize(Vector3 const &vec) noexcept
    -> GGEMSVulkanCamera::Vector3 {
  float length = std::sqrt(Dot(vec, vec));

  if (length <= 1.0e-6F) {
    return Vector3{};
  }

  float inverse_length = 1.0F / length;

  return Vector3{.x = vec.x * inverse_length,
                 .y = vec.y * inverse_length,
                 .z = vec.z * inverse_length};
}
} // namespace ggems::ui
