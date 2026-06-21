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

struct GGEMSPhiloxState {
  std::uint32_t counter_0{0U};
  std::uint32_t counter_1{0U};
  std::uint32_t counter_2{0U};
  std::uint32_t counter_3{0U};
  std::uint32_t key_0{0};
  std::uint32_t key_1{1};
};

static_assert(std::is_standard_layout_v<GGEMSPhiloxState>);
static_assert(std::is_trivially_copyable_v<GGEMSPhiloxState>);
static_assert(sizeof(GGEMSPhiloxState) == 6U * sizeof(std::uint32_t));

} // namespace ggems::core::random
