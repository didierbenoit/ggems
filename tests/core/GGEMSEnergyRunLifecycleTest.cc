#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>
#include <iostream>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

using DistributionType = ggems::core::sources::GGEMSEnergyDistributionType;
using Source = ggems::core::sources::GGEMSSource;

// =============================================================================
// =============================================================================

class TemporarySpectrumFile {
public:
  TemporarySpectrumFile(std::string_view name, std::string_view content)
      : path_{std::filesystem::path{::testing::TempDir()} / name} {
    std::ofstream output{path_, std::ios::binary};
    output << content;
  }

  ~TemporarySpectrumFile() {
    std::error_code ignored;
    std::filesystem::remove(path_, ignored);
  }

  [[nodiscard]] auto GetPath() const noexcept -> std::filesystem::path const & {
    return path_;
  }

private:
  std::filesystem::path path_;
};

// =============================================================================
// =============================================================================

struct EnergyState {

  DistributionType type{DistributionType::Unknown};
  std::uint64_t source_record_energy_milli_eV{0ULL};
  std::uint64_t mono_energy_milli_eV{0ULL};
  std::uint64_t regular_bin_width_milli_eV{0ULL};
  std::vector<std::uint64_t> energy_values_milli_eV;
  std::vector<double> relative_weights;
  std::vector<std::uint64_t> cumulative_ticket_upper;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto CaptureEnergyState(Source const &source) -> EnergyState {
  auto const &distribution = source.GetEnergyDistribution();
  auto const values = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const ticket_bounds = distribution.GetCumulativeTicketUpperBounds();

  return {
      .type = distribution.GetType(),
      .source_record_energy_milli_eV = source.BuildRecord().energy_milli_eV,
      .mono_energy_milli_eV = distribution.GetMonoEnergyMilliElectronVolt(),
      .regular_bin_width_milli_eV =
          distribution.GetRegularBinWidthMilliElectronVolt(),
      .energy_values_milli_eV = {values.begin(), values.end()},
      .relative_weights = {weights.begin(), weights.end()},
      .cumulative_ticket_upper = {ticket_bounds.begin(), ticket_bounds.end()},
  };
}

// =============================================================================
// =============================================================================

auto ExpectEnergyState(Source const &source, EnergyState const &expected)
    -> void {
  auto const &distribution = source.GetEnergyDistribution();
  EXPECT_EQ(distribution.GetType(), expected.type);
  EXPECT_EQ(source.BuildRecord().energy_milli_eV,
            expected.source_record_energy_milli_eV);
  EXPECT_EQ(distribution.GetMonoEnergyMilliElectronVolt(),
            expected.mono_energy_milli_eV);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(),
            expected.regular_bin_width_milli_eV);
  EXPECT_TRUE(
      std::ranges::equal(distribution.GetEnergyValuesMilliElectronVolt(),
                         expected.energy_values_milli_eV));
  EXPECT_TRUE(std::ranges::equal(distribution.GetRelativeWeights(),
                                 expected.relative_weights));
  EXPECT_TRUE(std::ranges::equal(distribution.GetCumulativeTicketUpperBounds(),
                                 expected.cumulative_ticket_upper));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom()
    -> std::shared_ptr<ggems::core::random::GGEMSRandom> {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox").SetSeed(9'876'543ULL);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSource() -> std::shared_ptr<Source> {
  auto source = std::make_shared<Source>();
  source->SetPrimaryCount(1ULL)
      .SetPointEmission()
      .SetFixedAngularDistribution();
  return source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetTotalOpenCLAllocationCount() -> std::size_t {
  std::size_t allocation_count{0U};

  for (auto const &context :
       ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    allocation_count += context.GetAllocationCountVRAM();
  }

  return allocation_count;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetTotalOpenCLAllocatedBytes() -> std::uint64_t {
  std::uint64_t allocated_bytes{0ULL};

  for (auto const &context :
       ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    allocated_bytes += context.GetAllocatedVRAM().value;
  }

  return allocated_bytes;
}

// =============================================================================
// =============================================================================

template <typename Function>
auto ExpectFinalizedRejection(Function &&function) -> void {
  try {
    std::forward<Function>(function)();
    FAIL() << "Expected finalized source energy rejection.";
  } catch (ggems::core::GGEMSExceptionBase const &exception) {
    EXPECT_NE(std::string_view{exception.what()}.find(
                  "after source initialization has been finalized."),
              std::string_view::npos);
  }
}

// =============================================================================
// =============================================================================

class GGEMSEnergyRunLifecycleTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       FailedInitialiseLeavesEnergyMutableAndRetryable) {
  constexpr std::array<double, 2U> k_lines{1.0, 3.0};
  constexpr std::array<double, 2U> k_weights{1.0, 1.0};
  constexpr std::array<double, 3U> k_centers{10.0, 12.0, 14.0};
  constexpr std::array<double, 3U> k_bin_weights{1.0, 2.0, 1.0};

  auto source = MakeSource();
  source->SetDiscreteEnergyLines(k_lines, k_weights, "MeV");

  ggems::core::GGEMSRun run{};
  run.SetSource(source);
  run.SetWorkerCount(64U);

  EXPECT_THROW(run.Initialise(), ggems::core::GGEMSExceptionBase);
  EXPECT_NO_THROW(source->SetCountDrivenPopulation(2ULL));
  EXPECT_EQ(source->GetPopulationMode(),
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source->GetPrimaryCount(), 2ULL);
  EXPECT_NO_THROW(
      source->SetRegularEnergySpectrum(k_centers, k_bin_weights, "MeV"));

  run.SetRandom(MakeRandom());
  ASSERT_NO_THROW(run.Initialise());

  EnergyState const finalized = CaptureEnergyState(*source);
  ExpectFinalizedRejection(
      [&]() -> void { source->SetEnergyMilliElectronVolt(90'000'000ULL); });
  ExpectEnergyState(*source, finalized);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       SuccessfulInitialiseRejectsEveryEnergySetterWithoutMutation) {
  constexpr std::array<double, 2U> k_initial_centers{10.0, 12.0};
  constexpr std::array<double, 2U> k_initial_weights{1.0, 3.0};
  constexpr std::array<double, 2U> k_lines{2.0, 6.0};
  constexpr std::array<double, 2U> k_line_weights{1.0, 1.0};
  constexpr std::array<double, 3U> k_replacement_centers{20.0, 22.0, 24.0};
  constexpr std::array<double, 3U> k_replacement_weights{1.0, 2.0, 1.0};
  TemporarySpectrumFile const valid_file{"ggems-energy-finalized-source.dat",
                                         "0.020 1.0\n0.022 2.0\n0.024 1.0\n"};

  auto source = MakeSource();
  source->SetRegularEnergySpectrum(k_initial_centers, k_initial_weights, "MeV");

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.SetSource(source);
  run.SetWorkerCount(64U);
  ASSERT_NO_THROW(run.Initialise());

  EnergyState const finalized = CaptureEnergyState(*source);
  auto const expect_unchanged = [&]() -> void {
    ExpectEnergyState(*source, finalized);
  };

  ExpectFinalizedRejection(
      [&]() -> void { source->SetEnergyMilliElectronVolt(511'000'000ULL); });
  expect_unchanged();
  ExpectFinalizedRejection([&]() -> void {
    source->SetDiscreteEnergyLines(k_lines, k_line_weights, "MeV");
  });
  expect_unchanged();
  ExpectFinalizedRejection([&]() -> void {
    source->SetRegularEnergySpectrum(k_replacement_centers,
                                     k_replacement_weights, "MeV");
  });
  expect_unchanged();
  ExpectFinalizedRejection([&]() -> void {
    source->LoadRegularEnergySpectrum(valid_file.GetPath(), "MeV");
  });
  expect_unchanged();

  Source replacement{};
  replacement.SetEnergyMilliElectronVolt(90'000'000ULL);
  ExpectFinalizedRejection([&]() -> void { *source = replacement; });
  expect_unchanged();
  ExpectFinalizedRejection([&]() -> void {
    Source moved{std::move(*source)};
    static_cast<void>(moved);
  });
  expect_unchanged();

  Source copied{*source};
  ExpectFinalizedRejection(
      [&]() -> void { copied.SetEnergyMilliElectronVolt(90'000'000ULL); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       SuccessfulInitialiseFreezesPopulationModeButKeepsCountMutable) {
  auto source = MakeSource();

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.SetSource(source);
  run.SetWorkerCount(64U);
  ASSERT_NO_THROW(run.Initialise());

  auto radionuclide = std::make_shared<
      ggems::core::radioactivity::GGEMSRadionuclideDefinition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());

  ExpectFinalizedRejection([&]() -> void {
    source->SetActivityDrivenRadionuclide(radionuclide,
                                          ggems::units::Activity{100.0L}, 0ULL);
  });
  ExpectFinalizedRejection(
      [&]() -> void { source->SetCountDrivenPopulation(7ULL); });

  EXPECT_EQ(source->GetPopulationMode(),
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_NO_THROW(source->SetPrimaryCount(7ULL));
  EXPECT_EQ(source->GetPrimaryCount(), 7ULL);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       DuplicateSlotsAndCompatibleRunsFinalizeIdempotently) {
  constexpr std::array<double, 2U> k_lines{2.0, 6.0};
  constexpr std::array<double, 2U> k_weights{1.0, 1.0};

  auto source = MakeSource();
  source->SetDiscreteEnergyLines(k_lines, k_weights, "MeV");

  ggems::core::GGEMSRun first_run{};
  first_run.SetRandom(MakeRandom());
  first_run.AddSource(source);
  first_run.AddSource(source);
  first_run.SetWorkerCount(64U);
  ASSERT_NO_THROW(first_run.Initialise());

  ggems::core::GGEMSRun second_run{};
  second_run.SetRandom(MakeRandom());
  second_run.SetSource(source);
  second_run.SetWorkerCount(64U);
  ASSERT_NO_THROW(second_run.Initialise());

  EnergyState const finalized = CaptureEnergyState(*source);
  ExpectFinalizedRejection(
      [&]() -> void { source->SetEnergyMilliElectronVolt(90'000'000ULL); });
  ExpectEnergyState(*source, finalized);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       RepeatedRunsReuseImmutableEnergyWithMutablePoseAndCount) {
  constexpr std::array<double, 3U> k_centers{10.0, 12.0, 14.0};
  constexpr std::array<double, 3U> k_weights{1.0, 2.0, 1.0};

  auto source = MakeSource();
  source->SetPrimaryCount(2ULL)
      .SetPositionPicoMeter(1LL, 2LL, 3LL)
      .SetDirection(0.0, 0.0, 1.0)
      .SetRegularEnergySpectrum(k_centers, k_weights, "MeV");

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.SetSource(source);
  run.SetWorkerCount(64U);
  ASSERT_NO_THROW(run.Initialise());

  std::size_t const allocation_count = GetTotalOpenCLAllocationCount();
  std::uint64_t const allocated_bytes = GetTotalOpenCLAllocatedBytes();

  ASSERT_NO_THROW(run.Run());
  auto first = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(first.has_value());
  ASSERT_EQ(first->GetRecords().size(), 1U);
  ASSERT_EQ(first->GetRanges().size(), 1U);
  EXPECT_EQ(first->GetRanges()[0U].primary_count, 2ULL);
  EXPECT_EQ(first->GetRecords()[0U].position_x_pm, 1LL);
  EXPECT_EQ(GetTotalOpenCLAllocationCount(), allocation_count);
  EXPECT_EQ(GetTotalOpenCLAllocatedBytes(), allocated_bytes);

  source->SetPrimaryCount(3ULL)
      .SetPositionPicoMeter(-4LL, 5LL, -6LL)
      .SetOrientation({0.0, 1.0, 0.0}, {0.0, 0.0, 1.0});

  ASSERT_NO_THROW(run.Run());
  auto second = run.GetLastSourceRunSnapshot();
  ASSERT_TRUE(second.has_value());
  ASSERT_EQ(second->GetRecords().size(), 1U);
  ASSERT_EQ(second->GetRanges().size(), 1U);
  EXPECT_EQ(second->GetRanges()[0U].primary_count, 3ULL);
  EXPECT_EQ(second->GetRecords()[0U].position_x_pm, -4LL);
  EXPECT_FLOAT_EQ(second->GetRecords()[0U].axis_z_y, 1.0F);

  ASSERT_EQ(first->GetEnergyDistributionRecords().size(),
            second->GetEnergyDistributionRecords().size());
  EXPECT_EQ(first->GetEnergyDistributionRecords().data(),
            second->GetEnergyDistributionRecords().data());
  EXPECT_EQ(first->GetEnergyValuesMilliElectronVolt(),
            second->GetEnergyValuesMilliElectronVolt());
  EXPECT_EQ(first->GetRelativeWeights(), second->GetRelativeWeights());
  EXPECT_EQ(first->GetCumulativeTicketUpperBounds(),
            second->GetCumulativeTicketUpperBounds());
  EXPECT_EQ(first->GetEnergyValuesMilliElectronVolt().data(),
            second->GetEnergyValuesMilliElectronVolt().data());
  EXPECT_EQ(first->GetRelativeWeights().data(),
            second->GetRelativeWeights().data());
  EXPECT_EQ(first->GetCumulativeTicketUpperBounds().data(),
            second->GetCumulativeTicketUpperBounds().data());
  EXPECT_EQ(GetTotalOpenCLAllocationCount(), allocation_count);
  EXPECT_EQ(GetTotalOpenCLAllocatedBytes(), allocated_bytes);
}
