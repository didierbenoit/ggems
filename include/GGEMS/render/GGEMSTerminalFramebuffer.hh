#pragma once

/// \cond
#include <vector>
/// \endcond

#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::render {

class GGEMSTerminalFramebuffer {
private:
  struct Cell {
    char32_t ch;
    ColourKey fg;
  };

public:
  struct CellView {
    char32_t ch{U' '};
    ColourKey fg{DEFAULT_FG};

    constexpr bool operator==(CellView const &other) const noexcept {
      return ch == other.ch && fg == other.fg;
    }

    constexpr bool operator!=(CellView const &other) const noexcept {
      return !(*this == other);
    }
  };

public:
  GGEMSTerminalFramebuffer();
  ~GGEMSTerminalFramebuffer() = default;

  GGEMSTerminalFramebuffer(GGEMSTerminalFramebuffer const &) = delete;
  GGEMSTerminalFramebuffer(GGEMSTerminalFramebuffer &&) = delete;
  GGEMSTerminalFramebuffer &
  operator=(GGEMSTerminalFramebuffer const &) = delete;
  GGEMSTerminalFramebuffer &operator=(GGEMSTerminalFramebuffer &&) = delete;

public:
  // --- Resize / Access ------------------------------------------------------
  void Resize(std::int16_t width, std::int16_t height);
  [[nodiscard]] inline constexpr std::int16_t GetWidth() const noexcept {
    return width_;
  }
  [[nodiscard]] inline constexpr std::int16_t GetHeight() const noexcept {
    return height_;
  }
  void UpdateSizeIfNeeded();

  [[nodiscard]] CellView GetCell(std::int16_t x, std::int16_t y) const noexcept;

  // --- Clear --------------------------------------------------------------
  void Clear(char32_t ch = U' ', ColourKey fg = DEFAULT_FG);

  // --- Drawing Primitives --------------------------------------------------
  void DrawChar(std::int16_t x, std::int16_t y, char32_t ch,
                ColourKey fg = DEFAULT_FG) noexcept;

  void DrawString(std::int16_t x, std::int16_t y, std::u32string_view text,
                  ColourKey fg = DEFAULT_FG) noexcept;

  void DrawStringsVertical(std::int16_t x, std::int16_t y,
                           std::vector<std::u32string> const &line,
                           ColourKey fg = DEFAULT_FG) noexcept;

  void DrawHLine(std::int16_t x, std::int16_t y, std::int16_t length,
                 char32_t ch, ColourKey fg = DEFAULT_FG) noexcept;

  void DrawVLine(std::int16_t x, std::int16_t y, std::int16_t length,
                 char32_t ch, ColourKey fg = DEFAULT_FG) noexcept;

  void DrawRectBorder(std::int16_t x, std::int16_t y, std::int16_t width,
                      std::int16_t height, ColourKey fg = DEFAULT_FG) noexcept;

  // --- Convert to UTF-8 buffer for printing ---------------------------------
  [[nodiscard]] std::string Render() const;

  void SetUseColour(bool use_colour) noexcept;

private:
  [[nodiscard]] inline std::size_t Index(std::int16_t x,
                                         std::int16_t y) const noexcept {
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
           static_cast<std::size_t>(x);
  }

  [[nodiscard]]
  static std::pair<std::int16_t, std::int16_t> DetectTerminalSize() noexcept;

private:
  std::int16_t width_;
  std::int16_t height_;
  std::vector<Cell> buffer_;
  bool use_colour_{true};
  char32_t default_char_{U' '};
  ColourKey default_fg_{DEFAULT_FG};
};
} // namespace ggems::render
