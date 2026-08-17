#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

struct GGEMSEnergyDistributionRecord {
  std::uint64_t regular_bin_width_milli_eV{0ULL};
  std::uint64_t table_offset{0ULL};
  std::uint32_t distribution_type{
      ToKernelEnergyDistributionType(GGEMSEnergyDistributionType::Unknown)};
  std::uint32_t table_count{0U};
};

static_assert(std::is_standard_layout_v<GGEMSEnergyDistributionRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSEnergyDistributionRecord>);
static_assert(sizeof(GGEMSEnergyDistributionRecord) == 24U);
static_assert(alignof(GGEMSEnergyDistributionRecord) == 8U);
static_assert(offsetof(GGEMSEnergyDistributionRecord,
                       regular_bin_width_milli_eV) == 0U);
static_assert(offsetof(GGEMSEnergyDistributionRecord, table_offset) == 8U);
static_assert(offsetof(GGEMSEnergyDistributionRecord, distribution_type) ==
              16U);
static_assert(offsetof(GGEMSEnergyDistributionRecord, table_count) == 20U);
} // namespace ggems::core::sources
