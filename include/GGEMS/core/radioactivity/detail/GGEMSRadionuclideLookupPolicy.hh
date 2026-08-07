#pragma once

#include <string>
#include <string_view>

namespace ggems::core::radioactivity::detail {

[[nodiscard]] inline auto NormalizeRadionuclideLookupName(std::string_view name)
    -> std::string {
  auto const is_ascii_whitespace =
      [](char character) constexpr noexcept -> bool {
    return character == ' ' || character == '\t' || character == '\n' ||
           character == '\r' || character == '\f' || character == '\v';
  };

  while (!name.empty() && is_ascii_whitespace(name.front())) {
    name.remove_prefix(1U);
  }

  while (!name.empty() && is_ascii_whitespace(name.back())) {
    name.remove_suffix(1U);
  }

  std::string normalized;
  normalized.reserve(name.size());

  for (char character : name) {
    if (character >= 'A' && character <= 'Z') {
      normalized.push_back(static_cast<char>(character - 'A' + 'a'));
    } else {
      normalized.push_back(character);
    }
  }

  return normalized;
}

} // namespace ggems::core::radioactivity::detail
