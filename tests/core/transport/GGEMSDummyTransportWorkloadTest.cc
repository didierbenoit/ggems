#include <cstdint>
#include <filesystem>
#include <memory>
#include <algorithm>

#include <gtest/gtest.h>

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"

namespace {

constexpr std::uint32_t k_worker_count{256U};
constexpr std::uint32_t k_total_primary_count{4096U};

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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST_F(GGEMSDummyTransportWorkloadTest, RunsBranchingAioninoPrototype) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000'000ULL)
      .SetPositionPicoMeter(0ULL, 0ULL, 0ULL)
      .SetDirection(0.0f, 0.0f, 1.0f)
      .SetWeight(1.0f);

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}, *random,
      k_worker_count};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = k_total_primary_count;
  config.projection_history_offset = 10'000'000ULL;
  config.device_primary_offset = 200'000ULL;
  config.source_record = source.BuildRecord();

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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST_F(GGEMSDummyTransportWorkloadTest, CapturesFirstPrimaryHistories) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(511'000'000ULL)
      .SetTimeWindowPicoSecond(0ULL, 1'000'000ULL)
      .SetPositionPicoMeter(0ULL, 0ULL, 0ULL)
      .SetDirection(0.0f, 0.0f, 1.0f)
      .SetWeight(1.0f);

  constexpr std::uint32_t k_observed_primary_count{2U};
  constexpr std::uint32_t k_observer_record_capacity{4096U};
  constexpr std::uint64_t k_run_id{42ULL};
  constexpr std::uint64_t k_projection_history_offset{10'000'000ULL};

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(),
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT},
      *random,
      k_worker_count,
      0ULL,
      0U,
      k_observer_record_capacity};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.run_id = k_run_id;
  config.total_primary_count = 64U;
  config.projection_history_offset = k_projection_history_offset;
  config.device_primary_offset = 0ULL;
  config.source_record = source.BuildRecord();
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
