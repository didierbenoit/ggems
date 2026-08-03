#include <cuchar>
#include <string>
#include <string_view>

#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::utf {

// =============================================================================
// =============================================================================

auto UTF8ToUTF32(std::string_view str8) -> std::u32string {
  std::u32string out;
  std::mbstate_t st{};
  char const *src = str8.data();
  char const *end = src + str8.size();
  char32_t cp = 0;

  while (src < end) {
    std::size_t r =
        mbrtoc32(&cp, src, static_cast<std::size_t>(end - src), &st);
    if (r == static_cast<std::size_t>(-1) ||
        r == static_cast<std::size_t>(-2)) {
      cp = U'?';
      ++src;
    } else if (r == 0) {
      ++src;
    } else {
      src += r;
    }
    out.push_back(cp);
  }
  return out;
}

// =============================================================================
// =============================================================================

auto UTF32ToUTF8(char32_t ch32) -> std::string {
  std::string out;
  if (ch32 <= 0x7F) {
    out.push_back(static_cast<char>(ch32));
  } else if (ch32 <= 0x7FF) {
    out.push_back(static_cast<char>(0xC0 | ((ch32 >> 6) & 0x1F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else if (ch32 <= 0xFFFF) {
    out.push_back(static_cast<char>(0xE0 | ((ch32 >> 12) & 0x0F)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else if (ch32 <= 0x10FFFF) {
    out.push_back(static_cast<char>(0xF0 | ((ch32 >> 18) & 0x07)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 12) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | ((ch32 >> 6) & 0x3F)));
    out.push_back(static_cast<char>(0x80 | (ch32 & 0x3F)));
  } else {
    out = "?";
  }
  return out;
}

// =============================================================================
// =============================================================================

auto UTF32ToUTF8(std::u32string_view str32) -> std::string {
  std::string out;
  out.reserve(str32.size() * 4);
  for (char32_t cp : str32) {
    out += UTF32ToUTF8(cp);
  }
  return out;
}
} // namespace ggems::utf
