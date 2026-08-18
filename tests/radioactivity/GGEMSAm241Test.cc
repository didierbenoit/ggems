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
using ggems::core::radioactivity::builtins::BuildAm241Radionuclide;
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

TEST(GGEMSAm241Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildAm241Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "Am-241");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 13'652'000'000.0L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);

  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_NEAR(static_cast<double>(emissions[0U].GetYieldPerDecay()), 1.00022736,
              1.0e-12);

  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[1U].GetYieldPerDecay()),
              0.3851753802, 1.0e-12);

  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[2U].GetYieldPerDecay()), 0.37661828,
              1.0e-12);

  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()),
              10.1459821737038, 1.0e-12);

  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[4U].GetYieldPerDecay()),
              0.910731347024, 1.0e-12);

  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[5U].GetYieldPerDecay()),
              4.0000082e-8, 1.0e-18);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              12.818734580927882, 1.0e-11);

  long double selection_weight_sum{0.0L};
  for (long double const weight : definition.GetChannelSelectionWeights()) {
    EXPECT_GT(weight, 0.0L);
    selection_weight_sum += weight;
  }
  EXPECT_NEAR(static_cast<double>(selection_weight_sum), 1.0, 1.0e-15);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAm241Test, PreservesLaraAlphaGammaAndXRayCatalogs) {
  GGEMSRadionuclideDefinition const definition = BuildAm241Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);

  GGEMSRadionuclideEmission const &alpha = emissions[0U];
  CheckReachableDiscreteChannel(alpha, GGEMSParticleType::Alpha, 23U);

  auto const alpha_energies =
      alpha.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const alpha_yields = alpha.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(alpha_energies[16U], 5'388'250'000ULL);
  EXPECT_EQ(alpha_energies[18U], 5'442'860'000ULL);
  EXPECT_EQ(alpha_energies[20U], 5'485'560'000ULL);
  EXPECT_EQ(alpha_energies[22U], 5'544'110'000ULL);
  EXPECT_DOUBLE_EQ(alpha_yields[16U], 0.0166);
  EXPECT_DOUBLE_EQ(alpha_yields[18U], 0.1323);
  EXPECT_DOUBLE_EQ(alpha_yields[20U], 0.8445);
  EXPECT_DOUBLE_EQ(alpha_yields[22U], 0.0038);

  GGEMSRadionuclideEmission const &gamma = emissions[1U];
  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma, 179U);

  auto const gamma_energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const gamma_yields = gamma.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(gamma_energies.front(), 26'344'600ULL);
  EXPECT_EQ(gamma_energies[8U], 59'540'900ULL);
  EXPECT_EQ(gamma_energies[14U], 98'970'000ULL);
  EXPECT_EQ(gamma_energies.back(), 1'014'330'000ULL);
  EXPECT_DOUBLE_EQ(gamma_yields.front(), 0.0231);
  EXPECT_DOUBLE_EQ(gamma_yields[8U], 0.3592);
  EXPECT_DOUBLE_EQ(gamma_yields[14U], 0.000203);
  EXPECT_DOUBLE_EQ(gamma_yields.back(), 1.0e-8);

  GGEMSRadionuclideEmission const &x_rays = emissions[2U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 9U);

  constexpr std::array<std::uint64_t, 9U> expected_x_ray_energies{{
      11'890'000ULL,
      13'852'000ULL,
      15'876'000ULL,
      16'960'000ULL,
      21'160'000ULL,
      97'069'000ULL,
      101'059'000ULL,
      114'149'700ULL,
      117'922'700ULL,
  }};

  constexpr std::array<double, 9U> expected_x_ray_yields{{
      0.00844,
      0.1302,
      0.00384,
      0.1858,
      0.0483,
      0.00001134,
      0.0000181,
      0.00000658,
      0.00000226,
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

TEST(GGEMSAm241Test, PreservesMirdAugerCatalog) {
  GGEMSRadionuclideDefinition const definition = BuildAm241Radionuclide();
  GGEMSRadionuclideEmission const &auger = definition.GetEmissions()[3U];

  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 15U);

  auto const energies =
      auger.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const yields = auger.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(energies.front(), 78'986ULL);
  EXPECT_EQ(energies[2U], 190'572ULL);
  EXPECT_EQ(energies[3U], 209'123ULL);
  EXPECT_EQ(energies.back(), 110'844'000ULL);
  EXPECT_DOUBLE_EQ(yields.front(), 3.26431);
  EXPECT_DOUBLE_EQ(yields[2U], 1.69908);
  EXPECT_DOUBLE_EQ(yields[3U], 2.93263);
  EXPECT_DOUBLE_EQ(yields.back(), 6.69978e-8);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAm241Test, PreservesAllLnhbConversionElectronLines) {
  GGEMSRadionuclideDefinition const definition = BuildAm241Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);

  GGEMSRadionuclideEmission const &main_lines = emissions[4U];
  GGEMSRadionuclideEmission const &weak_lines = emissions[5U];

  CheckReachableDiscreteChannel(main_lines, GGEMSParticleType::Electron, 268U);
  CheckReachableDiscreteChannel(weak_lines, GGEMSParticleType::Electron, 248U);

  EXPECT_NEAR(static_cast<double>(main_lines.GetYieldPerDecay() +
                                  weak_lines.GetYieldPerDecay()),
              0.910731387024082, 1.0e-12);

  auto const main_energies =
      main_lines.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const main_yields =
      main_lines.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(main_energies.front(), 3'917'600ULL);
  EXPECT_EQ(main_energies[5U], 10'769'300ULL);
  EXPECT_EQ(main_energies[27U], 37'940'900ULL);
  EXPECT_EQ(main_energies.back(), 699'533'000ULL);
  EXPECT_DOUBLE_EQ(main_yields.front(), 0.021);
  EXPECT_DOUBLE_EQ(main_yields[5U], 0.1039);
  EXPECT_DOUBLE_EQ(main_yields[27U], 0.171);
  EXPECT_DOUBLE_EQ(main_yields.back(), 1.582e-9);

  auto const weak_energies =
      weak_lines.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const weak_yields =
      weak_lines.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(weak_energies.front(), 78'090'000ULL);
  EXPECT_EQ(weak_energies[1U], 129'851'000ULL);
  EXPECT_EQ(weak_energies.back(), 804'870'000ULL);
  EXPECT_DOUBLE_EQ(weak_yields.front(), 4.0e-10);
  EXPECT_DOUBLE_EQ(weak_yields[1U], 7.04e-10);
  EXPECT_DOUBLE_EQ(weak_yields.back(), 2.0e-12);
}

} // namespace
