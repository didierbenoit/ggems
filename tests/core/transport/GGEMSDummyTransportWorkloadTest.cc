#include <cstdint>
#include <filesystem>
#include <memory>
#include <algorithm>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

namespace {

constexpr std::uint32_t k_worker_count{256U};
constexpr std::uint32_t k_total_primary_count{4'096U};

constexpr std::uint32_t k_source_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Source);

// =============================================================================
// =============================================================================

std::vector<ggems::core::observer::GGEMSObserverRecord>
BuildSortedSourceRecords(
    std::vector<ggems::core::observer::GGEMSObserverRecord> const &records) {
  std::vector<ggems::core::observer::GGEMSObserverRecord> result;

  for (auto const &record : records) {
    if (record.record_kind == k_source_record_kind) {
      result.push_back(record);
    }
  }

  std::sort(result.begin(), result.end(), [](auto const &lhs, auto const &rhs) {
    return lhs.global_primary_id < rhs.global_primary_id;
  });

  return result;
}

// =============================================================================
// =============================================================================

void ExpectSourceMatches(
    ggems::core::observer::GGEMSObserverRecord const &observed,
    ggems::core::sources::GGEMSSourceRecord const &expected) {
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

  static ggems::ocl::GGEMSOpenCLContext &GetContext() {
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
      .SetDirection(0.0f, 0.0f, 1.0f)
      .SetWeight(1.0f);

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
      .SetDirection(0.0f, 0.0f, 1.0f)
      .SetWeight(1.0f);

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

  for (std::size_t index = 0U; index < source_records.size(); ++index) {
    EXPECT_EQ(source_records[index].run_id, 91ULL);
    EXPECT_EQ(source_records[index].global_primary_id,
              k_global_begin + 1ULL + static_cast<std::uint64_t>(index));
    EXPECT_NE(source_records[index].energy_milli_eV,
              disabled_record.energy_milli_eV);

    ExpectSourceMatches(source_records[index],
                        index < 2U ? expected_a : expected_c);
  }

  auto second_report = workload.Run(config);

  EXPECT_EQ(second_report.counters.completed_history_count, 5U);
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

  auto construct_workload = [&]() {
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
