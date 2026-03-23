/// \cond
#include <iostream>
/// \endcond

#include "GGEMS/render/GGEMSTerminalRenderer.hh"
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSTerminalRenderer::GGEMSTerminalRenderer(GGEMSBanner &banner,
                                             GGEMSProgressBar &progress_bar,
                                             core::GGEMSOutputState &state)
    : banner_(banner), progress_bar_(progress_bar), state_(state) {}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSTerminalRenderer::~GGEMSTerminalRenderer() noexcept { Stop(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::Start() noexcept {
  if (started_)
    return;

  presenter_.Begin();
  started_ = true;

  force_next_refresh_ = true;
  last_frame_.clear();
  last_present_time_ = std::chrono::steady_clock::time_point();

  last_width_ = 0;
  last_height_ = 0;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::Stop() noexcept {
  if (!started_)
    return;
  presenter_.End();
  started_ = false;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::Refresh(bool force) {
  std::string out = framebuffer_.Render();

  auto now = std::chrono::steady_clock::now();

  bool frame_changed = (out != last_frame_);
  bool enough_time_elapsed =
      (now - last_present_time_) >= min_present_interval_;

  if (!force && !force_next_refresh_) {
    if (!frame_changed) {
      return;
    }

    if (!enough_time_elapsed) {
      return;
    }
  }

  presenter_.Present(out);
  last_frame_ = std::move(out);
  last_present_time_ = now;
  force_next_refresh_ = false;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::DrawFrame(std::u32string_view final_message) {
  framebuffer_.UpdateSizeIfNeeded();

  std::int16_t w = framebuffer_.GetWidth();
  std::int16_t h = framebuffer_.GetHeight();

  if (w != last_width_ || h != last_height_) {
    force_next_refresh_ = true;
    last_width_ = w;
    last_height_ = h;
  }

  framebuffer_.Clear(U' ', DEFAULT_FG);

  std::int16_t final_message_rows = 1;
  std::int16_t progress_rows = progress_bar_.GetHeight();

  std::int16_t content_x = 1;
  std::int16_t content_y = 1;
  std::int16_t content_w = static_cast<std::int16_t>(w - 2);
  std::int16_t content_h =
      static_cast<std::int16_t>(h - 1 - progress_rows - final_message_rows - 2);

  if (content_h < 0) {
    content_h = 0;
  }

  Rect content_rect{content_x, content_y, content_w, content_h};
  DrawScrollableContent(content_rect);

  std::int16_t progress_y =
      static_cast<std::int16_t>(h - final_message_rows - progress_rows);
  if (progress_y >= 0) {
    progress_bar_.Draw(framebuffer_, 1, progress_y,
                       static_cast<std::int16_t>(w - 2));
  }

  if (!final_message.empty()) {
    framebuffer_.DrawString(1, static_cast<std::int16_t>(h - 1), final_message,
                            render::YELLOW_Neon);
  }

  Refresh(false);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::RenderOnce() {
  if (!started_)
    Start();

  HandleInput(false);
  DrawFrame();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::RunFinalScreen(std::u32string_view message) {
  if (!started_) {
    Start();
  }

  force_next_refresh_ = true;

  bool done = false;
  while (!done) {
    DrawFrame(message);
    done = HandleInput(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::ScrollUp(std::int32_t lines) noexcept {
  if (lines <= 0) {
    return;
  }

  follow_tail_ = false;
  scroll_offset_ += lines;
  force_next_refresh_ = true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::ScrollDown(std::int32_t lines) noexcept {
  if (lines <= 0) {
    return;
  }

  scroll_offset_ -= lines;
  if (scroll_offset_ <= 0) {
    scroll_offset_ = 0;
    follow_tail_ = true;
  }

  force_next_refresh_ = true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::ResetFollowTail() noexcept {
  scroll_offset_ = 0;
  follow_tail_ = true;
  force_next_refresh_ = true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::pair<std::u32string, std::u32string>
GGEMSTerminalRenderer::SplitChunk(std::u32string_view text,
                                  std::int16_t max_width) {
  if (max_width <= 0 || text.empty()) {
    return {std::u32string{}, std::u32string{text}};
  }

  if (static_cast<std::int16_t>(text.size()) <= max_width) {
    return {std::u32string{text}, std::u32string{}};
  }

  std::size_t limit = static_cast<std::size_t>(max_width);

  // Search last space within the visible range
  std::size_t split_pos = std::u32string_view::npos;
  for (std::size_t i = 0; i < limit; ++i) {
    if (text[i] == U' ') {
      split_pos = i;
    }
  }

  if (split_pos != std::u32string_view::npos) {
    std::u32string head{text.substr(0, split_pos)};
    std::size_t tail_start = split_pos + 1;

    // Skip additional spaces at the beginning of the tail
    while (tail_start < text.size() && text[tail_start] == U' ') {
      ++tail_start;
    }

    std::u32string tail{text.substr(tail_start)};
    return {std::move(head), std::move(tail)};
  }

  // Hard warp
  std::u32string head{text.substr(0, limit)};
  std::u32string tail{text.substr(limit)};
  return {std::move(head), std::move(tail)};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<WrappedLine>
GGEMSTerminalRenderer::WrapLogLine(core::RenderedLogLine const &line,
                                   std::int16_t max_width) const {
  std::vector<WrappedLine> wrapped_lines{};

  if (max_width <= 0) {
    return wrapped_lines;
  }

  std::vector<VisualSegment> segments{
      {utf::UTF8ToUTF32(line.prefix), line.color},
      {U" ", DEFAULT_FG},
      {utf::UTF8ToUTF32(line.msg), DEFAULT_FG}};

  WrappedLine current_line{};
  std::int16_t current_width = 0;

  auto flush_current_line = [&]() {
    if (!current_line.segments.empty()) {
      wrapped_lines.push_back(std::move(current_line));
      current_line = WrappedLine{};
      current_width = 0;
    }
  };

  for (auto const &segment : segments) {
    std::u32string remaining = segment.text;

    while (!remaining.empty()) {
      std::int16_t available =
          static_cast<std::int16_t>(max_width - current_width);

      if (available <= 0) {
        flush_current_line();
        available = max_width;
      }

      if (static_cast<std::int16_t>(remaining.size()) <= available) {
        current_line.segments.push_back({std::move(remaining), segment.colour});
        current_width +=
            static_cast<std::int16_t>(current_line.segments.back().text.size());
        break;
      }

      auto [head, tail] = SplitChunk(remaining, available);

      if (!head.empty()) {
        current_line.segments.push_back({std::move(head), segment.colour});
        current_width +=
            static_cast<std::int16_t>(current_line.segments.back().text.size());
      }

      flush_current_line();
      remaining = std::move(tail);
    }
  }

  flush_current_line();
  return wrapped_lines;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::DrawScrollableContent(Rect const &rect) {
  if (rect.h <= 0 || rect.w <= 0)
    return;

  std::int16_t max_w = std::max<std::int16_t>(0, rect.w);

  if (max_w <= 0) {
    return;
  }

  std::vector<WrappedLine> visual_lines{};

  // Banner
  auto banner_lines = banner_.BuildLines(max_w);
  for (auto &line : banner_lines) {
    visual_lines.push_back(std::move(line));
  }

  // Wrapped log lines
  auto logical_lines = state_.GetLastLogLinesSnapshot(state_.GetLogCapacity());
  for (auto const &line : logical_lines) {
    auto wrapped = WrapLogLine(line, max_w);
    for (auto &wl : wrapped) {
      visual_lines.push_back(std::move(wl));
    }
  }

  std::size_t max_visible = static_cast<std::size_t>(rect.h);

  std::size_t tail_start = (visual_lines.size() > max_visible)
                               ? (visual_lines.size() - max_visible)
                               : 0;
  std::size_t start = tail_start;

  if (!follow_tail_) {
    std::size_t offset = static_cast<std::size_t>(scroll_offset_);
    start = (tail_start > offset) ? (tail_start - offset) : 0;
  }

  std::int32_t max_scroll = static_cast<std::int32_t>(tail_start);
  if (scroll_offset_ > max_scroll) {
    scroll_offset_ = max_scroll;
  }

  std::int16_t y = rect.y;

  for (std::size_t i = start; i < visual_lines.size(); ++i, ++y) {
    if (y >= rect.y + rect.h) {
      break;
    }

    std::int16_t x = rect.x;

    for (auto const &seg : visual_lines[i].segments) {
      if (!seg.text.empty()) {
        framebuffer_.DrawString(x, y, seg.text, seg.colour);
        x += static_cast<std::int16_t>(seg.text.size());
      }
    }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalRenderer::HandleInput(bool final_mode) noexcept {
  auto key = presenter_.PollKey();

  switch (key) {
  case GGEMSTerminalPresenter::TerminalKey::Up:
    ScrollUp(1);
    return false;

  case GGEMSTerminalPresenter::TerminalKey::Down:
    ScrollDown(1);
    return false;

  case GGEMSTerminalPresenter::TerminalKey::PageUp: {
    std::int16_t h = framebuffer_.GetHeight();
    std::int32_t page_step = std::max<std::int32_t>(1, h - 3);
    ScrollUp(page_step);
    return false;
  }

  case GGEMSTerminalPresenter::TerminalKey::PageDown: {
    std::int16_t h = framebuffer_.GetHeight();
    std::int32_t page_step = std::max<std::int32_t>(1, h - 3);
    ScrollDown(page_step);
    return false;
  }

  case GGEMSTerminalPresenter::TerminalKey::Space:
    ResetFollowTail();
    return false;

  case GGEMSTerminalPresenter::TerminalKey::Enter:
    return final_mode;

  case GGEMSTerminalPresenter::TerminalKey::None:
  default:
    return false;
  }
}
} // namespace ggems::render
