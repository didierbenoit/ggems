#pragma once

#include <cstdint>
#include <type_traits>

namespace ggems::core {
struct GGEMSTimeWindow {
  std::uint64_t start_ps{0ULL};
  std::uint64_t stop_ps{0ULL};

  auto operator==(GGEMSTimeWindow const &) const -> bool = default;
};

static_assert(std::is_standard_layout_v<GGEMSTimeWindow>);
static_assert(std::is_trivially_copyable_v<GGEMSTimeWindow>);
} // namespace ggems::core
