#include <cstddef>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSObserverRecord, ConfigRecordIsKernelFriendly) {
  EXPECT_TRUE(std::is_standard_layout_v<
              ggems::core::observer::GGEMSObserverConfigRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<
              ggems::core::observer::GGEMSObserverConfigRecord>);
  EXPECT_EQ(sizeof(ggems::core::observer::GGEMSObserverConfigRecord), 24U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSObserverRecord, CountersAreKernelFriendly) {
  EXPECT_TRUE(
      std::is_standard_layout_v<ggems::core::observer::GGEMSObserverCounters>);
  EXPECT_TRUE(std::is_trivially_copyable_v<
              ggems::core::observer::GGEMSObserverCounters>);
  EXPECT_EQ(sizeof(ggems::core::observer::GGEMSObserverCounters), 16U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSObserverRecord, RecordIsKernelFriendly) {
  using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

  EXPECT_TRUE(std::is_standard_layout_v<ObserverRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<ObserverRecord>);

  EXPECT_EQ(sizeof(ObserverRecord), 128U);
  EXPECT_EQ(alignof(ObserverRecord), 8U);

  EXPECT_EQ(offsetof(ObserverRecord, run_id), 0U);
  EXPECT_EQ(offsetof(ObserverRecord, global_primary_id), 8U);
  EXPECT_EQ(offsetof(ObserverRecord, source_local_primary_id), 16U);
  EXPECT_EQ(offsetof(ObserverRecord, global_particle_id), 24U);
  EXPECT_EQ(offsetof(ObserverRecord, track_id), 32U);
  EXPECT_EQ(offsetof(ObserverRecord, parent_track_id), 40U);
  EXPECT_EQ(offsetof(ObserverRecord, time_ps), 48U);

  EXPECT_EQ(offsetof(ObserverRecord, position_x_pm), 56U);
  EXPECT_EQ(offsetof(ObserverRecord, position_y_pm), 64U);
  EXPECT_EQ(offsetof(ObserverRecord, position_z_pm), 72U);

  EXPECT_EQ(offsetof(ObserverRecord, record_kind), 80U);
  EXPECT_EQ(offsetof(ObserverRecord, particle_type), 84U);
  EXPECT_EQ(offsetof(ObserverRecord, status), 88U);
  EXPECT_EQ(offsetof(ObserverRecord, generation), 92U);

  EXPECT_EQ(offsetof(ObserverRecord, direction_x), 96U);
  EXPECT_EQ(offsetof(ObserverRecord, direction_y), 100U);
  EXPECT_EQ(offsetof(ObserverRecord, direction_z), 104U);
  EXPECT_EQ(offsetof(ObserverRecord, direction_w), 108U);

  EXPECT_EQ(offsetof(ObserverRecord, energy_milli_eV), 112U);
  EXPECT_EQ(offsetof(ObserverRecord, weight), 120U);
  EXPECT_EQ(offsetof(ObserverRecord, source_index), 124U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSObserverRecord, DefaultRecordIsEmptyAndInactive) {
  ggems::core::observer::GGEMSObserverRecord record{};

  EXPECT_EQ(record.run_id, 0ULL);
  EXPECT_EQ(record.global_primary_id, ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(record.source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);
  EXPECT_EQ(record.source_index, ggems::core::particles::k_invalid_id_u32);
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

  EXPECT_FLOAT_EQ(record.direction_x, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_y, 0.0F);
  EXPECT_FLOAT_EQ(record.direction_z, 1.0F);
  EXPECT_FLOAT_EQ(record.weight, 1.0F);
}

// =============================================================================
// =============================================================================

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
