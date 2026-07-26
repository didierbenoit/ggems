#include <cmath>
#include <limits>
#include <cstdint>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

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
  EXPECT_EQ(actual.geometry_size_z_pm, expected.geometry_size_z_pm);
  EXPECT_EQ(actual.focus_position_x_pm, expected.focus_position_x_pm);
  EXPECT_EQ(actual.focus_position_y_pm, expected.focus_position_y_pm);
  EXPECT_EQ(actual.focus_position_z_pm, expected.focus_position_z_pm);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_lower,
                  expected.isotropic_cos_theta_lower);
  EXPECT_FLOAT_EQ(actual.isotropic_cos_theta_upper,
                  expected.isotropic_cos_theta_upper);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_min_rad, expected.isotropic_phi_min_rad);
  EXPECT_FLOAT_EQ(actual.isotropic_phi_max_rad, expected.isotropic_phi_max_rad);
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
  EXPECT_EQ(record.geometry_size_z_pm, 0ULL);
  EXPECT_EQ(record.focus_position_x_pm, 0LL);
  EXPECT_EQ(record.focus_position_y_pm, 0LL);
  EXPECT_EQ(record.focus_position_z_pm, 0LL);
  EXPECT_FLOAT_EQ(record.isotropic_cos_theta_lower, -1.0F);
  EXPECT_FLOAT_EQ(record.isotropic_cos_theta_upper, 1.0F);
  EXPECT_FLOAT_EQ(record.isotropic_phi_min_rad, 0.0F);
  EXPECT_FLOAT_EQ(record.isotropic_phi_max_rad,
                  ggems::core::sources::k_isotropic_full_sphere_phi_max_rad);
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
  EXPECT_EQ(rectangle.geometry_size_z_pm, 0ULL);

  source.SetEllipseEmissionPicoMeter(30ULL, 10ULL);
  auto ellipse = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(ellipse.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(ellipse.geometry_size_x_pm, 30ULL);
  EXPECT_EQ(ellipse.geometry_size_y_pm, 10ULL);
  EXPECT_EQ(ellipse.geometry_size_z_pm, 0ULL);

  source.SetCircleEmissionPicoMeter(12ULL);
  auto circle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(circle.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(circle.geometry_size_x_pm, 12ULL);
  EXPECT_EQ(circle.geometry_size_y_pm, 12ULL);
  EXPECT_EQ(circle.geometry_size_z_pm, 0ULL);

  source.SetPointEmission();
  auto point = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(point.emission_geometry_type),
            GGEMSEmissionGeometryType::Point);
  EXPECT_EQ(point.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(point.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(point.geometry_size_z_pm, 0ULL);
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

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresCanonicalVolumeDimensionsAndSwitching) {
  using ggems::core::sources::FromKernelEmissionGeometryType;
  using ggems::core::sources::GGEMSEmissionGeometryType;

  ggems::core::sources::GGEMSSource source{};

  source.SetBoxEmissionPicoMeter(40ULL, 20ULL, 10ULL);
  auto const box = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(box.emission_geometry_type),
            GGEMSEmissionGeometryType::Box);
  EXPECT_EQ(box.geometry_size_x_pm, 40ULL);
  EXPECT_EQ(box.geometry_size_y_pm, 20ULL);
  EXPECT_EQ(box.geometry_size_z_pm, 10ULL);

  source.SetSphereEmissionPicoMeter(12ULL);
  auto const sphere = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(sphere.emission_geometry_type),
            GGEMSEmissionGeometryType::Sphere);
  EXPECT_EQ(sphere.geometry_size_x_pm, 12ULL);
  EXPECT_EQ(sphere.geometry_size_y_pm, 12ULL);
  EXPECT_EQ(sphere.geometry_size_z_pm, 12ULL);

  source.SetCylinderEmissionPicoMeter(14ULL, 30ULL);
  auto const cylinder = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(cylinder.emission_geometry_type),
            GGEMSEmissionGeometryType::Cylinder);
  EXPECT_EQ(cylinder.geometry_size_x_pm, 14ULL);
  EXPECT_EQ(cylinder.geometry_size_y_pm, 14ULL);
  EXPECT_EQ(cylinder.geometry_size_z_pm, 30ULL);

  source.SetCircleEmissionPicoMeter(8ULL);
  auto const circle = source.BuildRecord();
  EXPECT_EQ(FromKernelEmissionGeometryType(circle.emission_geometry_type),
            GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(circle.geometry_size_x_pm, 8ULL);
  EXPECT_EQ(circle.geometry_size_y_pm, 8ULL);
  EXPECT_EQ(circle.geometry_size_z_pm, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidVolumeDimensionsAtomically) {
  ggems::core::sources::GGEMSSource source{};
  source.SetRectangleEmissionPicoMeter(40ULL, 20ULL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetBoxEmissionPicoMeter(0ULL, 20ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetBoxEmissionPicoMeter(40ULL, 0ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetBoxEmissionPicoMeter(40ULL, 20ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetSphereEmissionPicoMeter(0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCylinderEmissionPicoMeter(0ULL, 10ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCylinderEmissionPicoMeter(10ULL, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetSphereEmissionPicoMeter(
                   std::numeric_limits<std::uint64_t>::max()),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, ConfiguresBoundedAndCanonicalFullSphereIsotropicDomains) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  using ggems::units::MakeRadians;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution(
      MakeRadians(0.0L), MakeRadians(0.5L * k_pi), MakeRadians(-0.5L * k_pi),
      MakeRadians(0.5L * k_pi));

  auto const bounded = source.BuildRecord();
  EXPECT_NEAR(bounded.isotropic_cos_theta_lower, 0.0F,
              std::numeric_limits<float>::epsilon());
  EXPECT_FLOAT_EQ(bounded.isotropic_cos_theta_upper, 1.0F);
  EXPECT_FLOAT_EQ(bounded.isotropic_phi_min_rad,
                  static_cast<float>(-0.5L * k_pi));
  EXPECT_FLOAT_EQ(bounded.isotropic_phi_max_rad,
                  static_cast<float>(0.5L * k_pi));
  EXPECT_EQ(bounded.focus_position_x_pm, 0LL);
  EXPECT_EQ(bounded.focus_position_y_pm, 0LL);
  EXPECT_EQ(bounded.focus_position_z_pm, 0LL);

  source.SetIsotropicAngularDistribution(MakeRadians(0.0L), MakeRadians(k_pi),
                                         MakeRadians(0.0L),
                                         MakeRadians(2.0L * k_pi));
  auto const explicit_full_sphere = source.BuildRecord();

  ggems::core::sources::GGEMSSource no_argument{};
  no_argument.SetIsotropicAngularDistribution();
  auto const canonical_full_sphere = no_argument.BuildRecord();

  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_cos_theta_lower,
                  canonical_full_sphere.isotropic_cos_theta_lower);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_cos_theta_upper,
                  canonical_full_sphere.isotropic_cos_theta_upper);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_phi_min_rad,
                  canonical_full_sphere.isotropic_phi_min_rad);
  EXPECT_FLOAT_EQ(explicit_full_sphere.isotropic_phi_max_rad,
                  canonical_full_sphere.isotropic_phi_max_rad);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsInvalidOrCollapsedAngularDomainsAtomically) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  using ggems::units::MakeRadians;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution(MakeRadians(0.1L), MakeRadians(1.0L),
                                         MakeRadians(-1.0L), MakeRadians(1.0L));
  auto const before = source.BuildRecord();
  long double const nan = std::numeric_limits<long double>::quiet_NaN();
  long double const infinity = std::numeric_limits<long double>::infinity();

  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(nan), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(infinity), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(-0.1L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(1.0L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(k_pi + 0.1L),
                   MakeRadians(0.0L), MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(1.0L),
                   MakeRadians(1.0L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(0.0L),
                   MakeRadians(2.0L * k_pi + 0.1L)),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetIsotropicAngularDistribution(
                   MakeRadians(0.0L), MakeRadians(1.0L), MakeRadians(1.0L),
                   MakeRadians(std::nextafter(1.0L, 2.0L))),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectsFocusedTargetsInsideVolumeSupport) {
  ggems::core::sources::GGEMSSource box{};
  box.SetBoxEmissionPicoMeter(100ULL, 80ULL, 60ULL);
  EXPECT_THROW(box.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      box.SetFocusedAngularDistributionPicoMeter(1'000LL, 0LL, 0LL));

  ggems::core::sources::GGEMSSource sphere{};
  sphere.SetSphereEmissionPicoMeter(100ULL);
  EXPECT_THROW(sphere.SetFocusedAngularDistributionPicoMeter(50LL, 0LL, 0LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      sphere.SetFocusedAngularDistributionPicoMeter(1'000LL, 0LL, 0LL));

  ggems::core::sources::GGEMSSource cylinder{};
  cylinder.SetCylinderEmissionPicoMeter(100ULL, 200ULL);
  EXPECT_THROW(cylinder.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 100LL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(
      cylinder.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 1'000LL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSource, RejectedDirectionMovingFocusedBoxPreservesRecord) {
  ggems::core::sources::GGEMSSource source{};
  source.SetBoxEmissionPicoMeter(100ULL, 2ULL, 2ULL)
      .SetFocusedAngularDistributionPicoMeter(0LL, 40LL, 0LL);
  auto const before = source.BuildRecord();

  EXPECT_THROW(source.SetDirection(1.0, 0.0, 0.0),
               ggems::core::GGEMSExceptionBase);

  ExpectSourceRecordsEqual(source.BuildRecord(), before);
}
