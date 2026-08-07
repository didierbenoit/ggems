#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <utility>
#include <span>
#include <string_view>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace {

// =============================================================================
// =============================================================================

using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using ParticleStatus = ggems::core::particles::GGEMSParticleStatus;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;
using SourceConfigurationSnapshotPtr =
    ggems::core::sources::GGEMSSourceConfigurationSnapshotPtr;

constexpr std::int64_t k_one_meter_pm{1'000'000'000'000LL};

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeSourceRecord(std::array<std::int64_t, 3U> const &position,
                 std::array<double, 3U> const &direction,
                 ParticleType particle_type = ParticleType::Aionino)
    -> SourceRecord {
  ggems::core::sources::GGEMSSource source{};

  source.SetAnalytic()
      .SetEmittedParticleType(particle_type)
      .SetEnergyMilliElectronVolt(5'000'000'000ULL)
      .SetPositionPicoMeter(position[0U], position[1U], position[2U])
      .SetDirection(direction[0U], direction[1U], direction[2U])
      .SetWeight(0.75F);

  SourceRecord record = source.BuildRecord();
  record.time_start_ps = 123ULL;
  record.time_stop_ps = 123ULL;
  return record;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoSourceConfiguration(std::size_t source_count)
    -> SourceConfigurationSnapshotPtr {
  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>> sources;
  sources.reserve(source_count);

  for (std::size_t source_index = 0U; source_index < source_count;
       ++source_index) {
    sources.push_back(std::make_shared<ggems::core::sources::GGEMSSource>());
  }

  return ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRanges(std::span<std::uint64_t const> primary_counts)
    -> std::vector<SourceRunRange> {
  std::vector<SourceRunRange> ranges;
  ranges.reserve(primary_counts.size());

  std::uint64_t begin{0ULL};

  for (std::uint64_t count : primary_counts) {
    ranges.push_back(
        {.projection_primary_begin = begin, .primary_count = count});
    begin += count;
  }

  return ranges;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeConfig(std::vector<SourceRecord> records,
                              std::span<std::uint64_t const> primary_counts,
                              std::uint32_t workload_primary_count,
                              std::uint64_t projection_history_offset = 0ULL,
                              std::uint64_t device_primary_offset = 0ULL)
    -> TransportRunConfig {
  TransportRunConfig config{};
  config.total_primary_count = workload_primary_count;
  config.projection_history_offset = projection_history_offset;
  config.device_primary_offset = device_primary_offset;
  config.source_records = std::move(records);
  config.source_population_records.resize(config.source_records.size());
  config.source_ranges = BuildRanges(primary_counts);
  return config;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindHistoryRecords(TransportRunReport const &report,
                                      std::uint64_t global_primary_id)
    -> std::vector<ObserverRecord> {
  std::vector<ObserverRecord> records;

  for (ObserverRecord const &record : report.observer_records) {
    if (record.global_primary_id == global_primary_id) {
      records.push_back(record);
    }
  }

  std::ranges::sort(
      records,
      [](ObserverRecord const &left, ObserverRecord const &right) -> bool {
        return std::tie(left.record_kind, left.track_id) <
               std::tie(right.record_kind, right.track_id);
      });

  return records;
}

// =============================================================================
// =============================================================================

auto ExpectMinimalCounters(TransportRunReport const &report,
                           std::uint32_t expected_primary_count) -> void {
  EXPECT_EQ(report.counters.consumed_primary_count, expected_primary_count);
  EXPECT_EQ(report.counters.completed_history_count, expected_primary_count);
  EXPECT_EQ(report.counters.terminal_particle_count, expected_primary_count);
  EXPECT_EQ(report.counters.created_secondary_count, 0U);
  EXPECT_EQ(report.counters.aionino_to_gamma_count, 0U);
  EXPECT_EQ(report.counters.gamma_to_electron_count, 0U);
  EXPECT_EQ(report.counters.electron_to_electron_count, 0U);
  EXPECT_EQ(report.counters.overflow_count, 0U);
  EXPECT_EQ(report.counters.max_stack_depth, 0U);
  EXPECT_EQ(report.counters.total_fake_step_count, 0U);
}

// =============================================================================
// =============================================================================

auto ExpectHistory(TransportRunReport const &report, SourceRecord const &source,
                   std::uint64_t run_id, std::uint64_t global_primary_id,
                   std::uint32_t source_index,
                   std::uint64_t source_local_primary_id,
                   std::array<std::int64_t, 3U> const &terminal_position)
    -> void {
  auto records = FindHistoryRecords(report, global_primary_id);

  ASSERT_EQ(records.size(), 2U);

  ObserverRecord const &source_record = records[0U];
  ObserverRecord const &terminal_record = records[1U];

  EXPECT_EQ(source_record.record_kind,
            ggems::core::observer::ToKernelObserverRecordKind(
                ObserverRecordKind::Source));
  EXPECT_EQ(terminal_record.record_kind,
            ggems::core::observer::ToKernelObserverRecordKind(
                ObserverRecordKind::Terminal));

  EXPECT_EQ(
      source_record.status,
      ggems::core::particles::ToKernelParticleStatus(ParticleStatus::Alive));
  EXPECT_EQ(
      terminal_record.status,
      ggems::core::particles::ToKernelParticleStatus(ParticleStatus::Killed));

  for (ObserverRecord const &record : records) {
    EXPECT_EQ(record.run_id, run_id);
    EXPECT_EQ(record.global_primary_id, global_primary_id);
    EXPECT_EQ(record.global_particle_id, global_primary_id);
    EXPECT_EQ(record.track_id, 0ULL);
    EXPECT_EQ(record.parent_track_id, ggems::core::particles::k_invalid_id_u64);
    EXPECT_EQ(record.source_index, source_index);
    EXPECT_EQ(record.source_local_primary_id, source_local_primary_id);
    EXPECT_EQ(record.particle_type, source.emitted_particle_type);
    EXPECT_EQ(record.time_ps, source.time_start_ps);
    EXPECT_EQ(record.generation, 0U);
    EXPECT_FLOAT_EQ(record.direction_x, source.axis_z_x);
    EXPECT_FLOAT_EQ(record.direction_y, source.axis_z_y);
    EXPECT_FLOAT_EQ(record.direction_z, source.axis_z_z);
    EXPECT_FLOAT_EQ(record.direction_w, 0.0F);
    EXPECT_EQ(record.energy_milli_eV, source.energy_milli_eV);
    EXPECT_EQ(record.deposited_energy_milli_eV, 0ULL);
    EXPECT_FLOAT_EQ(record.weight, source.weight);
  }

  EXPECT_EQ(source_record.position_x_pm, source.position_x_pm);
  EXPECT_EQ(source_record.position_y_pm, source.position_y_pm);
  EXPECT_EQ(source_record.position_z_pm, source.position_z_pm);

  EXPECT_EQ(terminal_record.position_x_pm, terminal_position[0U]);
  EXPECT_EQ(terminal_record.position_y_pm, terminal_position[1U]);
  EXPECT_EQ(terminal_record.position_z_pm, terminal_position[2U]);
}

// =============================================================================
// =============================================================================

class GGEMSTransportWorkloadTest : public ::testing::Test {
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

TEST_F(GGEMSTransportWorkloadTest,
       ProjectsSixCardinalSourcesAndPreservesState) {
  constexpr std::array<std::array<std::int64_t, 3U>, 6U> k_positions{{
      {-1'500'000'000'000LL, 0LL, 0LL},
      {1'500'000'000'000LL, 0LL, 0LL},
      {0LL, -1'500'000'000'000LL, 0LL},
      {0LL, 1'500'000'000'000LL, 0LL},
      {0LL, 0LL, -1'500'000'000'000LL},
      {0LL, 0LL, 1'500'000'000'000LL},
  }};

  constexpr std::array<std::array<double, 3U>, 6U> k_directions{{
      {1.0, 0.0, 0.0},
      {-1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, -1.0, 0.0},
      {0.0, 0.0, 1.0},
      {0.0, 0.0, -1.0},
  }};

  constexpr std::array<std::array<std::int64_t, 3U>, 6U> k_terminals{{
      {-500'000'000'000LL, 0LL, 0LL},
      {500'000'000'000LL, 0LL, 0LL},
      {0LL, -500'000'000'000LL, 0LL},
      {0LL, 500'000'000'000LL, 0LL},
      {0LL, 0LL, -500'000'000'000LL},
      {0LL, 0LL, 500'000'000'000LL},
  }};

  std::vector<SourceRecord> source_records;

  for (std::size_t index = 0U; index < k_positions.size(); ++index) {
    source_records.push_back(
        MakeSourceRecord(k_positions[index], k_directions[index]));
  }

  constexpr std::array<std::uint64_t, 6U> k_counts{1ULL, 1ULL, 1ULL,
                                                   1ULL, 1ULL, 1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(7'777'777ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             65U,
                             *MakeMonoSourceConfiguration(6U),
                             0ULL,
                             0U,
                             12U};

  auto config = MakeConfig(source_records, k_counts, 6U, 100ULL);
  config.run_id = 9ULL;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;

  auto const report = workload.Run(config);

  ExpectMinimalCounters(report, 6U);
  EXPECT_EQ(report.observer_counters.captured_primary_count, 6U);
  EXPECT_EQ(report.observer_counters.record_count, 12U);
  EXPECT_EQ(report.observer_counters.overflow_count, 0U);

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    ExpectHistory(report, source_records[index], config.run_id, 100ULL + index,
                  static_cast<std::uint32_t>(index), 0ULL, k_terminals[index]);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest, ProjectsStoredBinary32DiagonalExactly) {
  SourceRecord const source =
      MakeSourceRecord({0LL, 0LL, 0LL}, {1.0, 1.0, 1.0});

  EXPECT_FLOAT_EQ(source.axis_z_x, 0.5773502588272095F);
  EXPECT_FLOAT_EQ(source.axis_z_y, 0.5773502588272095F);
  EXPECT_FLOAT_EQ(source.axis_z_z, 0.5773502588272095F);

  constexpr std::array<std::uint64_t, 1U> k_counts{1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(1ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(1U),
                             0ULL,
                             0U,
                             2U};

  auto config = MakeConfig({source}, k_counts, 1U);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;

  auto const report = workload.Run(config);

  ExpectHistory(report, source, 0ULL, 0ULL, 0U, 0ULL,
                {577'350'258'827LL, 577'350'258'827LL, 577'350'258'827LL});
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       InitializesConfiguredParticleTypeGenerically) {
  SourceRecord const source = MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 0.0, 1.0},
                                               ParticleType::Electron);

  constexpr std::array<std::uint64_t, 1U> k_counts{1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("pcg32").SetSeed(2ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(1U),
                             0ULL,
                             0U,
                             2U};

  auto config = MakeConfig({source}, k_counts, 1U);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;

  auto const report = workload.Run(config);

  ExpectHistory(report, source, 0ULL, 0ULL, 0U, 0ULL,
                {0LL, 0LL, k_one_meter_pm});
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       PreservesZeroPrimaryAndDuplicateSourceSlots) {
  SourceRecord const source_a =
      MakeSourceRecord({0LL, 0LL, 0LL}, {1.0, 0.0, 0.0});
  SourceRecord const source_b =
      MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, -1.0, 0.0});

  constexpr std::array<std::uint64_t, 4U> k_counts{2ULL, 0ULL, 2ULL, 1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(3ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             65U,
                             *MakeMonoSourceConfiguration(4U),
                             0ULL,
                             0U,
                             10U};

  auto config = MakeConfig({source_a, source_b, source_a, source_b}, k_counts,
                           5U, 1'000ULL);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 8U;

  auto const report = workload.Run(config);

  ExpectMinimalCounters(report, 5U);
  EXPECT_EQ(report.observer_counters.captured_primary_count, 5U);

  ExpectHistory(report, source_a, 0ULL, 1'000ULL, 0U, 0ULL,
                {k_one_meter_pm, 0LL, 0LL});
  ExpectHistory(report, source_a, 0ULL, 1'001ULL, 0U, 1ULL,
                {k_one_meter_pm, 0LL, 0LL});
  ExpectHistory(report, source_a, 0ULL, 1'002ULL, 2U, 0ULL,
                {k_one_meter_pm, 0LL, 0LL});
  ExpectHistory(report, source_a, 0ULL, 1'003ULL, 2U, 1ULL,
                {k_one_meter_pm, 0LL, 0LL});
  ExpectHistory(report, source_b, 0ULL, 1'004ULL, 3U, 0ULL,
                {0LL, -k_one_meter_pm, 0LL});
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       CombinesObserverPoliciesWithoutDuplicateHistories) {
  SourceRecord const source_0 =
      MakeSourceRecord({0LL, 0LL, 0LL}, {1.0, 0.0, 0.0});
  SourceRecord const source_1 =
      MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 0.0, 1.0});

  constexpr std::array<std::uint64_t, 2U> k_counts{3ULL, 2ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(4ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(2U),
                             0ULL,
                             0U,
                             8U};

  auto config = MakeConfig({source_0, source_1}, k_counts, 5U);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;
  config.observer_config.capture_specific_primary_enabled = 1U;
  config.observer_config.capture_source_index = 1U;
  config.observer_config.capture_source_local_primary_id = 1ULL;

  auto report = workload.Run(config);

  EXPECT_EQ(report.observer_counters.captured_primary_count, 3U);
  EXPECT_EQ(report.observer_counters.record_count, 6U);
  EXPECT_EQ(FindHistoryRecords(report, 0ULL).size(), 2U);
  EXPECT_TRUE(FindHistoryRecords(report, 1ULL).empty());
  EXPECT_TRUE(FindHistoryRecords(report, 2ULL).empty());
  EXPECT_EQ(FindHistoryRecords(report, 3ULL).size(), 2U);
  EXPECT_EQ(FindHistoryRecords(report, 4ULL).size(), 2U);

  config.projection_history_offset = 5ULL;
  config.observer_config.capture_source_index = 0U;
  config.observer_config.capture_source_local_primary_id = 0ULL;

  report = workload.Run(config);

  EXPECT_EQ(report.observer_counters.captured_primary_count, 2U);
  EXPECT_EQ(report.observer_counters.record_count, 4U);
  EXPECT_EQ(FindHistoryRecords(report, 5ULL).size(), 2U);
  EXPECT_EQ(FindHistoryRecords(report, 8ULL).size(), 2U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       ObserverOverflowIsIndependentFromTransportOverflow) {
  SourceRecord const source =
      MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 0.0, 1.0});
  constexpr std::array<std::uint64_t, 1U> k_counts{1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(5ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(1U),
                             0ULL,
                             0U,
                             1U};

  auto config = MakeConfig({source}, k_counts, 1U);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;

  auto const report = workload.Run(config);

  ExpectMinimalCounters(report, 1U);
  EXPECT_EQ(report.observer_counters.captured_primary_count, 1U);
  EXPECT_EQ(report.observer_counters.record_count, 1U);
  EXPECT_EQ(report.observer_counters.overflow_count, 1U);
  EXPECT_EQ(report.observer_records.size(), 1U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       PreservesSourceLocalIdsAcrossADeviceSliceBoundary) {
  SourceRecord const source_0 =
      MakeSourceRecord({0LL, 0LL, 0LL}, {1.0, 0.0, 0.0});
  SourceRecord const source_1 =
      MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 1.0, 0.0});
  constexpr std::array<std::uint64_t, 2U> k_counts{2ULL, 3ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(6ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             65U,
                             *MakeMonoSourceConfiguration(2U),
                             0ULL,
                             0U,
                             6U};

  auto config = MakeConfig({source_0, source_1}, k_counts, 3U, 100ULL, 1ULL);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 3U;

  auto const report = workload.Run(config);

  ExpectHistory(report, source_0, 0ULL, 101ULL, 0U, 1ULL,
                {k_one_meter_pm, 0LL, 0LL});
  ExpectHistory(report, source_1, 0ULL, 102ULL, 1U, 0ULL,
                {0LL, k_one_meter_pm, 0LL});
  ExpectHistory(report, source_1, 0ULL, 103ULL, 1U, 1ULL,
                {0LL, k_one_meter_pm, 0LL});
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       RunsAllRandomEnginesTwiceWithoutAdditionalSVMAllocation) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint64_t, 1U> k_counts{2ULL};

  SourceRecord const source =
      MakeSourceRecord({0LL, 0LL, 1'500'000'000'000LL}, {0.0, 0.0, -1.0});

  for (std::string_view engine : k_engines) {
    SCOPED_TRACE(engine);

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(7ULL);

    TransportWorkload workload{GetContext(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               65U,
                               *MakeMonoSourceConfiguration(1U),
                               1'024ULL,
                               0U,
                               4U};

    auto const allocated_after_construction =
        GetContext().GetAllocatedVRAM().value;
    auto const allocation_count_after_construction =
        GetContext().GetAllocationCountVRAM();

    auto config = MakeConfig({source}, k_counts, 2U, 200ULL);
    config.observer_config.enabled = 1U;
    config.observer_config.capture_first_primary_count_per_source = 2U;

    auto first_report = workload.Run(config);
    ExpectMinimalCounters(first_report, 2U);
    ExpectHistory(first_report, source, 0ULL, 200ULL, 0U, 0ULL,
                  {0LL, 0LL, 500'000'000'000LL});

    EXPECT_EQ(GetContext().GetAllocatedVRAM().value,
              allocated_after_construction);
    EXPECT_EQ(GetContext().GetAllocationCountVRAM(),
              allocation_count_after_construction);

    config.run_id = 1ULL;
    config.projection_history_offset = 202ULL;

    auto second_report = workload.Run(config);
    ExpectMinimalCounters(second_report, 2U);
    ExpectHistory(second_report, source, 1ULL, 202ULL, 0U, 0ULL,
                  {0LL, 0LL, 500'000'000'000LL});

    EXPECT_EQ(GetContext().GetAllocatedVRAM().value,
              allocated_after_construction);
    EXPECT_EQ(GetContext().GetAllocationCountVRAM(),
              allocation_count_after_construction);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       IgnoresUnsupportedZeroPrimarySlotButRejectsItWhenNonZero) {
  SourceRecord unsupported = MakeSourceRecord(
      {std::numeric_limits<std::int64_t>::max(), 0LL, 0LL}, {1.0, 0.0, 0.0});
  unsupported.source_type = ggems::core::sources::ToKernelSourceType(
      ggems::core::sources::GGEMSSourceType::Voxelized);

  SourceRecord const analytic =
      MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 0.0, 1.0});

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(8ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(2U),
                             0ULL,
                             0U,
                             2U};

  constexpr std::array<std::uint64_t, 2U> k_zero_then_active{0ULL, 1ULL};

  auto config = MakeConfig({unsupported, analytic}, k_zero_then_active, 1U);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;

  auto const report = workload.Run(config);
  ExpectHistory(report, analytic, 0ULL, 0ULL, 1U, 0ULL,
                {0LL, 0LL, k_one_meter_pm});

  constexpr std::array<std::uint64_t, 2U> k_unsupported_active{1ULL, 0ULL};
  config.source_ranges = BuildRanges(k_unsupported_active);

  EXPECT_THROW(workload.Run(config), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSTransportWorkloadTest,
       RejectsEndpointOverflowAndInvalidSourceRanges) {
  SourceRecord const overflowing = MakeSourceRecord(
      {std::numeric_limits<std::int64_t>::max(), 0LL, 0LL}, {1.0, 0.0, 0.0});
  constexpr std::array<std::uint64_t, 1U> k_counts{1ULL};

  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(9ULL);

  TransportWorkload workload{GetContext(),
                             std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                             random,
                             64U,
                             *MakeMonoSourceConfiguration(1U),
                             0ULL,
                             0U,
                             2U};

  auto config = MakeConfig({overflowing}, k_counts, 1U);
  EXPECT_THROW(workload.Run(config), ggems::core::GGEMSExceptionBase);

  SourceRecord const valid = MakeSourceRecord({0LL, 0LL, 0LL}, {0.0, 0.0, 1.0});

  config = MakeConfig({valid}, k_counts, 1U);
  config.source_ranges.clear();
  EXPECT_THROW(workload.Run(config), ggems::core::GGEMSExceptionBase);

  config = MakeConfig({valid}, k_counts, 1U, 0ULL, 1ULL);
  EXPECT_THROW(workload.Run(config), ggems::core::GGEMSExceptionBase);
}

TEST_F(GGEMSTransportWorkloadTest, RejectsZeroStableSourceCount) {
  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(10ULL);

  auto construct = [&]() -> void {
    TransportWorkload workload{GetContext(),
                               std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
                               random,
                               64U,
                               *MakeMonoSourceConfiguration(0U),
                               0ULL,
                               0U,
                               2U};
  };

  EXPECT_THROW(construct(), ggems::core::GGEMSExceptionBase);
}
