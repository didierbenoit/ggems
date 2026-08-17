#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceDescription.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"

namespace {

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeSnapshotSource(
    ggems::core::sources::GGEMSSourceRunSnapshot const &snapshot,
    std::size_t source_index) -> std::string {
  return ggems::core::sources::DescribeSource(
      snapshot.GetRecords().at(source_index),
      snapshot.GetRanges().at(source_index).primary_count,
      snapshot.GetEnergyDistributionRecords().at(source_index),
      snapshot.GetEnergyValuesMilliElectronVolt());
}

// =============================================================================
// =============================================================================

auto ExpectContains(std::string const &description, std::string_view expected)
    -> void {
  EXPECT_NE(description.find(expected), std::string::npos)
      << "Description: " << description;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDescription, DescribesExactMonoEnergy) {
  ggems::core::sources::GGEMSSource source{};
  source.SetEnergyMilliElectronVolt(100'000'000ULL);

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);
  std::string const description = DescribeSnapshotSource(snapshot, 0U);

  ExpectContains(description, "Energy: Mono (100.0000000 keV)");
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDescription, DescribesDiscreteLinesWithoutDumpingTable) {
  constexpr std::array<double, 3U> energies{40.0, 80.0, 120.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};

  ggems::core::sources::GGEMSSource source{};
  source.SetDiscreteEnergyLines(energies, weights, "keV");

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);
  std::string const description = DescribeSnapshotSource(snapshot, 0U);

  ExpectContains(description, "Energy: Discrete lines");
  ExpectContains(description, "Line count: 3");
  ExpectContains(description,
                 "Energy range: [40.0000000 keV, 120.0000000 keV]");
  EXPECT_EQ(description.find("80.0000000 keV"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDescription, DescribesRegularCenterGridAndBinWidth) {
  constexpr std::array<double, 3U> centers{20.0, 22.0, 24.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};

  ggems::core::sources::GGEMSSource source{};
  source.SetRegularEnergySpectrum(centers, weights, "keV");

  auto snapshot = ggems::core::sources::BuildSourceRunSnapshot(source);
  std::string const description = DescribeSnapshotSource(snapshot, 0U);

  ExpectContains(description, "Energy: Regular spectrum");
  ExpectContains(description, "Bin count: 3");
  ExpectContains(description, "Center range: [20.0000000 keV, 24.0000000 keV]");
  ExpectContains(description, "Bin width: 2.0000000 keV");
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDescription, OwnedSnapshotSurvivesSourceEnergyMutation) {
  constexpr std::array<double, 2U> first_energies{40.0, 80.0};
  constexpr std::array<double, 2U> first_weights{1.0, 1.0};
  constexpr std::array<double, 2U> second_energies{60.0, 120.0};
  constexpr std::array<double, 2U> second_weights{1.0, 3.0};

  ggems::core::sources::GGEMSSource source{};
  source.SetDiscreteEnergyLines(first_energies, first_weights, "keV");
  auto first = ggems::core::sources::BuildSourceRunSnapshot(source);

  source.SetDiscreteEnergyLines(second_energies, second_weights, "keV");
  auto second = ggems::core::sources::BuildSourceRunSnapshot(source);

  ExpectContains(DescribeSnapshotSource(first, 0U),
                 "Energy range: [40.0000000 keV, 80.0000000 keV]");
  ExpectContains(DescribeSnapshotSource(second, 0U),
                 "Energy range: [60.0000000 keV, 120.0000000 keV]");
}
