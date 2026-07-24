#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

namespace {

using GGEMSSourcePtr = std::shared_ptr<ggems::core::sources::GGEMSSource>;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSource(std::uint64_t primary_count) -> GGEMSSourcePtr {
  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();
  source->SetPrimaryCount(primary_count);
  return source;
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

auto ExpectSourceRange(ggems::core::sources::GGEMSSourceRunRange const &range,
                       std::uint64_t expected_begin,
                       std::uint64_t expected_count) -> void {
  EXPECT_EQ(range.projection_primary_begin, expected_begin);
  EXPECT_EQ(range.primary_count, expected_count);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, BuildsExpectedMonoSourceSnapshot) {
  constexpr std::uint64_t k_primary_count{17ULL};

  ggems::core::sources::GGEMSSource source{};
  source.SetPrimaryCount(k_primary_count)
      .SetAnalytic()
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetTimeWindowPicoSecond(12ULL, 34ULL)
      .SetPositionPicoMeter(-11LL, 22LL, -33LL)
      .SetDirection(0.0F, -4.0F, 0.0F)
      .SetWeight(0.25F);

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);

  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();

  ASSERT_EQ(records.size(), 1U);
  ASSERT_EQ(ranges.size(), 1U);
  EXPECT_EQ(records.size(), ranges.size());

  EXPECT_EQ(ranges[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(ranges[0U].primary_count, k_primary_count);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), k_primary_count);

  auto const &record = records[0U];

  EXPECT_EQ(record.source_id, 0ULL);
  EXPECT_EQ(record.time_start_ps, 12ULL);
  EXPECT_EQ(record.time_stop_ps, 34ULL);
  EXPECT_EQ(record.energy_milli_eV, 222'000'000ULL);

  EXPECT_EQ(record.position_x_pm, -11LL);
  EXPECT_EQ(record.position_y_pm, 22LL);
  EXPECT_EQ(record.position_z_pm, -33LL);

  EXPECT_EQ(record.source_type,
            ggems::core::sources::ToKernelSourceType(
                ggems::core::sources::GGEMSSourceType::Analytic));
  EXPECT_EQ(record.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Electron));
  EXPECT_EQ(record.flags, 0U);
  EXPECT_EQ(record.reserved_0, 0U);

  EXPECT_FLOAT_EQ(record.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_y, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_y_z, 1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(record.axis_z_y, -1.0F);
  EXPECT_FLOAT_EQ(record.axis_z_z, 0.0F);

  EXPECT_FLOAT_EQ(record.weight, 0.25F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, OwnsIndependentSourceState) {
  ggems::core::sources::GGEMSSource source{};

  source.SetPrimaryCount(3ULL)
      .SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(111'000'000ULL)
      .SetTimeWindowPicoSecond(10ULL, 20ULL)
      .SetPositionPicoMeter(11LL, -22LL, 33LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.125F);

  auto expected_a = source.BuildRecord();
  auto snapshot_a = ggems::core::sources::BuildSourceRunSnapshot(source);

  source.SetPrimaryCount(5ULL)
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetTimeWindowPicoSecond(30ULL, 40ULL)
      .SetPositionPicoMeter(-44LL, 55LL, -66LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.875F);

  auto expected_b = source.BuildRecord();
  auto snapshot_b = ggems::core::sources::BuildSourceRunSnapshot(source);

  ASSERT_EQ(snapshot_a.GetRecords().size(), 1U);
  ASSERT_EQ(snapshot_a.GetRanges().size(), 1U);
  ASSERT_EQ(snapshot_b.GetRecords().size(), 1U);
  ASSERT_EQ(snapshot_b.GetRanges().size(), 1U);

  EXPECT_EQ(snapshot_a.GetTotalPrimaryCount(), 3ULL);
  EXPECT_EQ(snapshot_a.GetRanges()[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot_a.GetRanges()[0U].primary_count, 3ULL);
  ExpectSourceRecordsEqual(snapshot_a.GetRecords()[0U], expected_a);

  EXPECT_EQ(snapshot_b.GetTotalPrimaryCount(), 5ULL);
  EXPECT_EQ(snapshot_b.GetRanges()[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot_b.GetRanges()[0U].primary_count, 5ULL);
  ExpectSourceRecordsEqual(snapshot_b.GetRecords()[0U], expected_b);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, RepresentsDisabledMonoSource) {
  ggems::core::sources::GGEMSSource source{};
  source.SetPrimaryCount(0ULL);

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);

  ASSERT_EQ(snapshot.GetRecords().size(), 1U);
  ASSERT_EQ(snapshot.GetRanges().size(), 1U);
  EXPECT_EQ(snapshot.GetRecords().size(), snapshot.GetRanges().size());

  EXPECT_EQ(snapshot.GetRanges()[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot.GetRanges()[0U].primary_count, 0ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, RangeAndTotalAreConsistent) {
  constexpr std::uint64_t k_primary_count{37ULL};

  ggems::core::sources::GGEMSSource source{};
  source.SetPrimaryCount(k_primary_count);

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);

  EXPECT_EQ(snapshot.GetRecords().size(), snapshot.GetRanges().size());
  ASSERT_EQ(snapshot.GetRanges().size(), 1U);

  auto const &range = snapshot.GetRanges()[0U];

  EXPECT_EQ(range.projection_primary_begin, 0ULL);
  EXPECT_EQ(range.primary_count, snapshot.GetTotalPrimaryCount());
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), k_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, BuildsOrderedMultiSourceSnapshot) {
  auto source_0 = MakeSource(3ULL);
  source_0->SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(101'000'000ULL)
      .SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  auto source_1 = MakeSource(5ULL);
  source_1->SetAnalytic()
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(202'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, 60LL)
      .SetDirection(0.0F, 1.0F, 0.0F)
      .SetWeight(0.50F);

  auto source_2 = MakeSource(2ULL);
  source_2->SetAnalytic()
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Positron)
      .SetEnergyMilliElectronVolt(303'000'000ULL)
      .SetPositionPicoMeter(70LL, -80LL, 90LL)
      .SetDirection(0.0F, 0.0F, -1.0F)
      .SetWeight(0.75F);

  std::vector<GGEMSSourcePtr> sources{source_0, source_1, source_2};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();

  ASSERT_EQ(records.size(), 3U);
  ASSERT_EQ(ranges.size(), 3U);
  EXPECT_EQ(records.size(), ranges.size());

  ExpectSourceRange(ranges[0U], 0ULL, 3ULL);
  ExpectSourceRange(ranges[1U], 3ULL, 5ULL);
  ExpectSourceRange(ranges[2U], 8ULL, 2ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 10ULL);

  ExpectSourceRecordsEqual(records[0U], source_0->BuildRecord());
  ExpectSourceRecordsEqual(records[1U], source_1->BuildRecord());
  ExpectSourceRecordsEqual(records[2U], source_2->BuildRecord());

  EXPECT_EQ(records[0U].energy_milli_eV, 101'000'000ULL);
  EXPECT_EQ(records[1U].position_x_pm, -40LL);
  EXPECT_EQ(records[2U].emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Positron));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, PreservesZeroPrimarySourceSlots) {
  std::vector<GGEMSSourcePtr> sources{MakeSource(3ULL), MakeSource(0ULL),
                                      MakeSource(5ULL), MakeSource(0ULL),
                                      MakeSource(2ULL)};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();

  ASSERT_EQ(records.size(), 5U);
  ASSERT_EQ(ranges.size(), 5U);
  EXPECT_EQ(records.size(), ranges.size());

  constexpr std::array<std::uint64_t, 5U> k_expected_begins{0ULL, 3ULL, 3ULL,
                                                            8ULL, 8ULL};
  constexpr std::array<std::uint64_t, 5U> k_expected_counts{3ULL, 0ULL, 5ULL,
                                                            0ULL, 2ULL};

  for (std::size_t i = 0U; i < ranges.size(); ++i) {
    SCOPED_TRACE(i);
    ExpectSourceRange(ranges[i], k_expected_begins[i], k_expected_counts[i]);
  }

  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 10ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, BuildsEmptySnapshot) {
  std::vector<GGEMSSourcePtr> const sources{};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  EXPECT_TRUE(snapshot.GetRecords().empty());
  EXPECT_TRUE(snapshot.GetRanges().empty());
  EXPECT_EQ(snapshot.GetRecords().size(), snapshot.GetRanges().size());
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, OwnsIndependentMultiSourceState) {
  auto source_0 = MakeSource(2ULL);
  source_0->SetEnergyMilliElectronVolt(111'000'000ULL)
      .SetPositionPicoMeter(1LL, 2LL, 3LL)
      .SetDirection(1.0F, 0.0F, 0.0F);

  auto source_1 = MakeSource(4ULL);
  source_1->SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetPositionPicoMeter(4LL, 5LL, 6LL)
      .SetDirection(0.0F, 1.0F, 0.0F);

  std::vector<GGEMSSourcePtr> sources{source_0, source_1};

  auto expected_a_0 = source_0->BuildRecord();
  auto expected_a_1 = source_1->BuildRecord();
  auto snapshot_a = ggems::core::sources::BuildSourceRunSnapshot(sources);

  source_0->SetPrimaryCount(5ULL)
      .SetEnergyMilliElectronVolt(333'000'000ULL)
      .SetPositionPicoMeter(-1LL, -2LL, -3LL)
      .SetDirection(0.0F, 1.0F, 0.0F);

  source_1->SetPrimaryCount(3ULL)
      .SetEnergyMilliElectronVolt(444'000'000ULL)
      .SetPositionPicoMeter(-4LL, -5LL, -6LL)
      .SetDirection(-1.0F, 0.0F, 0.0F);

  auto expected_b_0 = source_0->BuildRecord();
  auto expected_b_1 = source_1->BuildRecord();
  auto snapshot_b = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(snapshot_a.GetRecords().size(), 2U);
  ASSERT_EQ(snapshot_a.GetRanges().size(), 2U);
  ASSERT_EQ(snapshot_b.GetRecords().size(), 2U);
  ASSERT_EQ(snapshot_b.GetRanges().size(), 2U);

  ExpectSourceRange(snapshot_a.GetRanges()[0U], 0ULL, 2ULL);
  ExpectSourceRange(snapshot_a.GetRanges()[1U], 2ULL, 4ULL);
  EXPECT_EQ(snapshot_a.GetTotalPrimaryCount(), 6ULL);
  ExpectSourceRecordsEqual(snapshot_a.GetRecords()[0U], expected_a_0);
  ExpectSourceRecordsEqual(snapshot_a.GetRecords()[1U], expected_a_1);

  ExpectSourceRange(snapshot_b.GetRanges()[0U], 0ULL, 5ULL);
  ExpectSourceRange(snapshot_b.GetRanges()[1U], 5ULL, 3ULL);
  EXPECT_EQ(snapshot_b.GetTotalPrimaryCount(), 8ULL);
  ExpectSourceRecordsEqual(snapshot_b.GetRecords()[0U], expected_b_0);
  ExpectSourceRecordsEqual(snapshot_b.GetRecords()[1U], expected_b_1);

  EXPECT_NE(snapshot_a.GetRecords()[0U].energy_milli_eV,
            snapshot_b.GetRecords()[0U].energy_milli_eV);
  EXPECT_NE(snapshot_a.GetRecords()[1U].position_x_pm,
            snapshot_b.GetRecords()[1U].position_x_pm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, RejectsNullSource) {
  auto source_0 = MakeSource(2ULL);
  auto source_2 = MakeSource(3ULL);

  std::vector<GGEMSSourcePtr> sources{source_0, GGEMSSourcePtr{}, source_2};

  bool exception_caught = false;

  try {
    static_cast<void>(ggems::core::sources::BuildSourceRunSnapshot(sources));
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    exception_caught = true;

    std::string_view const diagnostic{exception.what()};
    EXPECT_NE(diagnostic.find("index 1"), std::string_view::npos);
  }

  EXPECT_TRUE(exception_caught);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, AcceptsDuplicateSourceSlots) {
  auto source = MakeSource(7ULL);
  source->SetEnergyMilliElectronVolt(123'000'000ULL)
      .SetPositionPicoMeter(11LL, 22LL, 33LL)
      .SetDirection(0.0F, 0.0F, 1.0F);

  std::vector<GGEMSSourcePtr> sources{source, source};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();

  ASSERT_EQ(records.size(), 2U);
  ASSERT_EQ(ranges.size(), 2U);
  EXPECT_EQ(records.size(), ranges.size());

  ExpectSourceRange(ranges[0U], 0ULL, 7ULL);
  ExpectSourceRange(ranges[1U], 7ULL, 7ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 14ULL);

  EXPECT_NE(&records[0U], &records[1U]);
  ExpectSourceRecordsEqual(records[0U], source->BuildRecord());
  ExpectSourceRecordsEqual(records[1U], source->BuildRecord());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, AcceptsMaximumRepresentableTotal) {
  constexpr std::uint64_t k_maximum = std::numeric_limits<std::uint64_t>::max();

  auto source_0 = MakeSource(k_maximum - 4ULL);
  auto source_1 = MakeSource(4ULL);
  std::vector<GGEMSSourcePtr> sources{source_0, source_1};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(snapshot.GetRecords().size(), 2U);
  ASSERT_EQ(snapshot.GetRanges().size(), 2U);

  ExpectSourceRange(snapshot.GetRanges()[0U], 0ULL, k_maximum - 4ULL);
  ExpectSourceRange(snapshot.GetRanges()[1U], k_maximum - 4ULL, 4ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), k_maximum);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, RejectsOverflowingTotal) {
  constexpr std::uint64_t k_maximum = std::numeric_limits<std::uint64_t>::max();

  auto source_0 = MakeSource(k_maximum);
  source_0->SetEnergyMilliElectronVolt(111'000'000ULL)
      .SetPositionPicoMeter(1LL, 2LL, 3LL);

  auto source_1 = MakeSource(1ULL);
  source_1->SetEnergyMilliElectronVolt(222'000'000ULL)
      .SetPositionPicoMeter(4LL, 5LL, 6LL);

  auto expected_0 = source_0->BuildRecord();
  auto expected_1 = source_1->BuildRecord();
  std::vector<GGEMSSourcePtr> sources{source_0, source_1};

  bool exception_caught = false;

  try {
    static_cast<void>(ggems::core::sources::BuildSourceRunSnapshot(sources));
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    exception_caught = true;

    std::string_view diagnostic{exception.what()};
    EXPECT_NE(diagnostic.find("overflow"), std::string_view::npos);
    EXPECT_NE(diagnostic.find("index 1"), std::string_view::npos);
  }

  EXPECT_TRUE(exception_caught);
  EXPECT_EQ(source_0->GetPrimaryCount(), k_maximum);
  EXPECT_EQ(source_1->GetPrimaryCount(), 1ULL);
  ExpectSourceRecordsEqual(source_0->BuildRecord(), expected_0);
  ExpectSourceRecordsEqual(source_1->BuildRecord(), expected_1);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, SingleSourceOverloadMatchesCollectionOverload) {
  auto source = MakeSource(19ULL);
  source->SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Proton)
      .SetEnergyMilliElectronVolt(555'000'000ULL)
      .SetTimeWindowPicoSecond(50ULL, 75ULL)
      .SetPositionPicoMeter(-10LL, 20LL, -30LL)
      .SetDirection(1.0F, -1.0F, 0.0F)
      .SetWeight(0.625F);

  std::vector<GGEMSSourcePtr> sources{source};

  auto mono_snapshot = ggems::core::sources::BuildSourceRunSnapshot(*source);
  auto generic_snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(mono_snapshot.GetRecords().size(),
            generic_snapshot.GetRecords().size());
  ASSERT_EQ(mono_snapshot.GetRanges().size(),
            generic_snapshot.GetRanges().size());
  ASSERT_EQ(mono_snapshot.GetRecords().size(), 1U);
  ASSERT_EQ(mono_snapshot.GetRanges().size(), 1U);

  ExpectSourceRecordsEqual(mono_snapshot.GetRecords()[0U],
                           generic_snapshot.GetRecords()[0U]);
  ExpectSourceRange(generic_snapshot.GetRanges()[0U],
                    mono_snapshot.GetRanges()[0U].projection_primary_begin,
                    mono_snapshot.GetRanges()[0U].primary_count);
  EXPECT_EQ(mono_snapshot.GetTotalPrimaryCount(),
            generic_snapshot.GetTotalPrimaryCount());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, PreservesZeroPrimaryAndDuplicateSlotsTogether) {
  auto active_source = MakeSource(3ULL);
  active_source->SetEnergyMilliElectronVolt(123'000'000ULL)
      .SetPositionPicoMeter(11LL, 22LL, 33LL)
      .SetDirection(0.0F, 0.0F, 1.0F);

  auto disabled_source = MakeSource(0ULL);
  disabled_source->SetEnergyMilliElectronVolt(456'000'000ULL)
      .SetPositionPicoMeter(-11LL, -22LL, -33LL)
      .SetDirection(0.0F, 1.0F, 0.0F);

  std::vector<GGEMSSourcePtr> sources{active_source, disabled_source,
                                      active_source};

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(snapshot.GetRecords().size(), 3U);
  ASSERT_EQ(snapshot.GetRanges().size(), 3U);

  ExpectSourceRange(snapshot.GetRanges()[0U], 0ULL, 3ULL);
  ExpectSourceRange(snapshot.GetRanges()[1U], 3ULL, 0ULL);
  ExpectSourceRange(snapshot.GetRanges()[2U], 3ULL, 3ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 6ULL);

  ExpectSourceRecordsEqual(snapshot.GetRecords()[0U],
                           active_source->BuildRecord());
  ExpectSourceRecordsEqual(snapshot.GetRecords()[1U],
                           disabled_source->BuildRecord());
  ExpectSourceRecordsEqual(snapshot.GetRecords()[2U],
                           active_source->BuildRecord());
  EXPECT_NE(&snapshot.GetRecords()[0U], &snapshot.GetRecords()[2U]);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, OwnsIndependentGeometryAndAngularConfiguration) {
  auto source = MakeSource(3ULL);
  source->SetRectangleEmissionPicoMeter(40ULL, 20ULL)
      .SetIsotropicAngularDistribution();

  std::vector<GGEMSSourcePtr> sources{source, source};
  auto first = ggems::core::sources::BuildSourceRunSnapshot(sources);

  source->SetEllipseEmissionPicoMeter(30ULL, 10ULL)
      .SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 100LL);

  auto second = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(first.GetRecords().size(), 2U);
  ASSERT_EQ(second.GetRecords().size(), 2U);

  EXPECT_EQ(ggems::core::sources::FromKernelEmissionGeometryType(
                first.GetRecords()[0U].emission_geometry_type),
            ggems::core::sources::GGEMSEmissionGeometryType::Rectangle);
  EXPECT_EQ(ggems::core::sources::FromKernelAngularDistributionType(
                first.GetRecords()[0U].angular_distribution_type),
            ggems::core::sources::GGEMSAngularDistributionType::Isotropic);

  EXPECT_EQ(ggems::core::sources::FromKernelEmissionGeometryType(
                second.GetRecords()[0U].emission_geometry_type),
            ggems::core::sources::GGEMSEmissionGeometryType::Ellipse);
  EXPECT_EQ(ggems::core::sources::FromKernelAngularDistributionType(
                second.GetRecords()[0U].angular_distribution_type),
            ggems::core::sources::GGEMSAngularDistributionType::Focused);

  ExpectSourceRecordsEqual(first.GetRecords()[0U], first.GetRecords()[1U]);
  ExpectSourceRecordsEqual(second.GetRecords()[0U], second.GetRecords()[1U]);
}
