#include <cmath>
#include <cstring>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

// =============================================================================
// =============================================================================

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

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountDefaultTo4096) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_EQ(source.GetPrimaryCount(), 4096ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountIsFluentAndAcceptsWholeUint64Range) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_EQ(&source.SetPrimaryCount(17ULL), &source);
  EXPECT_EQ(source.GetPrimaryCount(), 17ULL);

  EXPECT_EQ(&source.SetPrimaryCount(0ULL), &source);
  EXPECT_EQ(source.GetPrimaryCount(), 0ULL);

  EXPECT_EQ(&source.SetPrimaryCount(std::numeric_limits<std::uint64_t>::max()),
            &source);
  EXPECT_EQ(source.GetPrimaryCount(),
            std::numeric_limits<std::uint64_t>::max());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PrimaryCountDoesNotAlterSourceRecord) {
  ggems::core::sources::GGEMSSource source{};

  auto record_before = source.BuildRecord();

  source.SetPrimaryCount(17ULL);

  auto record_after = source.BuildRecord();

  EXPECT_EQ(std::memcmp(&record_before, &record_after, sizeof(record_before)),
            0);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsZeroEnergy) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetEnergyMilliElectronVolt(0ULL),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsReversedTimeWindow) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetTimeWindowPicoSecond(20ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsZeroDirection) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetDirection(0.0f, 0.0f, 0.0f),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNonFiniteDirectionComponents) {
  ggems::core::sources::GGEMSSource source{};

  float nan = std::numeric_limits<float>::quiet_NaN();
  float infinity = std::numeric_limits<float>::infinity();

  EXPECT_THROW(source.SetDirection(nan, 0.0f, 1.0f),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0f, infinity, 1.0f),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0f, 1.0f, -infinity),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNegativeWeight) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetWeight(-1.0f), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNonFiniteWeight) {
  ggems::core::sources::GGEMSSource source{};

  float nan = std::numeric_limits<float>::quiet_NaN();
  float infinity = std::numeric_limits<float>::infinity();

  EXPECT_THROW(source.SetWeight(nan), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetWeight(infinity), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetWeight(-infinity), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, AcceptsZeroWeight) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_NO_THROW(source.SetWeight(0.0f));
  EXPECT_FLOAT_EQ(source.BuildRecord().weight, 0.0f);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, NormalisesDirection) {
  ggems::core::sources::GGEMSSource source{};

  source.SetDirection(0.0F, 0.0F, 2.0F);

  auto const &record = source.GetRecord();

  EXPECT_FLOAT_EQ(record.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_z, 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, BuildRecordReturnsIndependentOwnedSnapshots) {
  ggems::core::sources::GGEMSSource source{};

  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(111'000'000ULL)
      .SetTimeWindowPicoSecond(10ULL, 20ULL)
      .SetPositionPicoMeter(11LL, -22LL, 33LL)
      .SetDirection(1.0f, 0.0, 0.0f)
      .SetWeight(0.25);

  auto record_a = source.BuildRecord();

  source
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetTimeWindowPicoSecond(30ULL, 40ULL)
      .SetPositionPicoMeter(-44LL, 55LL, -66LL)
      .SetDirection(0.0f, -1.0f, 0.0f)
      .SetWeight(0.75);

  auto record_b = source.BuildRecord();

  std::uint32_t analytic_source_type = ggems::core::sources::ToKernelSourceType(
      ggems::core::sources::GGEMSSourceType::Analytic);

  EXPECT_EQ(record_a.source_type, analytic_source_type);
  EXPECT_EQ(record_a.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Gamma));
  EXPECT_EQ(record_a.energy_milli_eV, 111'000'000ULL);
  EXPECT_EQ(record_a.time_start_ps, 10ULL);
  EXPECT_EQ(record_a.time_stop_ps, 20ULL);
  EXPECT_EQ(record_a.position_x_pm, 11LL);
  EXPECT_EQ(record_a.position_y_pm, -22LL);
  EXPECT_EQ(record_a.position_z_pm, 33LL);
  EXPECT_FLOAT_EQ(record_a.direction_x, 1.0F);
  EXPECT_FLOAT_EQ(record_a.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(record_a.direction_z, 0.0F);
  EXPECT_FLOAT_EQ(record_a.direction_w, 0.0F);
  EXPECT_FLOAT_EQ(record_a.weight, 0.25F);

  EXPECT_EQ(record_b.source_type, analytic_source_type);
  EXPECT_EQ(record_b.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Electron));

  EXPECT_EQ(record_b.energy_milli_eV, 222'000'000ULL);
  EXPECT_EQ(record_b.time_start_ps, 30ULL);
  EXPECT_EQ(record_b.time_stop_ps, 40ULL);
  EXPECT_EQ(record_b.position_x_pm, -44LL);
  EXPECT_EQ(record_b.position_y_pm, 55LL);
  EXPECT_EQ(record_b.position_z_pm, -66LL);
  EXPECT_FLOAT_EQ(record_b.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(record_b.direction_y, -1.0F);
  EXPECT_FLOAT_EQ(record_b.direction_z, 0.0F);
  EXPECT_FLOAT_EQ(record_b.direction_w, 0.0F);
  EXPECT_FLOAT_EQ(record_b.weight, 0.75F);
}
