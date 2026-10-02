#pragma once

#include <array>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::ui::detail {

[[nodiscard]] inline auto
ToVulkanClearColor(render::ColorKey const &color) noexcept
  -> std::array<float, 4U> {
  render::RGB const rgb =
    render::GetColorRGB(color.family, color.shade, color.variant);

  return {
    render::SRGBChannelToLinear(rgb.red),
    render::SRGBChannelToLinear(rgb.green),
    render::SRGBChannelToLinear(rgb.blue),
    1.0F,
  };
}
} // namespace ggems::ui::detail
