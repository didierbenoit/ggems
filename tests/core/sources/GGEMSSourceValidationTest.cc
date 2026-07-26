#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, AcceptsDefaultPointFixedRecord) {
  ggems::core::sources::GGEMSSource source{};
  EXPECT_NO_THROW(
      ggems::core::sources::ValidateAnalyticSourceRecord(source.BuildRecord()));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, RejectsUnknownEnumsAndInvalidFrame) {
  ggems::core::sources::GGEMSSource source{};
  auto record = source.BuildRecord();

  record.emission_geometry_type = 99U;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.angular_distribution_type = 99U;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.axis_x_x = 0.0F;
  record.axis_x_y = 0.0F;
  record.axis_x_z = 0.0F;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, ChecksExactSignedEnvelopeDistances) {
  using ggems::core::sources::HasSignedPicoMetreEnvelope;

  EXPECT_TRUE(HasSignedPicoMetreEnvelope(0LL, 0ULL));
  EXPECT_TRUE(HasSignedPicoMetreEnvelope(
      std::numeric_limits<std::int64_t>::max(), 0ULL));
  EXPECT_TRUE(HasSignedPicoMetreEnvelope(
      std::numeric_limits<std::int64_t>::min(), 0ULL));
  EXPECT_FALSE(HasSignedPicoMetreEnvelope(
      std::numeric_limits<std::int64_t>::max(), 1ULL));
  EXPECT_FALSE(HasSignedPicoMetreEnvelope(
      std::numeric_limits<std::int64_t>::min(), 1ULL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, RejectsGeometryOutsideSignedStorage) {
  ggems::core::sources::GGEMSSource source{};
  source.SetPositionPicoMeter(std::numeric_limits<std::int64_t>::max(), 0LL,
                              0LL);

  EXPECT_THROW(source.SetRectangleEmissionPicoMeter(2ULL, 2ULL),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, EnforcesCanonicalGeometryDimensions) {
  using ggems::core::sources::GGEMSEmissionGeometryType;
  using ggems::core::sources::ToKernelEmissionGeometryType;
  using ggems::core::sources::ValidateAnalyticSourceRecord;

  ggems::core::sources::GGEMSSource source{};
  auto record = source.BuildRecord();
  record.geometry_size_z_pm = 1ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Rectangle);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 20ULL;
  record.geometry_size_z_pm = 1ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Box);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 20ULL;
  record.geometry_size_z_pm = 0ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Sphere);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 10ULL;
  record.geometry_size_z_pm = 11ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.emission_geometry_type =
      ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Cylinder);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 11ULL;
  record.geometry_size_z_pm = 20ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, RejectsInvalidBinary32AngularRecords) {
  using ggems::core::sources::ValidateAnalyticSourceRecord;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution();
  auto record = source.BuildRecord();

  record.isotropic_cos_theta_lower = std::numeric_limits<float>::quiet_NaN();
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.isotropic_cos_theta_lower = 0.5F;
  record.isotropic_cos_theta_upper = 0.5F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.isotropic_phi_min_rad = 1.0F;
  record.isotropic_phi_max_rad = 1.0F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.isotropic_phi_max_rad =
      ggems::core::sources::k_isotropic_full_sphere_phi_max_rad + 1.0F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);

  record = source.BuildRecord();
  record.angular_distribution_type =
      ggems::core::sources::ToKernelAngularDistributionType(
          ggems::core::sources::GGEMSAngularDistributionType::Fixed);
  record.isotropic_phi_min_rad = -1.0F;
  record.isotropic_phi_max_rad = 1.0F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, RejectsVolumeEnvelopeOutsideSignedStorage) {
  ggems::core::sources::GGEMSSource source{};
  source.SetPositionPicoMeter(std::numeric_limits<std::int64_t>::max(), 0LL,
                              0LL);

  EXPECT_THROW(source.SetBoxEmissionPicoMeter(2ULL, 2ULL, 2ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetSphereEmissionPicoMeter(2ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source.SetCylinderEmissionPicoMeter(2ULL, 2ULL),
               ggems::core::GGEMSExceptionBase);
}
