#pragma once

#include <cstdint>
#include <string_view>

namespace ggems::ocl::detail {

[[nodiscard]] auto HashFNV1a64(std::string_view bytes) noexcept
    -> std::uint64_t;

} // namespace ggems::ocl::detail
