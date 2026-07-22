#include <cstdint>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>
#include <vector>
#include <functional>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace {

constexpr std::uint32_t k_worker_count{256U};
constexpr std::uint32_t k_total_primary_count{4'096U};

constexpr std::uint32_t k_source_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Source);

constexpr std::uint32_t k_step_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Step);

constexpr std::uint32_t k_secondary_step_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::SecondaryStep);

constexpr std::uint32_t k_terminal_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Terminal);

// =============================================================================
// =============================================================================

auto BuildSortedSourceRecords(
    std::vector<ggems::core::observer::GGEMSObserverRecord> const &records)
    -> std::vector<ggems::core::observer::GGEMSObserverRecord> {
  std::vector<ggems::core::observer::GGEMSObserverRecord> result;

  for (auto const &record : records) {
    if (record.record_kind == k_source_record_kind) {
      result.push_back(record);
    }
  }

  std::ranges::sort(
      result, std::ranges::less{},
      &ggems::core::observer::GGEMSObserverRecord::global_primary_id);

  return result;
}

// =============================================================================
// =============================================================================

auto ExpectSourceMatches(
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

class GGEMSDummyTransportWorkloadTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto GetContext() -> ggems::ocl::GGEMSOpenCLContext & {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    return opencl.GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest, RunsBranchingAioninoPrototype) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000'000ULL)
      .SetPositionPicoMeter(0ULL, 0ULL, 0ULL)
      .SetDirection(0.0F, 0.0F, 1.0F)
      .SetWeight(1.0F);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      k_worker_count,
      1U,           0ULL,
      0U,           1U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = k_total_primary_count;
  config.projection_history_offset = 10'000'000ULL;
  config.device_primary_offset = 200'000ULL;
  config.source_records = {source.BuildRecord()};
  config.source_ranges = {{.projection_primary_begin = 0ULL,
                           .primary_count = config.device_primary_offset +
                                            config.total_primary_count}};

  auto report = workload.Run(config);
  auto const &counters = report.counters;
  auto const &observer_counters = report.observer_counters;

  EXPECT_GT(report.host_time.value, 0U);
  EXPECT_GT(report.kernel_time.value, 0U);
  EXPECT_GT(report.command_time.value, 0U);

  EXPECT_GT(report.host_histories_per_second, 0.0);
  EXPECT_GT(report.kernel_histories_per_second, 0.0);

  EXPECT_GT(report.host_terminal_particles_per_second, 0.0);
  EXPECT_GT(report.kernel_terminal_particles_per_second, 0.0);

  EXPECT_EQ(counters.consumed_primary_count, k_total_primary_count);
  EXPECT_EQ(counters.completed_history_count, k_total_primary_count);
  EXPECT_EQ(counters.aionino_to_gamma_count, k_total_primary_count);

  EXPECT_GT(counters.created_secondary_count, 0U);
  EXPECT_GT(counters.gamma_to_electron_count, 0U);
  EXPECT_GT(counters.electron_to_electron_count, 0U);

  EXPECT_EQ(counters.created_secondary_count,
            counters.gamma_to_electron_count +
                counters.electron_to_electron_count);

  EXPECT_EQ(counters.terminal_particle_count,
            counters.consumed_primary_count + counters.created_secondary_count);

  EXPECT_EQ(counters.overflow_count, 0U);

  EXPECT_GT(counters.max_stack_depth, 0U);
  EXPECT_LE(counters.max_stack_depth, 16U);

  EXPECT_GT(counters.total_fake_step_count, k_total_primary_count);

  EXPECT_EQ(observer_counters.record_count, 0U);
  EXPECT_EQ(observer_counters.overflow_count, 0U);
  EXPECT_EQ(observer_counters.captured_primary_count, 0U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       GuardsPaddedWorkItemsForRoundedGlobalSize) {
  constexpr std::uint32_t k_non_multiple_worker_count{257U};
  constexpr std::uint32_t k_test_primary_count{320U};

  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      k_non_multiple_worker_count,
      1U,           0ULL,
      0U,           1U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = k_test_primary_count;
  config.max_generation = 0U;
  config.source_records = {ggems::core::sources::GGEMSSourceRecord{}};
  config.source_ranges = {{.projection_primary_begin = 0ULL,
                           .primary_count = k_test_primary_count}};

  auto report = workload.Run(config);
  auto const &counters = report.counters;

  EXPECT_EQ(workload.GetWorkerCount(), k_non_multiple_worker_count);
  EXPECT_EQ(counters.next_primary_id,
            k_test_primary_count + k_non_multiple_worker_count);
  EXPECT_EQ(counters.consumed_primary_count, k_test_primary_count);
  EXPECT_EQ(counters.completed_history_count, k_test_primary_count);
  EXPECT_EQ(counters.terminal_particle_count, k_test_primary_count);
  EXPECT_EQ(counters.created_secondary_count, 0U);
  EXPECT_EQ(counters.overflow_count, 0U);
  EXPECT_EQ(report.observer_counters.overflow_count, 0U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest, CapturesFirstPrimaryHistories) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000'000ULL)
      .SetPositionPicoMeter(0ULL, 0ULL, 0ULL)
      .SetDirection(0.0F, 0.0F, 1.0F)
      .SetWeight(1.0F);

  constexpr std::uint32_t k_observed_primary_count{2U};
  constexpr std::uint32_t k_observer_record_capacity{4'096U};
  constexpr std::uint64_t k_run_id{42ULL};
  constexpr std::uint64_t k_projection_history_offset{10'000'000ULL};

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      k_worker_count,
      1U,           0ULL,
      0U,           k_observer_record_capacity};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.run_id = k_run_id;
  config.total_primary_count = 64U;
  config.projection_history_offset = k_projection_history_offset;
  config.device_primary_offset = 0ULL;
  config.source_records = {source.BuildRecord()};
  config.source_ranges = {{.projection_primary_begin = 0ULL,
                           .primary_count = config.total_primary_count}};
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = k_observed_primary_count;

  auto report = workload.Run(config);

  auto const &observer_counters = report.observer_counters;
  auto const &records = report.observer_records;

  EXPECT_EQ(observer_counters.captured_primary_count, k_observed_primary_count);
  EXPECT_GT(observer_counters.record_count, k_observed_primary_count);
  EXPECT_EQ(observer_counters.overflow_count, 0U);

  EXPECT_EQ(records.size(),
            static_cast<std::size_t>(observer_counters.record_count));

  bool has_source_record{false};
  bool has_step_record{false};
  bool has_terminal_record{false};

  std::uint32_t source_record_count{0U};

  for (auto const &record : records) {
    EXPECT_EQ(record.run_id, k_run_id);
    EXPECT_EQ(record.deposited_energy_milli_eV, 0ULL);

    EXPECT_TRUE(record.global_primary_id == k_projection_history_offset ||
                record.global_primary_id == k_projection_history_offset + 1ULL);

    if (record.record_kind ==
        ggems::core::observer::ToKernelObserverRecordKind(
            ggems::core::observer::GGEMSObserverRecordKind::Source)) {
      has_source_record = true;
      ++source_record_count;
    }

    if (record.record_kind ==
        ggems::core::observer::ToKernelObserverRecordKind(
            ggems::core::observer::GGEMSObserverRecordKind::Step)) {
      has_step_record = true;
    }

    if (record.record_kind ==
        ggems::core::observer::ToKernelObserverRecordKind(
            ggems::core::observer::GGEMSObserverRecordKind::Terminal)) {
      has_terminal_record = true;
    }
  }

  EXPECT_EQ(source_record_count, k_observed_primary_count);
  EXPECT_TRUE(has_source_record);
  EXPECT_TRUE(has_step_record);
  EXPECT_TRUE(has_terminal_record);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       RecordsZeroDepositForLosslessMoveAndArtificialTermination) {
  constexpr std::uint64_t k_source_energy_milli_eV{511'000'001ULL};

  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEnergyMilliElectronVolt(k_source_energy_milli_eV)
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      1U,
      1U,           0ULL,
      0U,           8U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = 1U;
  config.source_records = {source.BuildRecord()};
  config.source_ranges = {
      {.projection_primary_begin = 0ULL, .primary_count = 1ULL}};
  config.max_generation = 0U;
  config.max_steps_per_track = 1U;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = 1U;

  auto const report = workload.Run(config);
  auto const &records = report.observer_records;

  ASSERT_EQ(records.size(), 3U);
  EXPECT_EQ(report.observer_counters.record_count, 3U);
  EXPECT_EQ(report.observer_counters.overflow_count, 0U);
  EXPECT_EQ(report.counters.created_secondary_count, 0U);
  EXPECT_EQ(report.counters.overflow_count, 0U);

  EXPECT_EQ(records[0U].record_kind, k_source_record_kind);
  EXPECT_EQ(records[1U].record_kind, k_step_record_kind);
  EXPECT_EQ(records[2U].record_kind, k_terminal_record_kind);

  for (auto const &record : records) {
    EXPECT_EQ(record.energy_milli_eV, k_source_energy_milli_eV);
    EXPECT_EQ(record.deposited_energy_milli_eV, 0ULL);
  }

  EXPECT_EQ(records[2U].status,
            ggems::core::particles::ToKernelParticleStatus(
                ggems::core::particles::GGEMSParticleStatus::Killed));

  EXPECT_EQ(records[0U].energy_milli_eV,
            records[1U].energy_milli_eV +
                records[1U].deposited_energy_milli_eV);

  EXPECT_EQ(records[1U].energy_milli_eV,
            records[2U].energy_milli_eV +
                records[2U].deposited_energy_milli_eV);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       SelectsRangesAcrossDeviceSliceWithoutPerRunAllocations) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source_a{};
  source_a.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(100ULL, 100ULL)
      .SetPositionPicoMeter(10LL, 20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  ggems::core::sources::GGEMSSource disabled_source{};
  disabled_source.SetAnalytic()
      .SetEnergyMilliElectronVolt(3'000'000ULL)
      .SetTimeWindowPicoSecond(300ULL, 300ULL)
      .SetPositionPicoMeter(40LL, 50LL, 60LL);

  ggems::core::sources::GGEMSSource source_c{};
  source_c.SetAnalytic()
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetTimeWindowPicoSecond(200ULL, 200ULL)
      .SetPositionPicoMeter(-70LL, 80LL, -90LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_a = source_a.BuildRecord();
  auto disabled_record = disabled_source.BuildRecord();
  auto expected_c = source_c.BuildRecord();

  constexpr std::uint32_t k_observer_capacity{64U};
  constexpr std::uint64_t k_global_begin{10'000'000ULL};

  auto &context = GetContext();

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random, 4U,
      3U,      0ULL,
      0U,      k_observer_capacity};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.run_id = 91ULL;
  config.total_primary_count = 5U;
  config.projection_history_offset = k_global_begin;
  config.device_primary_offset = 1ULL;
  config.source_records = {expected_a, disabled_record, expected_c};
  config.source_ranges = {
      {.projection_primary_begin = 0ULL, .primary_count = 3ULL},
      {.projection_primary_begin = 3ULL, .primary_count = 0ULL},
      {.projection_primary_begin = 3ULL, .primary_count = 5ULL}};
  config.max_generation = 0U;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = 8U;

  auto allocated_after_construction = context.GetAllocatedVRAM().value;
  auto allocation_count_after_construction = context.GetAllocationCountVRAM();

  auto report = workload.Run(config);

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
  EXPECT_EQ(context.GetAllocationCountVRAM(),
            allocation_count_after_construction);

  EXPECT_EQ(report.counters.consumed_primary_count, 5U);
  EXPECT_EQ(report.counters.completed_history_count, 5U);
  EXPECT_EQ(report.counters.overflow_count, 0U);
  EXPECT_EQ(report.observer_counters.captured_primary_count, 5U);

  auto source_records = BuildSortedSourceRecords(report.observer_records);

  ASSERT_EQ(source_records.size(), 5U);

  constexpr std::array<std::uint32_t, 5U> k_expected_source_indices{0U, 0U, 2U,
                                                                    2U, 2U};

  constexpr std::array<std::uint64_t, 5U> k_expected_source_local_ids{
      1ULL, 2ULL, 0ULL, 1ULL, 2ULL};

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].run_id, 91ULL);
    EXPECT_EQ(source_records[index].global_primary_id,
              k_global_begin + 1ULL + static_cast<std::uint64_t>(index));
    EXPECT_EQ(source_records[index].source_index,
              k_expected_source_indices[index]);
    EXPECT_EQ(source_records[index].source_local_primary_id,
              k_expected_source_local_ids[index]);
    EXPECT_NE(source_records[index].energy_milli_eV,
              disabled_record.energy_milli_eV);

    ExpectSourceMatches(source_records[index],
                        index < 2U ? expected_a : expected_c);
  }

  config.total_primary_count = 8U;
  config.device_primary_offset = 0ULL;

  auto second_report = workload.Run(config);

  EXPECT_EQ(second_report.counters.completed_history_count, 8U);
  EXPECT_EQ(second_report.observer_counters.captured_primary_count, 8U);

  auto second_source_records =
      BuildSortedSourceRecords(second_report.observer_records);

  ASSERT_EQ(second_source_records.size(), 8U);

  for (std::uint64_t projection_primary_id = 0ULL; projection_primary_id < 8ULL;
       ++projection_primary_id) {
    auto const &record =
        second_source_records[static_cast<std::size_t>(projection_primary_id)];

    std::uint32_t expected_source_index =
        projection_primary_id < 3ULL ? 0U : 2U;

    std::uint64_t expected_source_local_primary_id =
        projection_primary_id < 3ULL ? projection_primary_id
                                     : projection_primary_id - 3ULL;

    EXPECT_EQ(record.global_primary_id, k_global_begin + projection_primary_id);
    EXPECT_EQ(record.source_index, expected_source_index);
    EXPECT_EQ(record.source_local_primary_id, expected_source_local_primary_id);
    EXPECT_NE(record.energy_milli_eV, disabled_record.energy_milli_eV);

    ExpectSourceMatches(record,
                        expected_source_index == 0U ? expected_a : expected_c);
  }

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
  EXPECT_EQ(context.GetAllocationCountVRAM(),
            allocation_count_after_construction);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       RejectsInvalidArraysAndReportsUnmatchedProjectionPrimary) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  auto &context = GetContext();

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random, 1U,
      1U,      0ULL,
      0U,      8U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = 1U;
  config.projection_history_offset = 10'000'000ULL;
  config.source_records = {ggems::core::sources::GGEMSSourceRecord{}};
  config.source_ranges = {
      {.projection_primary_begin = 1ULL, .primary_count = 1ULL}};
  config.max_generation = 0U;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = 1U;

  auto allocated_after_construction = context.GetAllocatedVRAM().value;
  auto allocation_count_after_construction = context.GetAllocationCountVRAM();

  auto invalid_config = config;
  invalid_config.source_ranges.clear();

  EXPECT_THROW(workload.Run(invalid_config), ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
  EXPECT_EQ(context.GetAllocationCountVRAM(),
            allocation_count_after_construction);

  auto report = workload.Run(config);

  EXPECT_EQ(report.counters.overflow_count, 1U);
  EXPECT_EQ(report.counters.consumed_primary_count, 0U);
  EXPECT_EQ(report.counters.completed_history_count, 0U);
  EXPECT_EQ(report.counters.terminal_particle_count, 0U);

  EXPECT_EQ(report.observer_counters.captured_primary_count, 0U);
  EXPECT_EQ(report.observer_counters.record_count, 0U);
  EXPECT_EQ(report.observer_counters.overflow_count, 0U);
  EXPECT_TRUE(report.observer_records.empty());

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
  EXPECT_EQ(context.GetAllocationCountVRAM(),
            allocation_count_after_construction);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest, RejectsZeroStableSourceCount) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  auto &context = GetContext();

  auto construct_workload = [&]() -> void {
    ggems::core::transport::GGEMSDummyTransportWorkload workload{
        context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
        *random, 1U,
        0U,      0ULL,
        0U,      8U};
  };

  EXPECT_THROW(construct_workload(), ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       RejectsArraysThatDoNotMatchStableSourceCount) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  auto &context = GetContext();

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random, 1U,
      1U,      0ULL,
      0U,      8U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = 1U;

  config.source_records = {ggems::core::sources::GGEMSSourceRecord{},
                           ggems::core::sources::GGEMSSourceRecord{}};

  config.source_ranges = {
      {.projection_primary_begin = 0ULL, .primary_count = 1ULL},
      {.projection_primary_begin = 1ULL, .primary_count = 0ULL}};

  ASSERT_EQ(config.source_records.size(), config.source_ranges.size());
  ASSERT_EQ(config.source_records.size(), 2U);

  auto allocated_before = context.GetAllocatedVRAM().value;
  auto allocation_count_before = context.GetAllocationCountVRAM();

  EXPECT_THROW(workload.Run(config), ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_before);
  EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       RunsEveryRandomEngineAcrossSuccessiveCalls) {
  constexpr std::array<std::string_view, 3U> k_engine_names{"philox", "pcg32",
                                                            "jkiss"};

  constexpr std::uint32_t k_test_worker_count{64U};
  constexpr std::uint32_t k_test_primary_count{128U};
  constexpr std::uint32_t k_captured_primary_count{2U};
  constexpr std::uint32_t k_observer_record_capacity{512U};
  constexpr std::uint64_t k_first_run_id{17ULL};
  constexpr std::uint64_t k_first_history_offset{100'000ULL};
  constexpr std::uint64_t k_random_stream_offset{1'024ULL};

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'001ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000ULL)
      .SetPositionPicoMeter(0LL, 0LL, 0LL)
      .SetDirection(0.0F, 0.0F, 1.0F)
      .SetWeight(1.0F);

  auto source_record = source.BuildRecord();

  auto expect_report =
      [&](ggems::core::transport::GGEMSDummyTransportRunReport const &report,
          std::uint64_t expected_run_id,
          std::uint64_t expected_history_offset) -> void {
    EXPECT_EQ(report.counters.consumed_primary_count, k_test_primary_count);
    EXPECT_EQ(report.counters.completed_history_count, k_test_primary_count);
    EXPECT_EQ(report.counters.aionino_to_gamma_count, k_test_primary_count);

    EXPECT_EQ(report.counters.terminal_particle_count,
              report.counters.consumed_primary_count +
                  report.counters.created_secondary_count);

    EXPECT_EQ(report.counters.overflow_count, 0U);
    EXPECT_LE(report.counters.max_stack_depth, 16U);

    EXPECT_EQ(report.observer_counters.captured_primary_count,
              k_captured_primary_count);
    EXPECT_EQ(report.observer_counters.overflow_count, 0U);
    EXPECT_EQ(report.observer_records.size(),
              static_cast<std::size_t>(report.observer_counters.record_count));

    for (auto const &record : report.observer_records) {
      EXPECT_EQ(record.run_id, expected_run_id);
      EXPECT_GE(record.global_primary_id, expected_history_offset);
      EXPECT_LT(record.global_primary_id,
                expected_history_offset + k_captured_primary_count);
      EXPECT_EQ(record.source_index, 0U);
      EXPECT_EQ(record.source_local_primary_id,
                record.global_primary_id - expected_history_offset);
    }

    auto source_records = BuildSortedSourceRecords(report.observer_records);

    ASSERT_EQ(source_records.size(), k_captured_primary_count);
    EXPECT_EQ(source_records[0U].global_primary_id, expected_history_offset);
    EXPECT_EQ(source_records[1U].global_primary_id,
              expected_history_offset + 1ULL);
  };

  for (std::string_view engine_name : k_engine_names) {
    SCOPED_TRACE(engine_name);

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine(engine_name).SetSeed(7'777'777ULL);

    auto &context = GetContext();

    ggems::core::transport::GGEMSDummyTransportWorkload workload{
        context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
        random,  k_test_worker_count,
        1U,      k_random_stream_offset,
        0U,      k_observer_record_capacity};

    auto allocated_after_construction = context.GetAllocatedVRAM().value;
    auto allocation_count_after_construction = context.GetAllocationCountVRAM();

    ggems::core::transport::GGEMSDummyTransportRunConfig config{};
    config.run_id = k_first_run_id;
    config.total_primary_count = k_test_primary_count;
    config.projection_history_offset = k_first_history_offset;
    config.source_records = {source_record};
    config.source_ranges = {{.projection_primary_begin = 0ULL,
                             .primary_count = k_test_primary_count}};
    config.max_generation = 1U;
    config.observer_config.enabled = 1U;
    config.observer_config.capture_first_primary_count =
        k_captured_primary_count;

    auto first_report = workload.Run(config);
    expect_report(first_report, config.run_id,
                  config.projection_history_offset);

    EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
    EXPECT_EQ(context.GetAllocationCountVRAM(),
              allocation_count_after_construction);

    ++config.run_id;
    config.projection_history_offset += k_test_primary_count;

    auto second_report = workload.Run(config);
    expect_report(second_report, config.run_id,
                  config.projection_history_offset);

    EXPECT_EQ(context.GetAllocatedVRAM().value, allocated_after_construction);
    EXPECT_EQ(context.GetAllocationCountVRAM(),
              allocation_count_after_construction);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSDummyTransportWorkloadTest,
       PropagatesSourceProvenanceToEverySecondaryRecord) {
  constexpr std::uint32_t k_primary_count{128U};
  constexpr std::uint32_t k_observer_capacity{2'048U};
  constexpr std::uint64_t k_global_begin{500'000ULL};

  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000ULL);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      1U,
      1U,           0ULL,
      0U,           k_observer_capacity};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = k_primary_count;
  config.projection_history_offset = k_global_begin;
  config.source_records = {source.BuildRecord()};
  config.source_ranges = {
      {.projection_primary_begin = 0ULL, .primary_count = k_primary_count}};
  config.max_generation = 1U;
  config.max_steps_per_track = 2U;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = k_primary_count;

  auto report = workload.Run(config);

  ASSERT_GT(report.counters.created_secondary_count, 0U);
  ASSERT_EQ(report.observer_counters.overflow_count, 0U);

  auto const &records = report.observer_records;
  bool has_secondary_record{false};

  for (std::size_t record_index = 0U; record_index < records.size();
       ++record_index) {
    auto const &record = records[record_index];
    ASSERT_GE(record.global_primary_id, k_global_begin);
    ASSERT_LT(record.global_primary_id, k_global_begin + k_primary_count);
    EXPECT_EQ(record.deposited_energy_milli_eV, 0ULL);
    EXPECT_EQ(record.source_index, 0U);
    EXPECT_EQ(record.source_local_primary_id,
              +record.global_primary_id - k_global_begin);

    if (record.record_kind != k_secondary_step_record_kind) {
      continue;
    }

    has_secondary_record = true;
    ASSERT_GT(record_index, 0U);

    auto const &parent_before = records[record_index - 1U];

    ASSERT_EQ(parent_before.record_kind, k_step_record_kind);
    ASSERT_EQ(parent_before.global_primary_id, record.global_primary_id);
    ASSERT_EQ(parent_before.track_id, record.parent_track_id);

    ggems::core::observer::GGEMSObserverRecord const *parent_after{nullptr};

    for (std::size_t candidate_index = record_index + 1U;
         candidate_index < records.size(); ++candidate_index) {
      auto const &candidate = records[candidate_index];

      if (candidate.global_primary_id == record.global_primary_id &&
          candidate.track_id == record.parent_track_id &&
          candidate.record_kind == k_step_record_kind) {
        parent_after = &candidate;
        break;
      }
    }

    ASSERT_NE(parent_after, nullptr);
    ASSERT_LE(record.energy_milli_eV, parent_before.energy_milli_eV);

    EXPECT_EQ(parent_before.energy_milli_eV - record.energy_milli_eV,
              parent_after->energy_milli_eV);

    EXPECT_EQ(parent_before.energy_milli_eV,
              parent_after->energy_milli_eV + record.energy_milli_eV +
                  record.deposited_energy_milli_eV);
  }

  EXPECT_TRUE(has_secondary_record);
}
// =============================================================================
// =============================================================================
TEST_F(GGEMSDummyTransportWorkloadTest,
       SamplesTimeFromEachSourceLocalPrimaryId) {
  constexpr std::uint64_t k_time_start_ps{100ULL};
  constexpr std::uint64_t k_time_stop_ps{103ULL};
  constexpr std::uint64_t k_global_begin{10'000'000ULL};

  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7'777'777ULL);

  ggems::core::sources::GGEMSSource source_a{};
  source_a.SetPrimaryCount(4ULL)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(k_time_start_ps, k_time_stop_ps);

  ggems::core::sources::GGEMSSource source_b{};
  source_b.SetPrimaryCount(4ULL)
      .SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(k_time_start_ps, k_time_stop_ps);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,      4U,
      2U,           0ULL,
      0U,           64U};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = 8U;
  config.projection_history_offset = k_global_begin;
  config.source_records = {source_a.BuildRecord(), source_b.BuildRecord()};
  config.source_ranges = {
      {.projection_primary_begin = 0ULL, .primary_count = 4ULL},
      {.projection_primary_begin = 4ULL, .primary_count = 4ULL}};
  config.max_generation = 0U;
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count = 8U;

  auto report = workload.Run(config);

  auto source_records = BuildSortedSourceRecords(report.observer_records);

  ASSERT_EQ(source_records.size(), 8U);

  for (std::uint64_t projection_primary_id = 0ULL; projection_primary_id < 8ULL;
       ++projection_primary_id) {
    auto const &record =
        source_records[static_cast<std::size_t>(projection_primary_id)];

    std::uint32_t expected_source_index =
        projection_primary_id < 4ULL ? 0U : 1U;

    std::uint64_t expected_source_local_primary_id =
        projection_primary_id < 4ULL ? projection_primary_id
                                     : projection_primary_id - 4ULL;

    std::uint64_t expected_time_ps =
        k_time_start_ps +
        (expected_source_local_primary_id % (k_time_stop_ps - k_time_start_ps));

    EXPECT_EQ(record.global_primary_id, k_global_begin + projection_primary_id);
    EXPECT_EQ(record.source_index, expected_source_index);
    EXPECT_EQ(record.source_local_primary_id, expected_source_local_primary_id);
    EXPECT_EQ(record.time_ps, expected_time_ps);
    EXPECT_LT(record.time_ps, k_time_stop_ps);
  }
}
