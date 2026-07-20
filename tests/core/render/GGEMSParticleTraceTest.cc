#include <vector>
#include <cstdint>
#include <span>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::observer::GGEMSObserverRecord;
using ggems::core::observer::GGEMSObserverRecordKind;
using ggems::core::observer::ToKernelObserverRecordKind;
using ggems::core::particles::GGEMSParticleType;
using ggems::core::particles::k_invalid_id_u32;
using ggems::core::particles::k_invalid_id_u64;
using ggems::core::particles::ToKernelParticleType;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRecord(
    std::uint64_t run_id, std::uint64_t primary_id, std::uint64_t track_id,
    std::uint64_t time_ps, GGEMSObserverRecordKind kind,
    GGEMSParticleType particle_type, std::int64_t x_pm, std::int64_t y_pm,
    std::int64_t z_pm, std::uint32_t source_index = 0U,
    std::uint64_t source_local_primary_id = 0ULL,
    std::uint64_t parent_track_id = k_invalid_id_u64) -> GGEMSObserverRecord {
  GGEMSObserverRecord record{};

  record.run_id = run_id;
  record.global_primary_id = primary_id;
  record.source_local_primary_id = source_local_primary_id;
  record.global_particle_id = track_id;
  record.track_id = track_id;
  record.parent_track_id = parent_track_id;
  record.source_index = source_index;
  record.time_ps = time_ps;
  record.record_kind = ToKernelObserverRecordKind(kind);
  record.particle_type = ToKernelParticleType(particle_type);
  record.position_x_pm = x_pm;
  record.position_y_pm = y_pm;
  record.position_z_pm = z_pm;

  return record;
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, EmptyRecordsProduceNoSegment) {
  std::vector<GGEMSObserverRecord> const records{};

  EXPECT_TRUE(ggems::render::BuildParticleTraceSegments(records).empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, SingleRecordProducesNoSegment) {
  std::vector<GGEMSObserverRecord> records{
      MakeRecord(0ULL, 7ULL, 42ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL)};

  EXPECT_TRUE(ggems::render::BuildParticleTraceSegments(records).empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, ConsecutiveRecordsInSameTrackProduceSegments) {
  std::vector<GGEMSObserverRecord> records{
      MakeRecord(0ULL, 7ULL, 42ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL, 4U, 2ULL),
      MakeRecord(0ULL, 7ULL, 42ULL, 10ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL, 4U,
                 2ULL),
      MakeRecord(0ULL, 7ULL, 42ULL, 20ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 200'000'000'000LL, 4U,
                 2ULL)};

  std::vector<ggems::render::GGEMSParticleTraceSegment> segments =
      ggems::render::BuildParticleTraceSegments(records);

  ASSERT_EQ(segments.size(), 2U);

  EXPECT_EQ(segments[0].run_id, 0ULL);
  EXPECT_EQ(segments[0].global_primary_id, 7ULL);
  EXPECT_EQ(segments[0].source_index, 4U);
  EXPECT_EQ(segments[0].source_local_primary_id, 2ULL);
  EXPECT_EQ(segments[0].track_id, 42ULL);
  EXPECT_EQ(segments[0].particle_type, GGEMSParticleType::Gamma);
  EXPECT_EQ(segments[0].begin_kind, GGEMSObserverRecordKind::Source);
  EXPECT_EQ(segments[0].end_kind, GGEMSObserverRecordKind::Step);
  EXPECT_FLOAT_EQ(segments[0].begin.z_m, 0.0F);
  EXPECT_FLOAT_EQ(segments[0].end.z_m, 0.1F);

  EXPECT_FLOAT_EQ(segments[1].begin.z_m, 0.1F);
  EXPECT_FLOAT_EQ(segments[1].end.z_m, 0.2F);
  EXPECT_EQ(segments[1].source_index, 4U);
  EXPECT_EQ(segments[1].source_local_primary_id, 2ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, RecordsAreSortedBeforeSegmentConstruction) {
  std::vector<GGEMSObserverRecord> records{
      MakeRecord(0ULL, 3ULL, 9ULL, 20ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Electron, 100'000'000'000LL, 0LL, 0LL),
      MakeRecord(0ULL, 3ULL, 9ULL, 0ULL, GGEMSObserverRecordKind::SecondaryStep,
                 GGEMSParticleType::Electron, 0LL, 0LL, 0LL)};

  std::vector<ggems::render::GGEMSParticleTraceSegment> segments =
      ggems::render::BuildParticleTraceSegments(records);

  ASSERT_EQ(segments.size(), 1U);
  EXPECT_EQ(segments[0].begin_kind, GGEMSObserverRecordKind::SecondaryStep);
  EXPECT_EQ(segments[0].end_kind, GGEMSObserverRecordKind::Step);
  EXPECT_FLOAT_EQ(segments[0].begin.x_m, 0.0F);
  EXPECT_FLOAT_EQ(segments[0].end.x_m, 0.1F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, DifferentTrackAreNotConnectedTogether) {
  std::vector<GGEMSObserverRecord> records{
      MakeRecord(0ULL, 1ULL, 10ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL),
      MakeRecord(0ULL, 1ULL, 10ULL, 10ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL),
      MakeRecord(0ULL, 1ULL, 11ULL, 15ULL,
                 GGEMSObserverRecordKind::SecondaryStep,
                 GGEMSParticleType::Electron, 0LL, 0LL, 100'000'000'000LL, 0U,
                 0ULL, 10ULL),
      MakeRecord(0ULL, 1ULL, 11ULL, 25ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Electron, 50'000'000'000LL, 0LL,
                 125'000'000'000LL, 0U, 0ULL, 10ULL)};

  std::vector<ggems::render::GGEMSParticleTraceSegment> segments =
      ggems::render::BuildParticleTraceSegments(records);

  ASSERT_EQ(segments.size(), 2U);
  EXPECT_EQ(segments[0].track_id, 10ULL);
  EXPECT_EQ(segments[0].particle_type, GGEMSParticleType::Gamma);
  EXPECT_EQ(segments[1].track_id, 11ULL);
  EXPECT_EQ(segments[1].parent_track_id, 10ULL);
  EXPECT_EQ(segments[1].particle_type, GGEMSParticleType::Electron);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, DifferentSourceProvenanceIsNotConnected) {
  std::vector<GGEMSObserverRecord> different_source_index{
      MakeRecord(0ULL, 7ULL, 42ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL, 0U, 4ULL),
      MakeRecord(0ULL, 7ULL, 42ULL, 10ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL, 2U,
                 4ULL)};

  EXPECT_TRUE(ggems::render::BuildParticleTraceSegments(different_source_index)
                  .empty());

  std::vector<GGEMSObserverRecord> different_source_local_primary_id{
      MakeRecord(0ULL, 7ULL, 42ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL, 2U, 0ULL),
      MakeRecord(0ULL, 7ULL, 42ULL, 10ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL, 2U,
                 1ULL)};

  EXPECT_TRUE(ggems::render::BuildParticleTraceSegments(
                  different_source_local_primary_id)
                  .empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, ZeroLengthTerminalRecordDoesNotCreateSegment) {
  std::vector<GGEMSObserverRecord> records{
      MakeRecord(0ULL, 2ULL, 20ULL, 0ULL, GGEMSObserverRecordKind::Source,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 0LL),
      MakeRecord(0ULL, 2ULL, 20ULL, 10ULL, GGEMSObserverRecordKind::Step,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL),
      MakeRecord(0ULL, 2ULL, 20ULL, 10ULL, GGEMSObserverRecordKind::Terminal,
                 GGEMSParticleType::Gamma, 0LL, 0LL, 100'000'000'000LL)};

  std::vector<ggems::render::GGEMSParticleTraceSegment> segments =
      ggems::render::BuildParticleTraceSegments(records);

  ASSERT_EQ(segments.size(), 1U);
  EXPECT_EQ(segments[0].begin_kind, GGEMSObserverRecordKind::Source);
  EXPECT_EQ(segments[0].end_kind, GGEMSObserverRecordKind::Step);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, DefaultSegmentHasInvalidSourceProvenance) {
  ggems::render::GGEMSParticleTraceSegment segment{};

  EXPECT_EQ(segment.source_index, k_invalid_id_u32);
  EXPECT_EQ(segment.source_local_primary_id, k_invalid_id_u64);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTrace, SegmentsBuildLineVertices) {
  ggems::render::GGEMSParticleTraceSegment segment{};
  segment.particle_type = GGEMSParticleType::Gamma;
  segment.begin = ggems::render::GGEMSParticleTracePoint{
      .x_m = 1.0F, .y_m = 2.0F, .z_m = 3.0F};
  segment.end = ggems::render::GGEMSParticleTracePoint{
      .x_m = 4.0F, .y_m = 5.0F, .z_m = 6.0F};

  std::vector<ggems::render::GGEMSParticleTraceVertex> const vertices =
      ggems::render::BuildParticleTraceVertices(
          std::span<ggems::render::GGEMSParticleTraceSegment const>{&segment,
                                                                    1U});

  ASSERT_EQ(vertices.size(), 2U);

  EXPECT_FLOAT_EQ(vertices[0].position[0], 1.0F);
  EXPECT_FLOAT_EQ(vertices[0].position[1], 2.0F);
  EXPECT_FLOAT_EQ(vertices[0].position[2], 3.0F);

  EXPECT_FLOAT_EQ(vertices[1].position[0], 4.0F);
  EXPECT_FLOAT_EQ(vertices[1].position[1], 5.0F);
  EXPECT_FLOAT_EQ(vertices[1].position[2], 6.0F);

  EXPECT_GT(vertices[0].colour[1], vertices[0].colour[0]);
  EXPECT_FLOAT_EQ(vertices[0].colour[3], 1.0F);
  EXPECT_FLOAT_EQ(vertices[0].colour[0], vertices[1].colour[0]);
  EXPECT_FLOAT_EQ(vertices[0].colour[1], vertices[1].colour[1]);
  EXPECT_FLOAT_EQ(vertices[0].colour[2], vertices[1].colour[2]);
}
} // namespace
