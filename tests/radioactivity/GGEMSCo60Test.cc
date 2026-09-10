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
using ggems::core::radioactivity::builtins::BuildCo60Radionuclide;
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

constexpr std::array<ExpectedBetaBranch, 3U> k_expected_beta_branches{{
    {.yield_per_decay = 0.9988L,
     .table_count = 635U,
     .bin_width_micro_eV = 499'716'000ULL,
     .lower_edge_micro_eV = 340'000ULL,
     .endpoint_micro_eV = 317'320'000'000ULL,
     .represented_mean_energy_keV = 95.52460},
    {.yield_per_decay = 0.00002L,
     .table_count = 1329U,
     .bin_width_micro_eV = 499'968'000ULL,
     .lower_edge_micro_eV = 2'528'000ULL,
     .endpoint_micro_eV = 664'460'000'000ULL,
     .represented_mean_energy_keV = 273.60067},
    {.yield_per_decay = 0.0012L,
     .table_count = 2982U,
     .bin_width_micro_eV = 499'852'000ULL,
     .lower_edge_micro_eV = 1'336'000ULL,
     .endpoint_micro_eV = 1'490'560'000'000ULL,
     .represented_mean_energy_keV = 624.50478},
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 6U> k_expected_gamma_energies_micro_eV{{
    347'140'000'000ULL,
    826'100'000'000ULL,
    1'173'228'000'000ULL,
    1'332'492'000'000ULL,
    2'158'570'000'000ULL,
    2'505'692'000'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 6U> k_expected_gamma_line_yields{{
    0.000075,
    0.000076,
    0.9985,
    0.999826,
    0.000012,
    0.00000002,
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

TEST(GGEMSCo60Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildCo60Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "Co-60");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 166'340'000.0L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 7U);

  for (std::size_t index = 0U; index < k_expected_beta_branches.size();
       ++index) {
    EXPECT_EQ(emissions[index].GetParticleType(), GGEMSParticleType::Electron);
    EXPECT_EQ(emissions[index].GetYieldPerDecay(),
              k_expected_beta_branches[index].yield_per_decay);
  }

  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()), 1.99848902,
              1.0e-15);

  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[4U].GetYieldPerDecay()), 0.000114,
              1.0e-15);

  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[5U].GetYieldPerDecay()),
              0.00121289017, 1.0e-12);

  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[6U].GetYieldPerDecay()),
              0.000292517293712, 1.0e-15);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              3.000128427463712, 1.0e-12);

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Positron);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo60Test, PreservesThreeTabulatedBetaShapeSpectra) {
  GGEMSRadionuclideDefinition const definition = BuildCo60Radionuclide();
  auto const emissions = definition.GetEmissions();

  ASSERT_EQ(emissions.size(), 7U);

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

TEST(GGEMSCo60Test, PreservesSixLaraNuclearGammas) {
  GGEMSRadionuclideDefinition const definition = BuildCo60Radionuclide();
  GGEMSRadionuclideEmission const &gamma = definition.GetEmissions()[3U];

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

TEST(GGEMSCo60Test, PreservesLaraAndMirdAtomicRadiations) {
  GGEMSRadionuclideDefinition const definition = BuildCo60Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 7U);

  GGEMSRadionuclideEmission const &x_rays = emissions[4U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 4U);

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(x_ray_energies[0U], 840'000'000ULL);
  EXPECT_EQ(x_ray_energies[1U], 7'460'970'000ULL);
  EXPECT_EQ(x_ray_energies[2U], 7'478'240'000ULL);
  EXPECT_EQ(x_ray_energies[3U], 8'296'700'000ULL);
  EXPECT_DOUBLE_EQ(x_ray_yields[0U], 0.000002);
  EXPECT_DOUBLE_EQ(x_ray_yields[1U], 0.0000334);
  EXPECT_DOUBLE_EQ(x_ray_yields[2U], 0.000065);
  EXPECT_DOUBLE_EQ(x_ray_yields[3U], 0.0000136);

  GGEMSRadionuclideEmission const &auger = emissions[5U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 7U);

  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(auger_energies.front(), 47'980'000ULL);
  EXPECT_EQ(auger_energies[2U], 772'593'000ULL);
  EXPECT_EQ(auger_energies[4U], 6'497'880'000ULL);
  EXPECT_EQ(auger_energies.back(), 8'133'490'000ULL);
  EXPECT_DOUBLE_EQ(auger_yields.front(), 0.000583929);
  EXPECT_DOUBLE_EQ(auger_yields[2U], 0.000393729);
  EXPECT_DOUBLE_EQ(auger_yields[4U], 0.000124729);
  EXPECT_DOUBLE_EQ(auger_yields.back(), 0.00000205737);

  GGEMSRadionuclideEmission const &conversion = emissions[6U];
  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron, 12U);

  auto const conversion_energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const conversion_yields =
      conversion.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(conversion_energies.front(), 338'810'000'000ULL);
  EXPECT_EQ(conversion_energies[4U], 1'164'907'200'000ULL);
  EXPECT_EQ(conversion_energies[6U], 1'324'175'200'000ULL);
  EXPECT_EQ(conversion_energies[8U], 2'150'277'000'000ULL);
  EXPECT_EQ(conversion_energies.back(), 2'504'836'000'000ULL);
  EXPECT_DOUBLE_EQ(conversion_yields.front(), 3.74e-07);
  EXPECT_DOUBLE_EQ(conversion_yields[4U], 0.000151);
  EXPECT_DOUBLE_EQ(conversion_yields[6U], 0.000115);
  EXPECT_DOUBLE_EQ(conversion_yields[8U], 5.3e-10);
  EXPECT_DOUBLE_EQ(conversion_yields.back(), 1.52e-13);
}

} // namespace
