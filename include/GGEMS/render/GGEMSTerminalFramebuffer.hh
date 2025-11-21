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

enum class Encoding { ASCII, UTF8 };

class GGEMSTerminalFramebuffer {
public:
  GGEMSTerminalFramebuffer(std::size_t width, std::size_t height);
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
  void Put(std::size_t x, std::size_t y, std::string_view s,
           AsciiColour color = AsciiColour::Default) noexcept;

  void DrawStrings(std::size_t x, std::size_t y,
                   std::vector<std::string> const &line,
                   AsciiColour color = AsciiColour::Default) noexcept;
  void DrawString(std::size_t x, std::size_t y, std::string_view text,
                  AsciiColour color = AsciiColour::Default) noexcept;
  void DrawHLine(std::size_t x, std::size_t y, std::size_t length,
                 std::string_view s,
                 AsciiColour color = AsciiColour::Default) noexcept;
  void DrawVLine(std::size_t x, std::size_t y, std::size_t length,
                 std::string_view s,
                 AsciiColour color = AsciiColour::Default) noexcept;
  void DrawRect(std::size_t x, std::size_t y, std::size_t width,
                std::size_t height, std::string_view s,
                AsciiColour color = AsciiColour::Default) noexcept;

  [[nodiscard]] std::string Render() const;
  [[nodiscard]] constexpr std::size_t Width() const noexcept { return width_; }
  [[nodiscard]] constexpr std::size_t height() const noexcept {
    return height_;
  }

  void SetUseColour(bool use_colour) noexcept;
  [[nodiscard]] bool UseColour() const noexcept;

private:
  [[nodiscard]] static std::string_view
  ColourToAnsi(AsciiColour colour) noexcept;

private:
  std::size_t width_;
  std::size_t height_;
  std::vector<std::string> buffer_cells_;
  std::vector<AsciiColour> buffer_colours_;
  bool use_colour_{true};
};
} // namespace ggems::render
