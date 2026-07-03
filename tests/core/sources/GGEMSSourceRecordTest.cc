#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSSourceRecord, IsKernelFriendly) {
  EXPECT_TRUE(
      std::is_standard_layout_v<ggems::core::sources::GGEMSSourceRecord>);
  EXPECT_TRUE(
      std::is_trivially_copyable_v<ggems::core::sources::GGEMSSourceRecord>);

  EXPECT_EQ(sizeof(ggems::core::sources::GGEMSSourceRecord), 104U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

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
