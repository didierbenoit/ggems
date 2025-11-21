#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"

namespace ggems::render {

GGEMSTerminalFramebuffer::GGEMSTerminalFramebuffer(std::int16_t width,
                                                   std::int16_t height)
    : width_{width}, height_{height},
      buffer_cells_(static_cast<std::size_t>(width) *
                        static_cast<std::size_t>(height),
                    " "),
      buffer_colours_(static_cast<std::size_t>(width) *
                          static_cast<std::size_t>(height),
                      AsciiColour::Default),
      use_colour_{true} {}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::Clear(std::string_view s,
                                     AsciiColour colour) noexcept {
  std::fill(buffer_cells_.begin(), buffer_cells_.end(), s);
  std::fill(buffer_colours_.begin(), buffer_colours_.end(), colour);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::Put(std::int16_t x, std::int16_t y,
                                   std::string_view s,
                                   AsciiColour colour) noexcept {
  if (x >= width_ || y >= height_) {
    return;
  }

  std::size_t idx =
      static_cast<std::size_t>(x) +
      static_cast<std::size_t>(y) * static_cast<std::size_t>(width_);
  buffer_cells_[idx] = s;
  buffer_colours_[idx] = colour;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawStrings(std::int16_t x, std::int16_t y,
                                           std::vector<std::string> const &line,
                                           AsciiColour colour) noexcept {
  for (std::size_t i = 0; i < line.size(); ++i) {
    DrawString(x + static_cast<std::int16_t>(i), y, line[i], colour);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawString(std::int16_t x, std::int16_t y,
                                          std::string_view text,
                                          AsciiColour colour) noexcept {
  if (x >= width_ || y >= height_)
    return;

  std::size_t idx =
      static_cast<std::size_t>(x) +
      static_cast<std::size_t>(y) * static_cast<std::size_t>(width_);
  buffer_cells_[idx] = text;
  buffer_colours_[idx] = colour;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawHLine(std::int16_t x, std::int16_t y,
                                         std::int16_t length,
                                         std::string_view s,
                                         AsciiColour colour) noexcept {
  if (y >= height_) {
    return;
  }

  std::int16_t max_len =
      (x < width_) ? std::min(length, static_cast<std::int16_t>(width_ - x))
                   : 0;
  std::size_t row_offset =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(width_);

  for (std::int16_t i = 0; i < max_len; ++i) {
    std::size_t idx =
        row_offset + static_cast<std::size_t>(x) + static_cast<std::size_t>(i);
    buffer_cells_[idx] = s;
    buffer_colours_[idx] = colour;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawVLine(std::int16_t x, std::int16_t y,
                                         std::int16_t length,
                                         std::string_view s,
                                         AsciiColour colour) noexcept {
  if (x >= width_) {
    return;
  }

  std::int16_t max_len =
      (y < height_) ? std::min(length, static_cast<std::int16_t>(height_ - y))
                    : 0;

  for (std::int16_t i = 0; i < max_len; ++i) {
    std::size_t idx =
        static_cast<std::size_t>(y) +
        static_cast<std::size_t>(i) * static_cast<std::size_t>(width_) +
        static_cast<std::size_t>(x);
    buffer_cells_[idx] = s;
    buffer_colours_[idx] = colour;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::DrawRect(std::int16_t x, std::int16_t y,
                                        std::int16_t width, std::int16_t height,
                                        std::string_view s,
                                        AsciiColour colour) noexcept {
  if (width == 0U || height == 0U) {
    return;
  }

  DrawHLine(x, y, width, s, colour);
  if (height > 1) {
    DrawHLine(x, y + height - 1, width, s, colour);
  }

  if (height > 2) {
    DrawVLine(x, y + 1, height - 2, s, colour);
    if (width > 1) {
      DrawVLine(x + width - 1, y + 1, height - 2, s, colour);
    }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalFramebuffer::SetUseColour(bool const use_colour) noexcept {
  use_colour_ = use_colour;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalFramebuffer::UseColour() const noexcept {
  return use_colour_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSTerminalFramebuffer::Render() const {
  std::string out;
  out.reserve((static_cast<std::size_t>(width_) + 1) *
                  static_cast<std::size_t>(height_) +
              16);

  if (use_colour_) {
    out.append("\033[0m");
  }

  AsciiColour current_colour = AsciiColour::Default;

  for (std::int16_t y = 0; y < height_; ++y) {
    std::size_t row_offset =
        static_cast<std::size_t>(y) * static_cast<std::size_t>(width_);

    for (std::int16_t x = 0; x < width_; ++x) {
      std::size_t idx = row_offset + static_cast<std::size_t>(x);
      AsciiColour cell_colour = buffer_colours_[idx];

      if (use_colour_ && cell_colour != current_colour) {
        current_colour = cell_colour;
        std::string_view code = ColourToAnsi(current_colour);
        if (!code.empty()) {
          out.append(code);
        }
      }

      out.append(buffer_cells_[idx]);
    }

    if (use_colour_) {
      out.append("\033[0m");
      current_colour = AsciiColour::Default;
    }

    out.append("\n");
  }

  if (use_colour_) {
    out.append("\033[0m");
  }

  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string_view
GGEMSTerminalFramebuffer::ColourToAnsi(AsciiColour colour) noexcept {
  switch (colour) {
  case AsciiColour::Default:
    return "\033[0m";
  case AsciiColour::Faint:
    return "\033[2m";
  case AsciiColour::Bright:
    return "\033[1m";
  case AsciiColour::Green:
    return "\033[1;32m";
  case AsciiColour::Blue:
    return "\033[1;34m";
  case AsciiColour::Cyan:
    return "\033[1;36m";
  case AsciiColour::Yellow:
    return "\033[1;33m";
  case AsciiColour::Magenta:
    return "\033[1;35m";
  case AsciiColour::Red:
    return "\033[1;31m";
  case AsciiColour::Grey:
    return "\033[90m";
  default:
    return "\033[0m";
  }
}
} // namespace ggems::render
