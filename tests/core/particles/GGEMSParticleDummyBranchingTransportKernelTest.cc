#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleState.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace {
using ParticleState = ggems::core::particles::GGEMSParticleState;
using PhiloxState = ggems::core::random::GGEMSPhiloxState;

constexpr std::uint64_t k_seed{7777777ULL};

constexpr std::size_t k_worker_count{256U};
constexpr std::size_t k_total_primary_count{4096U};
constexpr std::size_t k_local_size{64U};

constexpr std::uint64_t k_initial_energy_milli_eV{511000000ULL};
constexpr std::uint64_t k_min_energy_milli_eV{10000000ULL};

constexpr std::uint32_t k_max_generation{6U};
constexpr std::uint32_t k_max_steps_per_track{12U};

constexpr std::size_t k_counter_count{11U};

enum DummyCounterIndex : std::size_t {
  NextPrimary = 0U,
  ConsumedPrimary = 1U,
  CompletedHistories = 2U,
  TerminalParticles = 3U,
  CreatedSecondaries = 4U,
  AioninoToGamma = 5U,
  GammaToElectron = 6U,
  ElectronToElectron = 7U,
  Overflow = 8U,
  MaxStackDepth = 9U,
  TotalFakeSteps = 10U
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::size_t RoundUp(std::size_t value, std::size_t multiple) noexcept {
  return ((value + multiple - 1U) / multiple) * multiple;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint64_t SplitMix64(std::uint64_t value) noexcept {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

PhiloxState MakePhiloxState(std::uint64_t seed, std::uint64_t index) noexcept {
  std::uint64_t key = SplitMix64(seed);

  return PhiloxState{.counter_0 = 0U,
                     .counter_1 = 0U,
                     .counter_2 = static_cast<std::uint32_t>(index),
                     .counter_3 = static_cast<std::uint32_t>(index >> 32U),
                     .key_0 = static_cast<std::uint32_t>(key),
                     .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

class GGEMSParticleDummyBranchingTransportKernelTest : public ::testing::Test {
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

TEST_F(GGEMSParticleDummyBranchingTransportKernelTest,
       StreamWorkersCompleteBranchingHistories) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{} "
                  "-DGGEMS_RANDOM_ENGINE=3 "
                  "-DGGEMS_DUMMY_LOCAL_STACK_CAPACITY=16",
                  kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "particle_dummy_stream_branching_transport",
      build_options);

  cl::Kernel raw_kernel =
      program.CreateKernel("particle_dummy_stream_branching_transport");

  ggems::ocl::GGEMSOpenCLKernel kernel{
      context, std::move(raw_kernel),
      "particle_dummy_stream_branching_transport"};

  EXPECT_EQ(kernel.GetNumArgs(), 8U);
}
