#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"

namespace {

// =============================================================================
// =============================================================================

using Random = ggems::core::random::GGEMSRandom;
using RandomEngine = ggems::core::random::GGEMSRandomEngine;

// =============================================================================
// =============================================================================

constexpr std::array<RandomEngine, 3U> k_engines{
    RandomEngine::JKISS, RandomEngine::PCG32, RandomEngine::Philox};

// =============================================================================
// =============================================================================

class GGEMSRadioactiveTimeRandomProgressionTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }
    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSRadioactiveTimeRandomProgressionTest,
       EveryEngineConsumesExactlyOneDedicatedTimeWord) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    Random random{};
    random.SetEngine(engine).SetSeed(91'337ULL);
    ggems::core::random::GGEMSHostRandomStream reference{random, 0ULL};
    std::uint32_t const expected_time_word = reference.NextUInt32();
    std::uint32_t const expected_next_word = reference.NextUInt32();

    auto state_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{random.GetStateSize()});
    auto raw_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{2U * sizeof(std::uint32_t)});
    auto time_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(std::uint64_t)});

    state_buffer.Map(CL_MAP_WRITE);
    random.InitializeStates(
        0ULL,
        std::span<std::byte>{static_cast<std::byte *>(state_buffer.GetData()),
                             random.GetStateSize()});
    state_buffer.Unmap();

    std::array<std::uint32_t, 2U> raw_values{};
    std::uint64_t sampled_time{0ULL};
    ggems::ocl::WriteSVMFromHost(raw_buffer, std::span{raw_values});
    ggems::ocl::WriteSVMFromHost(time_buffer, sampled_time);

    std::string const options = std::format("-I{} {}", root.generic_string(),
                                            random.GetKernelBuildDefinition());
    auto const &program = opencl.GetOrCreateProgram(
        context, root / "tests", "radioactive_time_random_probe", options);
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, program.CreateKernel("radioactive_time_random_probe"),
        "radioactive_time_random_probe"};

    constexpr std::uint64_t k_start{8'000'000'000'000'000ULL};
    constexpr std::uint64_t k_stop{k_start + 1'000'000'000'000ULL};
    constexpr float k_scaled_decay{0.125F};
    kernel.SetArgSVMPointer(0U, state_buffer.GetData());
    kernel.SetArgSVMPointer(1U, raw_buffer.GetData());
    kernel.SetArgSVMPointer(2U, time_buffer.GetData());
    kernel.SetArg(3U, static_cast<cl_ulong>(k_start));
    kernel.SetArg(4U, static_cast<cl_ulong>(k_stop));
    kernel.SetArg(5U, k_scaled_decay);
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(raw_buffer, std::span{raw_values});
    sampled_time = ggems::ocl::ReadSVMToHost<std::uint64_t>(time_buffer);

    EXPECT_EQ(raw_values[0U], expected_time_word);
    EXPECT_EQ(raw_values[1U], expected_next_word);
    EXPECT_GE(sampled_time, k_start);
    EXPECT_LT(sampled_time, k_stop);
  }
}
