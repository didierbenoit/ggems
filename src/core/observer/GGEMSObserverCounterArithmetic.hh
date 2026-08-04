#pragma once

#include <cstdint>
#include <limits>

namespace ggems::core::observer::detail {

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
SaturateObserverCounter(std::uint64_t value) noexcept -> std::uint32_t {
  constexpr auto maximum_counter = std::numeric_limits<std::uint32_t>::max();
  constexpr auto maximum_counter_wide =
      static_cast<std::uint64_t>(maximum_counter);

  if (value > maximum_counter_wide) {
    return maximum_counter;
  }

  return static_cast<std::uint32_t>(value);
}

// =============================================================================
// =============================================================================

constexpr auto AddSaturatedObserverCounter(std::uint32_t &destination,
                                           std::uint64_t value) noexcept
    -> void {
  constexpr auto maximum_counter = std::numeric_limits<std::uint32_t>::max();
  constexpr auto maximum_counter_wide =
      static_cast<std::uint64_t>(maximum_counter);
  auto const destination_wide = static_cast<std::uint64_t>(destination);
  auto const remaining_capacity = maximum_counter_wide - destination_wide;

  if (value >= remaining_capacity) {
    destination = maximum_counter;
    return;
  }

  destination += static_cast<std::uint32_t>(value);
}

} // namespace ggems::core::observer::detail
