#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
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
using ggems::core::radioactivity::builtins::BuildI131Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

struct ExpectedBetaBranch {
  long double yield_per_decay;
  std::size_t table_count;
  std::uint64_t bin_width_micro_eV;
  std::uint64_t lower_edge_micro_eV;
  std::uint64_t endpoint_micro_eV;
  double represented_mean_energy_keV;
};

// =============================================================================
// =============================================================================

constexpr std::array<ExpectedBetaBranch, 6U> k_expected_beta_branches{{
    {.yield_per_decay = 0.02130L,
     .table_count = 496U,
     .bin_width_micro_eV = 499'798'000ULL,
     .lower_edge_micro_eV = 192'000ULL,
     .endpoint_micro_eV = 247'900'000'000ULL,
     .represented_mean_energy_keV = 68.84135},
    {.yield_per_decay = 0.00643L,
     .table_count = 608U,
     .bin_width_micro_eV = 499'834'000ULL,
     .lower_edge_micro_eV = 928'000ULL,
     .endpoint_micro_eV = 303'900'000'000ULL,
     .represented_mean_energy_keV = 86.31060},
    {.yield_per_decay = 0.0720L,
     .table_count = 668U,
     .bin_width_micro_eV = 499'700'000ULL,
     .lower_edge_micro_eV = 400'000ULL,
     .endpoint_micro_eV = 333'800'000'000ULL,
     .represented_mean_energy_keV = 95.90408},
    {.yield_per_decay = 0.894L,
     .table_count = 1213U,
     .bin_width_micro_eV = 499'834'000ULL,
     .lower_edge_micro_eV = 1'358'000ULL,
     .endpoint_micro_eV = 606'300'000'000ULL,
     .represented_mean_energy_keV = 191.90012},
    {.yield_per_decay = 0.0006L,
     .table_count = 1260U,
     .bin_width_micro_eV = 499'760'000ULL,
     .lower_edge_micro_eV = 2'400'000ULL,
     .endpoint_micro_eV = 629'700'000'000ULL,
     .represented_mean_energy_keV = 199.14468},
    {.yield_per_decay = 0.00386L,
     .table_count = 1614U,
     .bin_width_micro_eV = 499'938'000ULL,
     .lower_edge_micro_eV = 68'000ULL,
     .endpoint_micro_eV = 806'900'000'000ULL,
     .represented_mean_energy_keV = 280.90999},
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 18U> k_expected_gamma_energies_micro_eV{{
    80'185'000'000ULL,
    85'900'000'000ULL,
    177'214'000'000ULL,
    232'180'000'000ULL,
    272'498'000'000ULL,
    284'305'000'000ULL,
    295'800'000'000ULL,
    302'400'000'000ULL,
    318'088'000'000ULL,
    324'651'000'000ULL,
    325'789'000'000ULL,
    358'400'000'000ULL,
    364'489'000'000ULL,
    404'814'000'000ULL,
    503'004'000'000ULL,
    636'989'000'000ULL,
    642'719'000'000ULL,
    722'911'000'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 18U> k_expected_gamma_line_yields{{
    0.02607,
    0.000051,
    0.00277,
    0.000023,
    0.000581,
    0.0614,
    0.000012,
    0.000046,
    0.000807,
    0.000244,
    0.00274,
    0.00017,
    0.812,
    0.000552,
    0.003540,
    0.0712,
    0.002183,
    0.01786,
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

  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
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

TEST(GGEMSI131Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildI131Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "I-131");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 693'213.12L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 10U);

  for (std::size_t index = 0U; index < k_expected_beta_branches.size();
       ++index) {
    EXPECT_EQ(emissions[index].GetParticleType(), GGEMSParticleType::Electron);
    EXPECT_EQ(emissions[index].GetYieldPerDecay(),
              k_expected_beta_branches[index].yield_per_decay);
  }

  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[6U].GetYieldPerDecay()), 1.002249,
              1.0e-15);

  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[7U].GetYieldPerDecay()), 0.0597,
              1.0e-15);

  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[8U].GetYieldPerDecay()),
              0.6975269601, 1.0e-12);

  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[9U].GetYieldPerDecay()),
              0.06307688239, 1.0e-12);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              2.82074284249, 1.0e-12);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI131Test, PreservesSixTabulatedBetaShapeSpectra) {
  GGEMSRadionuclideDefinition const definition = BuildI131Radionuclide();
  auto const emissions = definition.GetEmissions();

  ASSERT_EQ(emissions.size(), 10U);

  for (std::size_t branch_index = 0U;
       branch_index < k_expected_beta_branches.size(); ++branch_index) {
    ExpectedBetaBranch const &expected = k_expected_beta_branches[branch_index];
    GGEMSRadionuclideEmission const &emission = emissions[branch_index];
    auto const &distribution = emission.GetEnergyDistribution();
    auto const centers = distribution.GetEnergyValuesMicroElectronVolt();
    auto const weights = distribution.GetRelativeWeights();
    auto const tickets = distribution.GetCumulativeTicketUpperBounds();

    EXPECT_EQ(distribution.GetType(),
              GGEMSEnergyDistributionType::RegularSpectrum);
    EXPECT_EQ(distribution.GetTableCount(), expected.table_count);
    EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(),
              expected.bin_width_micro_eV);

    ASSERT_EQ(centers.size(), expected.table_count);
    ASSERT_EQ(weights.size(), centers.size());
    ASSERT_EQ(tickets.size(), centers.size());

    std::uint64_t const half_width = expected.bin_width_micro_eV / 2ULL;
    EXPECT_EQ(centers.front() - half_width, expected.lower_edge_micro_eV);
    EXPECT_EQ(centers.back() + half_width, expected.endpoint_micro_eV);

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
        weighted_center_sum / weight_sum /
        static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
    EXPECT_NEAR(static_cast<double>(mean_energy_keV),
                expected.represented_mean_energy_keV, 0.01);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSI131Test, PreservesEighteenPromptLaraGammaLines) {
  GGEMSRadionuclideDefinition const definition = BuildI131Radionuclide();
  GGEMSRadionuclideEmission const &gamma = definition.GetEmissions()[6U];

  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma,
                                k_expected_gamma_energies_micro_eV.size());

  auto const energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const yields = gamma.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_gamma_energies_micro_eV.size(); ++index) {
    EXPECT_EQ(energies[index], k_expected_gamma_energies_micro_eV[index]);
    EXPECT_DOUBLE_EQ(yields[index], k_expected_gamma_line_yields[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSI131Test, PreservesLaraAndMirdAtomicRadiations) {
  GGEMSRadionuclideDefinition const definition = BuildI131Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 10U);

  GGEMSRadionuclideEmission const &x_rays = emissions[7U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 5U);

  constexpr std::array<std::uint64_t, 5U> expected_x_ray_energies{{
      4'470'000'000ULL,
      29'459'000'000ULL,
      29'779'000'000ULL,
      33'689'300'000ULL,
      34'487'700'000ULL,
  }};
  constexpr std::array<double, 5U> expected_x_ray_yields{{
      0.00631,
      0.0152,
      0.0281,
      0.00816,
      0.00193,
  }};

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U; index < expected_x_ray_energies.size();
       ++index) {
    EXPECT_EQ(x_ray_energies[index], expected_x_ray_energies[index]);
    EXPECT_DOUBLE_EQ(x_ray_yields[index], expected_x_ray_yields[index]);
  }

  GGEMSRadionuclideEmission const &auger = emissions[8U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 13U);

  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(auger_energies.front(), 26'150'000ULL);
  EXPECT_EQ(auger_energies[1U], 36'237'000ULL);
  EXPECT_EQ(auger_energies[7U], 3'324'020'000ULL);
  EXPECT_EQ(auger_energies.back(), 32'903'200'000ULL);
  EXPECT_DOUBLE_EQ(auger_yields.front(), 0.118727);
  EXPECT_DOUBLE_EQ(auger_yields[1U], 0.36283);
  EXPECT_DOUBLE_EQ(auger_yields[7U], 0.0392981);
  EXPECT_DOUBLE_EQ(auger_yields.back(), 0.000189387);

  GGEMSRadionuclideEmission const &conversion = emissions[9U];
  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron, 104U);

  auto const conversion_energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const conversion_yields =
      conversion.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(conversion_energies.front(), 45'621'000'000ULL);
  EXPECT_EQ(conversion_energies[21U], 249'741'000'000ULL);
  EXPECT_EQ(conversion_energies[63U], 329'925'600'000ULL);
  EXPECT_EQ(conversion_energies[86U], 602'425'600'000ULL);
  EXPECT_EQ(conversion_energies.back(), 722'780'700'000ULL);
  EXPECT_DOUBLE_EQ(conversion_yields.front(), 0.0344);
  EXPECT_DOUBLE_EQ(conversion_yields[21U], 0.002505);
  EXPECT_DOUBLE_EQ(conversion_yields[63U], 0.01543);
  EXPECT_DOUBLE_EQ(conversion_yields[86U], 0.000286);
  EXPECT_DOUBLE_EQ(conversion_yields.back(), 0.00000041);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI131Test, ExcludesDelayedXe131mDeexcitation) {
  GGEMSRadionuclideDefinition const definition = BuildI131Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 10U);

  // The parent beta branch populating Xe-131m remains represented.
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.00386L);
  auto const beta_centers =
      emissions[5U].GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const beta_half_width = emissions[5U]
                                   .GetEnergyDistribution()
                                   .GetRegularBinWidthMicroElectronVolt() /
                               2ULL;
  ASSERT_FALSE(beta_centers.empty());
  EXPECT_EQ(beta_centers.back() + beta_half_width, 806'900'000'000ULL);

  // The 11.962 d Xe-131m -> Xe-131 transition is not emitted at parent time.
  auto const gamma_energies =
      emissions[6U].GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  for (std::uint64_t const energy : gamma_energies) {
    EXPECT_NE(energy, 163'930'000'000ULL);
  }

  auto const conversion_energies =
      emissions[9U].GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  for (std::uint64_t const energy : conversion_energies) {
    EXPECT_NE(energy, 129'366'000'000ULL);
    EXPECT_NE(energy, 158'477'000'000ULL);
    EXPECT_NE(energy, 158'826'000'000ULL);
    EXPECT_NE(energy, 159'148'000'000ULL);
    EXPECT_NE(energy, 163'038'000'000ULL);
    EXPECT_NE(energy, 163'802'000'000ULL);
  }
}

} // namespace
