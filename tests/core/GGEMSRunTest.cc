#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace {

constexpr std::uint32_t k_source_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Source);

// =============================================================================
// =============================================================================

auto IsSourceRecord(
    ggems::core::observer::GGEMSObserverRecord const &record) noexcept -> bool {
  return record.record_kind == k_source_record_kind;
}

// =============================================================================
// =============================================================================

auto BuildSortedSourceRecordSnapshot(
    std::vector<ggems::core::observer::GGEMSObserverRecord> const &records)
    -> std::vector<ggems::core::observer::GGEMSObserverRecord> {
  std::vector<ggems::core::observer::GGEMSObserverRecord> source_records;

  for (auto const &record : records) {
    if (IsSourceRecord(record)) {
      source_records.push_back(record);
    }
  }

  std::ranges::sort(
      source_records,
      [](ggems::core::observer::GGEMSObserverRecord const &lhs,
         ggems::core::observer::GGEMSObserverRecord const &rhs) -> bool {
        if (lhs.run_id != rhs.run_id) {
          return lhs.run_id < rhs.run_id;
        }

        return lhs.global_primary_id < rhs.global_primary_id;
      });

  return source_records;
}

// =============================================================================
// =============================================================================

auto MakePhiloxRandom() -> std::shared_ptr<ggems::core::random::GGEMSRandom> {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(9'876'543ULL);
  return random;
}

// =============================================================================
// =============================================================================

auto MakeLowEnergySource(std::uint64_t primary_count)
    -> std::shared_ptr<ggems::core::sources::GGEMSSource> {
  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();
  source->SetPrimaryCount(primary_count)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(100ULL, 100ULL);
  return source;
}

// =============================================================================
// =============================================================================

auto MakeCapturingObserver(std::uint32_t primary_count)
    -> std::shared_ptr<ggems::core::observer::GGEMSTransportObserver> {
  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->SetRecordCapacity(64U).CaptureFirstPrimaries(primary_count);
  return observer;
}

// =============================================================================
// =============================================================================

auto ExpectObserverSourceMatches(
    ggems::core::observer::GGEMSObserverRecord const &observed,
    ggems::core::sources::GGEMSSourceRecord const &expected) -> void {
  EXPECT_EQ(observed.record_kind, k_source_record_kind);
  EXPECT_EQ(observed.particle_type, expected.emitted_particle_type);
  EXPECT_EQ(observed.time_ps, expected.time_start_ps);

  EXPECT_EQ(observed.position_x_pm, expected.position_x_pm);
  EXPECT_EQ(observed.position_y_pm, expected.position_y_pm);
  EXPECT_EQ(observed.position_z_pm, expected.position_z_pm);

  EXPECT_FLOAT_EQ(observed.direction_x, expected.direction_x);
  EXPECT_FLOAT_EQ(observed.direction_y, expected.direction_y);
  EXPECT_FLOAT_EQ(observed.direction_z, expected.direction_z);
  EXPECT_FLOAT_EQ(observed.direction_w, expected.direction_w);

  EXPECT_EQ(observed.energy_milli_eV, expected.energy_milli_eV);
  EXPECT_EQ(observed.deposited_energy_milli_eV, 0ULL);
  EXPECT_FLOAT_EQ(observed.weight, expected.weight);
}

// =============================================================================
// =============================================================================

template <typename Function>
auto ExpectGGEMSExceptionContaining(Function &&function,
                                    std::string_view expected_message) -> void {
  bool exception_caught = false;

  try {
    std::forward<Function>(function)();
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    exception_caught = true;

    std::string_view diagnostic{exception.what()};
    EXPECT_NE(diagnostic.find(expected_message), std::string_view::npos);
  }

  EXPECT_TRUE(exception_caught);
}

// =============================================================================
// =============================================================================

class GGEMSRunTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }
};
} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRun, ReportsObserverAttachment) {
  ggems::core::GGEMSRun run{};

  EXPECT_FALSE(run.HasObserver());

  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();

  run.SetObserver(observer);

  EXPECT_TRUE(run.HasObserver());
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RejectsSecondInitialiseAndRemainsUsable) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetPrimaryCount(1U);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  EXPECT_THROW(run.Initialise(), ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(run.Run());
  EXPECT_NO_THROW(run.Run());
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, UsesIndependentSourceSnapshotsAcrossSequentialRuns) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();

  source->SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(100ULL, 100ULL)
      .SetPositionPicoMeter(10LL, -20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  auto expected_a = source->BuildRecord();

  ASSERT_EQ(expected_a.time_start_ps, expected_a.time_stop_ps);

  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->CaptureFirstPrimaries(1U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetPrimaryCount(1U);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  auto const &records_after_first_run = observer->GetRecords();

  auto first_source =
      std::ranges::find_if(records_after_first_run, IsSourceRecord);

  ASSERT_NE(first_source, records_after_first_run.end());
  ASSERT_EQ(std::count_if(records_after_first_run.begin(),
                          records_after_first_run.end(), IsSourceRecord),
            1);

  auto first_source_record = *first_source;
  ExpectObserverSourceMatches(first_source_record, expected_a);

  std::uint32_t const first_record_count = observer->GetRecordCount();

  EXPECT_GT(first_record_count, 0U);
  EXPECT_EQ(first_record_count, records_after_first_run.size());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);

  source
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetTimeWindowPicoSecond(200ULL, 200ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_b = source->BuildRecord();

  ASSERT_EQ(expected_b.time_start_ps, expected_b.time_stop_ps);

  ASSERT_NO_THROW(run.Run());

  auto const &records_after_second_run = observer->GetRecords();

  ASSERT_FALSE(records_after_second_run.empty());
  EXPECT_EQ(observer->GetRecordCount(), records_after_second_run.size());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);

  ASSERT_EQ(std::count_if(records_after_second_run.begin(),
                          records_after_second_run.end(), IsSourceRecord),
            1);

  auto second_source = std::ranges::find_if(
      records_after_second_run,
      [first_run_id = first_source_record.run_id](
          ggems::core::observer::GGEMSObserverRecord const &record) -> bool {
        return IsSourceRecord(record) && record.run_id != first_run_id;
      });

  ASSERT_NE(second_source, records_after_second_run.end());
  EXPECT_NE(second_source->run_id, first_source_record.run_id);
  EXPECT_EQ(second_source->global_primary_id,
            first_source_record.global_primary_id + 1ULL);

  EXPECT_TRUE(std::ranges::all_of(
      records_after_second_run,
      [second_run_id = second_source->run_id](
          ggems::core::observer::GGEMSObserverRecord const &record) -> bool {
        return record.run_id == second_run_id;
      }));

  ExpectObserverSourceMatches(*second_source, expected_b);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, LegacyPrimaryCountSetterDelegatesToCurrentSource) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(7ULL);
  auto observer = MakeCapturingObserver(3U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetPrimaryCount(3U);
  run.SetWorkerCount(64U);

  EXPECT_EQ(source->GetPrimaryCount(), 3ULL);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 3U);

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, UsesPrimaryCountOwnedByAttachedSource) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(3ULL);
  auto observer = MakeCapturingObserver(16U);

  ggems::core::GGEMSRun run{};
  run.SetPrimaryCount(11U);
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  EXPECT_EQ(source->GetPrimaryCount(), 3ULL);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 3U);

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, ReservesDisjointRangesForVariableSequentialSourceCounts) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(3ULL);
  auto expected_source_record = source->BuildRecord();
  auto observer = MakeCapturingObserver(8U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  auto const first_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(first_source_records.size(), 3U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  std::uint64_t first_run_id = first_source_records.front().run_id;

  for (std::size_t index = 0U; index < first_source_records.size(); ++index) {
    EXPECT_EQ(first_source_records[index].run_id, first_run_id);
    EXPECT_EQ(first_source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
  }

  source->SetPrimaryCount(5ULL);

  ASSERT_NO_THROW(run.Run());

  auto second_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(second_source_records.size(), 5U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 5U);

  std::uint64_t const second_run_id = second_source_records.front().run_id;

  EXPECT_EQ(second_run_id, first_run_id + 1ULL);

  for (std::size_t index = 0U; index < second_source_records.size(); ++index) {
    EXPECT_EQ(second_source_records[index].run_id, second_run_id);
    EXPECT_EQ(second_source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index) + 3ULL);
    ExpectObserverSourceMatches(second_source_records[index],
                                expected_source_record);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RejectsZeroSourcePrimaryCountBeforeReservingRange) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(0ULL);
  auto observer = MakeCapturingObserver(2U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  EXPECT_THROW(run.Run(), ggems::core::GGEMSExceptionBase);
  EXPECT_TRUE(observer->GetRecords().empty());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 0U);

  source->SetPrimaryCount(2ULL);

  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 2U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 2U);
  EXPECT_EQ(source_records[0U].global_primary_id, 0ULL);
  EXPECT_EQ(source_records[1U].global_primary_id, 1ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, LegacyPrimaryCountSetterRejectsZero) {
  ggems::core::GGEMSRun run{};

  EXPECT_THROW(run.SetPrimaryCount(0U), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, FirstAddSourceReplacesImplicitDefault) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(3ULL);

  source->SetPositionPicoMeter(10LL, -20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  auto expected_source_record = source->BuildRecord();
  auto observer = MakeCapturingObserver(3U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 3U);

  std::uint64_t run_id = source_records.front().run_id;

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].run_id, run_id);
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    ExpectObserverSourceMatches(source_records[index], expected_source_record);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, SetSourceReplacesEntireCollection) {
  auto random = MakePhiloxRandom();
  auto source_a = MakeLowEnergySource(2ULL);
  auto source_b = MakeLowEnergySource(4ULL);
  auto replacement = MakeLowEnergySource(3ULL);

  replacement
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_replacement_record = replacement->BuildRecord();
  auto observer = MakeCapturingObserver(3U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_a);
  run.AddSource(source_b);
  run.SetSource(replacement);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 3U);

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    ExpectObserverSourceMatches(source_records[index],
                                expected_replacement_record);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, LegacyPrimaryCountSetterRejectsMultipleSources) {
  auto source_a = MakeLowEnergySource(0ULL);
  auto source_b = MakeLowEnergySource(0ULL);

  ggems::core::GGEMSRun run{};
  run.SetSource(source_a);
  run.AddSource(source_b);

  ExpectGGEMSExceptionContaining(
      [&run]() -> void { run.SetPrimaryCount(5U); },
      "GGEMSRun::SetPrimaryCount is ambiguous with multiple sources.");

  EXPECT_EQ(source_a->GetPrimaryCount(), 0ULL);
  EXPECT_EQ(source_b->GetPrimaryCount(), 0ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RejectsCollectionMutationAfterInitialise) {
  auto random = MakePhiloxRandom();
  auto initial_source = MakeLowEnergySource(1ULL);
  auto replacement = MakeLowEnergySource(2ULL);
  auto additional = MakeLowEnergySource(3ULL);

  initial_source->SetPositionPicoMeter(1LL, 2LL, 3LL)
      .SetDirection(1.0F, 0.0F, 0.0F);

  replacement->SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(4LL, 5LL, 6LL)
      .SetDirection(0.0F, 1.0F, 0.0F);

  additional->SetEnergyMilliElectronVolt(3'000'000ULL)
      .SetPositionPicoMeter(7LL, 8LL, 9LL)
      .SetDirection(0.0F, 0.0F, 1.0F);

  auto expected_initial_record = initial_source->BuildRecord();
  auto observer = MakeCapturingObserver(8U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(initial_source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  EXPECT_THROW(run.SetSource(replacement), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(run.AddSource(additional), ggems::core::GGEMSExceptionBase);

  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 1U);
  EXPECT_EQ(source_records[0U].global_primary_id, 0ULL);
  ExpectObserverSourceMatches(source_records[0U], expected_initial_record);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RunsWithDisabledSlotBetweenActiveSources) {
  auto random = MakePhiloxRandom();
  auto source_0 = MakeLowEnergySource(2ULL);
  auto source_1 = MakeLowEnergySource(0ULL);
  auto source_2 = MakeLowEnergySource(3ULL);

  source_0->SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  source_1->SetEnergyMilliElectronVolt(3'000'000ULL)
      .SetPositionPicoMeter(40LL, 50LL, 60LL);

  source_2
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_0 = source_0->BuildRecord();
  auto expected_2 = source_2->BuildRecord();
  auto observer = MakeCapturingObserver(5U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_0);
  run.AddSource(source_1);
  run.AddSource(source_2);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 5U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 5U);

  std::uint64_t run_id = source_records.front().run_id;

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].run_id, run_id);
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    EXPECT_EQ(source_records[index].source_index, index < 2U ? 0U : 2U);
    EXPECT_EQ(source_records[index].source_local_primary_id,
              index < 2U ? static_cast<std::uint64_t>(index)
                         : static_cast<std::uint64_t>(index - 2U));
    ExpectObserverSourceMatches(source_records[index],
                                index < 2U ? expected_0 : expected_2);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, AlternatesActiveSourceAcrossSequentialRuns) {
  auto random = MakePhiloxRandom();
  auto source_a = MakeLowEnergySource(2ULL);
  auto source_b = MakeLowEnergySource(0ULL);

  source_a->SetPositionPicoMeter(10LL, -20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  source_b
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_a = source_a->BuildRecord();
  auto expected_b = source_b->BuildRecord();
  auto observer = MakeCapturingObserver(3U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_a);
  run.AddSource(source_b);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  auto const first_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(first_source_records.size(), 2U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 2U);

  std::uint64_t const first_run_id = first_source_records.front().run_id;

  for (std::size_t index = 0U; index < first_source_records.size(); ++index) {
    EXPECT_EQ(first_source_records[index].run_id, first_run_id);
    EXPECT_EQ(first_source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    EXPECT_EQ(first_source_records[index].source_index, 0U);
    EXPECT_EQ(first_source_records[index].source_local_primary_id,
              static_cast<std::uint64_t>(index));
    ExpectObserverSourceMatches(first_source_records[index], expected_a);
  }

  source_a->SetPrimaryCount(0ULL);
  source_b->SetPrimaryCount(3ULL);

  ASSERT_NO_THROW(run.Run());

  auto second_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(second_source_records.size(), 3U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 3U);

  std::uint64_t const second_run_id = second_source_records.front().run_id;
  EXPECT_NE(second_run_id, first_run_id);

  for (std::size_t index = 0U; index < 3U; ++index) {
    auto const &record = second_source_records[index];

    EXPECT_EQ(record.run_id, second_run_id);
    EXPECT_EQ(record.global_primary_id, static_cast<std::uint64_t>(index + 2U));
    EXPECT_EQ(record.source_index, 1U);
    EXPECT_EQ(record.source_local_primary_id,
              static_cast<std::uint64_t>(index));
    ExpectObserverSourceMatches(record, expected_b);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RebuildsMultipleActiveSourceRangesAcrossSequentialRuns) {
  auto random = MakePhiloxRandom();
  auto source_a = MakeLowEnergySource(3ULL);
  auto source_b = MakeLowEnergySource(5ULL);

  source_a->SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  source_b
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_a = source_a->BuildRecord();
  auto expected_b = source_b->BuildRecord();
  auto observer = MakeCapturingObserver(8U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_a);
  run.AddSource(source_b);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  auto first_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(first_source_records.size(), 8U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 8U);

  std::uint64_t first_run_id = first_source_records.front().run_id;

  for (std::size_t index = 0U; index < first_source_records.size(); ++index) {
    EXPECT_EQ(first_source_records[index].run_id, first_run_id);
    EXPECT_EQ(first_source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    EXPECT_EQ(first_source_records[index].source_index, index < 3U ? 0U : 1U);
    EXPECT_EQ(first_source_records[index].source_local_primary_id,
              index < 3U ? static_cast<std::uint64_t>(index)
                         : static_cast<std::uint64_t>(index - 3U));
    ExpectObserverSourceMatches(first_source_records[index],
                                index < 3U ? expected_a : expected_b);
  }

  source_a->SetPrimaryCount(2ULL);
  source_b->SetPrimaryCount(4ULL);

  ASSERT_NO_THROW(run.Run());

  auto second_source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(second_source_records.size(), 6U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 6U);

  std::uint64_t const second_run_id = second_source_records.front().run_id;

  EXPECT_EQ(second_run_id, first_run_id + 1ULL);

  for (std::size_t index = 0U; index < 6U; ++index) {
    auto const &record = second_source_records[index];

    EXPECT_EQ(record.run_id, second_run_id);
    EXPECT_EQ(record.global_primary_id, static_cast<std::uint64_t>(index + 8U));
    EXPECT_EQ(record.source_index, index < 2U ? 0U : 1U);
    EXPECT_EQ(record.source_local_primary_id,
              index < 2U ? static_cast<std::uint64_t>(index)
                         : static_cast<std::uint64_t>(index - 2U));
    ExpectObserverSourceMatches(record, index < 2U ? expected_a : expected_b);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, PreservesObserverResultWhenNextCaptureIsInvalid) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(2ULL);
  auto observer = MakeCapturingObserver(1U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  ASSERT_FALSE(observer->GetRecords().empty());

  std::uint64_t const successful_run_id = observer->GetRecords().front().run_id;

  std::uint32_t const successful_record_count = observer->GetRecordCount();
  std::uint32_t const successful_captured_primary_count =
      observer->GetCapturedPrimaryCount();
  std::uint32_t const successful_overflow_count = observer->GetOverflowCount();
  std::string const successful_dump = observer->BuildDump();

  observer->CapturePrimary(1U, 0ULL);

  ExpectGGEMSExceptionContaining(
      [&run]() -> void { run.Run(); },
      "Observer source index 1 is outside the current source snapshot.");

  EXPECT_EQ(observer->GetRecordCount(), successful_record_count);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(),
            successful_captured_primary_count);
  EXPECT_EQ(observer->GetOverflowCount(), successful_overflow_count);
  EXPECT_EQ(observer->BuildDump(), successful_dump);

  ASSERT_FALSE(observer->GetRecords().empty());
  EXPECT_TRUE(std::ranges::all_of(
      observer->GetRecords(),
      [successful_run_id](
          ggems::core::observer::GGEMSObserverRecord const &record) -> bool {
        return record.run_id == successful_run_id;
      }));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, ClearsObserverAfterSuccessfulRunWithDisabledSnapshot) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(1ULL);
  auto observer = MakeCapturingObserver(1U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  ASSERT_FALSE(observer->GetRecords().empty());
  EXPECT_GT(observer->GetRecordCount(), 0U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);

  observer->Disable();

  ASSERT_NO_THROW(run.Run());

  EXPECT_TRUE(observer->GetRecords().empty());
  EXPECT_EQ(observer->GetRecordCount(), 0U);
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 0U);
  EXPECT_EQ(observer->GetOverflowCount(), 0U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RejectsAllDisabledSourcesBeforeReservation) {
  auto random = MakePhiloxRandom();
  auto source_a = MakeLowEnergySource(0ULL);
  auto source_b = MakeLowEnergySource(0ULL);

  source_a->SetPositionPicoMeter(10LL, 20LL, 30LL);

  source_b
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F);

  auto expected_b = source_b->BuildRecord();
  auto observer = MakeCapturingObserver(2U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_a);
  run.AddSource(source_b);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  ExpectGGEMSExceptionContaining([&run]() -> void { run.Run(); },
                                 "at least one active source");

  EXPECT_TRUE(observer->GetRecords().empty());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 0U);

  source_b->SetPrimaryCount(2ULL);

  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 2U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 2U);
  EXPECT_EQ(source_records[0U].global_primary_id, 0ULL);
  EXPECT_EQ(source_records[1U].global_primary_id, 1ULL);
  EXPECT_EQ(source_records[0U].source_index, 1U);
  EXPECT_EQ(source_records[1U].source_index, 1U);
  EXPECT_EQ(source_records[0U].source_local_primary_id, 0ULL);
  EXPECT_EQ(source_records[1U].source_local_primary_id, 1ULL);

  ExpectObserverSourceMatches(source_records[0U], expected_b);
  ExpectObserverSourceMatches(source_records[1U], expected_b);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RunsDuplicateSourceSlots) {
  auto random = MakePhiloxRandom();
  auto source = MakeLowEnergySource(1ULL);

  source->SetPositionPicoMeter(10LL, 20LL, 30LL);

  auto expected = source->BuildRecord();
  auto observer = MakeCapturingObserver(2U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);

  EXPECT_NO_THROW(run.AddSource(source));
  EXPECT_NO_THROW(run.AddSource(source));

  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 2U);

  auto source_records = BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 2U);

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].global_primary_id,
              static_cast<std::uint64_t>(index));
    EXPECT_EQ(source_records[index].source_index,
              static_cast<std::uint32_t>(index));
    EXPECT_EQ(source_records[index].source_local_primary_id, 0ULL);
    ExpectObserverSourceMatches(source_records[index], expected);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, CapturesFirstPrimariesFromEverySourceSlot) {
  auto random = MakePhiloxRandom();
  auto source_0 = MakeLowEnergySource(4ULL);
  auto source_1 = MakeLowEnergySource(4ULL);
  auto observer = MakeCapturingObserver(2U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_0);
  run.AddSource(source_1);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 4U);

  auto const source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 4U);

  EXPECT_EQ(source_records[0U].source_index, 0U);
  EXPECT_EQ(source_records[0U].source_local_primary_id, 0ULL);
  EXPECT_EQ(source_records[0U].global_primary_id, 0ULL);

  EXPECT_EQ(source_records[1U].source_index, 0U);
  EXPECT_EQ(source_records[1U].source_local_primary_id, 1ULL);
  EXPECT_EQ(source_records[1U].global_primary_id, 1ULL);

  EXPECT_EQ(source_records[2U].source_index, 1U);
  EXPECT_EQ(source_records[2U].source_local_primary_id, 0ULL);
  EXPECT_EQ(source_records[2U].global_primary_id, 4ULL);

  EXPECT_EQ(source_records[3U].source_index, 1U);
  EXPECT_EQ(source_records[3U].source_local_primary_id, 1ULL);
  EXPECT_EQ(source_records[3U].global_primary_id, 5ULL);

  for (auto const &record : observer->GetRecords()) {
    ASSERT_LT(record.source_index, 2U);
    EXPECT_LT(record.source_local_primary_id, 2ULL);

    std::uint64_t const expected_global_primary_id =
        (record.source_index == 0U ? 0ULL : 4ULL) +
        record.source_local_primary_id;

    EXPECT_EQ(record.global_primary_id, expected_global_primary_id);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest,
       RejectsSpecificCaptureOutsideCurrentSnapshotBeforeReservation) {
  auto random = MakePhiloxRandom();
  auto source_0 = MakeLowEnergySource(2ULL);
  auto source_1 = MakeLowEnergySource(0ULL);

  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->SetRecordCapacity(64U).CapturePrimary(2U, 0ULL);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.AddSource(source_0);
  run.AddSource(source_1);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  ExpectGGEMSExceptionContaining(
      [&run]() -> void { run.Run(); },
      "Observer source index 2 is outside the current source snapshot.");

  observer->CapturePrimary(1U, 0ULL);

  ExpectGGEMSExceptionContaining(
      [&run]() -> void { run.Run(); },
      "Observer primary index 0 is outside source slot 1, which contains 0 "
      "primaries.");

  observer->CapturePrimary(0U, 2ULL);

  ExpectGGEMSExceptionContaining(
      [&run]() -> void { run.Run(); },
      "Observer primary index 2 is outside source slot 0, which contains 2 "
      "primaries.");

  EXPECT_TRUE(observer->GetRecords().empty());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 0U);

  observer->CapturePrimary(0U, 1ULL);

  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);

  auto const source_records =
      BuildSortedSourceRecordSnapshot(observer->GetRecords());

  ASSERT_EQ(source_records.size(), 1U);
  EXPECT_EQ(source_records[0U].source_index, 0U);
  EXPECT_EQ(source_records[0U].source_local_primary_id, 1ULL);
  EXPECT_EQ(source_records[0U].global_primary_id, 1ULL);
}
