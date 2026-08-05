#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace {

using PCG32State = ggems::core::random::GGEMSPCG32State;

// =============================================================================
// =============================================================================

constexpr std::uint32_t k_seed{77777ULL};
constexpr std::uint32_t k_alternative_seed{77778ULL};

constexpr std::size_t k_particle_count{1024U};
constexpr std::size_t k_samples_per_particle{8U};
constexpr std::size_t k_value_count{k_particle_count * k_samples_per_particle};
constexpr std::size_t k_local_size{64U};

constexpr std::size_t k_uniform4_blocks_per_particle{2U};
constexpr std::size_t k_uniform4_value_count{
    k_particle_count * k_uniform4_blocks_per_particle * 4U};

constexpr auto k_padded_global_work_size =
    ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_particle_count,
                                                       k_local_size);

static_assert(k_padded_global_work_size.has_value());

constexpr std::size_t k_global_work_size = *k_padded_global_work_size;

// =============================================================================
// =============================================================================

auto SplitMix64(std::uint64_t value) noexcept -> std::uint64_t {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

// =============================================================================
// =============================================================================

auto MakePCG32State(std::uint64_t seed, std::uint64_t index) noexcept
    -> PCG32State {
  std::uint64_t state =
      SplitMix64(seed + 0xD1B54A32D192ED03ULL * (index + 1ULL));

  std::uint64_t stream = SplitMix64(seed ^ (0xABC98388FB8FAC03ULL + index));

  return PCG32State{.state = state, .increment = stream | 1ULL};
}

// =============================================================================
// =============================================================================

auto AreStatesEqual(PCG32State &lhs, PCG32State &rhs) noexcept -> bool {
  return lhs.state == rhs.state && lhs.increment == rhs.increment;
}

// =============================================================================
// =============================================================================

class GGEMSPCG32KernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
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

TEST_F(GGEMSPCG32KernelTest, UniformValuesAreInsideUnitInterval) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_pcg32_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_pcg32_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_pcg32_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    states[i] = MakePCG32State(k_seed, static_cast<std::uint32_t>(i));
    ASSERT_EQ(states[i].increment & 1ULL, 1ULL);
  }

  std::fill(values, values + k_value_count, -1.0f);

  values_buffer.Unmap();
  states_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  kernel.Run({k_global_work_size}, {k_local_size});

  values_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);
  }

  values_buffer.Unmap();
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, SequenceContinuesBetweenKernelCalls) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_pcg32_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_pcg32_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_pcg32_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    states[i] = MakePCG32State(k_seed, static_cast<std::uint64_t>(i));
  }

  std::fill(values, values + k_value_count, -1.0f);

  values_buffer.Unmap();
  states_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  kernel.Run({k_global_work_size}, {k_local_size});

  std::vector<float> first_values(k_value_count);
  values_buffer.Map(CL_MAP_READ);
  std::copy(values, values + k_value_count, first_values.begin());
  values_buffer.Unmap();

  kernel.Run({k_global_work_size}, {k_local_size});

  values_buffer.Map(CL_MAP_READ);

  bool sequence_has_advanced{false};

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);

    if (values[i] != first_values[i]) {
      sequence_has_advanced = true;
    }
  }

  values_buffer.Unmap();

  EXPECT_TRUE(sequence_has_advanced);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, SameSeedProducesSameFirstSequence) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_pcg32_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_pcg32_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_pcg32_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  auto initialise_states = [&]() -> void {
    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakePCG32State(k_seed, static_cast<std::uint64_t>(i));
    }

    std::fill(values, values + k_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();
  };

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  initialise_states();

  kernel.Run({k_global_work_size}, {k_local_size});

  std::vector<float> first_values(k_value_count);

  values_buffer.Map(CL_MAP_READ);
  std::copy(values, values + k_value_count, first_values.begin());
  values_buffer.Unmap();

  initialise_states();

  kernel.Run({k_global_work_size}, {k_local_size});

  values_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_EQ(values[i], first_values[i]);
  }

  values_buffer.Unmap();
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, DifferentSeedsProduceDifferentFirstSequence) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_pcg32_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_pcg32_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_pcg32_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  auto initialise_states = [&](std::uint64_t seed) -> void {
    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakePCG32State(seed, static_cast<std::uint64_t>(i));
    }

    std::fill(values, values + k_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();
  };

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  initialise_states(k_seed);

  kernel.Run({k_global_work_size}, {k_local_size});

  std::vector<float> first_seed_values(k_value_count);

  values_buffer.Map(CL_MAP_READ);
  std::copy(values, values + k_value_count, first_seed_values.begin());
  values_buffer.Unmap();

  initialise_states(k_alternative_seed);

  kernel.Run({k_global_work_size}, {k_local_size});

  values_buffer.Map(CL_MAP_READ);

  std::size_t different_value_count{0U};

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);

    if (values[i] != first_seed_values[i]) {
      ++different_value_count;
    }
  }

  values_buffer.Unmap();

  EXPECT_GT(different_value_count, k_value_count / 2U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, RandomStatesAreAdvancedByKernelExecution) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_pcg32_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_pcg32_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_pcg32_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  std::vector<PCG32State> initial_states(k_particle_count);

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    PCG32State state = MakePCG32State(k_seed, static_cast<std::uint64_t>(i));

    states[i] = state;
    initial_states[i] = state;
  }

  std::fill(values, values + k_value_count, -1.0F);

  values_buffer.Unmap();
  states_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  kernel.Run({k_global_work_size}, {k_local_size});

  states_buffer.Map(CL_MAP_READ);
  values_buffer.Map(CL_MAP_READ);

  std::size_t changed_state_count{0U};

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    if (!AreStatesEqual(states[i], initial_states[i])) {
      ++changed_state_count;
    }

    EXPECT_NE(states[i].state, initial_states[i].state);
    EXPECT_EQ(states[i].increment, initial_states[i].increment);
  }

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);
  }

  values_buffer.Unmap();
  states_buffer.Unmap();

  EXPECT_EQ(changed_state_count, k_particle_count);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, GenericRandomUniformUsesSelectedPCG32Engine) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=2",
                  kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_generic_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_generic_uniform"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  std::vector<PCG32State> initial_states(k_particle_count);

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    PCG32State state = MakePCG32State(k_seed, static_cast<std::uint64_t>(i));

    states[i] = state;
    initial_states[i] = state;
  }

  std::fill(values, values + k_value_count, -1.0F);

  values_buffer.Unmap();
  states_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  kernel.Run({k_global_work_size}, {k_local_size});

  states_buffer.Map(CL_MAP_READ);
  values_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < k_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);
  }

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    EXPECT_NE(states[i].state, initial_states[i].state);
    EXPECT_EQ(states[i].increment, initial_states[i].increment);
  }

  values_buffer.Unmap();
  states_buffer.Unmap();
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSPCG32KernelTest, GenericRandomUniform4UsesSelectedPCG32Engine) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=2",
                  kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_generic_uniform4", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform4");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_generic_uniform4"};

  std::size_t state_bytes = k_particle_count * sizeof(PCG32State);
  std::size_t value_bytes = k_uniform4_value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<PCG32State *>(states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  std::vector<PCG32State> initial_states(k_particle_count);

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    PCG32State state = MakePCG32State(k_seed, static_cast<std::uint64_t>(i));

    states[i] = state;
    initial_states[i] = state;
  }

  std::fill(values, values + k_uniform4_value_count, -1.0F);

  values_buffer.Unmap();
  states_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_uniform4_blocks_per_particle));

  kernel.Run({k_global_work_size}, {k_local_size});

  states_buffer.Map(CL_MAP_READ);
  values_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < k_uniform4_value_count; ++i) {
    EXPECT_GE(values[i], 0.0F);
    EXPECT_LT(values[i], 1.0F);
  }

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    EXPECT_NE(states[i].state, initial_states[i].state);
    EXPECT_EQ(states[i].increment, initial_states[i].increment);
  }

  values_buffer.Unmap();
  states_buffer.Unmap();
}
