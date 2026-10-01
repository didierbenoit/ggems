#pragma once

#include <cstdint>

#include <imgui.h>

namespace ggems::ui::detail {

class GGEMSImGuiViewportPanel {
public:
  struct Result {
    bool visible{false};
    std::uint32_t width{0U};
    std::uint32_t height{0U};
    float orbit_delta_x_pixels{0.0F};
    float orbit_delta_y_pixels{0.0F};
    float pan_delta_x_pixels{0.0F};
    float pan_delta_y_pixels{0.0F};
    float zoom_delta{0.0F};
  };

  auto Build(bool &show_window, ImTextureID scene_texture_id,
             std::uint32_t texture_width, std::uint32_t texture_height)
    -> Result;
  auto CancelGesture() noexcept -> void;

private:
  enum class Gesture : std::uint8_t { None, Orbit, Pan };

  Gesture gesture_{Gesture::None};
};

} // namespace ggems::ui::detail
