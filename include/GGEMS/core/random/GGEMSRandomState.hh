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

struct GGEMSPCG32State {
  std::uint64_t state{0U};
  std::uint64_t increment{0U};
};

static_assert(std::is_standard_layout_v<GGEMSPCG32State>);
static_assert(std::is_trivially_copyable_v<GGEMSPCG32State>);
static_assert(sizeof(GGEMSPCG32State) == 2U * sizeof(std::uint64_t));

} // namespace ggems::core::random
