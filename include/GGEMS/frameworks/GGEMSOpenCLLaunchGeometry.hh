#pragma once

#include <cstddef>
#include <limits>
#include <optional>

namespace ggems::ocl::detail {

[[nodiscard]] constexpr auto
TryComputePaddedGlobalWorkSize(std::size_t logical_work_size,
                               std::size_t local_work_size) noexcept
    -> std::optional<std::size_t> {
  if (local_work_size == 0U) {
    return std::nullopt;
  }

  auto const remainder = logical_work_size % local_work_size;
  if (remainder == 0U) {
    return logical_work_size;
  }

  auto const increment = local_work_size - remainder;
  if (logical_work_size > std::numeric_limits<std::size_t>::max() - increment) {
    return std::nullopt;
  }

  return logical_work_size + increment;
}

} // namespace ggems::ocl::detail
