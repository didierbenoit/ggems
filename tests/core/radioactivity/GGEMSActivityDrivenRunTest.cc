#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace {

constexpr std::uint64_t k_second_ps{1'000'000'000'000ULL};
constexpr std::uint64_t k_mono_energy_milli_eV{123'456'789ULL};
constexpr long double k_activity_bq{512.0L};

constexpr std::uint32_t k_source_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Source);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom()
    -> std::shared_ptr<ggems::core::random::GGEMSRandom> {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(0x1259'7A31'B460'D8EFULL);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoRadionuclide() -> std::shared_ptr<
    ggems::core::radioactivity::GGEMSRadionuclideDefinition const> {
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
  using ggems::core::radioactivity::GGEMSRadionuclideEmission;
  using ggems::core::sources::GGEMSEnergyDistribution;

  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(
      GGEMSParticleType::Gamma, 1.0L,
      GGEMSEnergyDistribution::BuildMono(k_mono_energy_milli_eV));

  return std::make_shared<GGEMSRadionuclideDefinition const>(
      "ActivityDrivenRunTest", std::vector<std::string>{}, 1.0e9L,
      std::move(emissions));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeActivitySource(
    std::shared_ptr<
        ggems::core::radioactivity::GGEMSRadionuclideDefinition const>
        radionuclide,
    long double activity_bq = k_activity_bq)
    -> std::shared_ptr<ggems::core::sources::GGEMSSource> {
  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();
  source->SetRadionuclide(std::move(radionuclide),
                          ggems::units::Activity{activity_bq}, 0ULL);
  return source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeObserver()
    -> std::shared_ptr<ggems::core::observer::GGEMSTransportObserver> {
  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->CaptureFirstPrimaries(std::numeric_limits<std::uint32_t>::max());
  return observer;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetSortedSourceRecords(
    ggems::core::observer::GGEMSTransportObserver const &observer)
    -> std::vector<ggems::core::observer::GGEMSObserverRecord> {
  std::vector<ggems::core::observer::GGEMSObserverRecord> source_records;

  for (auto const &record : observer.GetRecords()) {
    if (record.record_kind == k_source_record_kind) {
      source_records.push_back(record);
    }
  }

  std::ranges::sort(source_records,
                    [](auto const &lhs, auto const &rhs) -> bool {
                      return lhs.global_primary_id < rhs.global_primary_id;
                    });
  return source_records;
}

// =============================================================================
// =============================================================================

class GGEMSActivityDrivenRunTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSActivityDrivenRunTest,
       RetriesDuplicateActivitySourceSlotsAfterInvalidReferenceTime) {
  auto radionuclide = MakeMonoRadionuclide();
  auto source = MakeActivitySource(radionuclide);
  source->SetRadionuclide(radionuclide, ggems::units::Activity{k_activity_bq},
                          3ULL * k_second_ps);

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.SetSource(source);
  run.AddSource(source);
  run.SetWorkerCount(64U);
  run.SetTimePicoSecond(2ULL * k_second_ps, 3ULL * k_second_ps, k_second_ps);

  EXPECT_THROW(run.Initialize(), ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(source->SetRadionuclide(
      radionuclide, ggems::units::Activity{k_activity_bq}, k_second_ps));

  ASSERT_NO_THROW(run.Initialize());

  auto const &configuration =
      source->BuildActivityDrivenPopulationConfiguration();
  EXPECT_EQ(configuration.radionuclide, radionuclide);
  EXPECT_EQ(configuration.activity_at_reference_time.value, k_activity_bq);
  EXPECT_EQ(configuration.reference_time_ps, k_second_ps);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSActivityDrivenRunTest,
       InitializesFinalizesAndKeepsIdsMonotoneAcrossResetTime) {
  auto radionuclide = MakeMonoRadionuclide();
  auto source = MakeActivitySource(radionuclide);
  auto observer = MakeObserver();
  auto random = MakeRandom();

  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>>
      reference_sources{MakeActivitySource(radionuclide)};
  auto reference_random = MakeRandom();
  ggems::core::sources::GGEMSSourcePopulationPlanner reference_planner{
      reference_sources, *reference_random};
  auto reference_first = reference_planner.BuildCandidate(
      {.start_ps = 0ULL, .stop_ps = k_second_ps});
  std::uint64_t const expected_first_count =
      reference_first.GetPlan().GetTotalPrimaryCount();
  reference_planner.CommitCandidate(reference_first);
  auto reference_second = reference_planner.BuildCandidate(
      {.start_ps = k_second_ps, .stop_ps = 2ULL * k_second_ps});
  std::uint64_t const expected_second_count =
      reference_second.GetPlan().GetTotalPrimaryCount();
  reference_planner.CommitCandidate(reference_second);
  auto reference_after_reset = reference_planner.BuildCandidate(
      {.start_ps = 0ULL, .stop_ps = k_second_ps});
  std::uint64_t const expected_after_reset_count =
      reference_after_reset.GetPlan().GetTotalPrimaryCount();

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);
  run.SetTimePicoSecond(0ULL, 3ULL * k_second_ps, k_second_ps);

  ASSERT_NO_THROW(run.Initialize());

  EXPECT_THROW(source->SetRadionuclide(
                   radionuclide, ggems::units::Activity{k_activity_bq}, 0ULL),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(source->SetCountDrivenPopulation(1ULL),
               ggems::core::GGEMSExceptionBase);

  ASSERT_NO_THROW(run.Run());
  auto const first_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(first_snapshot.has_value());
  ASSERT_GT(first_snapshot->GetTotalPrimaryCount(), 0ULL);
  EXPECT_EQ(first_snapshot->GetTotalPrimaryCount(), expected_first_count);
  ASSERT_EQ(first_snapshot->GetRecords().size(), 1U);
  ASSERT_EQ(first_snapshot->GetRanges().size(), 1U);
  EXPECT_EQ(first_snapshot->GetRecords()[0U].time_start_ps, 0ULL);
  EXPECT_EQ(first_snapshot->GetRecords()[0U].time_stop_ps, k_second_ps);

  auto const first_source_records = GetSortedSourceRecords(*observer);
  ASSERT_EQ(first_source_records.size(),
            static_cast<std::size_t>(first_snapshot->GetTotalPrimaryCount()));
  EXPECT_EQ(first_source_records.front().run_id, 0ULL);
  EXPECT_EQ(first_source_records.front().global_primary_id, 0ULL);
  EXPECT_EQ(observer->GetOverflowCount(), 0U);

  ASSERT_NO_THROW(run.Run());
  auto const second_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(second_snapshot.has_value());
  ASSERT_GT(second_snapshot->GetTotalPrimaryCount(), 0ULL);
  EXPECT_EQ(second_snapshot->GetTotalPrimaryCount(), expected_second_count);
  ASSERT_EQ(second_snapshot->GetRecords().size(), 1U);
  EXPECT_EQ(second_snapshot->GetRecords()[0U].time_start_ps, k_second_ps);
  EXPECT_EQ(second_snapshot->GetRecords()[0U].time_stop_ps, 2ULL * k_second_ps);

  auto const second_source_records = GetSortedSourceRecords(*observer);
  ASSERT_EQ(second_source_records.size(),
            static_cast<std::size_t>(second_snapshot->GetTotalPrimaryCount()));
  EXPECT_EQ(second_source_records.front().run_id, 1ULL);
  EXPECT_EQ(second_source_records.front().global_primary_id,
            first_snapshot->GetTotalPrimaryCount());
  EXPECT_EQ(run.GetCurrentTimePicoSecond(), 2ULL * k_second_ps);

  ASSERT_NO_THROW(run.ResetTime());
  EXPECT_EQ(run.GetCurrentTimePicoSecond(), 0ULL);

  auto const snapshot_after_reset = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(snapshot_after_reset.has_value());
  ASSERT_EQ(snapshot_after_reset->GetRecords().size(), 1U);
  EXPECT_EQ(snapshot_after_reset->GetRecords()[0U].time_start_ps, k_second_ps);
  EXPECT_EQ(snapshot_after_reset->GetRecords()[0U].time_stop_ps,
            2ULL * k_second_ps);

  ASSERT_NO_THROW(run.Run());
  auto const reset_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(reset_snapshot.has_value());
  ASSERT_GT(reset_snapshot->GetTotalPrimaryCount(), 0ULL);
  EXPECT_EQ(reset_snapshot->GetTotalPrimaryCount(), expected_after_reset_count);
  ASSERT_EQ(reset_snapshot->GetRecords().size(), 1U);
  EXPECT_EQ(reset_snapshot->GetRecords()[0U].time_start_ps, 0ULL);
  EXPECT_EQ(reset_snapshot->GetRecords()[0U].time_stop_ps, k_second_ps);

  auto const reset_source_records = GetSortedSourceRecords(*observer);
  ASSERT_EQ(reset_source_records.size(),
            static_cast<std::size_t>(reset_snapshot->GetTotalPrimaryCount()));
  EXPECT_EQ(reset_source_records.front().run_id, 2ULL);
  EXPECT_EQ(reset_source_records.front().global_primary_id,
            first_snapshot->GetTotalPrimaryCount() +
                second_snapshot->GetTotalPrimaryCount());
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSActivityDrivenRunTest,
       EmptyWindowCommitsPlannerAndAdvancesWithoutConsumingPrimaryIds) {
  constexpr std::uint64_t k_empty_then_non_empty_seed{2ULL};
  constexpr long double k_empty_then_non_empty_activity_bq{1.0L};

  auto radionuclide = MakeMonoRadionuclide();
  auto activity_source =
      MakeActivitySource(radionuclide, k_empty_then_non_empty_activity_bq);
  auto count_source = std::make_shared<ggems::core::sources::GGEMSSource>();
  count_source->SetPrimaryCount(0ULL).SetEnergyMilliElectronVolt(
      k_mono_energy_milli_eV);
  auto observer = MakeObserver();
  auto random = MakeRandom();
  random->SetSeed(k_empty_then_non_empty_seed);

  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>>
      reference_sources{
          MakeActivitySource(radionuclide, k_empty_then_non_empty_activity_bq)};
  auto reference_random = MakeRandom();
  reference_random->SetSeed(k_empty_then_non_empty_seed);
  ggems::core::sources::GGEMSSourcePopulationPlanner reference_planner{
      reference_sources, *reference_random};

  auto reference_first = reference_planner.BuildCandidate(
      {.start_ps = 0ULL, .stop_ps = k_second_ps});
  auto reference_second_without_commit = reference_planner.BuildCandidate(
      {.start_ps = k_second_ps, .stop_ps = 2ULL * k_second_ps});
  ASSERT_EQ(reference_first.GetPlan().GetTotalPrimaryCount(), 0ULL);
  ASSERT_EQ(reference_second_without_commit.GetPlan().GetTotalPrimaryCount(),
            0ULL);

  reference_planner.CommitCandidate(reference_first);
  auto reference_second_after_commit = reference_planner.BuildCandidate(
      {.start_ps = k_second_ps, .stop_ps = 2ULL * k_second_ps});
  std::uint64_t const expected_activity_primary_count =
      reference_second_after_commit.GetPlan().GetTotalPrimaryCount();
  ASSERT_EQ(expected_activity_primary_count, 2ULL);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(activity_source);
  run.AddSource(count_source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);
  run.SetTimePicoSecond(0ULL, 2ULL * k_second_ps, k_second_ps);

  ASSERT_NO_THROW(run.Initialize());
  ASSERT_NO_THROW(run.Run());

  EXPECT_EQ(run.GetCurrentTimePicoSecond(), k_second_ps);
  EXPECT_TRUE(observer->GetRecords().empty());
  EXPECT_EQ(observer->GetCapturedPrimaryCount(), 0U);

  auto const empty_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(empty_snapshot.has_value());
  ASSERT_EQ(empty_snapshot->GetRecords().size(), 2U);
  ASSERT_EQ(empty_snapshot->GetRanges().size(), 2U);
  EXPECT_EQ(empty_snapshot->GetTotalPrimaryCount(), 0ULL);
  EXPECT_EQ(empty_snapshot->GetRanges()[0U].primary_count, 0ULL);
  EXPECT_EQ(empty_snapshot->GetRanges()[1U].primary_count, 0ULL);
  EXPECT_EQ(empty_snapshot->GetRecords()[0U].time_start_ps, 0ULL);
  EXPECT_EQ(empty_snapshot->GetRecords()[0U].time_stop_ps, k_second_ps);

  ASSERT_NO_THROW(count_source->SetPrimaryCount(1ULL));
  ASSERT_NO_THROW(run.Run());

  auto const second_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(second_snapshot.has_value());
  ASSERT_EQ(second_snapshot->GetRanges().size(), 2U);
  EXPECT_EQ(second_snapshot->GetRanges()[0U].primary_count,
            expected_activity_primary_count);
  EXPECT_EQ(second_snapshot->GetRanges()[1U].primary_count, 1ULL);
  EXPECT_EQ(second_snapshot->GetTotalPrimaryCount(),
            expected_activity_primary_count + 1ULL);

  auto const source_records = GetSortedSourceRecords(*observer);
  ASSERT_EQ(source_records.size(),
            static_cast<std::size_t>(expected_activity_primary_count + 1ULL));
  EXPECT_EQ(source_records.front().run_id, 1ULL);
  EXPECT_EQ(source_records.front().global_primary_id, 0ULL);
  EXPECT_EQ(source_records.front().source_index, 0U);
  EXPECT_EQ(source_records.back().run_id, 1ULL);
  EXPECT_EQ(source_records.back().global_primary_id,
            expected_activity_primary_count);
  EXPECT_EQ(source_records.back().source_index, 1U);
  EXPECT_EQ(run.GetCurrentTimePicoSecond(), 2ULL * k_second_ps);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSActivityDrivenRunTest,
       PreflightFailurePreservesPlannerSnapshotObserverClockAndIds) {
  auto radionuclide = MakeMonoRadionuclide();
  auto source = MakeActivitySource(radionuclide);
  auto observer = MakeObserver();

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetWorkerCount(64U);
  run.SetTimePicoSecond(0ULL, 3ULL * k_second_ps, k_second_ps);

  ASSERT_NO_THROW(run.Initialize());
  ASSERT_NO_THROW(run.Run());

  auto const first_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(first_snapshot.has_value());
  ASSERT_GT(first_snapshot->GetTotalPrimaryCount(), 0ULL);
  ASSERT_EQ(first_snapshot->GetRecords().size(), 1U);
  std::string const first_observer_dump = observer->BuildDump();

  observer->CaptureFirstPrimaries(0U).CapturePrimary(
      0U, std::numeric_limits<std::uint64_t>::max());

  EXPECT_THROW(run.Run(), ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(run.GetCurrentTimePicoSecond(), k_second_ps);
  EXPECT_EQ(observer->BuildDump(), first_observer_dump);

  auto const snapshot_after_failure = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(snapshot_after_failure.has_value());
  ASSERT_EQ(snapshot_after_failure->GetRecords().size(), 1U);
  EXPECT_EQ(snapshot_after_failure->GetTotalPrimaryCount(),
            first_snapshot->GetTotalPrimaryCount());
  EXPECT_EQ(snapshot_after_failure->GetRecords()[0U].time_start_ps, 0ULL);
  EXPECT_EQ(snapshot_after_failure->GetRecords()[0U].time_stop_ps, k_second_ps);

  observer->ClearCapturedPrimary().CaptureFirstPrimaries(
      std::numeric_limits<std::uint32_t>::max());
  ASSERT_NO_THROW(run.Run());

  auto const retry_snapshot = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(retry_snapshot.has_value());
  ASSERT_GT(retry_snapshot->GetTotalPrimaryCount(), 0ULL);
  ASSERT_EQ(retry_snapshot->GetRecords().size(), 1U);
  auto const retry_source_records = GetSortedSourceRecords(*observer);
  ASSERT_EQ(retry_source_records.size(),
            static_cast<std::size_t>(retry_snapshot->GetTotalPrimaryCount()));
  EXPECT_EQ(retry_source_records.front().run_id, 1ULL);
  EXPECT_EQ(retry_source_records.front().global_primary_id,
            first_snapshot->GetTotalPrimaryCount());
  EXPECT_EQ(run.GetCurrentTimePicoSecond(), 2ULL * k_second_ps);

  auto reference_source = MakeActivitySource(radionuclide);
  ggems::core::GGEMSRun reference_run{};
  reference_run.SetRandom(MakeRandom());
  reference_run.SetSource(reference_source);
  reference_run.SetWorkerCount(64U);
  reference_run.SetTimePicoSecond(0ULL, 3ULL * k_second_ps, k_second_ps);

  ASSERT_NO_THROW(reference_run.Initialize());
  ASSERT_NO_THROW(reference_run.Run());
  ASSERT_NO_THROW(reference_run.Run());

  auto const reference_second_snapshot =
      reference_run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(reference_second_snapshot.has_value());
  ASSERT_EQ(reference_second_snapshot->GetRecords().size(), 1U);
  EXPECT_EQ(retry_snapshot->GetTotalPrimaryCount(),
            reference_second_snapshot->GetTotalPrimaryCount());
  EXPECT_EQ(reference_second_snapshot->GetRecords()[0U].time_start_ps,
            k_second_ps);
  EXPECT_EQ(reference_second_snapshot->GetRecords()[0U].time_stop_ps,
            2ULL * k_second_ps);
}
