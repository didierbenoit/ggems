#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"

namespace {

using EnergyDistributionRecord =
    ggems::core::sources::GGEMSEnergyDistributionRecord;
using EnergyType = ggems::core::sources::GGEMSEnergyDistributionType;
using EmissionRecord = ggems::core::sources::GGEMSSourceEmissionRecord;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using PopulationRecord = ggems::core::sources::GGEMSSourcePopulationRecord;
using Random = ggems::core::random::GGEMSRandom;
using RandomEngine = ggems::core::random::GGEMSRandomEngine;
using Source = ggems::core::sources::GGEMSSource;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;

// =============================================================================
// =============================================================================

constexpr std::array<RandomEngine, 3U> k_engines{
    RandomEngine::JKISS, RandomEngine::PCG32, RandomEngine::Philox};

// =============================================================================
// =============================================================================

class GGEMSActivitySourceRandomOrderTest : public ::testing::Test {
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

TEST_F(GGEMSActivitySourceRandomOrderTest,
       ConsumesTimeThenPositionThenDirectionThenEnergyForEveryEngine) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};

  Source source{};
  source.SetPositionPicoMeter(101LL, 202LL, 303LL)
      .SetRectangleEmissionPicoMeter(20ULL, 10ULL)
      .SetIsotropicAngularDistribution();
  SourceRecord source_record = source.BuildRecord();
  source_record.time_start_ps = 8'000'000'000'000'000ULL;
  source_record.time_stop_ps =
      source_record.time_start_ps + 1'000'000'000'000ULL;

  PopulationRecord const population{
      .population_mode = ggems::core::sources::ToKernelSourcePopulationMode(
          ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven),
      .first_emission_index = 0U,
      .emission_count = 1U,
      .scaled_decay = 0.125F};
  EmissionRecord const emission{
      .particle_type =
          ggems::core::particles::ToKernelParticleType(ParticleType::Gamma),
      .energy_distribution_record_index = 0U,
      .mono_energy_milli_eV = 0ULL};
  EnergyDistributionRecord const energy_distribution{
      .regular_bin_width_milli_eV = 0ULL,
      .table_offset = 0ULL,
      .distribution_type = ggems::core::sources::ToKernelEnergyDistributionType(
          EnergyType::DiscreteLines),
      .table_count = 2U};
  constexpr std::array<std::uint64_t, 2U> k_energy_values{40ULL, 80ULL};
  constexpr std::array<std::uint64_t, 2U> k_ticket_upper{2'147'483'648ULL,
                                                         4'294'967'296ULL};

  for (RandomEngine const engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    Random random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    std::vector<std::byte> initial_state(random.GetStateSize());
    random.InitializeStates(0ULL, std::span<std::byte>{initial_state});
    std::vector<std::byte> sample_state = initial_state;
    std::vector<std::byte> reference_state = initial_state;

    std::array<std::int64_t, 6U> sampled_positions{};
    std::array<float, 6U> sampled_directions{};
    std::array<std::uint64_t, 4U> sampled_values{};
    std::array<std::uint32_t, 2U> next_words{};

    auto sample_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sample_state.size()});
    auto reference_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{reference_state.size()});
    auto source_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(source_record)});
    auto population_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(population)});
    auto emission_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(emission)});
    auto distribution_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(energy_distribution)});
    auto energy_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(k_energy_values)});
    auto ticket_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(k_ticket_upper)});
    auto position_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_positions)});
    auto direction_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{sizeof(sampled_directions)});
    auto value_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(sampled_values)});
    auto next_word_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{sizeof(next_words)});

    ggems::ocl::WriteSVMFromHost(sample_buffer,
                                 std::span<std::byte const>{sample_state});
    ggems::ocl::WriteSVMFromHost(reference_buffer,
                                 std::span<std::byte const>{reference_state});
    ggems::ocl::WriteSVMFromHost(source_buffer, source_record);
    ggems::ocl::WriteSVMFromHost(population_buffer, population);
    ggems::ocl::WriteSVMFromHost(emission_buffer, emission);
    ggems::ocl::WriteSVMFromHost(distribution_buffer, energy_distribution);
    ggems::ocl::WriteSVMFromHost(energy_buffer, std::span{k_energy_values});
    ggems::ocl::WriteSVMFromHost(ticket_buffer, std::span{k_ticket_upper});
    ggems::ocl::WriteSVMFromHost(position_buffer, std::span{sampled_positions});
    ggems::ocl::WriteSVMFromHost(direction_buffer,
                                 std::span{sampled_directions});
    ggems::ocl::WriteSVMFromHost(value_buffer, std::span{sampled_values});
    ggems::ocl::WriteSVMFromHost(next_word_buffer, std::span{next_words});

    std::string const options =
        std::format("-cl-std=CL2.0 -I{} {}", root.generic_string(),
                    random.GetKernelBuildDefinition());
    auto const &program = opencl.GetOrCreateProgram(
        context, root / "tests", "activity_source_random_order_probe", options);
    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, program.CreateKernel("activity_source_random_order_probe"),
        "activity_source_random_order_probe"};

    kernel.SetArgSVMPointer(0U, sample_buffer.GetData());
    kernel.SetArgSVMPointer(1U, reference_buffer.GetData());
    kernel.SetArgSVMPointer(2U, source_buffer.GetData());
    kernel.SetArgSVMPointer(3U, population_buffer.GetData());
    kernel.SetArgSVMPointer(4U, emission_buffer.GetData());
    kernel.SetArgSVMPointer(5U, distribution_buffer.GetData());
    kernel.SetArgSVMPointer(6U, energy_buffer.GetData());
    kernel.SetArgSVMPointer(7U, ticket_buffer.GetData());
    kernel.SetArgSVMPointer(8U, position_buffer.GetData());
    kernel.SetArgSVMPointer(9U, direction_buffer.GetData());
    kernel.SetArgSVMPointer(10U, value_buffer.GetData());
    kernel.SetArgSVMPointer(11U, next_word_buffer.GetData());
    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(sample_buffer,
                              std::span<std::byte>{sample_state});
    ggems::ocl::ReadSVMToHost(reference_buffer,
                              std::span<std::byte>{reference_state});
    ggems::ocl::ReadSVMToHost(position_buffer, std::span{sampled_positions});
    ggems::ocl::ReadSVMToHost(direction_buffer, std::span{sampled_directions});
    ggems::ocl::ReadSVMToHost(value_buffer, std::span{sampled_values});
    ggems::ocl::ReadSVMToHost(next_word_buffer, std::span{next_words});

    EXPECT_EQ(sample_state, reference_state);
    EXPECT_EQ(next_words[0U], next_words[1U]);
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      EXPECT_EQ(sampled_positions[axis], sampled_positions[axis + 3U]);
      EXPECT_EQ(std::bit_cast<std::uint32_t>(sampled_directions[axis]),
                std::bit_cast<std::uint32_t>(sampled_directions[axis + 3U]));
    }
    EXPECT_EQ(sampled_values[0U], sampled_values[1U]);
    EXPECT_EQ(sampled_values[2U], sampled_values[3U]);
    EXPECT_GE(sampled_values[0U], source_record.time_start_ps);
    EXPECT_LT(sampled_values[0U], source_record.time_stop_ps);
  }
}
