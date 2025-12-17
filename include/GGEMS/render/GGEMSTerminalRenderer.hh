#pragma once

/// \cond
#include <cstdint>
/// \endcond

#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalPresenter.hh"

namespace ggems::render {
class GGEMSBanner;

class GGEMSTerminalRenderer {
public:
  struct Rect {
    std::int16_t x{};
    std::int16_t y{};
    std::int16_t w{};
    std::int16_t h{};
  };

public:
  explicit GGEMSTerminalRenderer(GGEMSBanner &banner) : banner_(banner) {}

  GGEMSTerminalRenderer(GGEMSTerminalRenderer const &) = delete;
  GGEMSTerminalRenderer(GGEMSTerminalRenderer &&) = delete;
  GGEMSTerminalRenderer &operator=(GGEMSTerminalRenderer const &) = delete;
  GGEMSTerminalRenderer &operator=(GGEMSTerminalRenderer &&) = delete;

  ~GGEMSTerminalRenderer() noexcept;

public:
  void Start() noexcept;
  void Stop() noexcept;

  void RenderOnce();

private:
  GGEMSBanner &banner_;
  GGEMSTerminalFramebuffer framebuffer_{};
  GGEMSTerminalPresenter presenter_;
};
} // namespace ggems::render
