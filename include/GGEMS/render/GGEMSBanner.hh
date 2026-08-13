#pragma once

#include <cstdint>
#include <vector>

#include "GGEMS/render/GGEMSVisualLine.hh"

namespace ggems::render {
class GGEMSBanner {
public:
  GGEMSBanner() = default;
  ~GGEMSBanner() = default;

  GGEMSBanner(GGEMSBanner const &) = delete;
  GGEMSBanner(GGEMSBanner &&) = delete;
  auto operator=(GGEMSBanner const &) -> GGEMSBanner & = delete;
  auto operator=(GGEMSBanner &&) -> GGEMSBanner & = delete;

  [[nodiscard]] auto BuildLines(std::int16_t max_width) const
      -> std::vector<WrappedLine>;

  [[nodiscard]] constexpr auto GetWidth() const noexcept -> std::int16_t {
    return width_;
  };

  [[nodiscard]] constexpr auto GetHeight() const noexcept -> std::int16_t {
    return height_;
  };

  [[nodiscard]] constexpr auto GetBottom() const noexcept -> std::int16_t {
    return bottom_;
  }

private:
  std::int16_t width_{54};
  std::int16_t height_{17};
  std::int16_t bottom_{18};
};
} // namespace ggems::render
