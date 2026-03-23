#pragma once

/// \cond
#include <cstdint>
#include <vector>
/// \endcond

#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/render/GGEMSTerminalPresenter.hh"
#include "GGEMS/render/GGEMSProgressBar.hh"

namespace ggems::render {

struct VisualSegment {
  std::u32string text{};
  render::ColourKey colour{render::DEFAULT_FG};
};

struct WrappedLine {
  std::vector<VisualSegment> segments{};
};

class GGEMSBanner;

class GGEMSTerminalRenderer {
private:
  struct Rect {
    std::int16_t x{};
    std::int16_t y{};
    std::int16_t w{};
    std::int16_t h{};
  };

public:
  explicit GGEMSTerminalRenderer(GGEMSBanner &banner,
                                 GGEMSProgressBar &progress_bar,
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
  void RunFinalScreen(std::u32string_view message);

  void ScrollUp(std::int32_t lines = 1) noexcept;
  void ScrollDown(std::int32_t lines = 1) noexcept;
  void ResetFollowTail() noexcept;

  bool HandleInput(bool final_mode) noexcept;

private:
  void DrawScrollableContent(Rect const &rect);
  void DrawFrame(std::u32string_view final_message = U"");
  void Refresh(bool force = false);

  [[nodiscard]] std::vector<WrappedLine>
  WrapLogLine(core::RenderedLogLine const &line, std::int16_t max_width) const;

  [[nodiscard]] static std::pair<std::u32string, std::u32string>
  SplitChunk(std::u32string_view text, std::int16_t max_width);

private:
  GGEMSBanner &banner_;
  GGEMSProgressBar &progress_bar_;
  core::GGEMSOutputState &state_;
  GGEMSTerminalFramebuffer framebuffer_{};
  GGEMSTerminalPresenter presenter_;

  bool started_{false};
  std::int32_t scroll_offset_{0};
  bool follow_tail_{true};

  std::string last_frame_{};
  std::chrono::steady_clock::time_point last_present_time_{};
  std::chrono::milliseconds min_present_interval_{33};
  bool force_next_refresh_{true};

  std::int16_t last_width_{0};
  std::int16_t last_height_{0};
};
} // namespace ggems::render
