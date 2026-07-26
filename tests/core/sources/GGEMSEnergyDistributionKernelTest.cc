#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"

namespace {

// =============================================================================
// =============================================================================

using Distribution = ggems::core::sources::GGEMSEnergyDistribution;
using EnergyRecord = ggems::core::sources::GGEMSEnergyDistributionRecord;
using EnergyType = ggems::core::sources::GGEMSEnergyDistributionType;
using Random = ggems::core::random::GGEMSRandom;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;

// =============================================================================
// =============================================================================

struct ProbeInput {
  SourceRecord source{};
  EnergyRecord distribution{};
  std::array<std::uint64_t, 3U> energies{};
  std::array<std::uint64_t, 3U> cumulative_ticket_upper{};
  std::uint32_t expected_draw_count{0U};
};

// =============================================================================
// =============================================================================

struct RawProbeResult {
  std::uint32_t raw_output{0U};
  float uniform_output{0.0F};
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoInput() -> ProbeInput {
  ProbeInput input{};
  input.source.energy_milli_eV = 511'000'000ULL;
  input.distribution.distribution_type =
      ggems::core::sources::ToKernelEnergyDistributionType(EnergyType::Mono);
  return input;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDiscreteInput() -> ProbeInput {
  ProbeInput input{};
  input.source.energy_milli_eV = 0ULL;
  input.distribution.distribution_type =
      ggems::core::sources::ToKernelEnergyDistributionType(
          EnergyType::DiscreteLines);
  input.distribution.table_count = 3U;
  input.energies = {2'000'000'000ULL, 4'000'000'000ULL, 6'000'000'000ULL};
  input.cumulative_ticket_upper = {2'147'483'648ULL, 2'147'483'648ULL,
                                   4'294'967'296ULL};
  input.expected_draw_count = 1U;
  return input;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRegularInput() -> ProbeInput {
  ProbeInput input{};
  input.source.energy_milli_eV = 0ULL;
  input.distribution.regular_bin_width_milli_eV = 2'000'000'000ULL;
  input.distribution.distribution_type =
      ggems::core::sources::ToKernelEnergyDistributionType(
          EnergyType::RegularSpectrum);
  input.distribution.table_count = 3U;
  input.energies = {10'000'000'000ULL, 12'000'000'000ULL, 14'000'000'000ULL};
  input.cumulative_ticket_upper = {2'147'483'648ULL, 2'147'483'648ULL,
                                   4'294'967'296ULL};
  input.expected_draw_count = 1U;
  return input;
}

// =============================================================================
// =============================================================================

class GGEMSEnergyDistributionKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
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

  [[nodiscard]] static auto RunSamplingProbe(Random const &random,
                                             ProbeInput const &input)
      -> std::uint64_t {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();

    std::vector<std::byte> initial_state(random.GetStateSize());
    random.InitialiseStates(0ULL, std::span<std::byte>{initial_state});
    std::vector<std::byte> sample_state = initial_state;
    std::vector<std::byte> reference_state = initial_state;
    std::uint64_t sampled_energy{0ULL};

    auto sample_state_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sample_state.size()});
    auto reference_state_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{reference_state.size()});
    auto source_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(SourceRecord)});
    auto distribution_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(EnergyRecord)});
    auto energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(input.energies)});
    auto ticket_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(input.cumulative_ticket_upper)});
    auto sampled_energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_energy)});

    ggems::ocl::WriteSVMFromHost(sample_state_buffer,
                                 std::span<std::byte const>{sample_state});
    ggems::ocl::WriteSVMFromHost(reference_state_buffer,
                                 std::span<std::byte const>{reference_state});
    ggems::ocl::WriteSVMFromHost(source_buffer, input.source);
    ggems::ocl::WriteSVMFromHost(distribution_buffer, input.distribution);
    ggems::ocl::WriteSVMFromHost(energy_buffer, std::span{input.energies});
    ggems::ocl::WriteSVMFromHost(ticket_buffer,
                                 std::span{input.cumulative_ticket_upper});
    ggems::ocl::WriteSVMFromHost(sampled_energy_buffer, sampled_energy);

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "energy_distribution_sampling_probe",
        BuildOptions(random));
    cl::Kernel raw_kernel =
        program.CreateKernel("energy_distribution_sampling_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "energy_distribution_sampling_probe"};

    kernel.SetArgSVMPointer(0U, sample_state_buffer.GetData());
    kernel.SetArgSVMPointer(1U, reference_state_buffer.GetData());
    kernel.SetArgSVMPointer(2U, source_buffer.GetData());
    kernel.SetArgSVMPointer(3U, distribution_buffer.GetData());
    kernel.SetArgSVMPointer(4U, energy_buffer.GetData());
    kernel.SetArgSVMPointer(5U, ticket_buffer.GetData());
    kernel.SetArg(6U, static_cast<cl_uint>(input.expected_draw_count));
    kernel.SetArgSVMPointer(7U, sampled_energy_buffer.GetData());
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(sample_state_buffer,
                              std::span<std::byte>{sample_state});
    ggems::ocl::ReadSVMToHost(reference_state_buffer,
                              std::span<std::byte>{reference_state});
    sampled_energy =
        ggems::ocl::ReadSVMToHost<std::uint64_t>(sampled_energy_buffer);

    EXPECT_EQ(sample_state, reference_state);
    return sampled_energy;
  }

  [[nodiscard]] static auto RunExplicitTicketProbe(ProbeInput const &input,
                                                   std::uint32_t raw_ticket)
      -> std::uint64_t {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    std::uint64_t sampled_energy{0ULL};

    auto distribution_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(EnergyRecord)});
    auto energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(input.energies)});
    auto ticket_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(input.cumulative_ticket_upper)});
    auto sampled_energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_energy)});

    ggems::ocl::WriteSVMFromHost(distribution_buffer, input.distribution);
    ggems::ocl::WriteSVMFromHost(energy_buffer, std::span{input.energies});
    ggems::ocl::WriteSVMFromHost(ticket_buffer,
                                 std::span{input.cumulative_ticket_upper});
    ggems::ocl::WriteSVMFromHost(sampled_energy_buffer, sampled_energy);

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "energy_distribution_sampling_probe",
        FixedBuildOptions());
    cl::Kernel raw_kernel =
        program.CreateKernel("energy_distribution_sample_ticket_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, std::move(raw_kernel),
        "energy_distribution_sample_ticket_probe"};

    kernel.SetArgSVMPointer(0U, distribution_buffer.GetData());
    kernel.SetArgSVMPointer(1U, energy_buffer.GetData());
    kernel.SetArgSVMPointer(2U, ticket_buffer.GetData());
    kernel.SetArg(3U, static_cast<cl_uint>(raw_ticket));
    kernel.SetArgSVMPointer(4U, sampled_energy_buffer.GetData());
    kernel.Run({1U}, {1U});

    return ggems::ocl::ReadSVMToHost<std::uint64_t>(sampled_energy_buffer);
  }

  [[nodiscard]] static auto
  RunFindIndexProbe(std::span<std::uint64_t const> cumulative_ticket_upper,
                    std::uint32_t raw_ticket) -> std::uint32_t {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    EnergyRecord distribution{
        .distribution_type =
            ggems::core::sources::ToKernelEnergyDistributionType(
                EnergyType::DiscreteLines),
        .table_count =
            static_cast<std::uint32_t>(cumulative_ticket_upper.size())};
    std::uint32_t selected_index{std::numeric_limits<std::uint32_t>::max()};

    auto distribution_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(distribution)});
    auto ticket_buffer = context.CreateSVMBuffer(ggems::units::Bytes{
        cumulative_ticket_upper.size() * sizeof(std::uint64_t)});
    auto result_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(selected_index)});
    ggems::ocl::WriteSVMFromHost(distribution_buffer, distribution);
    ggems::ocl::WriteSVMFromHost(ticket_buffer, cumulative_ticket_upper);
    ggems::ocl::WriteSVMFromHost(result_buffer, selected_index);

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "energy_distribution_sampling_probe",
        FixedBuildOptions());
    cl::Kernel raw_kernel =
        program.CreateKernel("energy_distribution_find_index_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, std::move(raw_kernel), "energy_distribution_find_index_probe"};

    kernel.SetArgSVMPointer(0U, distribution_buffer.GetData());
    kernel.SetArgSVMPointer(1U, ticket_buffer.GetData());
    kernel.SetArg(2U, static_cast<cl_uint>(raw_ticket));
    kernel.SetArgSVMPointer(3U, result_buffer.GetData());
    kernel.Run({1U}, {1U});

    return ggems::ocl::ReadSVMToHost<std::uint32_t>(result_buffer);
  }

  [[nodiscard]] static auto RunRegularOffsetProbe(std::uint64_t width,
                                                  std::uint64_t ticket_span,
                                                  std::uint64_t local_ticket)
      -> std::uint64_t {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    std::uint64_t offset{0ULL};
    auto result_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(offset)});
    ggems::ocl::WriteSVMFromHost(result_buffer, offset);

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "energy_distribution_sampling_probe",
        FixedBuildOptions());
    cl::Kernel raw_kernel =
        program.CreateKernel("energy_distribution_regular_offset_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, std::move(raw_kernel),
        "energy_distribution_regular_offset_probe"};

    kernel.SetArg(0U, static_cast<cl_ulong>(width));
    kernel.SetArg(1U, static_cast<cl_ulong>(ticket_span));
    kernel.SetArg(2U, static_cast<cl_ulong>(local_ticket));
    kernel.SetArgSVMPointer(3U, result_buffer.GetData());
    kernel.Run({1U}, {1U});

    return ggems::ocl::ReadSVMToHost<std::uint64_t>(result_buffer);
  }

  [[nodiscard]] static auto RunRawEquivalenceProbe(Random const &random)
      -> RawProbeResult {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto &context = Context();
    std::vector<std::byte> initial_state(random.GetStateSize());
    random.InitialiseStates(0ULL, std::span<std::byte>{initial_state});
    std::vector<std::byte> raw_state = initial_state;
    std::vector<std::byte> uniform_state = initial_state;
    RawProbeResult result{};

    auto raw_state_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{raw_state.size()});
    auto uniform_state_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{uniform_state.size()});
    auto raw_output_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(result.raw_output)});
    auto uniform_output_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(result.uniform_output)});
    ggems::ocl::WriteSVMFromHost(raw_state_buffer,
                                 std::span<std::byte const>{raw_state});
    ggems::ocl::WriteSVMFromHost(uniform_state_buffer,
                                 std::span<std::byte const>{uniform_state});
    ggems::ocl::WriteSVMFromHost(raw_output_buffer, result.raw_output);
    ggems::ocl::WriteSVMFromHost(uniform_output_buffer, result.uniform_output);

    std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path const kernel_test_root = kernel_root / "tests";
    auto &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "energy_distribution_sampling_probe",
        BuildOptions(random));
    cl::Kernel raw_kernel =
        program.CreateKernel("random_raw_scalar_equivalence_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_raw_scalar_equivalence_probe"};

    kernel.SetArgSVMPointer(0U, raw_state_buffer.GetData());
    kernel.SetArgSVMPointer(1U, uniform_state_buffer.GetData());
    kernel.SetArgSVMPointer(2U, raw_output_buffer.GetData());
    kernel.SetArgSVMPointer(3U, uniform_output_buffer.GetData());
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(raw_state_buffer,
                              std::span<std::byte>{raw_state});
    ggems::ocl::ReadSVMToHost(uniform_state_buffer,
                              std::span<std::byte>{uniform_state});
    result.raw_output =
        ggems::ocl::ReadSVMToHost<std::uint32_t>(raw_output_buffer);
    result.uniform_output =
        ggems::ocl::ReadSVMToHost<float>(uniform_output_buffer);

    EXPECT_EQ(raw_state, uniform_state);
    return result;
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionKernelTest,
       RawApiMatchesScalarStateProgressionAndRetainsAllBits) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint32_t, 3U> k_expected_raw{
      169'984'787U, 2'105'060'176U, 2'855'362'018U};

  for (std::size_t engine_index = 0U; engine_index < k_engines.size();
       ++engine_index) {
    SCOPED_TRACE(k_engines[engine_index]);
    Random random{};
    random.SetEngine(k_engines[engine_index]).SetSeed(44'444ULL);

    RawProbeResult const result = RunRawEquivalenceProbe(random);
    EXPECT_EQ(result.raw_output, k_expected_raw[engine_index]);
    EXPECT_NE(result.raw_output & 0xffU, 0U);
    float const expected_uniform =
        static_cast<float>(result.raw_output >> 8U) * 5.9604644775390625e-8F;
    EXPECT_FLOAT_EQ(result.uniform_output, expected_uniform);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionKernelTest,
       ExactEnergyDrawCountForEveryEngineAndMode) {
  constexpr std::array<std::string_view, 3U> k_engines{"jkiss", "pcg32",
                                                       "philox"};
  constexpr std::array<std::uint64_t, 3U> k_expected_discrete{
      2'000'000'000ULL, 2'000'000'000ULL, 6'000'000'000ULL};
  constexpr std::array<std::uint64_t, 3U> k_expected_regular{
      9'158'310'669ULL, 10'960'490'062ULL, 13'659'263'106ULL};

  for (std::size_t engine_index = 0U; engine_index < k_engines.size();
       ++engine_index) {
    std::string_view const engine = k_engines[engine_index];
    SCOPED_TRACE(engine);
    Random random{};
    random.SetEngine(engine).SetSeed(44'444ULL);

    EXPECT_EQ(RunSamplingProbe(random, MakeMonoInput()), 511'000'000ULL);
    EXPECT_EQ(RunSamplingProbe(random, MakeDiscreteInput()),
              k_expected_discrete[engine_index]);
    EXPECT_EQ(RunSamplingProbe(random, MakeRegularInput()),
              k_expected_regular[engine_index]);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionKernelTest,
       RawTicketBoundariesSkipZeroWidthEntries) {
  constexpr std::array<std::uint64_t, 5U> k_bounds{
      2ULL, 2ULL, 5ULL, 4'294'967'296ULL, 4'294'967'296ULL};
  EXPECT_EQ(RunFindIndexProbe(k_bounds, 0U), 0U);
  EXPECT_EQ(RunFindIndexProbe(k_bounds, 1U), 0U);
  EXPECT_EQ(RunFindIndexProbe(k_bounds, 2U), 2U);
  EXPECT_EQ(RunFindIndexProbe(k_bounds, 4U), 2U);
  EXPECT_EQ(RunFindIndexProbe(k_bounds, 5U), 3U);
  EXPECT_EQ(
      RunFindIndexProbe(k_bounds, std::numeric_limits<std::uint32_t>::max()),
      3U);

  constexpr std::array<std::uint64_t, 4U> k_leading_plateau{0ULL, 0ULL, 3ULL,
                                                            4'294'967'296ULL};
  EXPECT_EQ(RunFindIndexProbe(k_leading_plateau, 0U), 2U);

  ProbeInput discrete = MakeDiscreteInput();
  discrete.cumulative_ticket_upper = {2ULL, 2ULL, 4'294'967'296ULL};
  EXPECT_EQ(RunExplicitTicketProbe(discrete, 0U), 2'000'000'000ULL);
  EXPECT_EQ(RunExplicitTicketProbe(discrete, 1U), 2'000'000'000ULL);
  EXPECT_EQ(RunExplicitTicketProbe(discrete, 2U), 6'000'000'000ULL);
  EXPECT_EQ(RunExplicitTicketProbe(discrete,
                                   std::numeric_limits<std::uint32_t>::max()),
            6'000'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionKernelTest,
       SuppliedCtFirstTwoPositiveBinsAreReachable) {
  std::filesystem::path const path =
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}.parent_path() /
      "validation" / "source" / "data" / "spectrum_120kVp_2mmAl.dat";
  Distribution const distribution =
      Distribution::LoadRegularSpectrum(path, "MeV");
  auto const bounds = distribution.GetCumulativeTicketUpperBounds();

  ASSERT_GE(bounds.size(), 2U);
  ASSERT_GT(bounds[0U], 0ULL);
  ASSERT_GT(bounds[1U], bounds[0U]);
  ASSERT_LE(bounds[1U], 4'294'967'296ULL);
  ASSERT_LE(bounds[0U], static_cast<std::uint64_t>(
                            std::numeric_limits<std::uint32_t>::max()));
  ASSERT_LE(bounds[1U] - 1ULL, static_cast<std::uint64_t>(
                                   std::numeric_limits<std::uint32_t>::max()));

  EXPECT_EQ(RunFindIndexProbe(bounds, 0U), 0U);
  EXPECT_EQ(RunFindIndexProbe(bounds, static_cast<std::uint32_t>(bounds[0U])),
            1U);
  EXPECT_EQ(
      RunFindIndexProbe(bounds, static_cast<std::uint32_t>(bounds[1U] - 1ULL)),
      1U);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyDistributionKernelTest,
       RegularMappingIsExactBoundedAndOverflowSafe) {
  ProbeInput regular = MakeRegularInput();
  EXPECT_EQ(RunExplicitTicketProbe(regular, 0U), 9'000'000'000ULL);
  EXPECT_EQ(RunExplicitTicketProbe(regular, 2'147'483'647U), 10'999'999'999ULL);
  EXPECT_EQ(RunExplicitTicketProbe(regular, 2'147'483'648U), 13'000'000'000ULL);
  EXPECT_EQ(RunExplicitTicketProbe(regular,
                                   std::numeric_limits<std::uint32_t>::max()),
            14'999'999'999ULL);

  regular.cumulative_ticket_upper = {1ULL, 1ULL, 4'294'967'296ULL};
  EXPECT_EQ(RunExplicitTicketProbe(regular, 0U), 9'000'000'000ULL);

  constexpr std::array<std::uint64_t, 5U> k_width_3_expected{0ULL, 0ULL, 1ULL,
                                                             1ULL, 2ULL};
  constexpr std::array<std::uint64_t, 5U> k_width_5_expected{0ULL, 1ULL, 2ULL,
                                                             3ULL, 4ULL};
  constexpr std::array<std::uint64_t, 5U> k_width_13_expected{0ULL, 2ULL, 5ULL,
                                                              7ULL, 10ULL};

  for (std::uint64_t local = 0ULL; local < 5ULL; ++local) {
    EXPECT_EQ(RunRegularOffsetProbe(3ULL, 5ULL, local),
              k_width_3_expected[static_cast<std::size_t>(local)]);
    EXPECT_EQ(RunRegularOffsetProbe(5ULL, 5ULL, local),
              k_width_5_expected[static_cast<std::size_t>(local)]);
    EXPECT_EQ(RunRegularOffsetProbe(13ULL, 5ULL, local),
              k_width_13_expected[static_cast<std::size_t>(local)]);
  }

  EXPECT_EQ(RunRegularOffsetProbe(123ULL, 1ULL, 0ULL), 0ULL);
  EXPECT_EQ(RunRegularOffsetProbe(std::numeric_limits<std::uint64_t>::max(),
                                  4'294'967'296ULL, 4'294'967'295ULL),
            18'446'744'069'414'584'319ULL);
  EXPECT_EQ(
      RunRegularOffsetProbe(std::numeric_limits<std::uint64_t>::max() - 1ULL,
                            4'294'967'296ULL, 4'294'967'295ULL),
      18'446'744'069'414'584'318ULL);
}
