#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

// =============================================================================
// =============================================================================

constexpr std::array<GGEMSRandomEngine, 3> k_engines{GGEMSRandomEngine::JKISS,
                                                     GGEMSRandomEngine::PCG32,
                                                     GGEMSRandomEngine::Philox};
constexpr std::size_t k_sequence_size{32U};
constexpr std::size_t k_probe_sample_count{16U};

// =============================================================================
// =============================================================================

auto SequenceDiffers(GGEMSHostRandomStream &lhs,
                     GGEMSHostRandomStream &rhs) noexcept -> bool {
  for (std::size_t index = 0U; index < k_sequence_size; ++index) {
    if (lhs.NextUInt32() != rhs.NextUInt32()) {
      return true;
    }
  }

  return false;
}

// =============================================================================
// =============================================================================

class GGEMSHostRandomStreamKernelTest : public ::testing::Test {
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
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, ReportsCapturedEngineAndStreamId) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(123ULL);

  GGEMSHostRandomStream stream{random, 456ULL};

  EXPECT_EQ(stream.GetEngine(), GGEMSRandomEngine::PCG32);
  EXPECT_EQ(stream.GetStreamId(), 456ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, SameSeedAndStreamProduceSameSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream first{random, 42ULL};
    GGEMSHostRandomStream second{random, 42ULL};

    for (std::size_t index = 0U; index < k_sequence_size; ++index) {
      EXPECT_EQ(first.NextUInt32(), second.NextUInt32());
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, DifferentSeedChangesSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom first_random{};
    first_random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSRandom second_random{};
    second_random.SetEngine(engine).SetSeed(77'778ULL);
    GGEMSHostRandomStream first{first_random, 42ULL};
    GGEMSHostRandomStream second{second_random, 42ULL};

    EXPECT_TRUE(SequenceDiffers(first, second));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, DifferentStreamChangesSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream first{random, 42ULL};
    GGEMSHostRandomStream second{random, 43ULL};

    EXPECT_TRUE(SequenceDiffers(first, second));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, SequenceContinuesAcrossRepeatedCalls) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};
    GGEMSHostRandomStream reference{random, 42ULL};

    for (std::size_t index = 0U; index < 17U; ++index) {
      (void)stream.NextUInt32();
      (void)reference.NextUInt32();
    }

    for (std::size_t index = 0U; index < k_sequence_size; ++index) {
      EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest,
     LaterRandomConfigurationChangesDoNotAffectExistingStream) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream stream{random, 42ULL};

  random.SetEngine(GGEMSRandomEngine::Philox).SetSeed(1ULL);

  GGEMSRandom reference_random{};
  reference_random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream reference{reference_random, 42ULL};

  for (std::size_t index = 0U; index < k_sequence_size; ++index) {
    EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, JKISSRejectsOutOfRangeStreamId) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS);

  EXPECT_THROW((GGEMSHostRandomStream{
                   random, static_cast<std::uint64_t>(
                               std::numeric_limits<std::uint32_t>::max()) +
                               1ULL}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, UniformFloatStaysInsideHalfOpenUnitInterval) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};

    for (std::size_t index = 0U; index < 4096U; ++index) {
      float const value = stream.UniformFloat01();
      EXPECT_GE(value, 0.0F);
      EXPECT_LT(value, 1.0F);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, UniformDoubleStaysInsideOpenUnitInterval) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};

    for (std::size_t index = 0U; index < 4096U; ++index) {
      double const value = stream.UniformDoubleOpen01();
      EXPECT_GT(value, 0.0);
      EXPECT_LT(value, 1.0);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest,
     UniformDoubleUsesSpecifiedWordsAndConsumesExactlyTwo) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};
    GGEMSHostRandomStream reference{random, 42ULL};

    auto const a = static_cast<std::uint64_t>(reference.NextUInt32() >> 5U);
    auto const b = static_cast<std::uint64_t>(reference.NextUInt32() >> 6U);
    std::uint64_t const bits = (a << 26U) + b;
    double expected = (static_cast<double>(bits) + 0.5) * 0x1.0p-53;
    if (expected >= 1.0) {
      expected = 0x1.fffffffffffffp-1;
    }

    EXPECT_EQ(stream.UniformDoubleOpen01(), expected);
    EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSHostRandomStreamKernelTest,
       ScalarHostProgressionMatchesScalarOpenCLProgression) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();
  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";

  for (GGEMSRandomEngine engine : k_engines) {
    GGEMSRandom random{};
    random.SetEngine(engine);

    std::string const build_options =
        std::format("-cl-std=CL2.0 -I{} {}", kernel_root.generic_string(),
                    random.GetKernelBuildDefinition());
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_host_stream_probe", build_options);
    cl::Kernel raw_kernel = program.CreateKernel("random_host_stream_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_host_stream_probe"};

    std::array<std::uint64_t, 2> const seeds{0ULL, 77'777ULL};
    std::array<std::uint64_t, 2> const stream_ids{
        0ULL,
        engine == GGEMSRandomEngine::JKISS ? 42ULL : (1ULL << 40U) + 42ULL};

    for (std::uint64_t seed : seeds) {
      for (std::uint64_t stream_id : stream_ids) {
        SCOPED_TRACE(std::format("engine={} seed={} stream={}",
                                 static_cast<std::uint32_t>(engine), seed,
                                 stream_id));

        random.SetSeed(seed);
        auto raw_state_buffer =
            context.CreateSVMBuffer(ggems::units::Bytes{random.GetStateSize()});
        auto uniform_state_buffer =
            context.CreateSVMBuffer(ggems::units::Bytes{random.GetStateSize()});
        auto raw_values_buffer = context.CreateSVMBuffer(
            ggems::units::Bytes{k_probe_sample_count * sizeof(std::uint32_t)});
        auto uniform_values_buffer = context.CreateSVMBuffer(
            ggems::units::Bytes{k_probe_sample_count * sizeof(float)});

        raw_state_buffer.Map(CL_MAP_WRITE);
        uniform_state_buffer.Map(CL_MAP_WRITE);
        random.InitialiseStates(
            stream_id, std::span<std::byte>{
                           static_cast<std::byte *>(raw_state_buffer.GetData()),
                           random.GetStateSize()});
        random.InitialiseStates(
            stream_id, std::span<std::byte>{static_cast<std::byte *>(
                                                uniform_state_buffer.GetData()),
                                            random.GetStateSize()});
        uniform_state_buffer.Unmap();
        raw_state_buffer.Unmap();

        kernel.SetArgSVMPointer(0U, raw_state_buffer.GetData());
        kernel.SetArgSVMPointer(1U, uniform_state_buffer.GetData());
        kernel.SetArgSVMPointer(2U, raw_values_buffer.GetData());
        kernel.SetArgSVMPointer(3U, uniform_values_buffer.GetData());
        kernel.SetArg(4U, static_cast<cl_uint>(k_probe_sample_count));
        kernel.Run({1U}, {1U});

        GGEMSHostRandomStream host_raw{random, stream_id};
        GGEMSHostRandomStream host_uniform{random, stream_id};
        auto const *raw_values =
            static_cast<std::uint32_t const *>(raw_values_buffer.GetData());
        auto const *uniform_values =
            static_cast<float const *>(uniform_values_buffer.GetData());

        raw_values_buffer.Map(CL_MAP_READ);
        uniform_values_buffer.Map(CL_MAP_READ);
        for (std::size_t index = 0U; index < k_probe_sample_count; ++index) {
          EXPECT_EQ(host_raw.NextUInt32(), raw_values[index]);
          EXPECT_EQ(std::bit_cast<std::uint32_t>(host_uniform.UniformFloat01()),
                    std::bit_cast<std::uint32_t>(uniform_values[index]));
        }
        uniform_values_buffer.Unmap();
        raw_values_buffer.Unmap();
      }
    }
  }
}
