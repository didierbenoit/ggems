#pragma once

#include <string>
#include <string_view>

namespace ggems::utf {

auto UTF8ToUTF32(std::string_view str8) -> std::u32string;

auto UTF32ToUTF8(char32_t ch32) -> std::string;

auto UTF32ToUTF8(std::u32string_view str32) -> std::string;
} // namespace ggems::utf
