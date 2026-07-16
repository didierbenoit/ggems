#pragma once

#include <cstdint>
#include <type_traits>

namespace ggems::core::sources {

struct GGEMSSourceRunRange {
  std::uint64_t projection_primary_begin{0ULL};
  std::uint64_t primary_count{0ULL};
};

static_assert(std::is_standard_layout_v<GGEMSSourceRunRange>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceRunRange>);
static_assert(sizeof(GGEMSSourceRunRange) == 16U);

} // namespace ggems::core::sources
