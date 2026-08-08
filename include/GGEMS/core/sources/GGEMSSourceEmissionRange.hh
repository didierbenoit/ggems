#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ggems::core::sources {

struct GGEMSSourceEmissionRange {
  std::uint64_t source_local_primary_begin{0ULL};
  std::uint64_t primary_count{0ULL};
};

static_assert(std::is_standard_layout_v<GGEMSSourceEmissionRange>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceEmissionRange>);
static_assert(sizeof(GGEMSSourceEmissionRange) == 16U);
static_assert(alignof(GGEMSSourceEmissionRange) == 8U);
static_assert(offsetof(GGEMSSourceEmissionRange, source_local_primary_begin) ==
              0U);
static_assert(offsetof(GGEMSSourceEmissionRange, primary_count) == 8U);
} // namespace ggems::core::sources
