#pragma once
#include <array>

#include <vulkan/vulkan.hpp>

namespace ggems::ui {

class GGEMSVulkanCamera {
public:
  struct Matrix4Rows {
    std::array<float, 4> row_0{1.0F, 0.0F, 0.0F, 0.0F};
    std::array<float, 4> row_1{0.0F, 1.0F, 0.0F, 0.0F};
    std::array<float, 4> row_2{0.0F, 0.0F, 1.0F, 0.0F};
    std::array<float, 4> row_3{0.0F, 0.0F, 0.0F, 1.0F};
  };

  GGEMSVulkanCamera();
  ~GGEMSVulkanCamera() = default;

  GGEMSVulkanCamera(GGEMSVulkanCamera const &) = delete;
  GGEMSVulkanCamera(GGEMSVulkanCamera &&) = delete;
  auto operator=(GGEMSVulkanCamera const &) -> GGEMSVulkanCamera & = delete;
  auto operator=(GGEMSVulkanCamera &&) -> GGEMSVulkanCamera & = delete;

  auto SetViewportExtent(vk::Extent2D const &extent) noexcept -> void;
  auto SetOrbitAngles(float yaw_degrees, float pitch_degrees) noexcept -> void;
  auto SetZoom(float zoom) noexcept -> void;

  auto Orbit(float delta_yaw_degrees, float delta_pitch_degrees) noexcept
      -> void;
  auto Pan(float delta_x_pixels, float delta_y_pixels) noexcept -> void;
  auto ZoomBy(float wheel_delta) noexcept -> void;
  auto Reset() noexcept -> void;

  [[nodiscard]] auto BuildWorldToClipMatrix() const noexcept -> Matrix4Rows;

private:
  struct Vector3 {
    float x{0.0F};
    float y{0.0F};
    float z{0.0F};
  };

  [[nodiscard]] static auto Dot(Vector3 const &first,
                                Vector3 const &second) noexcept -> float;
  [[nodiscard]] static auto Cross(Vector3 const &first,
                                  Vector3 const &second) noexcept -> Vector3;
  [[nodiscard]] static auto Normalise(Vector3 const &vec) noexcept -> Vector3;

  struct CameraBasis {
    Vector3 right{};
    Vector3 up{};
    Vector3 forward{};
  };

  [[nodiscard]] auto BuildCameraBasis() const noexcept -> CameraBasis;

  vk::Extent2D viewport_extent_{.width = 1U, .height = 1U};
  Vector3 target_m_{.x = 0.0F, .y = 0.0F, .z = 0.0F};
  float yaw_radians_{};
  float pitch_radians_{};
  float zoom_{};
  float depth_scale_{0.05F};
};

} // namespace ggems::ui
