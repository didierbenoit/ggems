#include <array>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <utility>
#include <cstddef>
#include <numbers>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::transport::TryAddDiagnosticProjectionDisplacement;
using ggems::core::transport::TryScaleDiagnosticProjectionComponent;

constexpr std::int64_t k_output_sentinel{123'456'789LL};

// =============================================================================
// =============================================================================

class GGEMSDiagnosticProjectionKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
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

TEST(GGEMSDiagnosticProjection, ScalesExactAndBinary32Sentinels) {
  struct Case {
    float component;
    std::int64_t expected;
  };

  constexpr std::array<Case, 11U> k_cases{{
      {.component = 0.0F, .expected = 0LL},
      {.component = -0.0F, .expected = 0LL},
      {.component = 1.0F, .expected = 1'000'000'000'000LL},
      {.component = -1.0F, .expected = -1'000'000'000'000LL},
      {.component = 0.5F, .expected = 500'000'000'000LL},
      {.component = -0.5F, .expected = -500'000'000'000LL},
      {.component = 0x1p-13F, .expected = 122'070'313LL},
      {.component = -0x1p-13F, .expected = -122'070'313LL},
      {.component = std::numbers::inv_sqrt3_v<float>,
       .expected = 577'350'258'827LL},
      {.component = 0.1F, .expected = 100'000'001'490LL},
      {.component = -0.1F, .expected = -100'000'001'490LL},
  }};

  for (Case const &test : k_cases) {
    std::int64_t result{k_output_sentinel};

    ASSERT_TRUE(TryScaleDiagnosticProjectionComponent(test.component, result));
    EXPECT_EQ(result, test.expected);
  }
}

TEST(GGEMSDiagnosticProjection, HandlesSubnormalAndRejectsInvalidValues) {
  std::int64_t result{k_output_sentinel};

  ASSERT_TRUE(TryScaleDiagnosticProjectionComponent(
      std::numeric_limits<float>::denorm_min(), result));
  EXPECT_EQ(result, 0LL);

  constexpr std::array<float, 4U> k_invalid_values{
      std::numeric_limits<float>::quiet_NaN(),
      std::numeric_limits<float>::infinity(),
      -std::numeric_limits<float>::infinity(),
      std::numeric_limits<float>::max()};

  for (float value : k_invalid_values) {
    result = k_output_sentinel;
    EXPECT_FALSE(TryScaleDiagnosticProjectionComponent(value, result));
    EXPECT_EQ(result, k_output_sentinel);
  }
}

TEST(GGEMSDiagnosticProjection, IsSymmetricForRepresentablePairs) {
  constexpr std::array<float, 5U> k_values{
      0.1F, 0.5F, 0x1p-13F, std::numbers::inv_sqrt3_v<float>, 1.25F};

  for (float value : k_values) {
    std::int64_t positive{0LL};
    std::int64_t negative{0LL};

    ASSERT_TRUE(TryScaleDiagnosticProjectionComponent(value, positive));
    ASSERT_TRUE(TryScaleDiagnosticProjectionComponent(-value, negative));

    EXPECT_EQ(positive, -negative);
  }
}

TEST(GGEMSDiagnosticProjection, AddsWithoutSignedOverflowOrPartialWrite) {
  struct Case {
    std::int64_t position;
    std::int64_t displacement;
    bool succeeds;
    std::int64_t expected;
  };

  constexpr auto k_max = std::numeric_limits<std::int64_t>::max();
  constexpr auto k_min = std::numeric_limits<std::int64_t>::min();

  constexpr std::array<Case, 7U> k_cases{{
      {.position = 10LL,
       .displacement = 5LL,
       .succeeds = true,
       .expected = 15LL},
      {.position = k_max - 1LL,
       .displacement = 1LL,
       .succeeds = true,
       .expected = k_max},
      {.position = k_min + 1LL,
       .displacement = -1LL,
       .succeeds = true,
       .expected = k_min},
      {.position = 0LL,
       .displacement = k_min,
       .succeeds = true,
       .expected = k_min},
      {.position = 0LL,
       .displacement = k_max,
       .succeeds = true,
       .expected = k_max},
      {.position = k_max,
       .displacement = 1LL,
       .succeeds = false,
       .expected = k_output_sentinel},
      {.position = k_min,
       .displacement = -1LL,
       .succeeds = false,
       .expected = k_output_sentinel},
  }};

  for (Case const &test : k_cases) {
    std::int64_t endpoint{k_output_sentinel};

    EXPECT_EQ(TryAddDiagnosticProjectionDisplacement(
                  test.position, test.displacement, endpoint),
              test.succeeds);
    EXPECT_EQ(endpoint, test.expected);
  }
}

TEST_F(GGEMSDiagnosticProjectionKernelTest, OpenCLMirrorMatchesHost) {
  constexpr auto k_max = std::numeric_limits<std::int64_t>::max();
  constexpr auto k_min = std::numeric_limits<std::int64_t>::min();

  std::array<float, 16U> components{0.0F,
                                    -0.0F,
                                    1.0F,
                                    -1.0F,
                                    0.5F,
                                    -0.5F,
                                    0x1p-13F,
                                    -0x1p-13F,
                                    std::numbers::inv_sqrt3_v<float>,
                                    -std::numbers::inv_sqrt3_v<float>,
                                    0.1F,
                                    -0.1F,
                                    std::numeric_limits<float>::denorm_min(),
                                    std::numeric_limits<float>::quiet_NaN(),
                                    std::numeric_limits<float>::infinity(),
                                    std::numeric_limits<float>::max()};

  std::array<std::int64_t, components.size()> scaled_components{};
  scaled_components.fill(k_output_sentinel);
  std::array<std::uint32_t, components.size()> scale_success{};

  std::array<std::int64_t, 7U> positions{10LL, k_max - 1LL, k_min + 1LL, 0LL,
                                         0LL,  k_max,       k_min};

  std::array<std::int64_t, positions.size()> displacements{
      5LL, 1LL, -1LL, k_min, k_max, 1LL, -1LL};

  std::array<std::int64_t, positions.size()> endpoints{};
  endpoints.fill(k_output_sentinel);
  std::array<std::uint32_t, positions.size()> addition_success{};
  std::array<std::uint64_t, 1U> distance_pm{};

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = GetContext();

  auto make_buffer =
      [&context](std::uint64_t size) -> ggems::ocl::GGEMSOpenCLSVMBuffer {
    return context.CreateSVMBuffer(ggems::units::Bytes{size});
  };

  auto components_buffer = make_buffer(sizeof(components));
  auto scaled_buffer = make_buffer(sizeof(scaled_components));
  auto scale_success_buffer = make_buffer(sizeof(scale_success));
  auto positions_buffer = make_buffer(sizeof(positions));
  auto displacements_buffer = make_buffer(sizeof(displacements));
  auto endpoints_buffer = make_buffer(sizeof(endpoints));
  auto addition_success_buffer = make_buffer(sizeof(addition_success));
  auto distance_buffer = make_buffer(sizeof(distance_pm));

  ggems::ocl::WriteSVMFromHost(components_buffer, std::span{components});
  ggems::ocl::WriteSVMFromHost(scaled_buffer, std::span{scaled_components});
  ggems::ocl::WriteSVMFromHost(scale_success_buffer, std::span{scale_success});
  ggems::ocl::WriteSVMFromHost(positions_buffer, std::span{positions});
  ggems::ocl::WriteSVMFromHost(displacements_buffer, std::span{displacements});
  ggems::ocl::WriteSVMFromHost(endpoints_buffer, std::span{endpoints});
  ggems::ocl::WriteSVMFromHost(addition_success_buffer,
                               std::span{addition_success});
  ggems::ocl::WriteSVMFromHost(distance_buffer, std::span{distance_pm});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";
  std::string const build_options =
      std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

  auto &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "diagnostic_projection_probe", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("diagnostic_projection_probe");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "diagnostic_projection_probe"};

  kernel.SetArgSVMPointer(0U, components_buffer.GetData());
  kernel.SetArgSVMPointer(1U, scaled_buffer.GetData());
  kernel.SetArgSVMPointer(2U, scale_success_buffer.GetData());
  kernel.SetArg(3U, static_cast<cl_uint>(components.size()));
  kernel.SetArgSVMPointer(4U, positions_buffer.GetData());
  kernel.SetArgSVMPointer(5U, displacements_buffer.GetData());
  kernel.SetArgSVMPointer(6U, endpoints_buffer.GetData());
  kernel.SetArgSVMPointer(7U, addition_success_buffer.GetData());
  kernel.SetArg(8U, static_cast<cl_uint>(positions.size()));
  kernel.SetArgSVMPointer(9U, distance_buffer.GetData());
  kernel.Run({components.size()}, {1U});

  ggems::ocl::ReadSVMToHost(scaled_buffer, std::span{scaled_components});
  ggems::ocl::ReadSVMToHost(scale_success_buffer, std::span{scale_success});
  ggems::ocl::ReadSVMToHost(endpoints_buffer, std::span{endpoints});
  ggems::ocl::ReadSVMToHost(addition_success_buffer,
                            std::span{addition_success});
  ggems::ocl::ReadSVMToHost(distance_buffer, std::span{distance_pm});

  EXPECT_EQ(distance_pm[0U],
            ggems::core::transport::k_diagnostic_projection_distance_pm);

  for (std::size_t index = 0U; index < components.size(); ++index) {
    std::int64_t host_result{k_output_sentinel};
    bool const host_success =
        TryScaleDiagnosticProjectionComponent(components[index], host_result);

    EXPECT_EQ(scale_success[index], host_success ? 1U : 0U) << index;
    EXPECT_EQ(scaled_components[index], host_result) << index;
  }

  for (std::size_t index = 0U; index < positions.size(); ++index) {
    std::int64_t host_result{k_output_sentinel};
    bool const host_success = TryAddDiagnosticProjectionDisplacement(
        positions[index], displacements[index], host_result);

    EXPECT_EQ(addition_success[index], host_success ? 1U : 0U) << index;
    EXPECT_EQ(endpoints[index], host_result) << index;
  }
}
