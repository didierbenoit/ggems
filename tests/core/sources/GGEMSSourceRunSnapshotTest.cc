#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace {

using GGEMSSourcePtr = std::shared_ptr<ggems::core::sources::GGEMSSource>;

template <typename T>
concept HasEnergyTableReservation =
    requires(T value) { value.ReserveEnergyTableCapacity(std::size_t{1U}); };

static_assert(!HasEnergyTableReservation<ggems::core::sources::GGEMSSource>);

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

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot,
     OwnsPackedEnergyTablesWithZeroPrimaryAndDuplicateSlots) {
  constexpr std::array<double, 3U> k_discrete_energies{1.0, 2.0, 3.0};
  constexpr std::array<double, 3U> k_discrete_weights{1.0, 0.0, 3.0};
  constexpr std::array<double, 3U> k_regular_centers{10.0, 12.0, 14.0};
  constexpr std::array<double, 3U> k_regular_weights{0.0, 1.0, 1.0};

  auto mono = MakeSource(1ULL);
  mono->SetEnergyMilliElectronVolt(511'000'000ULL);

  auto discrete = MakeSource(0ULL);
  discrete->SetDiscreteEnergyLines(k_discrete_energies, k_discrete_weights,
                                   "MeV");

  auto regular = MakeSource(2ULL);
  regular->SetRegularEnergySpectrum(k_regular_centers, k_regular_weights,
                                    "MeV");

  std::vector<GGEMSSourcePtr> sources{mono, discrete, regular, discrete};
  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();
  auto const &energy_records = snapshot.GetEnergyDistributionRecords();
  auto const &values = snapshot.GetEnergyValuesMilliElectronVolt();
  auto const &relative_weights = snapshot.GetRelativeWeights();
  auto const &ticket_bounds = snapshot.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(records.size(), 4U);
  ASSERT_EQ(ranges.size(), 4U);
  ASSERT_EQ(energy_records.size(), 4U);

  ExpectSourceRange(ranges[0U], 0ULL, 1ULL);
  ExpectSourceRange(ranges[1U], 1ULL, 0ULL);
  ExpectSourceRange(ranges[2U], 1ULL, 2ULL);
  ExpectSourceRange(ranges[3U], 3ULL, 0ULL);
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 3ULL);

  EXPECT_EQ(records[0U].energy_milli_eV, 511'000'000ULL);
  EXPECT_EQ(records[1U].energy_milli_eV, 0ULL);
  EXPECT_EQ(records[2U].energy_milli_eV, 0ULL);
  EXPECT_EQ(records[3U].energy_milli_eV, 0ULL);

  EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                energy_records[0U].distribution_type),
            ggems::core::sources::GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(energy_records[0U].table_offset, 0ULL);
  EXPECT_EQ(energy_records[0U].table_count, 0U);

  EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                energy_records[1U].distribution_type),
            ggems::core::sources::GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_EQ(energy_records[1U].table_offset, 0ULL);
  EXPECT_EQ(energy_records[1U].table_count, 3U);

  EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                energy_records[2U].distribution_type),
            ggems::core::sources::GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(energy_records[2U].table_offset, 3ULL);
  EXPECT_EQ(energy_records[2U].table_count, 3U);
  EXPECT_EQ(energy_records[2U].regular_bin_width_milli_eV, 2'000'000'000ULL);

  EXPECT_EQ(energy_records[3U].table_offset, 6ULL);
  EXPECT_EQ(energy_records[3U].table_count, 3U);

  std::vector<std::uint64_t> const expected_values{
      1'000'000'000ULL,  2'000'000'000ULL,  3'000'000'000ULL,
      10'000'000'000ULL, 12'000'000'000ULL, 14'000'000'000ULL,
      1'000'000'000ULL,  2'000'000'000ULL,  3'000'000'000ULL};
  std::vector<double> const expected_relative_weights{1.0, 0.0, 3.0, 0.0, 1.0,
                                                      1.0, 1.0, 0.0, 3.0};
  std::vector<std::uint64_t> const expected_ticket_bounds{
      1'073'741'824ULL, 1'073'741'824ULL, 4'294'967'296ULL, 0ULL,
      2'147'483'648ULL, 4'294'967'296ULL, 1'073'741'824ULL, 1'073'741'824ULL,
      4'294'967'296ULL};

  EXPECT_EQ(values, expected_values);
  EXPECT_EQ(relative_weights, expected_relative_weights);
  EXPECT_EQ(ticket_bounds, expected_ticket_bounds);

  discrete->SetEnergyMilliElectronVolt(99'000'000ULL);
  regular->SetEnergyMilliElectronVolt(88'000'000ULL);

  EXPECT_EQ(snapshot.GetEnergyValuesMilliElectronVolt(), expected_values);
  EXPECT_EQ(snapshot.GetRelativeWeights(), expected_relative_weights);
  EXPECT_EQ(snapshot.GetCumulativeTicketUpperBounds(), expected_ticket_bounds);
  EXPECT_EQ(snapshot.GetEnergyDistributionRecords()[1U].table_offset, 0ULL);
  EXPECT_EQ(snapshot.GetEnergyDistributionRecords()[3U].table_offset, 6ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, AllMonoSourcesUseEmptyPackedEnergyTables) {
  auto source_0 = MakeSource(1ULL);
  auto source_1 = MakeSource(0ULL);
  auto source_2 = MakeSource(2ULL);

  std::vector<GGEMSSourcePtr> sources{source_0, source_1, source_2};
  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(sources);

  ASSERT_EQ(snapshot.GetEnergyDistributionRecords().size(), 3U);
  EXPECT_TRUE(snapshot.GetEnergyValuesMilliElectronVolt().empty());
  EXPECT_TRUE(snapshot.GetRelativeWeights().empty());
  EXPECT_TRUE(snapshot.GetCumulativeTicketUpperBounds().empty());

  for (auto const &record : snapshot.GetEnergyDistributionRecords()) {
    EXPECT_EQ(ggems::core::sources::FromKernelEnergyDistributionType(
                  record.distribution_type),
              ggems::core::sources::GGEMSEnergyDistributionType::Mono);
    EXPECT_EQ(record.table_offset, 0ULL);
    EXPECT_EQ(record.table_count, 0U);
    EXPECT_EQ(record.regular_bin_width_milli_eV, 0ULL);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot,
     PacksOneHundredThousandRegularBinsWithoutReservation) {
  constexpr std::size_t k_bin_count{100'000U};
  std::vector<double> bin_centers(k_bin_count);
  std::vector<double> relative_weights(k_bin_count, 1.0);

  for (std::size_t index = 0U; index < k_bin_count; ++index) {
    bin_centers[index] = 1'000'000.0 + 2.0 * static_cast<double>(index);
  }

  ggems::core::sources::GGEMSSource source{};
  source.SetRegularEnergySpectrum(bin_centers, relative_weights, "meV");

  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);
  auto const &energy_records = snapshot.GetEnergyDistributionRecords();
  auto const &values = snapshot.GetEnergyValuesMilliElectronVolt();
  auto const &weights = snapshot.GetRelativeWeights();
  auto const &ticket_bounds = snapshot.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(energy_records.size(), 1U);
  EXPECT_EQ(energy_records[0U].table_offset, 0ULL);
  EXPECT_EQ(energy_records[0U].table_count, k_bin_count);
  EXPECT_EQ(values.size(), k_bin_count);
  EXPECT_EQ(weights.size(), k_bin_count);
  ASSERT_EQ(ticket_bounds.size(), k_bin_count);
  EXPECT_EQ(ticket_bounds.back(),
            ggems::core::sources::k_energy_ticket_space_size);
  EXPECT_TRUE(std::ranges::all_of(
      weights, [](double weight) -> bool { return weight == 1.0; }));
  EXPECT_TRUE(std::ranges::is_sorted(ticket_bounds));
  EXPECT_EQ(std::ranges::adjacent_find(ticket_bounds), ticket_bounds.end());
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunSnapshot, OwnsVolumeAndBoundedAngularRecordFields) {
  constexpr long double k_pi{std::numbers::pi_v<long double>};
  ggems::core::sources::GGEMSSource source{};
  source.SetPrimaryCount(3ULL)
      .SetBoxEmissionPicoMeter(40ULL, 20ULL, 10ULL)
      .SetIsotropicAngularDistribution(ggems::units::MakeRadians(0.25L * k_pi),
                                       ggems::units::MakeRadians(0.5L * k_pi),
                                       ggems::units::MakeRadians(-0.25L * k_pi),
                                       ggems::units::MakeRadians(0.25L * k_pi));

  auto const expected = source.BuildRecord();
  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);

  source.SetCylinderEmissionPicoMeter(12ULL, 30ULL)
      .SetFixedAngularDistribution();

  ASSERT_EQ(snapshot.GetRecords().size(), 1U);
  ExpectSourceRecordsEqual(snapshot.GetRecords().front(), expected);
  EXPECT_EQ(snapshot.GetRecords().front().geometry_size_z_pm, 10ULL);
  EXPECT_FLOAT_EQ(snapshot.GetRecords().front().isotropic_phi_min_rad,
                  static_cast<float>(-0.25L * k_pi));
}
