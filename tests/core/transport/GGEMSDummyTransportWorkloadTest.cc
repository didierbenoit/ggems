#include <cstdint>
#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

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

  ggems::core::transport::GGEMSDummyTransportWorkload workload{
      GetContext(), std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}, *random,
      k_worker_count};

  ggems::core::transport::GGEMSDummyTransportRunConfig config{};
  config.total_primary_count = k_total_primary_count;

  workload.Run(config);

  auto const counters = workload.ReadCountersOnHost();

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
}
