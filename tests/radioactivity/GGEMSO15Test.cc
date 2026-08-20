#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::builtins::BuildC11Radionuclide;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::radioactivity::builtins::BuildO15Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

TEST(GGEMSO15Test, BuildsExactIdentityAndSinglePositronEmission) {
  GGEMSRadionuclideDefinition const definition = BuildO15Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "O-15");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 122.266L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.999001L);
  EXPECT_EQ(definition.GetTotalYieldPerDecay(), 0.999001L);

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Gamma);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Electron);
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Aionino);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSO15Test, PreservesTabulatedBetaShapeSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildO15Radionuclide();
  auto const &distribution =
      definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(distribution.GetTableCount(), 3'465U);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 499'906ULL);

  ASSERT_EQ(centers.size(), 3'465U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 249'953ULL, 5'710ULL);
  EXPECT_EQ(centers.back() + 249'953ULL, 1'732'180'000ULL);

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
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 733.47, 0.25);
}

} // namespace
