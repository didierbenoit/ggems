#include <algorithm>
#include <cmath>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::BuildBetaSpectrum;
using ggems::core::radioactivity::GGEMSBetaSign;
using ggems::core::radioactivity::GGEMSBetaSpectrumBuildResult;
using ggems::core::radioactivity::GGEMSBetaSpectrumModel;
using ggems::core::radioactivity::GGEMSBetaTransition;
using ggems::core::radioactivity::GGEMSBetaTransitionClass;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::builtins::BuildC11Radionuclide;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::radioactivity::builtins::BuildO15Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildExpectedO15Spectrum() -> GGEMSBetaSpectrumBuildResult {
  GGEMSBetaTransition const transition{GGEMSBetaSign::Plus, 7U, 15U,
                                       1'732'180'000ULL,
                                       GGEMSBetaTransitionClass::Allowed};
  return BuildBetaSpectrum(
      transition, {.model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
                   .grid = {.target_maximum_bin_width_milli_eV = 500'000ULL}});
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSO15Test, BuildsExactIdentityAndSinglePositronEmission) {
  GGEMSRadionuclideDefinition const definition = BuildO15Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "O-15");
  ASSERT_EQ(definition.GetAliases().size(), 3U);
  EXPECT_EQ(definition.GetAliases()[0U], "O15");
  EXPECT_EQ(definition.GetAliases()[1U], "15O");
  EXPECT_EQ(definition.GetAliases()[2U], "Oxygen-15");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 122.266L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.999001L);
  EXPECT_EQ(definition.GetTotalYieldPerDecay(), 0.999001L);
  ASSERT_EQ(definition.GetChannelSelectionWeights().size(), 1U);
  EXPECT_EQ(definition.GetChannelSelectionWeights()[0U], 1.0L);

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Gamma);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Electron);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Aionino);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSO15Test, BuildsReachableAllowedPointCoulombSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildO15Radionuclide();
  auto const &distribution =
      definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const expected = BuildExpectedO15Spectrum();
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(expected.diagnostics.model,
            GGEMSBetaSpectrumModel::AllowedPointCoulomb);
  EXPECT_EQ(expected.diagnostics.bin_count, 3'465U);
  EXPECT_EQ(expected.diagnostics.bin_width_milli_eV, 499'906ULL);
  EXPECT_EQ(expected.diagnostics.lower_edge_milli_eV, 5'710ULL);
  EXPECT_EQ(expected.diagnostics.upper_edge_milli_eV, 1'732'180'000ULL);
  EXPECT_GT(expected.diagnostics.minimum_assigned_ticket_count, 0ULL);

  EXPECT_TRUE(std::ranges::equal(
      centers, expected.distribution.GetEnergyValuesMilliElectronVolt()));
  EXPECT_TRUE(
      std::ranges::equal(weights, expected.distribution.GetRelativeWeights()));
  EXPECT_TRUE(std::ranges::equal(
      tickets, expected.distribution.GetCumulativeTicketUpperBounds()));

  long double weight_sum{0.0L};
  for (double const weight : weights) {
    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_GT(weight, 0.0);
    weight_sum += static_cast<long double>(weight);
  }
  EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);

  std::uint64_t previous_ticket{0ULL};
  for (std::uint64_t const upper : tickets) {
    EXPECT_GT(upper, previous_ticket);
    previous_ticket = upper;
  }
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
  ASSERT_FALSE(centers.empty());
  EXPECT_EQ(centers.back() +
                (distribution.GetRegularBinWidthMilliElectronVolt() / 2ULL),
            1'732'180'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSO15Test, LibraryRetainsStableImmutableDefinition) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(BuildO15Radionuclide());
  auto const *definition_address = registered.get();
  auto const *emissions_address = registered->GetEmissions().data();
  auto const *energy_data_address = registered->GetEmissions()[0U]
                                        .GetEnergyDistribution()
                                        .GetEnergyValuesMilliElectronVolt()
                                        .data();

  static_cast<void>(library.Add(BuildF18Radionuclide()));
  static_cast<void>(library.Add(BuildC11Radionuclide()));

  EXPECT_EQ(library.Find("O-15"), registered);
  EXPECT_EQ(library.Find(" o15 "), registered);
  EXPECT_EQ(library.Find("15o"), registered);
  EXPECT_EQ(library.Find("oxygen-15"), registered);

  auto const after_growth = library.Find("O-15");
  ASSERT_EQ(after_growth, registered);
  EXPECT_EQ(after_growth.get(), definition_address);
  EXPECT_EQ(after_growth->GetEmissions().data(), emissions_address);
  EXPECT_EQ(after_growth->GetEmissions()[0U]
                .GetEnergyDistribution()
                .GetEnergyValuesMilliElectronVolt()
                .data(),
            energy_data_address);
}
