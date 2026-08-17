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
using ggems::core::radioactivity::builtins::BuildLu177Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

struct ExpectedBetaBranch {
  long double yield_per_decay;
  std::size_t table_count;
  std::uint64_t bin_width_milli_eV;
  std::uint64_t lower_edge_milli_eV;
  std::uint64_t endpoint_milli_eV;
  double represented_mean_energy_keV;
};

constexpr std::array<ExpectedBetaBranch, 4U> k_expected_beta_branches{{
    {.yield_per_decay = 0.1155L,
     .table_count = 351U,
     .bin_width_milli_eV = 499'998ULL,
     .lower_edge_milli_eV = 702ULL,
     .endpoint_milli_eV = 175'500'000ULL,
     .represented_mean_energy_keV = 47.40062},
    {.yield_per_decay = 0.00003L,
     .table_count = 495U,
     .bin_width_milli_eV = 499'190ULL,
     .lower_edge_milli_eV = 950ULL,
     .endpoint_milli_eV = 247'100'000ULL,
     .represented_mean_energy_keV = 78.38925},
    {.yield_per_decay = 0.0899L,
     .table_count = 768U,
     .bin_width_milli_eV = 499'738ULL,
     .lower_edge_milli_eV = 1'216ULL,
     .endpoint_milli_eV = 383'800'000ULL,
     .represented_mean_energy_keV = 111.42761},
    {.yield_per_decay = 0.7945L,
     .table_count = 994U,
     .bin_width_milli_eV = 499'798ULL,
     .lower_edge_milli_eV = 788ULL,
     .endpoint_milli_eV = 496'800'000ULL,
     .represented_mean_energy_keV = 149.11609},
}};

constexpr std::array<std::uint64_t, 6U> k_expected_gamma_energies_milli_eV{{
    71'642'500ULL,
    112'950'050ULL,
    136'724'500ULL,
    208'366'100ULL,
    249'674'200ULL,
    321'315'900ULL,
}};

constexpr std::array<double, 6U> k_expected_gamma_line_yields{{
    0.001716,
    0.06223,
    0.0004731,
    0.10425,
    0.002007,
    0.002096,
}};

// =============================================================================
// =============================================================================

TEST(GGEMSLu177Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildLu177Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "Lu-177");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 574'067.52L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 8U);

  for (std::size_t index = 0U; index < k_expected_beta_branches.size();
       ++index) {
    EXPECT_EQ(emissions[index].GetParticleType(), GGEMSParticleType::Electron);
    EXPECT_EQ(emissions[index].GetYieldPerDecay(),
              k_expected_beta_branches[index].yield_per_decay);
  }

  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 0.1727721L);

  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[5U].GetYieldPerDecay()),
              1.37405878911315, 1.0e-12);

  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[6U].GetYieldPerDecay()),
              1.116556849, 1.0e-12);

  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[7U].GetYieldPerDecay()),
              0.154762329358, 1.0e-12);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              3.81808006747115, 1.0e-12);

  long double selection_weight_sum{0.0L};
  for (long double const weight : definition.GetChannelSelectionWeights()) {
    EXPECT_GT(weight, 0.0L);
    selection_weight_sum += weight;
  }
  EXPECT_NEAR(static_cast<double>(selection_weight_sum), 1.0, 1.0e-15);
}

// =============================================================================
// =============================================================================

TEST(GGEMSLu177Test, PreservesFourTabulatedBetaShapeSpectra) {
  GGEMSRadionuclideDefinition const definition = BuildLu177Radionuclide();
  auto const emissions = definition.GetEmissions();

  ASSERT_EQ(emissions.size(), 8U);

  for (std::size_t branch_index = 0U;
       branch_index < k_expected_beta_branches.size(); ++branch_index) {
    ExpectedBetaBranch const &expected = k_expected_beta_branches[branch_index];
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

TEST(GGEMSLu177Test, PreservesSixExactGammaLinesAndGlobalYields) {
  GGEMSRadionuclideDefinition const definition = BuildLu177Radionuclide();
  auto const &emission = definition.GetEmissions()[4U];
  auto const &distribution = emission.GetEnergyDistribution();

  EXPECT_EQ(distribution.GetType(), GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_EQ(distribution.GetTableCount(),
            k_expected_gamma_energies_milli_eV.size());

  auto const energies = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(energies.size(), k_expected_gamma_energies_milli_eV.size());
  ASSERT_EQ(weights.size(), k_expected_gamma_line_yields.size());
  ASSERT_EQ(tickets.size(), k_expected_gamma_line_yields.size());

  long double line_yield_sum{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_EQ(energies[index], k_expected_gamma_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(weights[index], k_expected_gamma_line_yields[index]);
    line_yield_sum += static_cast<long double>(weights[index]);

    EXPECT_GT(tickets[index], previous_ticket);
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(line_yield_sum), 0.1727721, 1.0e-15);
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
}

// =============================================================================
// =============================================================================

TEST(GGEMSLu177Test, PreservesMirdAtomicRadiationsAsReachableDiscreteLines) {
  GGEMSRadionuclideDefinition const definition = BuildLu177Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 8U);

  auto const check_channel =
      [](GGEMSRadionuclideEmission const &emission,
         GGEMSParticleType expected_particle_type,
         std::size_t expected_line_count) -> void {
    EXPECT_EQ(emission.GetParticleType(), expected_particle_type);

    auto const &distribution = emission.GetEnergyDistribution();
    EXPECT_EQ(distribution.GetType(),
              GGEMSEnergyDistributionType::DiscreteLines);
    EXPECT_EQ(distribution.GetTableCount(), expected_line_count);

    auto const energies = distribution.GetEnergyValuesMilliElectronVolt();
    auto const weights = distribution.GetRelativeWeights();
    auto const tickets = distribution.GetCumulativeTicketUpperBounds();

    ASSERT_EQ(energies.size(), expected_line_count);
    ASSERT_EQ(weights.size(), expected_line_count);
    ASSERT_EQ(tickets.size(), expected_line_count);

    long double line_yield_sum{0.0L};
    std::uint64_t previous_ticket{0ULL};

    for (std::size_t index = 0U; index < expected_line_count; ++index) {
      EXPECT_GT(energies[index], 0ULL);
      EXPECT_TRUE(std::isfinite(weights[index]));
      EXPECT_GT(weights[index], 0.0);
      line_yield_sum += static_cast<long double>(weights[index]);

      EXPECT_GT(tickets[index], previous_ticket);
      previous_ticket = tickets[index];
    }

    EXPECT_NEAR(static_cast<double>(line_yield_sum),
                static_cast<double>(emission.GetYieldPerDecay()), 1.0e-12);
    EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
  };

  GGEMSRadionuclideEmission const &x_rays = emissions[5U];
  check_channel(x_rays, GGEMSParticleType::Gamma, 60U);

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const x_ray_yields =
      x_rays.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(x_ray_energies.front(), 19'430ULL);
  EXPECT_EQ(x_ray_energies[46U], 54'719'100ULL);
  EXPECT_EQ(x_ray_energies[47U], 55'923'700ULL);
  EXPECT_EQ(x_ray_energies[48U], 63'123'800ULL);
  EXPECT_EQ(x_ray_energies.back(), 65'476'600ULL);
  EXPECT_DOUBLE_EQ(x_ray_yields.front(), 1.28087);
  EXPECT_DOUBLE_EQ(x_ray_yields[46U], 0.0163172);
  EXPECT_DOUBLE_EQ(x_ray_yields[47U], 0.0285488);
  EXPECT_DOUBLE_EQ(x_ray_yields[48U], 0.00303923);
  EXPECT_DOUBLE_EQ(x_ray_yields.back(), 1.92701e-07);

  GGEMSRadionuclideEmission const &auger_electrons = emissions[6U];
  check_channel(auger_electrons, GGEMSParticleType::Electron, 15U);

  auto const auger_energies =
      auger_electrons.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const auger_yields =
      auger_electrons.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(auger_energies.front(), 10'732ULL);
  EXPECT_EQ(auger_energies[9U], 6'202'910ULL);
  EXPECT_EQ(auger_energies.back(), 61'687'800ULL);
  EXPECT_DOUBLE_EQ(auger_yields.front(), 0.111216);
  EXPECT_DOUBLE_EQ(auger_yields[9U], 0.0633905);
  EXPECT_DOUBLE_EQ(auger_yields.back(), 0.000116604);

  GGEMSRadionuclideEmission const &conversion_electrons = emissions[7U];
  check_channel(conversion_electrons, GGEMSParticleType::Electron, 36U);

  auto const conversion_energies =
      conversion_electrons.GetEnergyDistribution()
          .GetEnergyValuesMilliElectronVolt();
  auto const conversion_yields =
      conversion_electrons.GetEnergyDistribution().GetRelativeWeights();

  EXPECT_EQ(conversion_energies.front(), 6'164'010ULL);
  EXPECT_EQ(conversion_energies[1U], 47'467'800ULL);
  EXPECT_EQ(conversion_energies[9U], 102'187'000ULL);
  EXPECT_EQ(conversion_energies[10U], 103'392'000ULL);
  EXPECT_EQ(conversion_energies[11U], 110'851'000ULL);
  EXPECT_EQ(conversion_energies.back(), 321'316'000ULL);
  EXPECT_DOUBLE_EQ(conversion_yields.front(), 0.00109753);
  EXPECT_DOUBLE_EQ(conversion_yields[1U], 0.0517072);
  EXPECT_DOUBLE_EQ(conversion_yields[9U], 0.0347404);
  EXPECT_DOUBLE_EQ(conversion_yields[10U], 0.0306777);
  EXPECT_DOUBLE_EQ(conversion_yields[11U], 0.0177125);
  EXPECT_DOUBLE_EQ(conversion_yields.back(), 7.27062e-07);
}

} // namespace
