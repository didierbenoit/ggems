#include "GGEMS/render/GGEMSTerminalRenderer.hh"
#include "GGEMS/render/GGEMSBanner.hh"
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

void GGEMSTerminalRenderer::RenderOnce() {
  if (!started_)
    Start();

  framebuffer_.UpdateSizeIfNeeded();
  framebuffer_.Clear(U' ', DEFAULT_FG);

  auto w = framebuffer_.Width();
  auto h = framebuffer_.Height();

  // Header (banner)
  std::int16_t header_h = std::min<std::int16_t>(h, 8);
  // Rect header{0, 0, w, header_h};
  banner_.Draw(framebuffer_);

  // Log Area
  //  std::int16_t logs_y = header_h;
  //  std::int16_t logs_h =
  //      std::max<std::int16_t>(0, static_cast<std::int16_t>(h - header_h));
  //  Rect logs_rect{0, logs_y, w, logs_h};
  //  DrawLogs(logs_rect);

  std::string const out = framebuffer_.Render();
  presenter_.Present(out);
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
  std::int16_t x0 = 1;
  std::int16_t max_w =
      std::max<std::int16_t>(0, static_cast<std::int16_t>(rect.w - 2));

  for (std::size_t i = 0; i < lines.size(); ++i) {
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

    std::int16_t prefix_size = static_cast<std::int16_t>(u32_prefix.size()) + 1;
    framebuffer_.DrawString(x0, y, u32_prefix, lines[i].color);
    framebuffer_.DrawString(x0 + prefix_size, y, u32_msg);
  }
}
} // namespace ggems::render
