#pragma once

#include <array>

#include "GGEMS/render/GGEMSColour.hh"

namespace ggems::ui::detail {

inline constexpr float k_inverse_color_channel_maximum{1.0F / 255.0F};

[[nodiscard]] constexpr auto
ToVulkanClearColor(render::ColourKey const &colour) noexcept
    -> std::array<float, 4U> {
  render::RGB const rgb =
      render::GetColourRGB(colour.family, colour.shade, colour.variant);

  return {static_cast<float>(rgb.r) * k_inverse_color_channel_maximum,
          static_cast<float>(rgb.g) * k_inverse_color_channel_maximum,
          static_cast<float>(rgb.b) * k_inverse_color_channel_maximum, 1.0F};
}

} // namespace ggems::ui::detail
