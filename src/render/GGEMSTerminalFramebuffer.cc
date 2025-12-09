#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

#if defined(_WIN32)
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
#endif

namespace ggems::render {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::pair<std::int16_t, std::int16_t>
GGEMSTerminalFramebuffer::DetectTerminalSize() noexcept {
  std::int16_t width = 120;
  std::int16_t height = 40;

#if defined(_WIN32)
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

  if (hOut != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hOut, &csbi)) {

    width =
        static_cast<std::int16_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);

    height =
        static_cast<std::int16_t>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
  }
#else
  struct winsize ws{};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
    width = static_cast<std::int16_t>(ws.ws_col);
    height = static_cast<std::int16_t>(ws.ws_row);
  }
#endif

  // Sécurité
  if (width < 20)
    width = 20;
  if (height < 5)
    height = 5;

  return {width, height};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSTerminalFramebuffer::GGEMSTerminalFramebuffer() {
  auto [w, h] = DetectTerminalSize();
  width_ = w;
  height_ = h;

  Resize(w, h);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::SetUseColour(bool use_colour) noexcept {
  use_colour_ = use_colour;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::Resize(std::int16_t width, std::int16_t height) {
  width_ = width;
  height_ = height;

  std::size_t count =
      static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_);

  Cell c{default_char_, default_fg_};
  buffer_.assign(count, c);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::UpdateSizeIfNeeded() {
  auto [w, h] = DetectTerminalSize();

  if (w != width_ || h != height_) {
    width_ = w;
    height_ = h;
    Resize(width_, height_);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::Clear(char32_t ch, ColourKey fg) {
  default_char_ = ch;
  default_fg_ = fg;

  Cell cell{ch, fg};
  std::fill(buffer_.begin(), buffer_.end(), cell);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawChar(std::int16_t x, std::int16_t y,
                                        char32_t ch, ColourKey fg) noexcept {
  if (x < 0 || y < 0)
    return;
  if (x >= width_ || y >= height_)
    return;

  std::size_t idx = Index(x, y);

  buffer_[idx] = {ch, fg};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawStringsVertical(
    std::int16_t x, std::int16_t y, std::vector<std::u32string> const &line,
    ColourKey fg) noexcept {
  std::int16_t cy = y;
  for (auto const &l : line) {
    DrawString(x, cy, l, fg);
    ++cy;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawString(std::int16_t x, std::int16_t y,
                                          std::u32string_view text,
                                          ColourKey fg) noexcept {
  std::int16_t cx = x;
  for (char32_t ch : text) {
    DrawChar(cx, y, ch, fg);
    ++cx;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawHLine(std::int16_t x, std::int16_t y,
                                         std::int16_t length, char32_t ch,
                                         ColourKey fg) noexcept {
  for (std::int16_t cx = x; cx < (x + length); ++cx) {
    DrawChar(cx, y, ch, fg);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawVLine(std::int16_t x, std::int16_t y,
                                         std::int16_t length, char32_t ch,
                                         ColourKey fg) noexcept {
  for (std::int16_t cy = y; cy < (y + length); ++cy) {
    DrawChar(x, cy, ch, fg);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawRectBorder(std::int16_t x, std::int16_t y,
                                              std::int16_t width,
                                              std::int16_t height,
                                              ColourKey fg) noexcept {
  auto const &g = utf::Glyphs();

  // Horizontal top/bottom
  DrawHLine(x + 1, y, width - 2, g.horizontal_line, fg);
  DrawHLine(x + 1, y + height - 1, width - 2, g.horizontal_line, fg);

  // Vertical left/right
  DrawVLine(x, y + 1, height - 2, g.vertical_line, fg);
  DrawVLine(x + width - 1, y + 1, height - 2, g.vertical_line, fg);

  // Corners
  DrawChar(x, y, g.border_top_left, fg);
  DrawChar(x + width - 1, y, g.border_top_right, fg);
  DrawChar(x, y + height - 1, g.border_bottom_left, fg);
  DrawChar(x + width - 1, y + height - 1, g.border_bottom_right, fg);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSTerminalFramebuffer::Render() const {
  std::string out;
  out.reserve(static_cast<std::size_t>(width_) *
                  static_cast<std::size_t>(height_) * 20U +
              128U);

  if (use_colour_) {
    out.append(AnsiColour(DEFAULT_BG));
  }

  for (int16_t y = 0; y < height_; y++) {
    for (int16_t x = 0; x < width_; x++) {

      auto const &cell = buffer_[Index(x, y)];

      if (use_colour_) {
        out.append(AnsiColour(cell.fg_));
      }

      out.append(utf::UTF32ToUTF8(cell.ch_));
    }

    out.push_back('\n');
  }

  if (use_colour_) {
    out.append("\033[0m");
  }

  return out;
}
} // namespace ggems::render
