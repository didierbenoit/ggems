#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <limits>
#include <numbers>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/radioactivity/GGEMSRadioactiveTimeSampling.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"

namespace {

constexpr long double k_maximum_cdf_error{1.0e-5L};

struct SamplingCase {
  std::uint64_t start_ps;
  std::uint64_t stop_ps;
  float scaled_decay;
  std::uint32_t raw_word;
};

struct OpenCLSamplingResult {
  std::uint64_t time_ps;
  std::uint32_t ticket;
  float relative;
};

static_assert(sizeof(OpenCLSamplingResult) == 16U);

[[nodiscard]] auto UniformFromRaw(std::uint32_t raw_word) noexcept
    -> long double {
  return static_cast<long double>(raw_word >> 8U) * 0x1.0p-24L;
}

[[nodiscard]] auto ReferenceRelative(std::uint32_t raw_word,
                                     float scaled_decay) noexcept
    -> long double {
  long double const uniform = UniformFromRaw(raw_word);
  long double const x = static_cast<long double>(scaled_decay);

  if (x == 0.0L) {
    return uniform;
  }

  return -std::log1p(-uniform * -std::expm1(-x)) / x;
}

[[nodiscard]] auto NormalizedCDF(long double relative,
                                 float scaled_decay) noexcept -> long double {
  auto const x = static_cast<long double>(scaled_decay);
  if (x == 0.0L) {
    return relative;
  }
  return -std::expm1(-x * relative) / -std::expm1(-x);
}

[[nodiscard]] auto ExpectedTimeFromTicket(SamplingCase const &sample,
                                          std::uint32_t ticket) noexcept
    -> std::uint64_t {
  std::uint64_t const width = sample.stop_ps - sample.start_ps;
  std::uint64_t offset =
      ggems::core::radioactivity::ScaleRadioactiveTimeTicket(width, ticket);
  if (offset >= width) {
    offset = width - 1ULL;
  }
  return sample.start_ps + offset;
}

[[nodiscard]] auto BuildSamplingGrid() -> std::vector<SamplingCase> {
  constexpr std::array<std::uint32_t, 5U> k_words{0U, 1U, 0x7FFF'FFFFU,
                                                  0x8000'0000U, 0xFFFF'FFFFU};
  constexpr float k_limit =
      ggems::core::radioactivity::k_radioactive_time_uniform_limit_scaled_decay;
  constexpr std::array<float, 8U> k_decays{
      0.0F,   0.5F * k_limit, k_limit, 2.0F * k_limit,
      0.001F, 0.01F,          1.0F,    20.0F};
  constexpr std::array<std::array<std::uint64_t, 2U>, 4U> k_windows{{
      {17ULL, 18ULL},
      {9'000'000'000'000'000ULL, 9'001'000'000'000'000ULL},
      {std::numeric_limits<std::uint64_t>::max() - 1'000'000ULL,
       std::numeric_limits<std::uint64_t>::max()},
      {123ULL, 10'000'000'000'000ULL + 123ULL},
  }};

  std::vector<SamplingCase> result;
  result.reserve(k_words.size() * k_decays.size() * k_windows.size());
  for (auto const &window : k_windows) {
    for (float decay : k_decays) {
      for (std::uint32_t word : k_words) {
        result.push_back({window[0U], window[1U], decay, word});
      }
    }
  }

  constexpr std::array<long double, 3U> k_built_in_half_lives_seconds{
      6'584.04L, 1'221.66L, 122.266L};
  constexpr std::uint64_t k_representative_start_ps{8'000'000'000'000'000ULL};
  constexpr std::uint64_t k_representative_stop_ps{k_representative_start_ps +
                                                   1'000'000'000'000ULL};
  for (long double half_life_seconds : k_built_in_half_lives_seconds) {
    auto const scaled_decay = static_cast<float>(
        std::numbers::ln2_v<long double> / half_life_seconds);
    for (std::uint32_t word : k_words) {
      result.push_back({k_representative_start_ps, k_representative_stop_ps,
                        scaled_decay, word});
    }
  }

  return result;
}

class GGEMSRadioactiveTimeSamplingKernelTest : public ::testing::Test {
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

TEST(GGEMSRadioactiveTimeSampling, HostBoundsScaleAndCDFContractAreExact) {
  constexpr float k_limit =
      ggems::core::radioactivity::k_radioactive_time_uniform_limit_scaled_decay;
  EXPECT_EQ(k_limit, 0x1.0p-14F);

  for (SamplingCase const &sample : BuildSamplingGrid()) {
    SCOPED_TRACE(std::format("start={} stop={} x={} raw={}", sample.start_ps,
                             sample.stop_ps, sample.scaled_decay,
                             sample.raw_word));
    std::uint64_t const time =
        ggems::core::radioactivity::SampleRadioactiveTimeFromRaw(
            sample.start_ps, sample.stop_ps, sample.scaled_decay,
            sample.raw_word);
    EXPECT_GE(time, sample.start_ps);
    EXPECT_LT(time, sample.stop_ps);

    float const relative =
        ggems::core::radioactivity::ComputeRadioactiveTimeRelative(
            static_cast<float>(UniformFromRaw(sample.raw_word)),
            sample.scaled_decay);
    EXPECT_GE(relative, 0.0F);
    EXPECT_LT(relative, 1.0F);
    EXPECT_LE(std::abs(NormalizedCDF(relative, sample.scaled_decay) -
                       UniformFromRaw(sample.raw_word)),
              k_maximum_cdf_error);
    EXPECT_LE(std::abs(NormalizedCDF(ReferenceRelative(sample.raw_word,
                                                       sample.scaled_decay),
                                     sample.scaled_decay) -
                       UniformFromRaw(sample.raw_word)),
              1.0e-15L);
  }

  EXPECT_EQ(ggems::core::radioactivity::ScaleRadioactiveTimeTicket(
                1ULL, std::numeric_limits<std::uint32_t>::max()),
            0ULL);
  EXPECT_EQ(
      ggems::core::radioactivity::ScaleRadioactiveTimeTicket(1ULL << 32U, 1U),
      1ULL);
  EXPECT_EQ(ggems::core::radioactivity::ScaleRadioactiveTimeTicket(
                std::numeric_limits<std::uint64_t>::max(),
                std::numeric_limits<std::uint32_t>::max()),
            std::numeric_limits<std::uint64_t>::max() - (1ULL << 32U));
  EXPECT_EQ(ggems::core::radioactivity::SampleRadioactiveTimeFromRaw(
                77ULL, 78ULL, 20.0F, 0xFFFF'FFFFU),
            77ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRadioactiveTimeSamplingKernelTest,
       OpenCLSatisfiesCurrentBuildModeCDFAndHalfOpenContract) {
  auto cases = BuildSamplingGrid();
  std::vector<std::uint64_t> starts;
  std::vector<std::uint64_t> stops;
  std::vector<float> decays;
  std::vector<std::uint32_t> words;
  std::vector<OpenCLSamplingResult> results(cases.size());
  starts.reserve(cases.size());
  stops.reserve(cases.size());
  decays.reserve(cases.size());
  words.reserve(cases.size());
  for (SamplingCase const &sample : cases) {
    starts.push_back(sample.start_ps);
    stops.push_back(sample.stop_ps);
    decays.push_back(sample.scaled_decay);
    words.push_back(sample.raw_word);
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();
  auto starts_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{starts.size() * sizeof(std::uint64_t)});
  auto stops_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{stops.size() * sizeof(std::uint64_t)});
  auto decay_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{decays.size() * sizeof(float)});
  auto word_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{words.size() * sizeof(std::uint32_t)});
  auto result_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{results.size() * sizeof(OpenCLSamplingResult)});
  ggems::ocl::WriteSVMFromHost(starts_buffer, std::span{starts});
  ggems::ocl::WriteSVMFromHost(stops_buffer, std::span{stops});
  ggems::ocl::WriteSVMFromHost(decay_buffer, std::span{decays});
  ggems::ocl::WriteSVMFromHost(word_buffer, std::span{words});
  ggems::ocl::WriteSVMFromHost(result_buffer, std::span{results});

  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  std::string const options =
      std::format("-cl-std=CL2.0 -I{}", root.generic_string());
  auto &program = opencl.GetOrCreateProgram(
      context, root / "tests", "radioactive_time_sampling_probe", options);
  ggems::ocl::GGEMSOpenCLKernel kernel{
      context, program.CreateKernel("radioactive_time_sampling_probe"),
      "radioactive_time_sampling_probe"};
  kernel.SetArgSVMPointer(0U, starts_buffer.GetData());
  kernel.SetArgSVMPointer(1U, stops_buffer.GetData());
  kernel.SetArgSVMPointer(2U, decay_buffer.GetData());
  kernel.SetArgSVMPointer(3U, word_buffer.GetData());
  kernel.SetArgSVMPointer(4U, result_buffer.GetData());
  kernel.SetArg(5U, static_cast<cl_uint>(cases.size()));
  kernel.Run({cases.size()}, {1U});
  ggems::ocl::ReadSVMToHost(result_buffer, std::span{results});

  for (std::size_t index = 0U; index < cases.size(); ++index) {
    SamplingCase const &sample = cases[index];
    OpenCLSamplingResult const &result = results[index];
    SCOPED_TRACE(std::format("index={} x={} raw={}", index, sample.scaled_decay,
                             sample.raw_word));
    EXPECT_GE(result.time_ps, sample.start_ps);
    EXPECT_LT(result.time_ps, sample.stop_ps);
    EXPECT_GE(result.relative, 0.0F);
    EXPECT_LT(result.relative, 1.0F);
    EXPECT_EQ(result.ticket,
              ggems::core::radioactivity::QuantizeRadioactiveTimeRelative(
                  result.relative));
    EXPECT_EQ(result.time_ps, ExpectedTimeFromTicket(sample, result.ticket));
    EXPECT_LE(std::abs(NormalizedCDF(result.relative, sample.scaled_decay) -
                       UniformFromRaw(sample.raw_word)),
              k_maximum_cdf_error);
  }
}
