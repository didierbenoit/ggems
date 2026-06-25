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

  auto random_states_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{k_worker_count * sizeof(PhiloxState)});
  auto worker_final_states_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{k_worker_count * sizeof(ParticleState)});
  auto counters_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{k_counter_count * sizeof(std::uint32_t)});

  auto *random_states =
      static_cast<PhiloxState *>(random_states_buffer.GetData());
  auto *worker_final_states =
      static_cast<ParticleState *>(worker_final_states_buffer.GetData());
  auto *counters = static_cast<std::uint32_t *>(counters_buffer.GetData());

  random_states_buffer.Map(CL_MAP_WRITE);
  worker_final_states_buffer.Map(CL_MAP_WRITE);
  counters_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_worker_count; ++i) {
    random_states[i] = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));
  }

  std::fill(worker_final_states, worker_final_states + k_worker_count,
            ParticleState{});

  std::fill(counters, counters + k_counter_count, 0U);

  random_states_buffer.Unmap();
  worker_final_states_buffer.Unmap();
  counters_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, random_states);
  kernel.SetArgSVMPointer(1U, worker_final_states);
  kernel.SetArgSVMPointer(2U, counters);
  kernel.SetArg(3U, static_cast<cl_uint>(k_total_primary_count));
  kernel.SetArg(4U, static_cast<cl_ulong>(k_initial_energy_milli_eV));
  kernel.SetArg(5U, static_cast<cl_ulong>(k_min_energy_milli_eV));
  kernel.SetArg(6U, static_cast<cl_uint>(k_max_generation));
  kernel.SetArg(7U, static_cast<cl_uint>(k_max_steps_per_track));

  std::size_t global_size = RoundUp(k_worker_count, k_local_size);

  kernel.Run({global_size}, {k_local_size});

  counters_buffer.Map(CL_MAP_READ);

  EXPECT_EQ(counters[ConsumedPrimary],
            static_cast<std::uint32_t>(k_total_primary_count));

  EXPECT_EQ(counters[CompletedHistories],
            static_cast<std::uint32_t>(k_total_primary_count));

  EXPECT_EQ(counters[AioninoToGamma],
            static_cast<std::uint32_t>(k_total_primary_count));

  EXPECT_GT(counters[CreatedSecondaries], 0U);
  EXPECT_GT(counters[GammaToElectron], 0U);
  EXPECT_GT(counters[ElectronToElectron], 0U);

  EXPECT_EQ(counters[CreatedSecondaries],
            counters[GammaToElectron] + counters[ElectronToElectron]);

  EXPECT_EQ(counters[TerminalParticles],
            counters[ConsumedPrimary] + counters[CreatedSecondaries]);

  EXPECT_EQ(counters[Overflow], 0U);

  EXPECT_GT(counters[MaxStackDepth], 0U);
  EXPECT_LE(counters[MaxStackDepth], 16U);

  EXPECT_GT(counters[TotalFakeSteps],
            static_cast<std::uint32_t>(k_total_primary_count));

  counters_buffer.Unmap();
}
