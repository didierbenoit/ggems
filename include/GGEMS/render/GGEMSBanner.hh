#pragma once

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalRenderer.hh"

namespace ggems::render {
class GGEMSBanner {
public:
  GGEMSBanner();
  ~GGEMSBanner() = default;

  GGEMSBanner(GGEMSBanner const &) = delete;
  GGEMSBanner(GGEMSBanner &&) = delete;
  GGEMSBanner &operator=(GGEMSBanner const &) = delete;
  GGEMSBanner &operator=(GGEMSBanner &&) = delete;

public:
  void Draw(GGEMSTerminalFramebuffer &framebuffer);
  void EmitToLogger(core::GGEMSLogger &) { ; }

  [[nodiscard]] consteval std::int16_t Width() const noexcept;
  [[nodiscard]] consteval std::int16_t Height() const noexcept;

private:
  std::int16_t width_;
  std::int16_t height_;
};
} // namespace ggems::render
