#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <fstream>
#include <iterator>

#include <gtest/gtest.h>

#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"

namespace {

// =============================================================================
// =============================================================================

using AngularType = ggems::core::sources::GGEMSAngularDistributionType;
using EnergyDistributionRecord =
    ggems::core::sources::GGEMSEnergyDistributionRecord;
using EnergyType = ggems::core::sources::GGEMSEnergyDistributionType;
using GeometryType = ggems::core::sources::GGEMSEmissionGeometryType;
using Random = ggems::core::random::GGEMSRandom;
using Source = ggems::core::sources::GGEMSSource;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;

// =============================================================================
// =============================================================================

struct SamplingResult {
  std::array<std::int64_t, 3U> position{};
  std::array<float, 3U> direction{};
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildSourceRecord(GeometryType geometry, AngularType angular,
                                     bool bounded = false) -> SourceRecord {
  Source source{};

  switch (geometry) {
  case GeometryType::Point:
    source.SetPointEmission();
    break;
  case GeometryType::Rectangle:
    source.SetRectangleEmissionPicoMeter(20ULL, 10ULL);
    break;
  case GeometryType::Ellipse:
    source.SetEllipseEmissionPicoMeter(20ULL, 10ULL);
    break;
  case GeometryType::Box:
    source.SetBoxEmissionPicoMeter(20ULL, 10ULL, 8ULL);
    break;
  case GeometryType::Sphere:
    source.SetSphereEmissionPicoMeter(20ULL);
    break;
  case GeometryType::Cylinder:
    source.SetCylinderEmissionPicoMeter(20ULL, 10ULL);
    break;
  case GeometryType::Unknown:
    ADD_FAILURE() << "Unknown geometry in test setup";
    break;
  }

  switch (angular) {
  case AngularType::Fixed:
    source.SetFixedAngularDistribution();
    break;
  case AngularType::Focused:
    source.SetFocusedAngularDistributionPicoMeter(0LL, 0LL, 10'000LL);
    break;
  case AngularType::Isotropic:
    if (bounded) {
      source.SetIsotropicAngularDistribution(
          ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L),
          ggems::units::MakeDegrees(-45.0L), ggems::units::MakeDegrees(45.0L));
    } else {
      source.SetIsotropicAngularDistribution();
    }
    break;
  case AngularType::Unknown:
    ADD_FAILURE() << "Unknown angular distribution in test setup";
    break;
  }

  SourceRecord record = source.BuildRecord();
  record.time_start_ps = 123ULL;
  record.time_stop_ps = 456ULL;
  return record;
}

// =============================================================================
// =============================================================================

class GGEMSSourceSamplingKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  static auto Context() -> ggems::ocl::GGEMSOpenCLContext & {
    return ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  }

  [[nodiscard]] static auto BuildOptions(Random const &random) -> std::string {
    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    return std::format("-cl-std=CL2.0 -I{} {}", kernel_root.generic_string(),
                       random.GetKernelBuildDefinition());
  }

  [[nodiscard]] static auto FixedBuildOptions() -> std::string {
    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    return std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=3",
                       kernel_root.generic_string());
  }

  [[nodiscard]] static auto
  RunImposedProbe(SourceRecord const &source,
                  std::array<float, 8U> const &uniforms) -> SamplingResult {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    SamplingResult result{};

    auto source_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(source)});
    auto uniform_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(uniforms)});
    auto position_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(result.position)});
    auto direction_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(result.direction)});

    ggems::ocl::WriteSVMFromHost(source_buffer, source);
    ggems::ocl::WriteSVMFromHost(uniform_buffer, std::span{uniforms});
    ggems::ocl::WriteSVMFromHost(position_buffer, std::span{result.position});
    ggems::ocl::WriteSVMFromHost(direction_buffer, std::span{result.direction});

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program =
        opencl.GetOrCreateProgram(context, kernel_test_root,
                                  "source_sampling_probe", FixedBuildOptions());
    cl::Kernel raw_kernel =
        program.CreateKernel("source_sampling_imposed_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "source_sampling_imposed_probe"};

    kernel.SetArgSVMPointer(0U, source_buffer.GetData());
    kernel.SetArgSVMPointer(1U, uniform_buffer.GetData());
    kernel.SetArgSVMPointer(2U, position_buffer.GetData());
    kernel.SetArgSVMPointer(3U, direction_buffer.GetData());
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(position_buffer, std::span{result.position});
    ggems::ocl::ReadSVMToHost(direction_buffer, std::span{result.direction});
    return result;
  }

  static auto ExpectDrawPlan(SourceRecord const &source,
                             std::string_view engine,
                             std::uint32_t expected_position_vector_count,
                             std::uint32_t expected_direction_vector_count,
                             bool table_energy) -> void {

    Random random{};
    random.SetEngine(engine).SetSeed(77'777ULL);

    std::vector<std::byte> initial_state(random.GetStateSize());
    random.InitializeStates(0ULL, std::span<std::byte>{initial_state});
    std::vector<std::byte> sample_state = initial_state;
    std::vector<std::byte> reference_state = initial_state;

    EnergyDistributionRecord distribution{
        .regular_bin_width_milli_eV = 0ULL,
        .table_offset = 0ULL,
        .distribution_type =
            ggems::core::sources::ToKernelEnergyDistributionType(
                table_energy ? EnergyType::DiscreteLines : EnergyType::Mono),
        .table_count = table_energy ? 2U : 0U};
    constexpr std::array<std::uint64_t, 2U> energy_values{40ULL, 80ULL};
    constexpr std::array<std::uint64_t, 2U> ticket_upper{2'147'483'648ULL,
                                                         4'294'967'296ULL};
    std::array<std::int64_t, 6U> sampled_positions{};
    std::array<float, 6U> sampled_directions{};
    std::array<std::uint64_t, 4U> sampled_values{};

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    auto sample_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sample_state.size()});
    auto reference_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{reference_state.size()});
    auto source_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(source)});
    auto distribution_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(distribution)});
    auto energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(energy_values)});
    auto ticket_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(ticket_upper)});
    auto position_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_positions)});
    auto direction_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(sampled_directions)});
    auto value_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_values)});

    ggems::ocl::WriteSVMFromHost(sample_buffer,
                                 std::span<std::byte const>{sample_state});
    ggems::ocl::WriteSVMFromHost(reference_buffer,
                                 std::span<std::byte const>{reference_state});
    ggems::ocl::WriteSVMFromHost(source_buffer, source);
    ggems::ocl::WriteSVMFromHost(distribution_buffer, distribution);
    ggems::ocl::WriteSVMFromHost(energy_buffer, std::span{energy_values});
    ggems::ocl::WriteSVMFromHost(ticket_buffer, std::span{ticket_upper});
    ggems::ocl::WriteSVMFromHost(position_buffer, std::span{sampled_positions});
    ggems::ocl::WriteSVMFromHost(direction_buffer,
                                 std::span{sampled_directions});
    ggems::ocl::WriteSVMFromHost(value_buffer, std::span{sampled_values});

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(context, kernel_test_root,
                                              "source_sampling_probe",
                                              BuildOptions(random));
    cl::Kernel raw_kernel =
        program.CreateKernel("source_initialization_random_state_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, std::move(raw_kernel),
        "source_initialization_random_state_probe"};

    kernel.SetArgSVMPointer(0U, sample_buffer.GetData());
    kernel.SetArgSVMPointer(1U, reference_buffer.GetData());
    kernel.SetArgSVMPointer(2U, source_buffer.GetData());
    kernel.SetArgSVMPointer(3U, distribution_buffer.GetData());
    kernel.SetArgSVMPointer(4U, energy_buffer.GetData());
    kernel.SetArgSVMPointer(5U, ticket_buffer.GetData());
    kernel.SetArg(6U, static_cast<cl_uint>(expected_position_vector_count));
    kernel.SetArg(7U, static_cast<cl_uint>(expected_direction_vector_count));
    kernel.SetArg(8U, static_cast<cl_uint>(table_energy ? 1U : 0U));
    kernel.SetArg(9U, static_cast<cl_ulong>(97ULL));
    kernel.SetArgSVMPointer(10U, position_buffer.GetData());
    kernel.SetArgSVMPointer(11U, direction_buffer.GetData());
    kernel.SetArgSVMPointer(12U, value_buffer.GetData());
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(sample_buffer,
                              std::span<std::byte>{sample_state});
    ggems::ocl::ReadSVMToHost(reference_buffer,
                              std::span<std::byte>{reference_state});
    ggems::ocl::ReadSVMToHost(position_buffer, std::span{sampled_positions});
    ggems::ocl::ReadSVMToHost(direction_buffer, std::span{sampled_directions});
    ggems::ocl::ReadSVMToHost(value_buffer, std::span{sampled_values});

    EXPECT_EQ(sample_state, reference_state);
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      EXPECT_EQ(sampled_positions[axis], sampled_positions[axis + 3U]);
      EXPECT_EQ(std::bit_cast<std::uint32_t>(sampled_directions[axis]),
                std::bit_cast<std::uint32_t>(sampled_directions[axis + 3U]));
    }
    EXPECT_EQ(sampled_values[0U], source.time_start_ps);
    EXPECT_EQ(sampled_values[0U], sampled_values[1U]);
    EXPECT_EQ(sampled_values[2U], sampled_values[3U]);
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       SamplesPlanarPositionsFromImposedVectors) {
  Source rectangle{};
  rectangle.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetRectangleEmissionPicoMeter(8ULL, 12ULL);
  SamplingResult const rectangle_result =
      RunImposedProbe(rectangle.BuildRecord(),
                      {0.75F, 0.25F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(rectangle_result.position,
            (std::array<std::int64_t, 3U>{102LL, 197LL, 300LL}));

  Source ellipse{};
  ellipse.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetEllipseEmissionPicoMeter(8ULL, 12ULL);
  SamplingResult const ellipse_result = RunImposedProbe(
      ellipse.BuildRecord(), {0.25F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(ellipse_result.position,
            (std::array<std::int64_t, 3U>{102LL, 200LL, 300LL}));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       FocusedDirectionUsesImposedSampledPosition) {
  Source rectangle{};
  rectangle.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetRectangleEmissionPicoMeter(8ULL, 12ULL)
      .SetFocusedAngularDistributionPicoMeter(102LL, 197LL, 1'300LL);

  SamplingResult const result =
      RunImposedProbe(rectangle.BuildRecord(),
                      {0.75F, 0.25F, 0.0F, 0.0F, 0.9F, 0.1F, 0.8F, 0.2F});

  EXPECT_EQ(result.position,
            (std::array<std::int64_t, 3U>{102LL, 197LL, 300LL}));
  EXPECT_NEAR(result.direction[0U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(result.direction[1U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(result.direction[2U], 1.0F, 1.0e-6F);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       SamplesVolumePositionsFromImposedVectors) {
  Source box{};
  box.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetBoxEmissionPicoMeter(8ULL, 12ULL, 16ULL);
  SamplingResult const box_result = RunImposedProbe(
      box.BuildRecord(), {0.75F, 0.25F, 0.5F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(box_result.position,
            (std::array<std::int64_t, 3U>{100LL, 202LL, 297LL}));

  Source sphere{};
  sphere.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetSphereEmissionPicoMeter(8ULL);
  SamplingResult const sphere_result = RunImposedProbe(
      sphere.BuildRecord(), {0.125F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(sphere_result.position,
            (std::array<std::int64_t, 3U>{102LL, 200LL, 300LL}));

  Source cylinder{};
  cylinder.SetPositionPicoMeter(100LL, 200LL, 300LL)
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetCylinderEmissionPicoMeter(8ULL, 8ULL);
  SamplingResult const cylinder_result =
      RunImposedProbe(cylinder.BuildRecord(),
                      {0.25F, 0.0F, 0.75F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(cylinder_result.position,
            (std::array<std::int64_t, 3U>{102LL, 202LL, 300LL}));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       DefaultFullSphereRemainsGlobalAndBitwiseFrameInvariant) {
  Source source_a{};
  source_a.SetPointEmission().SetIsotropicAngularDistribution();
  Source source_b{};
  source_b.SetPointEmission()
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetIsotropicAngularDistribution();

  std::array<float, 8U> const uniforms{0.0F,  0.0F,   0.0F, 0.0F,
                                       0.25F, 0.375F, 0.7F, 0.2F};
  auto const result_a = RunImposedProbe(source_a.BuildRecord(), uniforms);
  auto const result_b = RunImposedProbe(source_b.BuildRecord(), uniforms);

  for (std::size_t axis = 0U; axis < result_a.direction.size(); ++axis) {
    EXPECT_EQ(std::bit_cast<std::uint32_t>(result_a.direction[axis]),
              std::bit_cast<std::uint32_t>(result_b.direction[axis]));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       BoundedIsotropicUsesCosineAndStoredSourceFrame) {
  Source source{};
  source.SetPointEmission()
      .SetOrientation({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0})
      .SetIsotropicAngularDistribution(
          ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L),
          ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L));

  SamplingResult const result = RunImposedProbe(
      source.BuildRecord(), {0.0F, 0.0F, 0.0F, 0.0F, 0.5F, 0.0F, 0.0F, 0.0F});

  EXPECT_NEAR(result.direction[0U], 0.5F, 1.0e-6F);
  EXPECT_NEAR(result.direction[1U], 0.8660254F, 1.0e-6F);
  EXPECT_NEAR(result.direction[2U], 0.0F, 1.0e-6F);
  float const norm = std::hypot(result.direction[0U], result.direction[1U],
                                result.direction[2U]);
  EXPECT_NEAR(norm, 1.0F, 1.0e-6F);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest, SamplesBoxCenterAndHalfOpenSides) {
  constexpr std::uint64_t k_size_pm{16'777'216ULL};
  constexpr std::int64_t k_half_size_pm{8'388'608LL};
  Source box{};
  box.SetBoxEmissionPicoMeter(k_size_pm, k_size_pm, k_size_pm);

  auto const center = RunImposedProbe(
      box.BuildRecord(), {0.5F, 0.5F, 0.5F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(center.position, (std::array<std::int64_t, 3U>{0LL, 0LL, 0LL}));

  auto const lower = RunImposedProbe(
      box.BuildRecord(), {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(lower.position,
            (std::array<std::int64_t, 3U>{-k_half_size_pm, -k_half_size_pm,
                                          -k_half_size_pm}));

  float const upper_uniform = std::nextafter(1.0F, 0.0F);
  auto const upper = RunImposedProbe(
      box.BuildRecord(), {upper_uniform, upper_uniform, upper_uniform, 0.0F,
                          0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(upper.position, (std::array<std::int64_t, 3U>{
                                k_half_size_pm - 1LL, k_half_size_pm - 1LL,
                                k_half_size_pm - 1LL}));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       SamplesSphereCubeRootRadiusAndKeepsPointsInside) {
  Source sphere{};
  sphere.SetSphereEmissionPicoMeter(8ULL);

  auto const zero_radius = RunImposedProbe(
      sphere.BuildRecord(), {0.0F, 0.75F, 0.5F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(zero_radius.position,
            (std::array<std::int64_t, 3U>{0LL, 0LL, 0LL}));

  auto const cube_root_radius = RunImposedProbe(
      sphere.BuildRecord(), {0.125F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(cube_root_radius.position,
            (std::array<std::int64_t, 3U>{0LL, 0LL, 2LL}));

  Source large_sphere{};
  constexpr std::uint64_t k_diameter_pm{2'000'000ULL};
  large_sphere.SetSphereEmissionPicoMeter(k_diameter_pm);
  auto const interior = RunImposedProbe(large_sphere.BuildRecord(),
                                        {std::nextafter(1.0F, 0.0F), 0.25F,
                                         0.125F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  long double const radius =
      std::hypot(static_cast<long double>(interior.position[0U]),
                 static_cast<long double>(interior.position[1U]),
                 static_cast<long double>(interior.position[2U]));
  EXPECT_LE(radius, (0.5L * static_cast<long double>(k_diameter_pm)) + 1.0L);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       SamplesCylinderAreaAzimuthAndHalfOpenHeight) {
  Source cylinder{};
  constexpr std::uint64_t k_height_pm{16'777'216ULL};
  constexpr std::int64_t k_half_height_pm{8'388'608LL};
  cylinder.SetCylinderEmissionPicoMeter(8ULL, k_height_pm);

  auto const lower =
      RunImposedProbe(cylinder.BuildRecord(),
                      {0.0F, 0.75F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(lower.position,
            (std::array<std::int64_t, 3U>{0LL, 0LL, -k_half_height_pm}));

  float const upper_uniform = std::nextafter(1.0F, 0.0F);
  auto const upper =
      RunImposedProbe(cylinder.BuildRecord(), {0.0F, 0.0F, upper_uniform, 0.0F,
                                               0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_EQ(upper.position,
            (std::array<std::int64_t, 3U>{0LL, 0LL, k_half_height_pm - 1LL}));

  auto const radial =
      RunImposedProbe(cylinder.BuildRecord(),
                      {0.25F, 0.25F, 0.5F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_NEAR(static_cast<double>(radial.position[0U]), 0.0, 1.0);
  EXPECT_EQ(radial.position[1U], 2LL);
  EXPECT_EQ(radial.position[2U], 0LL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       BoundedIsotropicUsesLocalCardinalConventionsAndVolumeSecondVector) {
  Source point{};
  point.SetPointEmission().SetIsotropicAngularDistribution(
      ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L),
      ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L));

  auto const plus_x = RunImposedProbe(
      point.BuildRecord(), {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_NEAR(plus_x.direction[0U], 1.0F, 1.0e-6F);
  EXPECT_NEAR(plus_x.direction[1U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(plus_x.direction[2U], 0.0F, 1.0e-6F);

  auto const plus_y = RunImposedProbe(
      point.BuildRecord(), {0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F});
  EXPECT_NEAR(plus_y.direction[0U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(plus_y.direction[1U], 1.0F, 1.0e-6F);
  EXPECT_NEAR(plus_y.direction[2U], 0.0F, 1.0e-6F);

  auto const plus_z = RunImposedProbe(
      point.BuildRecord(), {0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F});
  EXPECT_NEAR(plus_z.direction[0U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(plus_z.direction[1U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(plus_z.direction[2U], 1.0F, 1.0e-6F);

  Source box{};
  box.SetBoxEmissionPicoMeter(8ULL, 8ULL, 8ULL)
      .SetIsotropicAngularDistribution(
          ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L),
          ggems::units::MakeDegrees(0.0L), ggems::units::MakeDegrees(90.0L));
  auto const volume = RunImposedProbe(
      box.BuildRecord(), {0.5F, 0.5F, 0.5F, 0.9F, 0.5F, 0.0F, 0.0F, 0.0F});
  EXPECT_NEAR(volume.direction[0U], 0.8660254F, 1.0e-6F);
  EXPECT_NEAR(volume.direction[1U], 0.0F, 1.0e-6F);
  EXPECT_NEAR(volume.direction[2U], 0.5F, 1.0e-6F);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSSourceSamplingKernelTest,
       ExactSourceInitializationDrawPlanForEveryEngine) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<GeometryType, 6U> k_geometries{
      GeometryType::Point, GeometryType::Rectangle, GeometryType::Ellipse,
      GeometryType::Box,   GeometryType::Sphere,    GeometryType::Cylinder};

  constexpr std::array<AngularType, 3U> k_angular_distributions{
      AngularType::Fixed, AngularType::Focused, AngularType::Isotropic};

  for (std::string_view const engine : k_engines) {
    for (GeometryType const geometry : k_geometries) {
      for (AngularType const angular : k_angular_distributions) {
        std::size_t const bounded_case_count =
            angular == AngularType::Isotropic ? 2U : 1U;

        for (std::size_t bounded_case = 0U; bounded_case < bounded_case_count;
             ++bounded_case) {
          bool const bounded = bounded_case != 0U;
          std::uint32_t const position_draw_count =
              geometry == GeometryType::Point ? 0U : 1U;
          std::uint32_t const direction_draw_count =
              angular == AngularType::Isotropic ? 1U : 0U;

          SCOPED_TRACE(engine);
          SCOPED_TRACE(static_cast<std::uint32_t>(geometry));
          SCOPED_TRACE(static_cast<std::uint32_t>(angular));
          SCOPED_TRACE(bounded);

          SourceRecord const record =
              BuildSourceRecord(geometry, angular, bounded);
          ExpectDrawPlan(record, engine, position_draw_count,
                         direction_draw_count, false);
          ExpectDrawPlan(record, engine, position_draw_count,
                         direction_draw_count, true);
        }
      }
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceSamplingKernelSource,
     ProductionTransportDelegatesRandomSamplingToSource) {
  std::filesystem::path const transport_path =
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT} / "transport" /
      "particle_stream_transport.cl";
  std::ifstream transport_file{transport_path};
  ASSERT_TRUE(transport_file.is_open()) << transport_path;

  std::string const transport_source{
      std::istreambuf_iterator<char>{transport_file},
      std::istreambuf_iterator<char>{}};

  EXPECT_NE(transport_source.find("GGEMS_SourceTryInitializePrimary"),
            std::string::npos);
  EXPECT_EQ(transport_source.find("GGEMS_Rndm"), std::string::npos);
  EXPECT_EQ(transport_source.find("primary_random_values"), std::string::npos);
  EXPECT_EQ(transport_source.find("secondary_random_values"),
            std::string::npos);
  EXPECT_EQ(transport_source.find("GGEMS_SourceRandomVectorDrawCount"),
            std::string::npos);
  EXPECT_EQ(transport_source.find("GGEMS_EnergyDistributionSample("),
            std::string::npos);
}
