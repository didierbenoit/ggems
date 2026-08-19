#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace {

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::builtins::BuildTc99mRadionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

auto CheckReachableDiscreteChannel(GGEMSRadionuclideEmission const &emission,
                                   GGEMSParticleType expected_particle_type,
                                   std::size_t expected_line_count) -> void {
  EXPECT_EQ(emission.GetParticleType(), expected_particle_type);

  auto const &distribution = emission.GetEnergyDistribution();
  EXPECT_EQ(distribution.GetType(), GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_EQ(distribution.GetTableCount(), expected_line_count);

  auto const energies = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(energies.size(), expected_line_count);
  ASSERT_EQ(weights.size(), expected_line_count);
  ASSERT_EQ(tickets.size(), expected_line_count);

  long double line_yield_sum{0.0L};
  std::uint64_t previous_energy{0ULL};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < expected_line_count; ++index) {
    EXPECT_GT(energies[index], previous_energy);
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);

    line_yield_sum += static_cast<long double>(weights[index]);
    previous_energy = energies[index];
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(line_yield_sum),
              static_cast<double>(emission.GetYieldPerDecay()), 1.0e-12);
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTc99mTest, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildTc99mRadionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "Tc-99m");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 21'624.12L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);

  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 3.706e-5L);

  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[1U].GetYieldPerDecay()),
              0.885241444, 1.0e-15);

  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 7.4e-11L);

  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()), 0.08209,
              1.0e-15);

  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[4U].GetYieldPerDecay()),
              4.4144146979128, 1.0e-12);

  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[5U].GetYieldPerDecay()),
              1.1131270181, 1.0e-12);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              6.4949102200868, 1.0e-12);

  long double selection_weight_sum{0.0L};
  for (long double const weight : definition.GetChannelSelectionWeights()) {
    EXPECT_GT(weight, 0.0L);
    selection_weight_sum += weight;
  }
  EXPECT_NEAR(static_cast<double>(selection_weight_sum), 1.0, 1.0e-15);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTc99mTest, PreservesCombinedBetaShapeSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildTc99mRadionuclide();
  auto const &emission = definition.GetEmissions()[0U];
  auto const &distribution = emission.GetEnergyDistribution();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(distribution.GetTableCount(), 873U);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 499'770ULL);

  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(centers.size(), 873U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());

  constexpr std::uint64_t half_width{499'770ULL / 2ULL};
  EXPECT_EQ(centers.front() - half_width, 790ULL);
  EXPECT_EQ(centers.back() + half_width, 436'300'000ULL);

  long double weight_sum{0.0L};
  long double weighted_center_sum{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);

    weight_sum += static_cast<long double>(weights[index]);
    weighted_center_sum += static_cast<long double>(weights[index]) *
                           static_cast<long double>(centers[index]);

    EXPECT_GT(tickets[index], previous_ticket);
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);

  long double const mean_energy_keV =
      weighted_center_sum / weight_sum / 1'000'000.0L;
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 113.562304553545, 0.001);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTc99mTest, PreservesLaraGammaAndXRayEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildTc99mRadionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);

  GGEMSRadionuclideEmission const &gamma = emissions[1U];
  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma, 5U);

  constexpr std::array<std::uint64_t, 5U> expected_gamma_energies{{
      89'600'000ULL,
      140'511'000ULL,
      142'683'000ULL,
      232'700'000ULL,
      322'400'000ULL,
  }};

  constexpr std::array<double, 5U> expected_gamma_yields{{
      0.0000104,
      0.885,
      0.00023,
      0.000000084,
      0.00000096,
  }};

  auto const gamma_energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const gamma_yields = gamma.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U; index < expected_gamma_energies.size();
       ++index) {
    EXPECT_EQ(gamma_energies[index], expected_gamma_energies[index]);
    EXPECT_DOUBLE_EQ(gamma_yields[index], expected_gamma_yields[index]);
  }

  GGEMSRadionuclideEmission const &ultra_weak_gamma = emissions[2U];
  EXPECT_EQ(ultra_weak_gamma.GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(ultra_weak_gamma.GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(
      ultra_weak_gamma.GetEnergyDistribution().GetMonoEnergyMilliElectronVolt(),
      2'172'600ULL);
  EXPECT_EQ(ultra_weak_gamma.GetYieldPerDecay(), 7.4e-11L);

  GGEMSRadionuclideEmission const &x_rays = emissions[3U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 5U);

  constexpr std::array<std::uint64_t, 5U> expected_x_ray_energies{{
      2'568'000ULL,
      18'251'000ULL,
      18'367'200ULL,
      20'669'000ULL,
      21'023'500ULL,
  }};

  constexpr std::array<double, 5U> expected_x_ray_yields{{
      0.00482,
      0.0222,
      0.0421,
      0.0112,
      0.00177,
  }};

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U; index < expected_x_ray_energies.size();
       ++index) {
    EXPECT_EQ(x_ray_energies[index], expected_x_ray_energies[index]);
    EXPECT_DOUBLE_EQ(x_ray_yields[index], expected_x_ray_yields[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTc99mTest, PreservesMirdAugerCatalog) {
  GGEMSRadionuclideDefinition const definition = BuildTc99mRadionuclide();
  GGEMSRadionuclideEmission const &auger = definition.GetEmissions()[4U];

  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 22U);

  auto const energies =
      auger.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const yields = auger.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(energies.front(), 29'608ULL);
  EXPECT_EQ(energies[4U], 114'154ULL);
  EXPECT_EQ(energies[10U], 2'053'920ULL);
  EXPECT_EQ(energies.back(), 21'241'700ULL);
  EXPECT_DOUBLE_EQ(yields.front(), 2.4663);
  EXPECT_DOUBLE_EQ(yields[4U], 0.708794);
  EXPECT_DOUBLE_EQ(yields[10U], 0.0903225);
  EXPECT_DOUBLE_EQ(yields.back(), 6.27073e-8);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTc99mTest, PreservesLaraConversionElectronCatalog) {
  GGEMSRadionuclideDefinition const definition = BuildTc99mRadionuclide();
  GGEMSRadionuclideEmission const &conversion = definition.GetEmissions()[5U];

  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron, 18U);

  auto const energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const yields = conversion.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(energies.front(), 1'787'960ULL);
  EXPECT_EQ(energies[1U], 2'142'640ULL);
  EXPECT_EQ(energies[4U], 119'467'000ULL);
  EXPECT_EQ(energies[9U], 139'633'000ULL);
  EXPECT_EQ(energies.back(), 300'280'000ULL);

  EXPECT_DOUBLE_EQ(yields.front(), 0.881);
  EXPECT_DOUBLE_EQ(yields[1U], 0.1169);
  EXPECT_DOUBLE_EQ(yields[4U], 0.092);
  EXPECT_DOUBLE_EQ(yields[9U], 0.00116);
  EXPECT_DOUBLE_EQ(yields.back(), 0.0000000146);
}

} // namespace
