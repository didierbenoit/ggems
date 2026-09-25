#pragma once

#include <cstdint>

namespace ggems::core::sources {

struct GGEMSSourceRunRange {
  std::uint64_t projection_primary_begin{0ULL};
  std::uint64_t primary_count{0ULL};
};

} // namespace ggems::core::sources
