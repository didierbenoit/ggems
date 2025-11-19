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

class GGEMSAsciiFrameBuffer {
public:
  GGEMSAsciiFrameBuffer(std::size_t width, std::size_t height);
  ~GGEMSAsciiFrameBuffer() = default;

  GGEMSAsciiFrameBuffer(GGEMSAsciiFrameBuffer const &) = delete;
  GGEMSAsciiFrameBuffer(GGEMSAsciiFrameBuffer const &&) = delete;
  GGEMSAsciiFrameBuffer &operator=(GGEMSAsciiFrameBuffer const &) = delete;
  GGEMSAsciiFrameBuffer &operator=(GGEMSAsciiFrameBuffer const &&) = delete;

public:
  void Clear(char c = ' ', AsciiColour color = AsciiColour::Default) noexcept;
  void Put(std::size_t x, std::size_t y, char c,
           AsciiColour color = AsciiColour::Default) noexcept;

  void DrawText(std::size_t x, std::size_t y, std::string_view text,
                AsciiColour color = AsciiColour::Default) noexcept;
  void DrawHLine(std::size_t x, std::size_t y, std::size_t length, char c,
                 AsciiColour color = AsciiColour::Default) noexcept;
  void DrawVLine(std::size_t x, std::size_t y, std::size_t length, char c,
                 AsciiColour color = AsciiColour::Default) noexcept;
  void DrawRect(std::size_t x, std::size_t y, std::size_t width,
                std::size_t height, char c,
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
  std::vector<char> buffer_chars_;
  std::vector<AsciiColour> buffer_colours_;
  bool use_colour_{true};
};
} // namespace ggems::render
