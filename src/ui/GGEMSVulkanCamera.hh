#pragma once

#include <vulkan/vulkan.hpp>

namespace ggems::ui {

class GGEMSVulkanCamera {
public:
  struct Matrix4Rows {
    float row_0[4]{1.0f, 0.0f, 0.0f, 0.0f};
    float row_1[4]{0.0f, 1.0f, 0.0f, 0.0f};
    float row_2[4]{0.0f, 0.0f, 1.0f, 0.0f};
    float row_3[4]{0.0f, 0.0f, 0.0f, 1.0f};
  };

public:
  GGEMSVulkanCamera() = default;
  ~GGEMSVulkanCamera() = default;

  GGEMSVulkanCamera(GGEMSVulkanCamera const &) = delete;
  GGEMSVulkanCamera(GGEMSVulkanCamera &&) = delete;
  GGEMSVulkanCamera &operator=(GGEMSVulkanCamera const &) = delete;
  GGEMSVulkanCamera &operator=(GGEMSVulkanCamera &&) = delete;

public:
  void SetViewportExtent(vk::Extent2D const &extent) noexcept;
  void SetOrbitAngles(float yaw_degrees, float pitch_degrees) noexcept;
  void SetZoom(float zoom) noexcept;

  void Orbit(float delta_yaw_degrees, float delta_pitch_degrees) noexcept;
  void ZoomBy(float wheel_delta) noexcept;
  void Reset() noexcept;

  [[nodiscard]] Matrix4Rows BuildWorldToClipMatrix() const noexcept;

private:
  struct Vector3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
  };

private:
  [[nodiscard]] static float DegreesToRadians(float degrees) noexcept;
  [[nodiscard]] static float Dot(Vector3 const &a, Vector3 const &b) noexcept;
  [[nodiscard]] static Vector3 Cross(Vector3 const &a,
                                     Vector3 const &b) noexcept;
  [[nodiscard]] static Vector3 Normalise(Vector3 const &v) noexcept;

private:
  vk::Extent2D viewport_extent_{1U, 1U};
  Vector3 target_cm_{0.0f, 0.0f, 0.0f};
  float yaw_radians_{0.7853981633974483f};
  float pitch_radians_{0.7853981633974483f};
  float zoom_{0.8f};
  float depth_scale_{0.05f};
};

} // namespace ggems::ui
