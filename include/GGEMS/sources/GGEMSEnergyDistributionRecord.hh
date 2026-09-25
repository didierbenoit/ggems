#pragma once

#include <cstdint>

#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

struct GGEMSEnergyDistributionRecord {
  std::uint64_t regular_bin_width_micro_eV{0ULL};
  std::uint64_t table_offset{0ULL};
  std::uint32_t distribution_type{
    ToKernelEnergyDistributionType(GGEMSEnergyDistributionType::Unknown)};
  std::uint32_t table_count{0U};
};

} // namespace ggems::core::sources
