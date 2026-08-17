#pragma once

#include <cstddef>
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
static_assert(alignof(GGEMSSourceRunRange) == 8U);
static_assert(offsetof(GGEMSSourceRunRange, projection_primary_begin) == 0U);
static_assert(offsetof(GGEMSSourceRunRange, primary_count) == 8U);

} // namespace ggems::core::sources
