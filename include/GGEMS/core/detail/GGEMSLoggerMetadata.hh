#pragma once

#include <string>
#include <string_view>

namespace ggems::core::logging::detail {

[[nodiscard]] auto ThreadTag() -> std::string;

[[nodiscard]] auto SimplifyFunctionName(std::string_view function_name) noexcept
    -> std::string_view;

} // namespace ggems::core::logging::detail
