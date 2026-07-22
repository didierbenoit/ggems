#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, DefaultsSpecificCaptureToInvalidCoordinates) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  auto const config = observer.BuildConfigRecord();

  EXPECT_EQ(config.enabled, 0U);
  EXPECT_EQ(config.capture_first_primary_count_per_source, 0U);
  EXPECT_EQ(config.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(config.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(config.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, BuildsCombinedPerSourceAndSpecificCaptureConfig) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  observer.CaptureFirstPrimaries(4U);

  auto const first_config = observer.BuildConfigRecord();

  EXPECT_EQ(first_config.enabled, 1U);
  EXPECT_EQ(first_config.capture_first_primary_count_per_source, 4U);
  EXPECT_EQ(first_config.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(first_config.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(first_config.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);

  observer.CapturePrimary(2U, 7ULL);

  auto const combined_config = observer.BuildConfigRecord();

  EXPECT_EQ(combined_config.enabled, 1U);
  EXPECT_EQ(combined_config.capture_first_primary_count_per_source, 4U);
  EXPECT_EQ(combined_config.capture_specific_primary_enabled, 1U);
  EXPECT_EQ(combined_config.capture_source_index, 2U);
  EXPECT_EQ(combined_config.capture_source_local_primary_id, 7ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, ClearResetsOnlySpecificCapture) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  observer.CaptureFirstPrimaries(5U).CapturePrimary(3U, 11ULL);

  auto const configured = observer.BuildConfigRecord();

  ASSERT_EQ(configured.enabled, 1U);
  ASSERT_EQ(configured.capture_first_primary_count_per_source, 5U);
  ASSERT_EQ(configured.capture_specific_primary_enabled, 1U);
  ASSERT_EQ(configured.capture_source_index, 3U);
  ASSERT_EQ(configured.capture_source_local_primary_id, 11ULL);

  observer.ClearCapturedPrimary();

  auto const cleared = observer.BuildConfigRecord();

  EXPECT_EQ(cleared.enabled, 1U);
  EXPECT_EQ(cleared.capture_first_primary_count_per_source, 5U);
  EXPECT_EQ(cleared.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(cleared.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(cleared.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);
}
