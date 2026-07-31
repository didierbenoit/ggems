#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ggems::core::radioactivity {

struct GGEMSRadionuclideGroupRange {
  std::uint64_t source_local_primary_begin{0ULL};
  std::uint64_t primary_count{0ULL};
};

static_assert(std::is_standard_layout_v<GGEMSRadionuclideGroupRange>);
static_assert(std::is_trivially_copyable_v<GGEMSRadionuclideGroupRange>);
static_assert(sizeof(GGEMSRadionuclideGroupRange) == 16U);
static_assert(alignof(GGEMSRadionuclideGroupRange) == 8U);
static_assert(offsetof(GGEMSRadionuclideGroupRange,
                       source_local_primary_begin) == 0U);
static_assert(offsetof(GGEMSRadionuclideGroupRange, primary_count) == 8U);

} // namespace ggems::core::radioactivity
