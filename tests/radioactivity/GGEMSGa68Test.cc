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
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::builtins::BuildGa68Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

struct ExpectedPositronBranch {
  long double yield_per_decay;
  std::size_t table_count;
  std::uint64_t bin_width_milli_eV;
  std::uint64_t lower_edge_milli_eV;
  std::uint64_t endpoint_milli_eV;
  double represented_mean_energy_keV;
};

// =============================================================================
// =============================================================================

constexpr std::array<ExpectedPositronBranch, 3U> k_expected_positron_branches{{
    {.yield_per_decay = 0.8768L,
     .table_count = 3799U,
     .bin_width_milli_eV = 499'894ULL,
     .lower_edge_milli_eV = 2'694ULL,
     .endpoint_milli_eV = 1'899'100'000ULL,
     .represented_mean_energy_keV = 834.88330},
    {.yield_per_decay = 0.0120L,
     .table_count = 1644U,
     .bin_width_milli_eV = 499'846ULL,
     .lower_edge_milli_eV = 3'176ULL,
     .endpoint_milli_eV = 821'750'000ULL,
     .represented_mean_energy_keV = 351.95880},
    {.yield_per_decay = 0.0000026L,
     .table_count = 487U,
     .bin_width_milli_eV = 499'444ULL,
     .lower_edge_milli_eV = 772ULL,
     .endpoint_milli_eV = 243'230'000ULL,
     .represented_mean_energy_keV = 107.38222},
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 13U> k_expected_gamma_energies_milli_eV{{
    227'310'000ULL,
    483'350'000ULL,
    578'520'000ULL,
    682'570'000ULL,
    805'830'000ULL,
    938'610'000ULL,
    1'077'340'000ULL,
    1'165'920'000ULL,
    1'261'080'000ULL,
    1'744'420'000ULL,
    1'883'160'000ULL,
    2'338'440'000ULL,
    2'821'730'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 13U> k_expected_gamma_line_yields{{
    1.2e-06,
    2.65e-06,
    0.000343,
    3.14e-06,
    0.000928,
    1.78e-06,
    0.03235,
    1.6e-07,
    0.000954,
    0.000096,
    0.00142,
    0.0000113,
    0.00000466,
}};

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

TEST(GGEMSGa68Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildGa68Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "Ga-68");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 4'069.8L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 7U);

  for (std::size_t index = 0U; index < k_expected_positron_branches.size();
       ++index) {
    EXPECT_EQ(emissions[index].GetParticleType(), GGEMSParticleType::Positron);
    EXPECT_EQ(emissions[index].GetYieldPerDecay(),
              k_expected_positron_branches[index].yield_per_decay);
  }

  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()), 0.03611589,
              1.0e-15);

  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[4U].GetYieldPerDecay()), 0.04919,
              1.0e-15);

  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[5U].GetYieldPerDecay()),
              0.41082910426, 1.0e-12);

  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[6U].GetYieldPerDecay()),
              9.159860474e-06, 1.0e-15);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              1.384946754120474, 1.0e-12);

  long double positron_yield{0.0L};
  for (std::size_t index = 0U; index < 3U; ++index) {
    positron_yield += emissions[index].GetYieldPerDecay();
  }
  EXPECT_NEAR(static_cast<double>(positron_yield), 0.8888026, 1.0e-15);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa68Test, PreservesThreeTabulatedBetaShapePositronSpectra) {
  GGEMSRadionuclideDefinition const definition = BuildGa68Radionuclide();
  auto const emissions = definition.GetEmissions();

  ASSERT_EQ(emissions.size(), 7U);

  for (std::size_t branch_index = 0U;
       branch_index < k_expected_positron_branches.size(); ++branch_index) {
    ExpectedPositronBranch const &expected =
        k_expected_positron_branches[branch_index];
    GGEMSRadionuclideEmission const &emission = emissions[branch_index];
    auto const &distribution = emission.GetEnergyDistribution();
    auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
    auto const weights = distribution.GetRelativeWeights();
    auto const tickets = distribution.GetCumulativeTicketUpperBounds();

    EXPECT_EQ(distribution.GetType(),
              GGEMSEnergyDistributionType::RegularSpectrum);
    EXPECT_EQ(distribution.GetTableCount(), expected.table_count);
    EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(),
              expected.bin_width_milli_eV);

    ASSERT_EQ(centers.size(), expected.table_count);
    ASSERT_EQ(weights.size(), centers.size());
    ASSERT_EQ(tickets.size(), centers.size());

    std::uint64_t const half_width = expected.bin_width_milli_eV / 2ULL;
    EXPECT_EQ(centers.front() - half_width, expected.lower_edge_milli_eV);
    EXPECT_EQ(centers.back() + half_width, expected.endpoint_milli_eV);

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
    EXPECT_NEAR(static_cast<double>(mean_energy_keV),
                expected.represented_mean_energy_keV, 0.01);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa68Test, PreservesThirteenLaraNuclearGammasWithoutAnnihilationLine) {
  GGEMSRadionuclideDefinition const definition = BuildGa68Radionuclide();
  GGEMSRadionuclideEmission const &gamma = definition.GetEmissions()[3U];

  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma,
                                k_expected_gamma_energies_milli_eV.size());

  auto const energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const yields = gamma.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_gamma_energies_milli_eV.size(); ++index) {
    EXPECT_EQ(energies[index], k_expected_gamma_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(yields[index], k_expected_gamma_line_yields[index]);
    EXPECT_NE(energies[index], 511'000'000ULL);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa68Test, PreservesLaraAndMirdAtomicRadiations) {
  GGEMSRadionuclideDefinition const definition = BuildGa68Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 7U);

  GGEMSRadionuclideEmission const &x_rays = emissions[4U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 4U);

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(x_ray_energies[0U], 1'035'000ULL);
  EXPECT_EQ(x_ray_energies[1U], 8'615'870ULL);
  EXPECT_EQ(x_ray_energies[2U], 8'638'960ULL);
  EXPECT_EQ(x_ray_energies[3U], 9'611'000ULL);
  EXPECT_DOUBLE_EQ(x_ray_yields[0U], 0.00146);
  EXPECT_DOUBLE_EQ(x_ray_yields[1U], 0.0142);
  EXPECT_DOUBLE_EQ(x_ray_yields[2U], 0.0276);
  EXPECT_DOUBLE_EQ(x_ray_yields[3U], 0.00593);

  GGEMSRadionuclideEmission const &auger = emissions[5U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 9U);

  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(auger_energies.front(), 56'775ULL);
  EXPECT_EQ(auger_energies[3U], 929'539ULL);
  EXPECT_EQ(auger_energies[6U], 7'486'970ULL);
  EXPECT_EQ(auger_energies.back(), 9'428'040ULL);
  EXPECT_DOUBLE_EQ(auger_yields.front(), 0.188911);
  EXPECT_DOUBLE_EQ(auger_yields[3U], 0.141792);
  EXPECT_DOUBLE_EQ(auger_yields[6U], 0.0411537);
  EXPECT_DOUBLE_EQ(auger_yields.back(), 0.000739583);

  GGEMSRadionuclideEmission const &conversion = emissions[6U];
  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron, 78U);

  auto const conversion_energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const conversion_yields =
      conversion.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(conversion_energies.front(), 217'650'000ULL);
  EXPECT_EQ(conversion_energies[12U], 568'860'000ULL);
  EXPECT_EQ(conversion_energies[24U], 796'180'000ULL);
  EXPECT_EQ(conversion_energies[36U], 1'067'690'000ULL);
  EXPECT_EQ(conversion_energies[54U], 1'734'780'000ULL);
  EXPECT_EQ(conversion_energies[72U], 2'812'130'000ULL);
  EXPECT_EQ(conversion_energies.back(), 2'821'790'000ULL);
  EXPECT_DOUBLE_EQ(conversion_yields.front(), 3.2e-08);
  EXPECT_DOUBLE_EQ(conversion_yields[12U], 3.91e-07);
  EXPECT_DOUBLE_EQ(conversion_yields[24U], 3.91e-07);
  EXPECT_DOUBLE_EQ(conversion_yields[36U], 7.15e-06);
  EXPECT_DOUBLE_EQ(conversion_yields[54U], 7.39e-09);
  EXPECT_DOUBLE_EQ(conversion_yields[72U], 1.6e-10);
  EXPECT_DOUBLE_EQ(conversion_yields.back(), 9.2e-14);
}

} // namespace
