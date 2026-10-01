#pragma once

#include <array>
#include <cstdint>

namespace ggems::ui::detail {

class GGEMSSceneCamera {
public:
  struct Matrix4Rows {
    std::array<float, 4> row_0{1.0F, 0.0F, 0.0F, 0.0F};
    std::array<float, 4> row_1{0.0F, 1.0F, 0.0F, 0.0F};
    std::array<float, 4> row_2{0.0F, 0.0F, 1.0F, 0.0F};
    std::array<float, 4> row_3{0.0F, 0.0F, 0.0F, 1.0F};
  };

  GGEMSSceneCamera() noexcept;

  auto SetViewportSize(std::uint32_t width, std::uint32_t height) noexcept
    -> void;
  auto SetOrbitAngles(float yaw_degrees, float pitch_degrees) noexcept -> void;
  auto SetZoom(float zoom) noexcept -> void;

  auto Orbit(float delta_yaw_degrees, float delta_pitch_degrees) noexcept
    -> void;
  auto OrbitByPixels(float delta_x_pixels, float delta_y_pixels) noexcept
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
  [[nodiscard]] static auto Normalize(Vector3 const &vec) noexcept -> Vector3;

  struct CameraBasis {
    Vector3 right{};
    Vector3 up{};
    Vector3 forward{};
  };

  [[nodiscard]] auto BuildCameraBasis() const noexcept -> CameraBasis;

  std::uint32_t viewport_width_{1U};
  std::uint32_t viewport_height_{1U};
  Vector3 target_m_{.x = 0.0F, .y = 0.0F, .z = 0.0F};
  float yaw_radians_{};
  float pitch_radians_{};
  float zoom_{};
  float depth_scale_{0.05F};
};

} // namespace ggems::ui::detail
