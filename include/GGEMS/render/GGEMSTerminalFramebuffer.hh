#pragma once

/// \cond
#include <string>
#include <string_view>
#include <vector>
/// \endcond

namespace ggems::render {

enum class AsciiColour : std::uint8_t {
  Default = 0,
  Faint,
  Bright,
  Green,
  Blue,
  Cyan,
  Yellow,
  Magenta,
  Red,
  Grey
};

class GGEMSTerminalFramebuffer {
public:
  GGEMSTerminalFramebuffer(std::int16_t width, std::int16_t height);
  ~GGEMSTerminalFramebuffer() = default;

  GGEMSTerminalFramebuffer(GGEMSTerminalFramebuffer const &) = delete;
  GGEMSTerminalFramebuffer(GGEMSTerminalFramebuffer const &&) = delete;
  GGEMSTerminalFramebuffer &
  operator=(GGEMSTerminalFramebuffer const &) = delete;
  GGEMSTerminalFramebuffer &
  operator=(GGEMSTerminalFramebuffer const &&) = delete;

public:
  void Clear(std::string_view s = " ",
             AsciiColour color = AsciiColour::Default) noexcept;
  void Put(std::int16_t x, std::int16_t y, std::string_view s,
           AsciiColour color = AsciiColour::Default) noexcept;

  void DrawStrings(std::int16_t x, std::int16_t y,
                   std::vector<std::string> const &line,
                   AsciiColour color = AsciiColour::Default) noexcept;
  void DrawString(std::int16_t x, std::int16_t y, std::string_view text,
                  AsciiColour color = AsciiColour::Default) noexcept;

  void DrawHLine(std::int16_t x, std::int16_t y, std::int16_t length,
                 std::string_view s,
                 AsciiColour color = AsciiColour::Default) noexcept;
  void DrawVLine(std::int16_t x, std::int16_t y, std::int16_t length,
                 std::string_view s,
                 AsciiColour color = AsciiColour::Default) noexcept;

  void DrawRect(std::int16_t x, std::int16_t y, std::int16_t width,
                std::int16_t height, std::string_view s,
                AsciiColour color = AsciiColour::Default) noexcept;

  [[nodiscard]] std::string Render() const;
  [[nodiscard]] constexpr std::int16_t Width() const noexcept { return width_; }
  [[nodiscard]] constexpr std::int16_t height() const noexcept {
    return height_;
  }

  void SetUseColour(bool use_colour) noexcept;
  [[nodiscard]] bool UseColour() const noexcept;

private:
  [[nodiscard]] static std::string_view
  ColourToAnsi(AsciiColour colour) noexcept;

private:
  std::int16_t width_;
  std::int16_t height_;
  std::vector<std::string> buffer_cells_;
  std::vector<AsciiColour> buffer_colours_;
  bool use_colour_{true};
};
} // namespace ggems::render
