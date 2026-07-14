#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRecord, IsKernelFriendly) {
  using SourceRecord = ggems::core::sources::GGEMSSourceRecord;

  EXPECT_TRUE(std::is_standard_layout_v<SourceRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<SourceRecord>);

  EXPECT_EQ(sizeof(SourceRecord), 104U);
  EXPECT_EQ(alignof(SourceRecord), 8U);

  EXPECT_EQ(offsetof(SourceRecord, source_id), 0U);
  EXPECT_EQ(offsetof(SourceRecord, time_start_ps), 8U);
  EXPECT_EQ(offsetof(SourceRecord, time_stop_ps), 16U);
  EXPECT_EQ(offsetof(SourceRecord, energy_milli_eV), 24U);

  EXPECT_EQ(offsetof(SourceRecord, position_x_pm), 32U);
  EXPECT_EQ(offsetof(SourceRecord, position_y_pm), 40U);
  EXPECT_EQ(offsetof(SourceRecord, position_z_pm), 48U);

  EXPECT_EQ(offsetof(SourceRecord, source_type), 56U);
  EXPECT_EQ(offsetof(SourceRecord, emitted_particle_type), 60U);
  EXPECT_EQ(offsetof(SourceRecord, flags), 64U);
  EXPECT_EQ(offsetof(SourceRecord, reserved_0), 68U);

  EXPECT_EQ(offsetof(SourceRecord, direction_x), 72U);
  EXPECT_EQ(offsetof(SourceRecord, direction_y), 76U);
  EXPECT_EQ(offsetof(SourceRecord, direction_z), 80U);
  EXPECT_EQ(offsetof(SourceRecord, direction_w), 84U);

  EXPECT_EQ(offsetof(SourceRecord, weight), 88U);
  EXPECT_EQ(offsetof(SourceRecord, reserved_1), 92U);
  EXPECT_EQ(offsetof(SourceRecord, reserved_2), 96U);
  EXPECT_EQ(offsetof(SourceRecord, reserved_3), 100U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRecord, DefaultSourceRecordIsAnalyticGammaPointSource) {
  ggems::core::sources::GGEMSSourceRecord source{};

  EXPECT_EQ(source.source_type,
            ggems::core::sources::ToKernelSourceType(
                ggems::core::sources::GGEMSSourceType::Analytic));

  EXPECT_EQ(source.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma));

  EXPECT_EQ(source.energy_milli_eV, 511'000'000ULL);

  EXPECT_EQ(source.position_x_pm, 0LL);
  EXPECT_EQ(source.position_y_pm, 0LL);
  EXPECT_EQ(source.position_z_pm, 0LL);

  EXPECT_FLOAT_EQ(source.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(source.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(source.direction_z, 1.0F);

  EXPECT_FLOAT_EQ(source.weight, 1.0F);
}
