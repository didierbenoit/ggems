#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"

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
