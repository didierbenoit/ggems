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
                                             core::GGEMSOutputState &state)
    : banner_(banner), state_(state) {}

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

void GGEMSTerminalRenderer::Refresh() {
  std::string out = framebuffer_.Render();
  presenter_.Present(out);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::RenderFinalMessage(std::u32string_view message) {

  std::int16_t h = framebuffer_.GetHeight();
  framebuffer_.DrawString(1, h - 1, message, render::YELLOW_Neon);

  Refresh();

  std::string dummy;
  std::getline(std::cin, dummy);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::RenderOnce() {
  if (!started_)
    Start();

  framebuffer_.UpdateSizeIfNeeded();
  framebuffer_.Clear(U' ', DEFAULT_FG);

  std::int16_t w = framebuffer_.GetWidth();
  std::int16_t h = framebuffer_.GetHeight();

  // Header (banner)
  banner_.Draw(framebuffer_);

  // Log Area
  std::int16_t logs_y = static_cast<std::int16_t>(banner_.GetBottom());
  std::int16_t logs_h =
      std::max<std::int16_t>(0, static_cast<std::int16_t>(h - logs_y));
  Rect logs_rect{1, logs_y, w, logs_h};
  DrawLogs(logs_rect);

  Refresh();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::pair<std::u32string, std::u32string>
GGEMSTerminalRenderer::SplitChunk(std::u32string_view text,
                                  std::int16_t max_width) {
  ;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<GGEMSTerminalRenderer::WrappedLine>
GGEMSTerminalRenderer::WrapLogLine(core::RenderedLogLine const &line,
                                   std::int16_t max_width) const {
  // Convert UTF-8 (std::string) -> UTF-32 and create logic segment
  std::vector<VisualSegment> segments{
      {utf::UTF8ToUTF32(line.prefix), line.color},
      {U" "},
      {utf::UTF8ToUTF32(line.msg)}};

  std::vector<WrappedLine> wrap_lines{};
  WrappedLine wrap_line{};
  std::int16_t remaining_width = max_width;
  for (std::size_t i = 0; i < segments.size(); ++i) {
    if (static_cast<std::int16_t>(segments[i].text.size()) <= remaining_width) {
      wrap_line.segments.push_back(segments[i]);
      remaining_width -= segments[i].text.size();
    } else { // New line
      wrap_lines.push_back(wrap_line);
      wrap_line.segments.clear();
      remaining_width =
          max_width - static_cast<std::int16_t>(segments[i].text.size());
      wrap_line.segments.push_back(segments[i]);
    }
  }
  wrap_lines.push_back(wrap_line);

  return wrap_lines;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalRenderer::DrawLogs(Rect const &rect) {
  if (rect.h <= 0 || rect.w <= 0)
    return;

  std::size_t max_lines = static_cast<std::size_t>(rect.h);
  auto lines = state_.GetLastLogLinesSnapshot(max_lines);

  // Leave 1 column padding; reserve last column to avoid overflow.
  std::int16_t x0 = rect.x;
  std::int16_t max_w =
      std::max<std::int16_t>(0, static_cast<std::int16_t>(rect.w - 2));

  std::int16_t y = static_cast<std::int16_t>(rect.y);

  for (std::size_t i = 0; i < lines.size(); ++i) {
    // std::int16_t y =
    //     static_cast<std::int16_t>(rect.y + static_cast<std::int16_t>(i));
    // if (y < 0)
    //   continue;
    // if (y >= rect.y + rect.h)
    //   break;

    auto wrap_log_lines = WrapLogLine(lines[i], max_w);

    for (std::size_t j = 0; j < wrap_log_lines.size(); ++j, ++y) {
      std::int16_t written_size = 0;
      for (std::size_t k = 0; k < wrap_log_lines[j].segments.size(); ++k) {
        framebuffer_.DrawString(x0 + written_size, y,
                                wrap_log_lines[j].segments[k].text,
                                wrap_log_lines[j].segments[k].colour);
        written_size += wrap_log_lines[j].segments[k].text.size();
      }
    }
  }
  /*  for (std::size_t i = 0; i < lines.size(); ++i) {
      std::int16_t y =
          static_cast<std::int16_t>(rect.y + static_cast<std::int16_t>(i));
      if (y < 0)
        continue;
      if (y >= rect.y + rect.h)
        break;

      // Convert UTF-8 (std::string) -> UTF-32 for framebuffer drawing.
      std::u32string u32_prefix = utf::UTF8ToUTF32(lines[i].prefix);
      std::u32string u32_msg = utf::UTF8ToUTF32(lines[i].msg);

      // Truncate to available width.
      if (static_cast<std::int16_t>(u32_prefix.size()) > max_w) {
        u32_prefix.resize(static_cast<std::size_t>(max_w));
      }

      if (static_cast<std::int16_t>(u32_msg.size()) > max_w) {
        u32_msg.resize(static_cast<std::size_t>(max_w));
      }

      std::int16_t prefix_size = static_cast<std::int16_t>(u32_prefix.size()) +
    1; framebuffer_.DrawString(x0, y, u32_prefix, lines[i].color);
      framebuffer_.DrawString(x0 + prefix_size, y, u32_msg);
    }*/
}
} // namespace ggems::render
