#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace {

using ggems::units::operator""_B;

constexpr std::uint32_t k_seed{77777U};
constexpr std::size_t k_particle_count{1024U};
constexpr std::size_t k_samples_per_particle{8U};
constexpr std::size_t k_local_size{64U};

std::size_t RoundUp(std::size_t value, std::size_t multiple) noexcept {
  return ((value + multiple - 1U) / multiple) * multiple;
}

ggems::core::random::GGEMSJKissState
MakeJKissState(std::uint32_t seed, std::uint32_t index) noexcept {
  return ggems::core::random::GGEMSJKissState{
      .x = seed + 123456789U + 1013904223U * index,
      .y = seed ^ (362436069U + 1664525U * index),
      .z = seed + 521288629U + 69069U * index,
      .w = seed ^ (88675123U + 22695477U * index),
      .c = index & 1U};
}

class GGEMSJKissKernelTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    opencl.SelectDevices({"gpu"});
    opencl.Initialise();

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static ggems::ocl::GGEMSOpenCLContext &GetContext() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    return opencl.GetContext().front();
  }
};

} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST_F(GGEMSJKissKernelTest, UniformValuesAreInsideUnitInterval) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path kernel_test_root = kernel_root / "tests";

  std::string build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_jkiss_uniform", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_jkiss_uniform"};

  std::size_t state_bytes =
      k_particle_count * sizeof(ggems::core::random::GGEMSJKissState);
  std::size_t value_count = k_particle_count * k_samples_per_particle;
  std::size_t value_bytes = value_count * sizeof(float);

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

  auto *states = static_cast<ggems::core::random::GGEMSJKissState *>(
      states_buffer.GetData());
  auto *values = static_cast<float *>(values_buffer.GetData());

  states_buffer.Map(CL_MAP_WRITE);
  values_buffer.Map(CL_MAP_WRITE);

  for (std::size_t i = 0U; i < k_particle_count; ++i) {
    states[i] = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));
  }

  std::fill(values, values + value_count, -1.0f);

  states_buffer.Unmap();
  values_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

  std::size_t global_size = RoundUp(k_particle_count, k_local_size);

  kernel.Run({global_size}, {k_local_size});

  values_buffer.Map(CL_MAP_READ);

  for (std::size_t i = 0U; i < value_count; ++i) {
    EXPECT_GE(values[i], 0.0f);
    EXPECT_LT(values[i], 1.0f);
  }

  values_buffer.Unmap();
}
