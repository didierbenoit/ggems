#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRecord, IsKernelFriendly) {
  using SourceRecord = ggems::core::sources::GGEMSSourceRecord;

  EXPECT_TRUE(std::is_standard_layout_v<SourceRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<SourceRecord>);

  EXPECT_EQ(sizeof(SourceRecord), 160U);
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

  EXPECT_EQ(offsetof(SourceRecord, axis_x_x), 72U);
  EXPECT_EQ(offsetof(SourceRecord, axis_x_y), 76U);
  EXPECT_EQ(offsetof(SourceRecord, axis_x_z), 80U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_x), 84U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_y), 88U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_z), 92U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_x), 96U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_y), 100U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_z), 104U);
  EXPECT_EQ(offsetof(SourceRecord, weight), 108U);
  EXPECT_EQ(offsetof(SourceRecord, emission_geometry_type), 112U);
  EXPECT_EQ(offsetof(SourceRecord, angular_distribution_type), 116U);
  EXPECT_EQ(offsetof(SourceRecord, geometry_size_x_pm), 120U);
  EXPECT_EQ(offsetof(SourceRecord, geometry_size_y_pm), 128U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_x_pm), 136U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_y_pm), 144U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_z_pm), 152U);
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

  EXPECT_FLOAT_EQ(source.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(source.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_y_y, 1.0F);
  EXPECT_FLOAT_EQ(source.axis_y_z, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_z, 1.0F);

  EXPECT_FLOAT_EQ(source.weight, 1.0F);

  EXPECT_EQ(source.emission_geometry_type,
            ggems::core::sources::ToKernelEmissionGeometryType(
                ggems::core::sources::GGEMSEmissionGeometryType::Point));
  EXPECT_EQ(source.angular_distribution_type,
            ggems::core::sources::ToKernelAngularDistributionType(
                ggems::core::sources::GGEMSAngularDistributionType::Fixed));
  EXPECT_EQ(source.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(source.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(source.focus_position_x_pm, 0LL);
  EXPECT_EQ(source.focus_position_y_pm, 0LL);
  EXPECT_EQ(source.focus_position_z_pm, 0LL);
}
