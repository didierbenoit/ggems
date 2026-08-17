#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {
using EnergyRecord = ggems::core::sources::GGEMSEnergyDistributionRecord;
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, DefaultsToUnknownAndEmptyTable) {
  EnergyRecord const record{};
  EXPECT_EQ(record.regular_bin_width_milli_eV, 0ULL);
  EXPECT_EQ(record.table_offset, 0ULL);
  EXPECT_EQ(record.distribution_type,
            ggems::core::sources::ToKernelEnergyDistributionType(
                ggems::core::sources::GGEMSEnergyDistributionType::Unknown));
  EXPECT_EQ(record.table_count, 0U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, HasApprovedHostLayout) {
  EXPECT_TRUE(std::is_standard_layout_v<EnergyRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<EnergyRecord>);
  EXPECT_EQ(sizeof(EnergyRecord), 24U);
  EXPECT_EQ(alignof(EnergyRecord), 8U);
  EXPECT_EQ(offsetof(EnergyRecord, regular_bin_width_milli_eV), 0U);
  EXPECT_EQ(offsetof(EnergyRecord, table_offset), 8U);
  EXPECT_EQ(offsetof(EnergyRecord, distribution_type), 16U);
  EXPECT_EQ(offsetof(EnergyRecord, table_count), 20U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, PreservesEveryField) {
  EnergyRecord const record{
      .regular_bin_width_milli_eV = 2'000'000ULL,
      .table_offset = 17ULL,
      .distribution_type = ggems::core::sources::ToKernelEnergyDistributionType(
          ggems::core::sources::GGEMSEnergyDistributionType::RegularSpectrum),
      .table_count = 111U,
  };

  EXPECT_EQ(record.regular_bin_width_milli_eV, 2'000'000ULL);
  EXPECT_EQ(record.table_offset, 17ULL);
  EXPECT_EQ(record.distribution_type, 3U);
  EXPECT_EQ(record.table_count, 111U);
}
