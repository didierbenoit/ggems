#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/core/radioactivity/GGEMSRadioactiveTimeSampling.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace {

// =============================================================================
// =============================================================================

using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using Emission = ggems::core::radioactivity::GGEMSRadionuclideEmission;
using HostRandomStream = ggems::core::random::GGEMSHostRandomStream;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using Planner = ggems::core::sources::GGEMSSourcePopulationPlanner;
using Random = ggems::core::random::GGEMSRandom;
using Source = ggems::core::sources::GGEMSSource;
using SourceConfigurationSnapshotPtr =
    ggems::core::sources::GGEMSSourceConfigurationSnapshotPtr;
using SourcePtr = std::shared_ptr<Source>;
using SourceRunSnapshot = ggems::core::sources::GGEMSSourceRunSnapshot;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;

constexpr std::uint64_t k_mono_energy_milli_eV{123'456'789ULL};
constexpr std::uint32_t k_worker_count{1U};
constexpr std::uint32_t k_launch_primary_count_limit{3U};
constexpr std::uint64_t k_projection_history_offset{9'000ULL};
constexpr long double k_uniform_limit_half_life_seconds{1.0e12L};
constexpr ggems::core::GGEMSTimeWindow k_time_window{
    .start_ps = 2'000'000'000'000ULL, .stop_ps = 3'000'000'000'000ULL};

// =============================================================================
// =============================================================================

struct ActivityScenario {
  std::vector<SourcePtr> sources;
  SourceConfigurationSnapshotPtr source_configuration;
  SourceRunSnapshot source_snapshot;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom() -> Random {
  Random random{};
  random.SetEngine("philox").SetSeed(0xB32ULL);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeActivityScenario(Random const &random)
    -> ActivityScenario {
  std::vector<Emission> emissions;
  emissions.emplace_back(
      ParticleType::Gamma, 1.0L,
      ggems::core::sources::GGEMSEnergyDistribution::BuildMono(
          k_mono_energy_milli_eV));

  auto definition = std::make_shared<Definition const>(
      "ChunkTransport", k_uniform_limit_half_life_seconds,
      std::move(emissions));

  auto source = std::make_shared<Source>();
  source->SetPointEmission()
      .SetFixedAngularDistribution()
      .SetPositionPicoMeter(11LL, 22LL, 33LL)
      .SetDirection(0.0, 0.0, 1.0)
      .SetWeight(0.5F)
      .SetRadionuclide(std::move(definition),
                                     ggems::units::Activity{64.0L},
                                     k_time_window.start_ps);

  std::vector<SourcePtr> sources{std::move(source)};
  auto source_configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  Planner planner{sources, random};
  auto candidate = planner.BuildCandidate(k_time_window);
  auto source_snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, source_configuration, candidate.GetPlan());

  return {.sources = std::move(sources),
          .source_configuration = std::move(source_configuration),
          .source_snapshot = std::move(source_snapshot)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRunConfig(ActivityScenario const &scenario,
                                 std::uint64_t run_id,
                                 std::uint64_t projection_history_offset)
    -> TransportRunConfig {
  TransportRunConfig config{};
  config.run_id = run_id;
  config.total_primary_count = scenario.source_snapshot.GetTotalPrimaryCount();
  config.projection_history_offset = projection_history_offset;
  config.device_primary_offset = 0ULL;
  config.source_records = scenario.source_snapshot.GetRecords();
  config.source_population_records =
      scenario.source_snapshot.GetPopulationRecords();
  config.source_ranges = scenario.source_snapshot.GetRanges();
  config.source_emission_ranges = scenario.source_snapshot.GetGroupRanges();
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source =
      std::numeric_limits<std::uint32_t>::max();
  return config;
}

// =============================================================================
// =============================================================================

auto ExpectLogicalTransportCounters(TransportRunReport const &report,
                                    std::uint64_t expected_primary_count)
    -> void {
  EXPECT_EQ(report.counters.next_primary_id, expected_primary_count);
  EXPECT_EQ(report.counters.consumed_primary_count, expected_primary_count);
  EXPECT_EQ(report.counters.completed_history_count, expected_primary_count);
  EXPECT_EQ(report.counters.terminal_particle_count, expected_primary_count);
  EXPECT_EQ(report.counters.created_secondary_count, 0ULL);
  EXPECT_EQ(report.counters.aionino_to_gamma_count, 0ULL);
  EXPECT_EQ(report.counters.gamma_to_electron_count, 0ULL);
  EXPECT_EQ(report.counters.electron_to_electron_count, 0ULL);
  EXPECT_EQ(report.counters.overflow_count, 0ULL);
  EXPECT_EQ(report.counters.max_stack_depth, 0ULL);
  EXPECT_EQ(report.counters.total_fake_step_count, 0ULL);
}

// =============================================================================
// =============================================================================

auto ExpectCompleteActivityRecords(TransportRunReport const &report,
                                   std::uint64_t run_id,
                                   std::uint64_t projection_history_offset,
                                   std::uint64_t primary_count,
                                   ggems::core::GGEMSTimeWindow time_window)
    -> void {
  ASSERT_LE(primary_count, static_cast<std::uint64_t>(
                               std::numeric_limits<std::size_t>::max()));

  auto const primary_count_size = static_cast<std::size_t>(primary_count);
  std::vector<std::uint32_t> source_record_counts(primary_count_size, 0U);
  std::vector<std::uint32_t> terminal_record_counts(primary_count_size, 0U);
  std::vector<std::uint64_t> source_times(
      primary_count_size, std::numeric_limits<std::uint64_t>::max());

  for (ObserverRecord const &record : report.observer_records) {
    ASSERT_LT(record.source_local_primary_id, primary_count);
    auto const local_index =
        static_cast<std::size_t>(record.source_local_primary_id);

    EXPECT_EQ(record.run_id, run_id);
    EXPECT_EQ(record.global_primary_id,
              projection_history_offset + record.source_local_primary_id);
    EXPECT_EQ(record.global_particle_id, record.global_primary_id);
    EXPECT_EQ(record.source_index, 0U);
    EXPECT_EQ(
        record.particle_type,
        ggems::core::particles::ToKernelParticleType(ParticleType::Gamma));
    EXPECT_EQ(record.energy_milli_eV, k_mono_energy_milli_eV);
    EXPECT_GE(record.time_ps, time_window.start_ps);
    EXPECT_LT(record.time_ps, time_window.stop_ps);
    EXPECT_FLOAT_EQ(record.weight, 0.5F);

    switch (ggems::core::observer::FromKernelObserverRecordKind(
        record.record_kind)) {
    case ObserverRecordKind::Source:
      ++source_record_counts[local_index];
      source_times[local_index] = record.time_ps;
      EXPECT_EQ(record.position_x_pm, 11LL);
      EXPECT_EQ(record.position_y_pm, 22LL);
      EXPECT_EQ(record.position_z_pm, 33LL);
      break;
    case ObserverRecordKind::Terminal:
      ++terminal_record_counts[local_index];
      EXPECT_EQ(record.time_ps, source_times[local_index]);
      break;
    default:
      ADD_FAILURE() << "Unexpected ActivityDriven observer record kind.";
      break;
    }
  }

  EXPECT_TRUE(std::ranges::all_of(
      source_record_counts, [](std::uint32_t count) { return count == 1U; }));
  EXPECT_TRUE(std::ranges::all_of(
      terminal_record_counts, [](std::uint32_t count) { return count == 1U; }));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CollectSourceTimes(TransportRunReport const &report)
    -> std::vector<std::uint64_t> {
  std::vector<std::pair<std::uint64_t, std::uint64_t>> indexed_times;
  for (ObserverRecord const &record : report.observer_records) {
    if (ggems::core::observer::FromKernelObserverRecordKind(
            record.record_kind) == ObserverRecordKind::Source) {
      indexed_times.emplace_back(record.source_local_primary_id,
                                 record.time_ps);
    }
  }
  std::ranges::sort(indexed_times);

  std::vector<std::uint64_t> times;
  times.reserve(indexed_times.size());
  for (auto const &indexed_time : indexed_times) {
    times.push_back(indexed_time.second);
  }
  return times;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildExpectedSourceTimes(HostRandomStream &random, std::uint64_t primary_count,
                         ggems::core::GGEMSTimeWindow time_window,
                         float scaled_decay) -> std::vector<std::uint64_t> {
  std::vector<std::uint64_t> times;
  times.reserve(static_cast<std::size_t>(primary_count));

  for (std::uint64_t index = 0ULL; index < primary_count; ++index) {
    times.push_back(ggems::core::radioactivity::SampleRadioactiveTimeFromRaw(
        time_window.start_ps, time_window.stop_ps, scaled_decay,
        random.NextUInt32()));
  }

  return times;
}

// =============================================================================
// =============================================================================

class GGEMSRadionuclideTransportTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto GetContext() -> ggems::ocl::GGEMSOpenCLContext & {
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSRadionuclideTransportTest,
       PreservesActivityGroupAcrossChunksAndReusesStableSVM) {
  Random const random = MakeRandom();
  ActivityScenario const scenario = MakeActivityScenario(random);
  std::uint64_t const primary_count =
      scenario.source_snapshot.GetTotalPrimaryCount();
  constexpr ggems::core::GGEMSTimeWindow k_grown_time_window{
      .start_ps = k_time_window.start_ps,
      .stop_ps = k_time_window.start_ps + 4'000'000'000'000ULL};
  Planner grown_planner{scenario.sources, random};
  auto grown_candidate = grown_planner.BuildCandidate(k_grown_time_window);
  auto grown_snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      scenario.sources, scenario.source_configuration,
      grown_candidate.GetPlan());
  std::uint64_t const grown_primary_count =
      grown_snapshot.GetTotalPrimaryCount();

  ASSERT_GT(primary_count, k_launch_primary_count_limit);
  ASSERT_GT(grown_primary_count, primary_count);
  ASSERT_EQ(scenario.source_snapshot.GetGroupRanges().size(), 1U);
  EXPECT_EQ(
      scenario.source_snapshot.GetGroupRanges()[0U].source_local_primary_begin,
      0ULL);
  EXPECT_EQ(scenario.source_snapshot.GetGroupRanges()[0U].primary_count,
            primary_count);
  ASSERT_LE(grown_primary_count,
            static_cast<std::uint64_t>(
                std::numeric_limits<std::uint32_t>::max() / 2U));

  auto const observer_capacity =
      static_cast<std::uint32_t>(grown_primary_count * 2ULL);
  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             observer_capacity,
                             k_launch_primary_count_limit};

  auto const allocated_after_construction =
      GetContext().GetAllocatedVRAM().value;
  auto const allocation_count_after_construction =
      GetContext().GetAllocationCountVRAM();

  auto config = MakeRunConfig(scenario, 17ULL, k_projection_history_offset);
  auto const first_report = workload.Run(config);
  std::uint64_t const expected_record_count = primary_count * 2ULL;
  HostRandomStream time_reference{random, 0ULL};
  float const scaled_decay =
      scenario.source_snapshot.GetPopulationRecords()[0U].scaled_decay;
  auto const expected_first_times = BuildExpectedSourceTimes(
      time_reference, primary_count, k_time_window, scaled_decay);

  ExpectLogicalTransportCounters(first_report, primary_count);
  EXPECT_EQ(first_report.logical_observer_counters.captured_primary_count,
            primary_count);
  EXPECT_EQ(first_report.logical_observer_counters.record_count,
            expected_record_count);
  EXPECT_EQ(first_report.logical_observer_counters.overflow_count, 0ULL);
  EXPECT_EQ(first_report.observer_records.size(), expected_record_count);
  ExpectCompleteActivityRecords(first_report, config.run_id,
                                config.projection_history_offset, primary_count,
                                k_time_window);
  EXPECT_EQ(CollectSourceTimes(first_report), expected_first_times);

  EXPECT_EQ(GetContext().GetAllocatedVRAM().value,
            allocated_after_construction);
  EXPECT_EQ(GetContext().GetAllocationCountVRAM(),
            allocation_count_after_construction);

  config.run_id = 18ULL;
  config.projection_history_offset = 10'000ULL;
  auto const continuation_report = workload.Run(config);
  auto const expected_continuation_times = BuildExpectedSourceTimes(
      time_reference, primary_count, k_time_window, scaled_decay);

  ASSERT_NE(expected_continuation_times, expected_first_times);
  ExpectLogicalTransportCounters(continuation_report, primary_count);
  EXPECT_EQ(
      continuation_report.logical_observer_counters.captured_primary_count,
      primary_count);
  EXPECT_EQ(continuation_report.logical_observer_counters.record_count,
            expected_record_count);
  EXPECT_EQ(continuation_report.logical_observer_counters.overflow_count, 0ULL);
  ExpectCompleteActivityRecords(continuation_report, config.run_id,
                                config.projection_history_offset, primary_count,
                                k_time_window);
  EXPECT_EQ(CollectSourceTimes(continuation_report),
            expected_continuation_times);

  EXPECT_EQ(GetContext().GetAllocatedVRAM().value,
            allocated_after_construction);
  EXPECT_EQ(GetContext().GetAllocationCountVRAM(),
            allocation_count_after_construction);

  config.run_id = 19ULL;
  config.projection_history_offset = 20'000ULL;
  config.total_primary_count = grown_primary_count;
  config.source_records = grown_snapshot.GetRecords();
  config.source_population_records = grown_snapshot.GetPopulationRecords();
  config.source_ranges = grown_snapshot.GetRanges();
  config.source_emission_ranges = grown_snapshot.GetGroupRanges();
  auto const grown_report = workload.Run(config);

  ExpectLogicalTransportCounters(grown_report, grown_primary_count);
  EXPECT_EQ(grown_report.logical_observer_counters.captured_primary_count,
            grown_primary_count);
  EXPECT_EQ(grown_report.logical_observer_counters.record_count,
            grown_primary_count * 2ULL);
  EXPECT_EQ(grown_report.logical_observer_counters.overflow_count, 0ULL);
  ExpectCompleteActivityRecords(grown_report, config.run_id,
                                config.projection_history_offset,
                                grown_primary_count, k_grown_time_window);

  EXPECT_EQ(GetContext().GetAllocatedVRAM().value,
            allocated_after_construction);
  EXPECT_EQ(GetContext().GetAllocationCountVRAM(),
            allocation_count_after_construction);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRadionuclideTransportTest,
       AggregatesExactObserverOverflowAcrossChunks) {
  constexpr std::uint32_t k_observer_capacity{7U};

  Random const random = MakeRandom();
  ActivityScenario const scenario = MakeActivityScenario(random);
  std::uint64_t const primary_count =
      scenario.source_snapshot.GetTotalPrimaryCount();
  ASSERT_GT(primary_count, k_launch_primary_count_limit);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             k_worker_count,
                             *scenario.source_configuration,
                             0ULL,
                             0U,
                             k_observer_capacity,
                             k_launch_primary_count_limit};

  auto const config =
      MakeRunConfig(scenario, 19ULL, k_projection_history_offset);
  auto const report = workload.Run(config);
  std::uint64_t const attempted_record_count = primary_count * 2ULL;
  std::uint64_t const expected_overflow =
      attempted_record_count - k_observer_capacity;

  ExpectLogicalTransportCounters(report, primary_count);
  EXPECT_EQ(report.logical_observer_counters.captured_primary_count,
            primary_count);
  EXPECT_EQ(report.logical_observer_counters.record_count, k_observer_capacity);
  EXPECT_EQ(report.logical_observer_counters.overflow_count, expected_overflow);
  EXPECT_EQ(report.observer_counters.record_count, k_observer_capacity);
  EXPECT_EQ(report.observer_counters.captured_primary_count, primary_count);
  EXPECT_EQ(report.observer_counters.overflow_count, expected_overflow);
  ASSERT_EQ(report.observer_records.size(), k_observer_capacity);

  bool const has_record_before_chunk_boundary = std::ranges::any_of(
      report.observer_records, [](ObserverRecord const &record) {
        return record.source_local_primary_id < k_launch_primary_count_limit;
      });
  bool const has_record_after_chunk_boundary = std::ranges::any_of(
      report.observer_records, [](ObserverRecord const &record) {
        return record.source_local_primary_id >= k_launch_primary_count_limit;
      });

  EXPECT_TRUE(has_record_before_chunk_boundary);
  EXPECT_TRUE(has_record_after_chunk_boundary);
}
