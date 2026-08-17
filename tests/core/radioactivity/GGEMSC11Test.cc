#include <cmath>
#include <cstdint>
#include <cstddef>

#include <gtest/gtest.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
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

TEST(GGEMSC11Test, BuildsExactIdentityAndSinglePositronEmission) {
  GGEMSRadionuclideDefinition const definition = BuildC11Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "C-11");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 1'221.66L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.99750L);
  EXPECT_EQ(definition.GetTotalYieldPerDecay(), 0.99750L);
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

TEST(GGEMSC11Test, PreservesTabulatedBetaShapeSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildC11Radionuclide();
  auto const &distribution =
      definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);

  EXPECT_EQ(distribution.GetTableCount(), 1'921U);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 499'998ULL);

  ASSERT_EQ(centers.size(), 1'921U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 249'999ULL, 3'842ULL);
  EXPECT_EQ(centers.back() + 249'999ULL, 960'500'000ULL);

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
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 385.33, 0.01);
}

// =============================================================================
// =============================================================================

TEST(GGEMSC11Test, LibraryRetainsStableImmutableDefinition) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(BuildC11Radionuclide());
  auto const *definition_address = registered.get();
  auto const *emissions_address = registered->GetEmissions().data();
  auto const *energy_data_address = registered->GetEmissions()[0U]
                                        .GetEnergyDistribution()
                                        .GetEnergyValuesMilliElectronVolt()
                                        .data();

  static_cast<void>(library.Add(BuildF18Radionuclide()));
  static_cast<void>(library.Add(BuildO15Radionuclide()));

  EXPECT_EQ(library.Find("C-11"), registered);
  EXPECT_EQ(library.Find(" c11 "), nullptr);
  EXPECT_EQ(library.Find("11c"), nullptr);
  EXPECT_EQ(library.Find("carbon-11"), nullptr);

  auto const after_growth = library.Find("C-11");
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
