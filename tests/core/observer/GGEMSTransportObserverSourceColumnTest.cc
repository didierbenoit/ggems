#include <array>
#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildObserverDump(std::uint32_t source_slot_count,
                                     std::uint32_t captured_source_index)
    -> std::string {
  using ggems::core::observer::GGEMSObserverCounters;
  using ggems::core::observer::GGEMSObserverRecord;
  using ggems::core::observer::GGEMSObserverRecordKind;
  using ggems::core::observer::GGEMSTransportObserver;
  using ggems::core::observer::ToKernelObserverRecordKind;
  using ggems::core::particles::GGEMSParticleStatus;
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::particles::ToKernelParticleStatus;
  using ggems::core::particles::ToKernelParticleType;

  GGEMSTransportObserver observer{};
  observer.Enable();
  observer.SetRunSourceSlotCount(source_slot_count);

  GGEMSObserverRecord record{};
  record.run_id = 0ULL;
  record.global_primary_id = 4ULL;
  record.source_local_primary_id = 0ULL;
  record.global_particle_id = 7ULL;
  record.track_id = 7ULL;
  record.parent_track_id = ggems::core::particles::k_invalid_id_u64;
  record.record_kind =
      ToKernelObserverRecordKind(GGEMSObserverRecordKind::Source);
  record.particle_type = ToKernelParticleType(GGEMSParticleType::Gamma);
  record.status = ToKernelParticleStatus(GGEMSParticleStatus::Alive);
  record.source_index = captured_source_index;

  std::array<GGEMSObserverRecord, 1U> const records{record};

  GGEMSObserverCounters counters{};
  counters.record_count = 1U;
  counters.captured_primary_count = 1U;

  observer.Accumulate(records, counters);
  return observer.BuildDump();
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverSourceColumn,
     IsAbsentForOneSnapshotSlotRegardlessOfCapturedIndex) {
  std::string const dump = BuildObserverDump(1U, 7U);

  EXPECT_EQ(dump.find("| Src |"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverSourceColumn,
     IsPresentForTwoSnapshotSlotsWithOneCapturedIndex) {
  std::string const dump = BuildObserverDump(2U, 0U);

  EXPECT_NE(dump.find("| Src |"), std::string::npos);
  EXPECT_EQ(dump.find("source_local_primary_id"), std::string::npos);
}

} // namespace
