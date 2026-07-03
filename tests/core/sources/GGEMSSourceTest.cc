#include <cmath>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSSource, DefaultSourceIsAnalyticGammaPointSource) {
  ggems::core::sources::GGEMSSource source{};

  auto const &record = source.GetRecord();

  EXPECT_EQ(record.source_type,
            ggems::core::sources::ToKernelSourceType(
                ggems::core::sources::GGEMSSourceType::Analytic));

  EXPECT_EQ(record.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma));

  EXPECT_EQ(record.energy_milli_eV, 511'000'000ULL);

  EXPECT_EQ(record.time_start_ps, 0ULL);
  EXPECT_EQ(record.time_stop_ps, 1'000'000ULL);

  EXPECT_EQ(record.position_x_pm, 0LL);
  EXPECT_EQ(record.position_y_pm, 0LL);
  EXPECT_EQ(record.position_z_pm, 0LL);

  EXPECT_FLOAT_EQ(record.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_z, 1.0F);
  EXPECT_FLOAT_EQ(record.weight, 1.0F);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSSource, NormalisesDirection) {
  ggems::core::sources::GGEMSSource source{};

  source.SetDirection(0.0F, 0.0F, 2.0F);

  auto const &record = source.GetRecord();

  EXPECT_FLOAT_EQ(record.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_z, 1.0F);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSSource, CanBuildElectronSourceRecord) {
  ggems::core::sources::GGEMSSource source{};

  source.SetEmittedParticleType(
      ggems::core::particles::GGEMSParticleType::Electron);
  source.SetEnergyMilliElectronVolt(1'000'000ULL);
  source.SetPositionPM(1LL, 2LL, 3LL);
  source.SetTimeWindow(10ULL, 20ULL);

  auto const record = source.BuildRecord();

  EXPECT_EQ(record.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Electron));

  EXPECT_EQ(record.energy_milli_eV, 1'000'000ULL);
  EXPECT_EQ(record.position_x_pm, 1LL);
  EXPECT_EQ(record.position_y_pm, 2LL);
  EXPECT_EQ(record.position_z_pm, 3LL);
  EXPECT_EQ(record.time_start_ps, 10ULL);
  EXPECT_EQ(record.time_stop_ps, 20ULL);
}
