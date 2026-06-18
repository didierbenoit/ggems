#pragma once

#include <cstdint>
#include <type_traits>

namespace ggems::core::random {

struct GGEMSJKissState {
  std::uint32_t x{0U};
  std::uint32_t y{0U};
  std::uint32_t z{0U};
  std::uint32_t w{0U};
  std::uint32_t c{0U};
};

static_assert(std::is_standard_layout_v<GGEMSJKissState>);
static_assert(std::is_trivially_copyable_v<GGEMSJKissState>);
static_assert(sizeof(GGEMSJKissState) == 5U * sizeof(std::uint32_t));

} // namespace ggems::core::random
