#include <algorithm>
#include <cmath>

#include "GGEMSVulkanCamera.hh"

namespace ggems::ui {

namespace {
constexpr float k_pi{3.14159265358979323846f};
constexpr float k_max_pitch_radians{1.4835298641951802f};
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::SetViewportExtent(vk::Extent2D const &extent) noexcept {
  if (extent.width == 0U || extent.height == 0U) {
    return;
  }

  viewport_extent_ = extent;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::SetOrbitAngles(float yaw_degrees,
                                       float pitch_degrees) noexcept {
  yaw_degrees = DegreesToRadians(yaw_degrees);
  pitch_radians_ = std::clamp(DegreesToRadians(pitch_degrees),
                              -k_max_pitch_radians, k_max_pitch_radians);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::SetZoom(float zoom) noexcept {
  zoom_ = std::max(0.05f, zoom);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::Orbit(float delta_yaw_degrees,
                              float delta_pitch_degrees) noexcept {
  yaw_radians_ += DegreesToRadians(delta_yaw_degrees);

  pitch_radians_ =
      std::clamp(pitch_radians_ + DegreesToRadians(delta_pitch_degrees),
                 -k_max_pitch_radians, k_max_pitch_radians);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::ZoomBy(float wheel_delta) noexcept {
  if (std::abs(wheel_delta) <= 1.0e-6f) {
    return;
  }

  float zoom_factor = std::pow(1.12f, wheel_delta);
  SetZoom(zoom_ * zoom_factor);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanCamera::Reset() noexcept {
  SetOrbitAngles(45.0f, 45.0f);
  SetZoom(0.8f);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanCamera::Matrix4Rows
GGEMSVulkanCamera::BuildWorldToClipMatrix() const noexcept {
  float inverse_aspect_ratio = static_cast<float>(viewport_extent_.height) /
                               static_cast<float>(viewport_extent_.width);

  float cos_pitch = std::cos(pitch_radians_);

  Vector3 camera_offset{.x = std::cos(-yaw_radians_) * cos_pitch,
                        .y = std::sin(pitch_radians_),
                        .z = std::sin(-yaw_radians_) * cos_pitch};

  Vector3 forward = Normalise(Vector3{
      .x = -camera_offset.x, .y = -camera_offset.y, .z = -camera_offset.z});

  Vector3 world_up{.x = 0.0f, .y = 1.0f, .z = 0.0f};

  Vector3 right = Normalise(Cross(forward, world_up));

  if (std::abs(right.x) < 1.0e-6f && std::abs(right.y) < 1.0e-6f &&
      std::abs(right.z) < 1.0e-6f) {
    right = Vector3{.x = 1.0f, .y = 0.0f, .z = 0.0f};
  }

  Vector3 up = Cross(right, forward);

  float horizontal_scale = zoom_ * inverse_aspect_ratio;
  float vertical_scale = zoom_;

  Matrix4Rows matrix{};

  matrix.row_0[0] = right.x * horizontal_scale;
  matrix.row_0[1] = right.y * horizontal_scale;
  matrix.row_0[2] = right.z * horizontal_scale;
  matrix.row_0[3] = -Dot(right, target_cm_) * horizontal_scale;

  matrix.row_1[0] = up.x * vertical_scale;
  matrix.row_1[1] = up.y * vertical_scale;
  matrix.row_1[2] = up.z * vertical_scale;
  matrix.row_1[3] = -Dot(up, target_cm_) * vertical_scale;

  matrix.row_2[0] = forward.x * depth_scale_;
  matrix.row_2[1] = forward.y * depth_scale_;
  matrix.row_2[2] = forward.z * depth_scale_;
  matrix.row_2[3] = 0.5f - Dot(forward, target_cm_) * depth_scale_;

  matrix.row_3[0] = 0.0f;
  matrix.row_3[1] = 0.0f;
  matrix.row_3[2] = 0.0f;
  matrix.row_3[3] = 1.0f;

  return matrix;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

float GGEMSVulkanCamera::DegreesToRadians(float degrees) noexcept {
  return degrees * k_pi / 180.0f;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

float GGEMSVulkanCamera::Dot(Vector3 const &a, Vector3 const &b) noexcept {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanCamera::Vector3 GGEMSVulkanCamera::Cross(Vector3 const &a,
                                                    Vector3 const &b) noexcept {
  return Vector3{.x = a.y * b.z - a.z * b.y,
                 .y = a.z * b.x - a.x * b.z,
                 .z = a.x * b.y - a.y * b.x};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanCamera::Vector3
GGEMSVulkanCamera::Normalise(Vector3 const &v) noexcept {
  float length = std::sqrt(Dot(v, v));

  if (length <= 1.0e-6f) {
    return Vector3{};
  }

  float inverse_length = 1.0f / length;

  return Vector3{.x = v.x * inverse_length,
                 .y = v.y * inverse_length,
                 .z = v.z * inverse_length};
}

} // namespace ggems::ui
