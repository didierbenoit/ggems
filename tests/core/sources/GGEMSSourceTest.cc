#include <cstring>
#include <limits>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

// =============================================================================
// =============================================================================

auto ExpectSourceFramesEqual(
    ggems::core::sources::GGEMSSourceRecord const &actual,
    ggems::core::sources::GGEMSSourceRecord const &expected) -> void {
  EXPECT_FLOAT_EQ(actual.axis_x_x, expected.axis_x_x);
  EXPECT_FLOAT_EQ(actual.axis_x_y, expected.axis_x_y);
  EXPECT_FLOAT_EQ(actual.axis_x_z, expected.axis_x_z);

  EXPECT_FLOAT_EQ(actual.axis_y_x, expected.axis_y_x);
  EXPECT_FLOAT_EQ(actual.axis_y_y, expected.axis_y_y);
  EXPECT_FLOAT_EQ(actual.axis_y_z, expected.axis_y_z);

  EXPECT_FLOAT_EQ(actual.axis_z_x, expected.axis_z_x);
  EXPECT_FLOAT_EQ(actual.axis_z_y, expected.axis_z_y);
  EXPECT_FLOAT_EQ(actual.axis_z_z, expected.axis_z_z);
}

// =============================================================================
// =============================================================================

auto ExpectSourceRecordsEqual(
    ggems::core::sources::GGEMSSourceRecord const &actual,
    ggems::core::sources::GGEMSSourceRecord const &expected) -> void {
  EXPECT_EQ(actual.source_id, expected.source_id);
  EXPECT_EQ(actual.time_start_ps, expected.time_start_ps);
  EXPECT_EQ(actual.time_stop_ps, expected.time_stop_ps);
  EXPECT_EQ(actual.energy_milli_eV, expected.energy_milli_eV);

  EXPECT_EQ(actual.position_x_pm, expected.position_x_pm);
  EXPECT_EQ(actual.position_y_pm, expected.position_y_pm);
  EXPECT_EQ(actual.position_z_pm, expected.position_z_pm);

  EXPECT_EQ(actual.source_type, expected.source_type);
  EXPECT_EQ(actual.emitted_particle_type, expected.emitted_particle_type);
  EXPECT_EQ(actual.flags, expected.flags);
  EXPECT_EQ(actual.reserved_0, expected.reserved_0);

  EXPECT_FLOAT_EQ(actual.axis_x_x, expected.axis_x_x);
  EXPECT_FLOAT_EQ(actual.axis_x_y, expected.axis_x_y);
  EXPECT_FLOAT_EQ(actual.axis_x_z, expected.axis_x_z);

  EXPECT_FLOAT_EQ(actual.axis_y_x, expected.axis_y_x);
  EXPECT_FLOAT_EQ(actual.axis_y_y, expected.axis_y_y);
  EXPECT_FLOAT_EQ(actual.axis_y_z, expected.axis_y_z);

  EXPECT_FLOAT_EQ(actual.axis_z_x, expected.axis_z_x);
  EXPECT_FLOAT_EQ(actual.axis_z_y, expected.axis_z_y);
  EXPECT_FLOAT_EQ(actual.axis_z_z, expected.axis_z_z);

  EXPECT_FLOAT_EQ(actual.weight, expected.weight);
  EXPECT_EQ(actual.emission_geometry_type, expected.emission_geometry_type);
  EXPECT_EQ(actual.angular_distribution_type,
            expected.angular_distribution_type);
  EXPECT_EQ(actual.geometry_size_x_pm, expected.geometry_size_x_pm);
  EXPECT_EQ(actual.geometry_size_y_pm, expected.geometry_size_y_pm);
  EXPECT_EQ(actual.focus_position_x_pm, expected.focus_position_x_pm);
  EXPECT_EQ(actual.focus_position_y_pm, expected.focus_position_y_pm);
  EXPECT_EQ(actual.focus_position_z_pm, expected.focus_position_z_pm);
}

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

  EXPECT_FLOAT_EQ(record.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 1.0F);
  EXPECT_FLOAT_EQ(record.weight, 1.0F);
  EXPECT_EQ(record.emission_geometry_type,
            ggems::core::sources::ToKernelEmissionGeometryType(
                ggems::core::sources::GGEMSEmissionGeometryType::Point));
  EXPECT_EQ(record.angular_distribution_type,
            ggems::core::sources::ToKernelAngularDistributionType(
                ggems::core::sources::GGEMSAngularDistributionType::Fixed));
  EXPECT_EQ(record.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(record.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(record.focus_position_x_pm, 0LL);
  EXPECT_EQ(record.focus_position_y_pm, 0LL);
  EXPECT_EQ(record.focus_position_z_pm, 0LL);
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

  ExpectSourceRecordsEqual(record_after, record_before);
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

  EXPECT_THROW(source.SetDirection(0.0F, 0.0F, 0.0F),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNonFiniteDirectionComponents) {
  ggems::core::sources::GGEMSSource source{};

  auto nan = std::numeric_limits<double>::quiet_NaN();
  auto infinity = std::numeric_limits<double>::infinity();

  EXPECT_THROW(source.SetDirection(nan, 0.0F, 1.0F),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0F, infinity, 1.0F),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetDirection(0.0F, 1.0F, -infinity),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsNegativeWeight) {
  ggems::core::sources::GGEMSSource source{};

  EXPECT_THROW(source.SetWeight(-1.0F), ggems::core::GGEMSExceptionBase);
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

  EXPECT_NO_THROW(source.SetWeight(0.0F));
  EXPECT_FLOAT_EQ(source.BuildRecord().weight, 0.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, NormalisesDirection) {
  ggems::core::sources::GGEMSSource source{};

  source.SetDirection(0.0, 0.0, 2.0);

  auto const &record = source.GetRecord();

  EXPECT_FLOAT_EQ(record.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, SetDirectionBuildsAutomaticCtFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetDirection(1.0, 0.0, 0.0);

  auto const &record = source.GetRecord();
  EXPECT_FLOAT_EQ(record.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 0.0F);
}

TEST(GGEMSSource, SetOrientationStoresValidatedCompleteFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetOrientation({1.0, 0.0, 0.0}, {1.0, 0.0, 2.0});

  auto const &record = source.GetRecord();
  EXPECT_FLOAT_EQ(record.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 0.0F);
}

TEST(GGEMSSource, RejectedOrientationDoesNotMutateExistingFrame) {
  ggems::core::sources::GGEMSSource source{};
  source.SetDirection(1.0, 0.0, 0.0);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetOrientation({0.0, 0.0, 1.0}, {0.001, 0.0, 1.0}),
               ggems::core::GGEMSExceptionBase);

  auto const after = source.BuildRecord();

  ExpectSourceFramesEqual(after, before);
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
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25);

  auto record_a = source.BuildRecord();

  source
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetTimeWindowPicoSecond(30ULL, 40ULL)
      .SetPositionPicoMeter(-44LL, 55LL, -66LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
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
  EXPECT_FLOAT_EQ(record_a.axis_x_x, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_x, 1.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(record_a.axis_z_z, 0.0F);
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
  EXPECT_FLOAT_EQ(record_b.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_y, -1.0F);
  EXPECT_FLOAT_EQ(record_b.axis_z_z, 0.0F);
  EXPECT_FLOAT_EQ(record_b.weight, 0.75F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresPointRectangleEllipseAndCircle) {
  using ggems::core::sources::FromKernelEmissionGeometryType;
  using ggems::core::sources::GGEMSEmissionGeometryType;

  ggems::core::sources::GGEMSSource source{};

  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto rectangle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(rectangle.emission_geometry_type),
            GGEMSEmissionGeometryType::Rectangle);
  EXPECT_EQ(rectangle.geometry_size_x_pm, 40ULL);
  EXPECT_EQ(rectangle.geometry_size_y_pm, 20ULL);

  source.SetEllipseEmissionPicoMeter(30ULL, 10ULL);
  auto ellipse = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(ellipse.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(ellipse.geometry_size_x_pm, 30ULL);
  EXPECT_EQ(ellipse.geometry_size_y_pm, 10ULL);

  source.SetCircleEmissionPicoMeter(12ULL);
  auto circle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(circle.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(circle.geometry_size_x_pm, 12ULL);
  EXPECT_EQ(circle.geometry_size_y_pm, 12ULL);

  source.SetPointEmission();
  auto point = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(point.emission_geometry_type),
            GGEMSEmissionGeometryType::Point);
  EXPECT_EQ(point.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(point.geometry_size_y_pm, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidEmissionDimensionsAtomically) {
  ggems::core::sources::GGEMSSource source{};
  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetRectangleEmissionPicoMeter(0ULL, 20ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetEllipseEmissionPicoMeter(10ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCircleEmissionPicoMeter(0ULL),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresAngularDistributionsAndCanonicalisesFocus) {
  using ggems::core::sources::FromKernelAngularDistributionType;
  using ggems::core::sources::GGEMSAngularDistributionType;

  ggems::core::sources::GGEMSSource source{};
  source.SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetFocusedAngularDistributionPicoMeter(40LL, 50LL, 60LL);

  auto focused = source.BuildRecord();
  EXPECT_EQ(
      FromKernelAngularDistributionType(focused.angular_distribution_type),
      GGEMSAngularDistributionType::Focused);
  EXPECT_EQ(focused.focus_position_x_pm, 40LL);
  EXPECT_EQ(focused.focus_position_y_pm, 50LL);
  EXPECT_EQ(focused.focus_position_z_pm, 60LL);

  source.SetIsotropicAngularDistribution();
  auto isotropic = source.BuildRecord();
  EXPECT_EQ(
      FromKernelAngularDistributionType(isotropic.angular_distribution_type),
      GGEMSAngularDistributionType::Isotropic);
  EXPECT_EQ(isotropic.focus_position_x_pm, 0LL);
  EXPECT_EQ(isotropic.focus_position_y_pm, 0LL);
  EXPECT_EQ(isotropic.focus_position_z_pm, 0LL);

  source.SetFixedAngularDistribution();
  auto fixed = source.BuildRecord();
  EXPECT_EQ(FromKernelAngularDistributionType(fixed.angular_distribution_type),
            GGEMSAngularDistributionType::Fixed);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsDegenerateFocusedConfigurations) {
  ggems::core::sources::GGEMSSource point{};

  EXPECT_THROW(point.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);

  ggems::core::sources::GGEMSSource rectangle{};
  rectangle.SetRectangleEmissionPicoMeter(10'000ULL, 20'000ULL);

  EXPECT_THROW(
      rectangle.SetFocusedAngularDistributionPicoMeter(10LL, 20LL, 0LL),
      ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(
      rectangle.SetFocusedAngularDistributionPicoMeter(10LL, 20LL, 1'000LL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, PoseChangesDoNotSelectAnotherAngularMode) {
  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution()
      .SetDirection(1.0, 0.0, 0.0)
      .SetOrientation({0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});

  EXPECT_EQ(ggems::core::sources::FromKernelAngularDistributionType(
                source.BuildRecord().angular_distribution_type),
            ggems::core::sources::GGEMSAngularDistributionType::Isotropic);
}
