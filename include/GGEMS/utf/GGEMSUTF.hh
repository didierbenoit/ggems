#pragma once

/// \cond
#include <format>
#include <string>
#include <string_view>
/// \endcond

namespace ggems::utf {

std::u32string UTF8ToUTF32(std::string_view str8);

std::string UTF32ToUTF8(char32_t ch32);

std::string UTF32ToUTF8(std::u32string_view str32);

} // namespace ggems::utf

// formatter for char32_t
template <> struct std::formatter<char32_t, char> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(char32_t cp, FormatContext &ctx) const {
    return std::format_to(ctx.out(), "{}", ggems::utf::UTF32ToUTF8(cp));
  }
};
