#pragma once

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalRenderer.hh"

namespace ggems::render {
class GGEMSBanner {
public:
  GGEMSBanner() = default;
  ~GGEMSBanner() = default;

  GGEMSBanner(GGEMSBanner const &) = delete;
  GGEMSBanner(GGEMSBanner &&) = delete;
  GGEMSBanner &operator=(GGEMSBanner const &) = delete;
  GGEMSBanner &operator=(GGEMSBanner &&) = delete;

public:
  void Draw(GGEMSTerminalFramebuffer &framebuffer);
  void EmitToLogger(core::GGEMSLogger &) { ; }

  [[nodiscard]] inline constexpr std::int16_t GetWidth() const noexcept {
    return width_;
  };

  [[nodiscard]] inline constexpr std::int16_t GetHeight() const noexcept {
    return height_;
  };

  [[nodiscard]] inline constexpr std::int16_t GetBottom() const noexcept {
    return bottom_;
  }

private:
  std::int16_t width_{54};
  std::int16_t height_{17};
  std::int16_t bottom_{18};
};
} // namespace ggems::render
