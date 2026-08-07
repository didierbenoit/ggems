#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionPlan.hh"
#include "GGEMS/core/radioactivity/GGEMSRadioactiveTimeSampling.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
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
using Planner = ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanner;
using Random = ggems::core::random::GGEMSRandom;
using Source = ggems::core::sources::GGEMSSource;
using SourcePtr = std::shared_ptr<Source>;
using SourceRunSnapshot = ggems::core::sources::GGEMSSourceRunSnapshot;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;

constexpr ggems::core::GGEMSTimeWindow k_time_window{
    .start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL};
constexpr std::uint64_t k_projection_history_offset{10'000ULL};
constexpr std::uint64_t k_run_id{27ULL};
constexpr std::uint32_t k_single_worker_count{1U};
constexpr std::uint32_t k_worker_count{64U};
constexpr std::uint32_t k_chunk_primary_count{3U};
constexpr std::array<ParticleType, 3U> k_activity_particles{
    ParticleType::Gamma, ParticleType::Electron, ParticleType::Positron};
constexpr std::array<std::uint64_t, 3U> k_activity_energies{111ULL, 222ULL,
                                                            333ULL};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDefinition() -> std::shared_ptr<Definition const> {
  std::vector<Emission> emissions;
  emissions.reserve(k_activity_particles.size());
  emissions.emplace_back(
      k_activity_particles[0U], 1.0L,
      ggems::core::sources::GGEMSEnergyDistribution::BuildMono(
          k_activity_energies[0U]));
  emissions.emplace_back(
      k_activity_particles[1U], 1.0L,
      ggems::core::sources::GGEMSEnergyDistribution::BuildMono(
          k_activity_energies[1U]));
  emissions.emplace_back(
      k_activity_particles[2U], std::numeric_limits<long double>::min(),
      ggems::core::sources::GGEMSEnergyDistribution::BuildMono(
          k_activity_energies[2U]));

  return std::make_shared<Definition const>(
      "MixedLookup", std::vector<std::string>{}, 1.0e12L, std::move(emissions));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSources() -> std::vector<SourcePtr> {
  auto first_count = std::make_shared<Source>();
  first_count->SetPrimaryCount(2ULL)
      .SetEmittedParticleType(ParticleType::Gamma)
      .SetEnergyMilliElectronVolt(1'001ULL)
      .SetPositionPicoMeter(-1'000LL, 0LL, 0LL);

  auto activity = std::make_shared<Source>();
  activity
      ->SetActivityDrivenRadionuclide(MakeDefinition(),
                                      ggems::units::Activity{100.0L}, 0ULL)
      .SetPositionPicoMeter(0LL, 2'000LL, 0LL);

  auto last_count = std::make_shared<Source>();
  last_count->SetPrimaryCount(2ULL)
      .SetEmittedParticleType(ParticleType::Electron)
      .SetEnergyMilliElectronVolt(3'003ULL)
      .SetPositionPicoMeter(0LL, 0LL, 3'000LL);

  return {std::move(first_count), std::move(activity), std::move(last_count)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom() -> Random {
  Random random{};
  random.SetEngine("philox").SetSeed(0x1234'5678ULL);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRunConfig(SourceRunSnapshot const &snapshot,
                                 std::uint64_t device_primary_offset,
                                 std::uint64_t primary_count)
    -> TransportRunConfig {
  TransportRunConfig config{};
  config.run_id = k_run_id;
  config.total_primary_count = primary_count;
  config.projection_history_offset = k_projection_history_offset;
  config.device_primary_offset = device_primary_offset;
  config.source_records = snapshot.GetRecords();
  config.source_population_records = snapshot.GetPopulationRecords();
  config.source_ranges = snapshot.GetRanges();
  config.radionuclide_group_ranges = snapshot.GetGroupRanges();
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source =
      std::numeric_limits<std::uint32_t>::max();
  return config;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ExtractSourceRecords(std::span<TransportRunReport const> reports,
                     std::uint64_t total_primary_count)
    -> std::vector<ObserverRecord> {
  std::vector<ObserverRecord> records(
      static_cast<std::size_t>(total_primary_count));
  std::vector<bool> seen(static_cast<std::size_t>(total_primary_count), false);
  std::uint64_t observed_count{0ULL};

  for (TransportRunReport const &report : reports) {
    for (ObserverRecord const &record : report.observer_records) {
      if (record.record_kind !=
          ggems::core::observer::ToKernelObserverRecordKind(
              ObserverRecordKind::Source)) {
        continue;
      }

      EXPECT_GE(record.global_primary_id, k_projection_history_offset);
      if (record.global_primary_id < k_projection_history_offset) {
        continue;
      }

      std::uint64_t const projection_primary_id =
          record.global_primary_id - k_projection_history_offset;
      EXPECT_LT(projection_primary_id, total_primary_count);
      if (projection_primary_id >= total_primary_count) {
        continue;
      }

      std::size_t const index = static_cast<std::size_t>(projection_primary_id);
      EXPECT_FALSE(seen[index]);
      if (seen[index]) {
        continue;
      }

      seen[index] = true;
      records[index] = record;
      ++observed_count;
    }
  }

  EXPECT_EQ(observed_count, total_primary_count);
  EXPECT_TRUE(std::ranges::all_of(seen, [](bool value) { return value; }));
  return records;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CollectSourceTimes(TransportRunReport const &report)
    -> std::vector<std::uint64_t> {
  std::vector<std::pair<std::uint64_t, std::uint64_t>> indexed_times;
  for (ObserverRecord const &record : report.observer_records) {
    if (record.record_kind == ggems::core::observer::ToKernelObserverRecordKind(
                                  ObserverRecordKind::Source)) {
      indexed_times.emplace_back(record.global_primary_id, record.time_ps);
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

[[nodiscard]] auto BuildExpectedSourceTimes(HostRandomStream &random,
                                            std::uint64_t primary_count,
                                            float scaled_decay)
    -> std::vector<std::uint64_t> {
  std::vector<std::uint64_t> times;
  times.reserve(static_cast<std::size_t>(primary_count));

  for (std::uint64_t index = 0ULL; index < primary_count; ++index) {
    times.push_back(ggems::core::radioactivity::SampleRadioactiveTimeFromRaw(
        k_time_window.start_ps, k_time_window.stop_ps, scaled_decay,
        random.NextUInt32()));
  }

  return times;
}

// =============================================================================
// =============================================================================

auto ExpectCounters(TransportRunReport const &report,
                    std::uint64_t primary_count) -> void {
  EXPECT_EQ(report.counters.consumed_primary_count, primary_count);
  EXPECT_EQ(report.counters.completed_history_count, primary_count);
  EXPECT_EQ(report.counters.terminal_particle_count, primary_count);
  EXPECT_EQ(report.counters.created_secondary_count, 0ULL);
  EXPECT_EQ(report.counters.overflow_count, 0ULL);
  EXPECT_EQ(report.logical_observer_counters.captured_primary_count,
            primary_count);
  EXPECT_EQ(report.logical_observer_counters.record_count,
            2ULL * primary_count);
  EXPECT_EQ(report.logical_observer_counters.overflow_count, 0ULL);
}

// =============================================================================
// =============================================================================

auto ExpectSourceAndGroupLookup(SourceRunSnapshot const &snapshot,
                                std::span<ObserverRecord const> source_records)
    -> void {
  auto const &ranges = snapshot.GetRanges();
  auto const &population_records = snapshot.GetPopulationRecords();
  auto const &group_ranges = snapshot.GetGroupRanges();
  auto const &emission_records = snapshot.GetRadionuclideEmissionRecords();
  auto const &source_host_records = snapshot.GetRecords();

  ASSERT_EQ(ranges.size(), 3U);
  ASSERT_EQ(population_records.size(), 3U);
  ASSERT_EQ(group_ranges.size(), 3U);
  ASSERT_EQ(emission_records.size(), 3U);
  ASSERT_EQ(source_records.size(), snapshot.GetTotalPrimaryCount());

  EXPECT_EQ(ranges[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(ranges[0U].primary_count, 2ULL);
  EXPECT_EQ(ranges[1U].projection_primary_begin, 2ULL);
  EXPECT_EQ(ranges[2U].projection_primary_begin,
            ranges[1U].projection_primary_begin + ranges[1U].primary_count);
  EXPECT_EQ(ranges[2U].primary_count, 2ULL);

  EXPECT_EQ(
      population_records[1U].population_mode,
      ggems::core::sources::ToKernelSourcePopulationMode(
          ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven));
  EXPECT_EQ(population_records[1U].first_emission_index, 0U);
  EXPECT_EQ(population_records[1U].emission_count, 3U);

  ASSERT_GT(group_ranges[0U].primary_count, 1ULL);
  ASSERT_GT(group_ranges[1U].primary_count, 1ULL);
  EXPECT_EQ(group_ranges[0U].source_local_primary_begin, 0ULL);
  EXPECT_EQ(group_ranges[1U].source_local_primary_begin,
            group_ranges[0U].primary_count);
  EXPECT_EQ(group_ranges[2U].source_local_primary_begin,
            group_ranges[0U].primary_count + group_ranges[1U].primary_count);
  EXPECT_EQ(group_ranges[2U].primary_count, 0ULL);
  EXPECT_EQ(ranges[1U].primary_count,
            group_ranges[2U].source_local_primary_begin);

  for (std::size_t projection_primary_id = 0U;
       projection_primary_id < source_records.size(); ++projection_primary_id) {
    ObserverRecord const &record = source_records[projection_primary_id];
    EXPECT_EQ(record.run_id, k_run_id);
    EXPECT_EQ(record.global_primary_id,
              k_projection_history_offset + projection_primary_id);
    EXPECT_EQ(record.global_particle_id, record.global_primary_id);
    EXPECT_EQ(record.track_id, 0ULL);
    EXPECT_EQ(record.parent_track_id, ggems::core::particles::k_invalid_id_u64);
    EXPECT_EQ(record.deposited_energy_milli_eV, 0ULL);

    std::uint32_t expected_source_index{0U};
    for (std::size_t source_index = 0U; source_index < ranges.size();
         ++source_index) {
      auto const &range = ranges[source_index];
      if (projection_primary_id >= range.projection_primary_begin &&
          projection_primary_id - range.projection_primary_begin <
              range.primary_count) {
        expected_source_index = static_cast<std::uint32_t>(source_index);
        break;
      }
    }

    auto const &source_range = ranges[expected_source_index];
    std::uint64_t const source_local_primary_id =
        projection_primary_id - source_range.projection_primary_begin;
    EXPECT_EQ(record.source_index, expected_source_index);
    EXPECT_EQ(record.source_local_primary_id, source_local_primary_id);

    if (expected_source_index != 1U) {
      auto const &source = source_host_records[expected_source_index];
      EXPECT_EQ(record.particle_type, source.emitted_particle_type);
      EXPECT_EQ(record.energy_milli_eV, source.energy_milli_eV);
      EXPECT_EQ(record.time_ps, k_time_window.start_ps);
      continue;
    }

    std::uint32_t expected_emission_index{
        std::numeric_limits<std::uint32_t>::max()};
    for (std::uint32_t emission_index = 0U; emission_index < 3U;
         ++emission_index) {
      auto const &group = group_ranges[emission_index];
      if (group.primary_count != 0ULL &&
          source_local_primary_id >= group.source_local_primary_begin &&
          source_local_primary_id - group.source_local_primary_begin <
              group.primary_count) {
        expected_emission_index = emission_index;
        break;
      }
    }

    ASSERT_NE(expected_emission_index,
              std::numeric_limits<std::uint32_t>::max());
    EXPECT_LT(expected_emission_index, 2U);
    EXPECT_EQ(record.particle_type,
              ggems::core::particles::ToKernelParticleType(
                  k_activity_particles[expected_emission_index]));
    EXPECT_EQ(record.energy_milli_eV,
              k_activity_energies[expected_emission_index]);
    EXPECT_GE(record.time_ps, k_time_window.start_ps);
    EXPECT_LT(record.time_ps, k_time_window.stop_ps);
  }

  for (std::size_t source_index = 0U; source_index < ranges.size();
       ++source_index) {
    auto const &range = ranges[source_index];
    ASSERT_NE(range.primary_count, 0ULL);
    ObserverRecord const &first = source_records[static_cast<std::size_t>(
        range.projection_primary_begin)];
    ObserverRecord const &last = source_records[static_cast<std::size_t>(
        range.projection_primary_begin + range.primary_count - 1ULL)];
    EXPECT_EQ(first.source_index, source_index);
    EXPECT_EQ(first.source_local_primary_id, 0ULL);
    EXPECT_EQ(last.source_index, source_index);
    EXPECT_EQ(last.source_local_primary_id, range.primary_count - 1ULL);
  }

  for (std::size_t emission_index = 0U; emission_index < 2U; ++emission_index) {
    auto const &group = group_ranges[emission_index];
    std::uint64_t const projection_begin =
        ranges[1U].projection_primary_begin + group.source_local_primary_begin;
    ObserverRecord const &first =
        source_records[static_cast<std::size_t>(projection_begin)];
    ObserverRecord const &last = source_records[static_cast<std::size_t>(
        projection_begin + group.primary_count - 1ULL)];
    EXPECT_EQ(first.particle_type, ggems::core::particles::ToKernelParticleType(
                                       k_activity_particles[emission_index]));
    EXPECT_EQ(first.energy_milli_eV, k_activity_energies[emission_index]);
    EXPECT_EQ(last.particle_type, first.particle_type);
    EXPECT_EQ(last.energy_milli_eV, first.energy_milli_eV);
  }

  EXPECT_TRUE(
      std::ranges::none_of(source_records, [](ObserverRecord const &record) {
        return record.particle_type ==
                   ggems::core::particles::ToKernelParticleType(
                       k_activity_particles[2U]) ||
               record.energy_milli_eV == k_activity_energies[2U];
      }));
}

// =============================================================================
// =============================================================================

class GGEMSRadionuclideMixedLookupTransportTest : public ::testing::Test {
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

TEST_F(GGEMSRadionuclideMixedLookupTransportTest,
       ResolvesMixedSourcesGroupsAndDisjointDeviceSlices) {
  auto sources = MakeSources();
  Random random = MakeRandom();
  auto source_configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  Planner planner{sources, random};
  auto candidate = planner.BuildCandidate(k_time_window);
  SourceRunSnapshot snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, source_configuration, candidate.GetPlan());

  std::uint64_t const total_primary_count = snapshot.GetTotalPrimaryCount();
  ASSERT_GT(total_primary_count, 8ULL);
  ASSERT_LE(total_primary_count,
            static_cast<std::uint64_t>(
                std::numeric_limits<std::uint32_t>::max() / 2U));
  auto const observer_capacity =
      static_cast<std::uint32_t>(2ULL * total_primary_count);

  TransportWorkload complete_workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      random,
      k_single_worker_count,
      snapshot.GetSourceConfiguration(),
      0ULL,
      0U,
      observer_capacity,
      k_chunk_primary_count};
  TransportRunReport complete_report =
      complete_workload.Run(MakeRunConfig(snapshot, 0ULL, total_primary_count));
  ExpectCounters(complete_report, total_primary_count);

  std::array<TransportRunReport, 1U> complete_reports{
      std::move(complete_report)};
  auto complete_source_records =
      ExtractSourceRecords(complete_reports, total_primary_count);
  ExpectSourceAndGroupLookup(snapshot, complete_source_records);

  auto const &source_ranges = snapshot.GetRanges();
  auto const &group_ranges = snapshot.GetGroupRanges();
  ggems::core::random::GGEMSHostRandomStream random_reference{random, 0ULL};
  for (std::uint64_t source_local_primary_id = 0ULL;
       source_local_primary_id < source_ranges[1U].primary_count;
       ++source_local_primary_id) {
    std::uint32_t const time_word = random_reference.NextUInt32();
    std::uint64_t const expected_time =
        ggems::core::radioactivity::SampleRadioactiveTimeFromRaw(
            k_time_window.start_ps, k_time_window.stop_ps,
            snapshot.GetPopulationRecords()[1U].scaled_decay, time_word);
    std::uint64_t const projection_primary_id =
        source_ranges[1U].projection_primary_begin + source_local_primary_id;
    EXPECT_EQ(
        complete_source_records[static_cast<std::size_t>(projection_primary_id)]
            .time_ps,
        expected_time);
  }

  ASSERT_GT(group_ranges[0U].primary_count, 2ULL);
  std::uint64_t const split_primary_offset =
      source_ranges[1U].projection_primary_begin +
      group_ranges[0U].source_local_primary_begin +
      (group_ranges[0U].primary_count / 2ULL);
  ASSERT_GT(split_primary_offset,
            source_ranges[1U].projection_primary_begin +
                group_ranges[0U].source_local_primary_begin);
  ASSERT_LT(split_primary_offset,
            source_ranges[1U].projection_primary_begin +
                group_ranges[0U].source_local_primary_begin +
                group_ranges[0U].primary_count);

  std::uint64_t const first_slice_count = split_primary_offset;
  std::uint64_t const second_slice_count =
      total_primary_count - split_primary_offset;
  auto const first_slice_capacity =
      static_cast<std::uint32_t>(2ULL * first_slice_count);
  auto const second_slice_capacity =
      static_cast<std::uint32_t>(2ULL * second_slice_count);

  TransportWorkload first_slice_workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      random,
      k_worker_count,
      snapshot.GetSourceConfiguration(),
      k_worker_count,
      0U,
      first_slice_capacity,
      k_chunk_primary_count};
  TransportWorkload second_slice_workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      random,
      k_worker_count,
      snapshot.GetSourceConfiguration(),
      2ULL * k_worker_count,
      1U,
      second_slice_capacity,
      k_chunk_primary_count};

  std::array<TransportRunReport, 2U> slice_reports{
      first_slice_workload.Run(
          MakeRunConfig(snapshot, 0ULL, first_slice_count)),
      second_slice_workload.Run(
          MakeRunConfig(snapshot, split_primary_offset, second_slice_count))};
  ExpectCounters(slice_reports[0U], first_slice_count);
  ExpectCounters(slice_reports[1U], second_slice_count);

  auto sliced_source_records =
      ExtractSourceRecords(slice_reports, total_primary_count);
  ExpectSourceAndGroupLookup(snapshot, sliced_source_records);

  constexpr std::uint64_t k_first_stream_offset{512ULL};
  constexpr std::uint64_t k_second_stream_offset{1'024ULL};
  constexpr std::uint64_t k_stream_primary_count{3ULL};
  constexpr std::uint32_t k_stream_observer_capacity{
      2U * static_cast<std::uint32_t>(k_stream_primary_count)};
  ASSERT_GE(group_ranges[0U].primary_count, k_stream_primary_count);

  std::uint64_t const stream_projection_offset =
      source_ranges[1U].projection_primary_begin +
      group_ranges[0U].source_local_primary_begin;
  TransportWorkload first_stream_workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      random,
      k_single_worker_count,
      snapshot.GetSourceConfiguration(),
      k_first_stream_offset,
      2U,
      k_stream_observer_capacity,
      k_chunk_primary_count};
  TransportWorkload second_stream_workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      random,
      k_single_worker_count,
      snapshot.GetSourceConfiguration(),
      k_second_stream_offset,
      3U,
      k_stream_observer_capacity,
      k_chunk_primary_count};

  HostRandomStream first_stream_reference{random, k_first_stream_offset};
  HostRandomStream second_stream_reference{random, k_second_stream_offset};
  float const scaled_decay = snapshot.GetPopulationRecords()[1U].scaled_decay;

  auto const first_stream_first_report =
      first_stream_workload.Run(MakeRunConfig(
          snapshot, stream_projection_offset, k_stream_primary_count));
  auto const second_stream_first_report =
      second_stream_workload.Run(MakeRunConfig(
          snapshot, stream_projection_offset, k_stream_primary_count));
  auto const expected_first_stream_first_times = BuildExpectedSourceTimes(
      first_stream_reference, k_stream_primary_count, scaled_decay);
  auto const expected_second_stream_first_times = BuildExpectedSourceTimes(
      second_stream_reference, k_stream_primary_count, scaled_decay);

  ASSERT_NE(expected_first_stream_first_times,
            expected_second_stream_first_times);
  ExpectCounters(first_stream_first_report, k_stream_primary_count);
  ExpectCounters(second_stream_first_report, k_stream_primary_count);
  EXPECT_EQ(CollectSourceTimes(first_stream_first_report),
            expected_first_stream_first_times);
  EXPECT_EQ(CollectSourceTimes(second_stream_first_report),
            expected_second_stream_first_times);

  auto const first_stream_second_report =
      first_stream_workload.Run(MakeRunConfig(
          snapshot, stream_projection_offset, k_stream_primary_count));
  auto const second_stream_second_report =
      second_stream_workload.Run(MakeRunConfig(
          snapshot, stream_projection_offset, k_stream_primary_count));
  auto const expected_first_stream_second_times = BuildExpectedSourceTimes(
      first_stream_reference, k_stream_primary_count, scaled_decay);
  auto const expected_second_stream_second_times = BuildExpectedSourceTimes(
      second_stream_reference, k_stream_primary_count, scaled_decay);

  ASSERT_NE(expected_first_stream_first_times,
            expected_first_stream_second_times);
  ASSERT_NE(expected_second_stream_first_times,
            expected_second_stream_second_times);
  ASSERT_NE(expected_first_stream_second_times,
            expected_second_stream_second_times);
  ExpectCounters(first_stream_second_report, k_stream_primary_count);
  ExpectCounters(second_stream_second_report, k_stream_primary_count);
  EXPECT_EQ(CollectSourceTimes(first_stream_second_report),
            expected_first_stream_second_times);
  EXPECT_EQ(CollectSourceTimes(second_stream_second_report),
            expected_second_stream_second_times);
}
