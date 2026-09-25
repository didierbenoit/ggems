#pragma once

#include <cstdint>

namespace ggems::core::sources {

struct GGEMSSourceEmissionRange {
  std::uint64_t source_local_primary_begin{0ULL};
  std::uint64_t primary_count{0ULL};
};

} // namespace ggems::core::sources
