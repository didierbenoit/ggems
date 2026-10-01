#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ggems::ui::detail {

inline constexpr float k_default_user_ui_scale{1.0F};

inline constexpr float k_min_content_scale{1.0F};
inline constexpr float k_max_content_scale{2.5F};

struct GGEMSPixelExtent {
  std::uint32_t width{0U};
  std::uint32_t height{0U};

  [[nodiscard]] auto operator==(GGEMSPixelExtent const &) const noexcept
    -> bool = default;
};

[[nodiscard]] constexpr auto NormalizeContentScale(float content_scale) noexcept
  -> float {
  return std::clamp(content_scale, k_min_content_scale, k_max_content_scale);
}

[[nodiscard]] inline auto
ComputeSceneTargetExtent(GGEMSPixelExtent const &logical_extent,
                         float density_x, float density_y) noexcept
  -> GGEMSPixelExtent {
  auto const ScaleAxis = [](std::uint32_t logical,
                            float density) noexcept -> std::uint32_t {
    long const pixels =
      std::lround(static_cast<double>(logical) * static_cast<double>(density));
    return static_cast<std::uint32_t>(std::max(pixels, 1L));
  };

  return GGEMSPixelExtent{
    .width = ScaleAxis(logical_extent.width, density_x),
    .height = ScaleAxis(logical_extent.height, density_y),
  };
}

} // namespace ggems::ui::detail
