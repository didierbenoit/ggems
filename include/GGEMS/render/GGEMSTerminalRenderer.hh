#pragma once

/// \cond
#include <cstdint>
#include <vector>
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

  void ScrollUp(std::int32_t lines = 1) noexcept;
  void ScrollDown(std::int32_t lines = 1) noexcept;
  void ResetFollowTail() noexcept;

private:
  void DrawLogs(Rect const &rect);
  void Refresh();

  [[nodiscard]] std::vector<WrappedLine>
  WrapLogLine(core::RenderedLogLine const &line, std::int16_t max_width) const;

  [[nodiscard]] static std::pair<std::u32string, std::u32string>
  SplitChunk(std::u32string_view text, std::int16_t max_width);

private:
  GGEMSBanner &banner_;
  core::GGEMSOutputState &state_;
  GGEMSTerminalFramebuffer framebuffer_{};
  GGEMSTerminalPresenter presenter_;

  bool started_{false};
  std::int32_t scroll_offset_{0};
  bool follow_tail_{true};
};
} // namespace ggems::render
