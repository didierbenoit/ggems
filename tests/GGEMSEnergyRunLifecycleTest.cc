// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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
#include <ios>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"

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
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output << content;
    output.close();
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
  std::uint64_t source_record_energy_micro_eV{0ULL};
  std::uint64_t mono_energy_micro_eV{0ULL};
  std::uint64_t regular_bin_width_micro_eV{0ULL};
  std::vector<std::uint64_t> energy_values_micro_eV;
  std::vector<double> relative_weights;
  std::vector<std::uint64_t> cumulative_ticket_upper;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto CaptureEnergyState(Source const &source) -> EnergyState {
  auto const &distribution = source.GetEnergyDistribution();
  auto const values = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const ticket_bounds = distribution.GetCumulativeTicketUpperBounds();

  return {
    .type = distribution.GetType(),
    .source_record_energy_micro_eV = source.BuildRecord().energy_micro_eV,
    .mono_energy_micro_eV = distribution.GetMonoEnergyMicroElectronVolt(),
    .regular_bin_width_micro_eV =
      distribution.GetRegularBinWidthMicroElectronVolt(),
    .energy_values_micro_eV = {values.begin(), values.end()},
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
  EXPECT_EQ(source.BuildRecord().energy_micro_eV,
            expected.source_record_energy_micro_eV);
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            expected.mono_energy_micro_eV);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(),
            expected.regular_bin_width_micro_eV);
  EXPECT_TRUE(
    std::ranges::equal(distribution.GetEnergyValuesMicroElectronVolt(),
                       expected.energy_values_micro_eV));
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
    FAIL() << "Expected finalized source configuration rejection.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
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
      opencl.Initialize();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }

  auto SetUp() -> void override {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    previous_worker_count_ = opencl.GetWorkerCount();
    opencl.SetWorkerCount(64U);
  }

  auto TearDown() -> void override {
    ggems::ocl::GGEMSOpenCL::GetInstance().SetWorkerCount(
      previous_worker_count_);
  }

private:
  std::uint32_t previous_worker_count_{0U};
};

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       FailedInitializeLeavesEnergyMutableAndRetryable) {
  constexpr std::array<double, 2U> k_lines{1.0, 3.0};
  constexpr std::array<double, 2U> k_weights{1.0, 1.0};
  constexpr std::array<double, 3U> k_centers{10.0, 12.0, 14.0};
  constexpr std::array<double, 3U> k_bin_weights{1.0, 2.0, 1.0};

  auto source = MakeSource();
  source->SetDiscreteEnergyLines(k_lines, k_weights, "MeV");

  ggems::core::GGEMSRun run{};
  run.AddSource(source);

  EXPECT_THROW(run.Initialize(), ggems::core::GGEMSRecoverable);
  EXPECT_NO_THROW(source->SetCountDrivenPopulation(2ULL));
  EXPECT_EQ(source->GetPopulationMode(),
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source->GetPrimaryCount(), 2ULL);
  EXPECT_NO_THROW(
    source->SetRegularEnergySpectrum(k_centers, k_bin_weights, "MeV"));

  run.SetRandom(MakeRandom());
  ASSERT_NO_THROW(run.Initialize());

  EnergyState const finalized = CaptureEnergyState(*source);
  ExpectFinalizedRejection(
    [&] -> void { source->SetEnergyMicroElectronVolt(90'000'000'000ULL); });
  ExpectEnergyState(*source, finalized);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       SuccessfulInitializeRejectsEveryEnergySetterWithoutMutation) {
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
  run.AddSource(source);
  ASSERT_NO_THROW(run.Initialize());

  EnergyState const finalized = CaptureEnergyState(*source);
  auto const expect_unchanged = [&] -> void {
    ExpectEnergyState(*source, finalized);
  };

  ExpectFinalizedRejection(
    [&] -> void { source->SetEnergyMicroElectronVolt(511'000'000'000ULL); });
  expect_unchanged();
  ExpectFinalizedRejection([&] -> void {
    source->SetDiscreteEnergyLines(k_lines, k_line_weights, "MeV");
  });
  expect_unchanged();
  ExpectFinalizedRejection([&] -> void {
    source->SetRegularEnergySpectrum(k_replacement_centers,
                                     k_replacement_weights, "MeV");
  });
  expect_unchanged();
  ExpectFinalizedRejection([&] -> void {
    source->LoadRegularEnergySpectrum(valid_file.GetPath(), "MeV");
  });
  expect_unchanged();

  Source replacement{};
  replacement.SetEnergyMicroElectronVolt(90'000'000'000ULL);
  ExpectFinalizedRejection([&] -> void { *source = replacement; });
  expect_unchanged();
  ExpectFinalizedRejection([&] -> void {
    Source moved{std::move(*source)};
    static_cast<void>(moved);
  });
  expect_unchanged();

  Source copied{*source};
  ExpectFinalizedRejection(
    [&] -> void { copied.SetEnergyMicroElectronVolt(90'000'000'000ULL); });
  ExpectEnergyState(copied, finalized);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSEnergyRunLifecycleTest,
       SuccessfulInitializeFreezesPopulationModeButKeepsCountMutable) {
  auto source = MakeSource();

  ggems::core::GGEMSRun run{};
  run.SetRandom(MakeRandom());
  run.AddSource(source);
  ASSERT_NO_THROW(run.Initialize());

  auto radionuclide = std::make_shared<
    ggems::core::radioactivity::GGEMSRadionuclideDefinition const>(
    ggems::core::radioactivity::builtins::BuildF18Radionuclide());

  ExpectFinalizedRejection([&] -> void {
    source->SetRadionuclide(radionuclide, ggems::units::Activity{100.0L}, 0ULL);
  });
  ExpectFinalizedRejection(
    [&] -> void { source->SetCountDrivenPopulation(7ULL); });

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
  ASSERT_NO_THROW(first_run.Initialize());

  ggems::core::GGEMSRun second_run{};
  second_run.SetRandom(MakeRandom());
  second_run.AddSource(source);
  ASSERT_NO_THROW(second_run.Initialize());

  EnergyState const finalized = CaptureEnergyState(*source);
  ExpectFinalizedRejection(
    [&]() -> void { source->SetEnergyMicroElectronVolt(90'000'000'000ULL); });
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
  run.AddSource(source);
  ASSERT_NO_THROW(run.Initialize());

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

  EXPECT_EQ(first->GetRanges()[0U].primary_count, 2ULL);
  EXPECT_EQ(first->GetRecords()[0U].position_x_pm, 1LL);
  EXPECT_FLOAT_EQ(first->GetRecords()[0U].axis_z_z, 1.0F);

  ASSERT_EQ(first->GetEnergyDistributionRecords().size(),
            second->GetEnergyDistributionRecords().size());
  EXPECT_EQ(first->GetEnergyDistributionRecords().data(),
            second->GetEnergyDistributionRecords().data());
  EXPECT_EQ(first->GetEnergyValuesMicroElectronVolt(),
            second->GetEnergyValuesMicroElectronVolt());
  EXPECT_EQ(first->GetRelativeWeights(), second->GetRelativeWeights());
  EXPECT_EQ(first->GetCumulativeTicketUpperBounds(),
            second->GetCumulativeTicketUpperBounds());
  EXPECT_EQ(first->GetEnergyValuesMicroElectronVolt().data(),
            second->GetEnergyValuesMicroElectronVolt().data());
  EXPECT_EQ(first->GetRelativeWeights().data(),
            second->GetRelativeWeights().data());
  EXPECT_EQ(first->GetCumulativeTicketUpperBounds().data(),
            second->GetCumulativeTicketUpperBounds().data());
  EXPECT_EQ(GetTotalOpenCLAllocationCount(), allocation_count);
  EXPECT_EQ(GetTotalOpenCLAllocatedBytes(), allocated_bytes);
}
