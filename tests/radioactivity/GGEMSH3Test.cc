#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::builtins::BuildC11Radionuclide;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::radioactivity::builtins::BuildH3Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

TEST(GGEMSH3Test, BuildsExactIdentityAndSingleBetaMinusEmission) {
  GGEMSRadionuclideDefinition const definition = BuildH3Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "H-3");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 388'500'000.0L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
  EXPECT_EQ(definition.GetTotalYieldPerDecay(), 1.0L);
  ASSERT_EQ(definition.GetChannelSelectionWeights().size(), 1U);
  EXPECT_EQ(definition.GetChannelSelectionWeights()[0U], 1.0L);

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Gamma);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Positron);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Aionino);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSH3Test, PreservesExperimentalBetaShapeSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildH3Radionuclide();
  auto const &distribution =
      definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(distribution.GetTableCount(), 38U);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 489236ULL);

  ASSERT_EQ(centers.size(), 38U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 244618ULL, 32ULL);
  EXPECT_EQ(centers.back() + 244618ULL, 18'591'000ULL);

  long double weight_sum{0.0L};
  long double weighted_center_sum{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    double const weight = weights[index];
    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_GT(weight, 0.0);
    weight_sum += static_cast<long double>(weight);
    weighted_center_sum += static_cast<long double>(weight) *
                           static_cast<long double>(centers[index]);

    EXPECT_GT(tickets[index], previous_ticket);
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);

  long double const mean_energy_keV =
      weighted_center_sum / weight_sum / 1'000'000.0L;
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 5.69565, 0.005);
}

// =============================================================================
// =============================================================================

TEST(GGEMSH3Test, LibraryRetainsStableImmutableDefinition) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(BuildH3Radionuclide());
  auto const *definition_address = registered.get();
  auto const *emissions_address = registered->GetEmissions().data();
  auto const *energy_data_address = registered->GetEmissions()[0U]
                                        .GetEnergyDistribution()
                                        .GetEnergyValuesMilliElectronVolt()
                                        .data();

  static_cast<void>(library.Add(BuildF18Radionuclide()));
  static_cast<void>(library.Add(BuildC11Radionuclide()));

  EXPECT_EQ(library.Find("H-3"), registered);
  EXPECT_EQ(library.Find(" h3 "), nullptr);
  EXPECT_EQ(library.Find("3h"), nullptr);
  EXPECT_EQ(library.Find("tritium"), nullptr);

  auto const after_growth = library.Find("H-3");
  ASSERT_EQ(after_growth, registered);
  EXPECT_EQ(after_growth.get(), definition_address);
  EXPECT_EQ(after_growth->GetEmissions().data(), emissions_address);
  EXPECT_EQ(after_growth->GetEmissions()[0U]
                .GetEnergyDistribution()
                .GetEnergyValuesMilliElectronVolt()
                .data(),
            energy_data_address);
}

} // namespace
