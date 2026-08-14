#pragma once

#include <array>

#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::ui::detail {

inline constexpr float k_inverse_color_channel_maximum{1.0F / 255.0F};

[[nodiscard]] constexpr auto
ToVulkanClearColor(render::ColorKey const &color) noexcept
    -> std::array<float, 4U> {
  render::RGB const rgb =
      render::GetColorRGB(color.family, color.shade, color.variant);

  return {static_cast<float>(rgb.red) * k_inverse_color_channel_maximum,
          static_cast<float>(rgb.green) * k_inverse_color_channel_maximum,
          static_cast<float>(rgb.blue) * k_inverse_color_channel_maximum, 1.0F};
}

} // namespace ggems::ui::detail
