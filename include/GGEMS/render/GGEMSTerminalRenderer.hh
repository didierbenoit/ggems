#pragma once

/// \cond
#include <cstdint>
/// \endcond

#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalPresenter.hh"

namespace ggems::render {
class GGEMSBanner;

class GGEMSTerminalRenderer {
private:
  struct Rect {
    std::int16_t x{};
    std::int16_t y{};
    std::int16_t w{};
    std::int16_t h{};
  };

  struct VisualSegment {
    std::u32string text{};
    render::ColourKey colour{render::DEFAULT_FG};
  };

  struct WrappedLine {
    std::vector<VisualSegment> segments{};
  };

public:
  explicit GGEMSTerminalRenderer(GGEMSBanner &banner,
                                 core::GGEMSOutputState &state);

  GGEMSTerminalRenderer(GGEMSTerminalRenderer const &) = delete;
  GGEMSTerminalRenderer(GGEMSTerminalRenderer &&) = delete;
  GGEMSTerminalRenderer &operator=(GGEMSTerminalRenderer const &) = delete;
  GGEMSTerminalRenderer &operator=(GGEMSTerminalRenderer &&) = delete;

  ~GGEMSTerminalRenderer() noexcept;

public:
  void Start() noexcept;
  void Stop() noexcept;

  void RenderOnce();
  void RenderFinalMessage(std::u32string_view message);

private:
  void DrawLogs(Rect const &rect);
  void Refresh();

private:
  GGEMSBanner &banner_;
  core::GGEMSOutputState &state_;
  GGEMSTerminalFramebuffer framebuffer_{};
  GGEMSTerminalPresenter presenter_;

  bool started_{false};
};
} // namespace ggems::render
