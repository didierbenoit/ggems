#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSObserverRecord, ConfigRecordIsKernelFriendly) {
  EXPECT_TRUE(std::is_standard_layout_v<
              ggems::core::observer::GGEMSObserverConfigRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<
              ggems::core::observer::GGEMSObserverConfigRecord>);
  EXPECT_EQ(sizeof(ggems::core::observer::GGEMSObserverConfigRecord), 24U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSObserverRecord, CountersAreKernelFriendly) {
  EXPECT_TRUE(
      std::is_standard_layout_v<ggems::core::observer::GGEMSObserverCounters>);
  EXPECT_TRUE(std::is_trivially_copyable_v<
              ggems::core::observer::GGEMSObserverCounters>);
  EXPECT_EQ(sizeof(ggems::core::observer::GGEMSObserverCounters), 16U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSObserverRecord, RecordIsKernelFriendly) {
  EXPECT_TRUE(
      std::is_standard_layout_v<ggems::core::observer::GGEMSObserverRecord>);
  EXPECT_TRUE(
      std::is_trivially_copyable_v<ggems::core::observer::GGEMSObserverRecord>);
  EXPECT_EQ(sizeof(ggems::core::observer::GGEMSObserverRecord), 120U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSObserverRecord, DefaultRecordIsEmptyAndInactive) {
  ggems::core::observer::GGEMSObserverRecord record{};

  EXPECT_EQ(record.run_id, 0ULL);
  EXPECT_EQ(record.global_primary_id, ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(record.global_particle_id,
            ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(record.track_id, ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(record.parent_track_id, ggems::core::particles::k_invalid_id_u64);

  EXPECT_EQ(record.record_kind,
            ggems::core::observer::ToKernelObserverRecordKind(
                ggems::core::observer::GGEMSObserverRecordKind::Unknown));

  EXPECT_EQ(record.particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown));

  EXPECT_EQ(record.status,
            ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Inactive));

  EXPECT_FLOAT_EQ(record.direction_x, 0.0f);
  EXPECT_FLOAT_EQ(record.direction_y, 0.0f);
  EXPECT_FLOAT_EQ(record.direction_z, 1.0f);
  EXPECT_FLOAT_EQ(record.weight, 1.0f);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSObserverTypes, RecordKindsAreKernelCompatible) {
  using ggems::core::observer::GGEMSObserverRecordKind;
  using ggems::core::observer::ToKernelObserverRecordKind;

  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::Unknown), 0U);
  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::Source), 1U);
  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::Step), 2U);
  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::SecondaryStep),
            3U);
  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::Terminal), 4U);
  EXPECT_EQ(ToKernelObserverRecordKind(GGEMSObserverRecordKind::Anomaly), 5U);
}
