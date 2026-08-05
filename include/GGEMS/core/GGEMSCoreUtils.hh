#pragma once

#include <string>
#include <cctype>

namespace ggems::core {

[[nodiscard]] inline auto Lower(std::string text) -> std::string {
  for (char &letter : text) {
    letter =
        static_cast<char>(std::tolower(static_cast<unsigned char>(letter)));
  }
  return text;
}

} // namespace ggems::core
