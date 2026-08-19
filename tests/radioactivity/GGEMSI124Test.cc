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
using ggems::core::radioactivity::builtins::BuildI124Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

constexpr std::uint64_t k_energy_ticket_space_size{1ULL << 32U};

// =============================================================================
// =============================================================================

struct ExpectedPositronBranch {
  long double yield_per_decay;
  std::size_t table_count;
  std::uint64_t lower_edge_milli_eV;
  std::uint64_t bin_width_milli_eV;
  std::uint64_t endpoint_milli_eV;
  double represented_mean_energy_keV;
};

// =============================================================================
// =============================================================================

constexpr std::array<ExpectedPositronBranch, 8U> k_expected_positron_branches{{
    {.yield_per_decay = 0.1032L,
     .table_count = 306U,
     .lower_edge_milli_eV = 280ULL,
     .bin_width_milli_eV = 6'985'620ULL,
     .endpoint_milli_eV = 2'137'600'000ULL,
     .represented_mean_energy_keV = 975.15341844},
    {.yield_per_decay = 0.1145L,
     .table_count = 307U,
     .lower_edge_milli_eV = 82ULL,
     .bin_width_milli_eV = 4'999'674ULL,
     .endpoint_milli_eV = 1'534'900'000ULL,
     .represented_mean_energy_keV = 681.867638504},
    {.yield_per_decay = 1.88e-06L,
     .table_count = 445U,
     .lower_edge_milli_eV = 360ULL,
     .bin_width_milli_eV = 1'997'752ULL,
     .endpoint_milli_eV = 889'000'000ULL,
     .represented_mean_energy_keV = 420.633952159},
    {.yield_per_decay = 0.00287L,
     .table_count = 407U,
     .lower_edge_milli_eV = 690ULL,
     .bin_width_milli_eV = 1'995'330ULL,
     .endpoint_milli_eV = 812'100'000ULL,
     .represented_mean_energy_keV = 365.865892205},
    {.yield_per_decay = 1.21e-06L,
     .table_count = 481U,
     .lower_edge_milli_eV = 336ULL,
     .bin_width_milli_eV = 998'544ULL,
     .endpoint_milli_eV = 480'300'000ULL,
     .represented_mean_energy_keV = 238.786505926},
    {.yield_per_decay = 1.5e-08L,
     .table_count = 319U,
     .lower_edge_milli_eV = 192ULL,
     .bin_width_milli_eV = 798'432ULL,
     .endpoint_milli_eV = 254'700'000ULL,
     .represented_mean_energy_keV = 133.292204559},
    {.yield_per_decay = 1.8e-09L,
     .table_count = 328U,
     .lower_edge_milli_eV = 368ULL,
     .bin_width_milli_eV = 299'694ULL,
     .endpoint_milli_eV = 98'300'000ULL,
     .represented_mean_energy_keV = 51.5744083795},
    {.yield_per_decay = 2.5e-10L,
     .table_count = 461U,
     .lower_edge_milli_eV = 498ULL,
     .bin_width_milli_eV = 99'782ULL,
     .endpoint_milli_eV = 46'000'000ULL,
     .represented_mean_energy_keV = 25.6395521213},
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

  std::uint64_t previous_energy{0ULL};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < expected_line_count; ++index) {
    EXPECT_GT(energies[index], previous_energy);
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    previous_energy = energies[index];
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI124Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildI124Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "I-124");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 360'806.4L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 13U);

  long double positron_yield{0.0L};
  for (std::size_t index = 0U; index < k_expected_positron_branches.size();
       ++index) {
    EXPECT_EQ(emissions[index].GetParticleType(), GGEMSParticleType::Positron);
    EXPECT_EQ(emissions[index].GetYieldPerDecay(),
              k_expected_positron_branches[index].yield_per_decay);
    positron_yield += emissions[index].GetYieldPerDecay();
  }
  EXPECT_NEAR(static_cast<double>(positron_yield), 0.22057310705, 1.0e-15);

  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[8U].GetYieldPerDecay()), 0.984815,
              1.0e-15);
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[9U].GetYieldPerDecay()), 0.6411,
              1.0e-15);
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[10U].GetYieldPerDecay()),
              9.165447885, 1.0e-12);
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[11U].GetYieldPerDecay()),
              0.003555243101, 1.0e-15);
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[12U].GetYieldPerDecay()),
              4.41368e-08, 1.0e-18);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              11.0154912792878, 1.0e-12);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI124Test, PreservesEightBetaShape24PositronBranches) {
  GGEMSRadionuclideDefinition const definition = BuildI124Radionuclide();
  auto const emissions = definition.GetEmissions();

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
      EXPECT_GE(weight, 0.0);
      if (weight > 0.0) {
        EXPECT_GT(tickets[index], previous_ticket);
      } else {
        EXPECT_EQ(tickets[index], previous_ticket);
      }
      weight_sum += static_cast<long double>(weight);
      weighted_center_sum += static_cast<long double>(weight) *
                             static_cast<long double>(centers[index]);
      previous_ticket = tickets[index];
    }
    EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);
    EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);

    long double const mean_energy_keV =
        weighted_center_sum / weight_sum / 1'000'000.0L;
    EXPECT_NEAR(static_cast<double>(mean_energy_keV),
                expected.represented_mean_energy_keV, 0.001);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSI124Test, PreservesDdepPhotonsWithoutSourceAnnihilationLine) {
  GGEMSRadionuclideDefinition const definition = BuildI124Radionuclide();
  auto const emissions = definition.GetEmissions();

  GGEMSRadionuclideEmission const &gamma = emissions[8U];
  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma, 85U);
  auto const gamma_energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const gamma_yields = gamma.GetEnergyDistribution().GetRelativeWeights();
  EXPECT_EQ(gamma_energies.front(), 166'222'000ULL);
  EXPECT_EQ(gamma_energies[15U], 602'725'500ULL);
  EXPECT_EQ(gamma_energies[64U], 1'690'971'600ULL);
  EXPECT_EQ(gamma_energies.back(), 2'988'200'000ULL);
  EXPECT_DOUBLE_EQ(gamma_yields[15U], 0.623);
  EXPECT_DOUBLE_EQ(gamma_yields[64U], 0.1089);
  for (std::uint64_t const energy : gamma_energies) {
    EXPECT_NE(energy, 511'000'000ULL);
    EXPECT_NE(energy, 1'658'000'000ULL);
  }

  GGEMSRadionuclideEmission const &x_rays = emissions[9U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma, 5U);
  auto const x_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const x_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();
  EXPECT_EQ(x_energies.front(), 4'078'800ULL);
  EXPECT_EQ(x_energies.back(), 31'762'300ULL);
  EXPECT_DOUBLE_EQ(x_yields[0U], 0.0595);
  EXPECT_DOUBLE_EQ(x_yields[2U], 0.3085);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI124Test, PreservesMirdAugerAndAllLaraConversionElectrons) {
  GGEMSRadionuclideDefinition const definition = BuildI124Radionuclide();
  auto const emissions = definition.GetEmissions();

  GGEMSRadionuclideEmission const &auger = emissions[10U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron, 13U);
  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();
  EXPECT_EQ(auger_energies.front(), 22'924ULL);
  EXPECT_EQ(auger_energies.back(), 30'346'100ULL);
  EXPECT_DOUBLE_EQ(auger_yields.front(), 4.85977);
  EXPECT_DOUBLE_EQ(auger_yields.back(), 0.00250035);

  GGEMSRadionuclideEmission const &main = emissions[11U];
  CheckReachableDiscreteChannel(main, GGEMSParticleType::Electron, 401U);
  auto const main_energies =
      main.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const main_yields = main.GetEnergyDistribution().GetRelativeWeights();
  EXPECT_EQ(main_energies.front(), 134'408'000ULL);
  EXPECT_EQ(main_energies.back(), 2'983'300'000ULL);
  EXPECT_DOUBLE_EQ(main_yields.front(), 3.1e-06);
  EXPECT_DOUBLE_EQ(main_yields.back(), 1.57e-09);

  GGEMSRadionuclideEmission const &weak = emissions[12U];
  CheckReachableDiscreteChannel(weak, GGEMSParticleType::Electron, 109U);
  auto const weak_energies =
      weak.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const weak_yields = weak.GetEnergyDistribution().GetRelativeWeights();
  EXPECT_EQ(weak_energies.front(), 366'117'000ULL);
  EXPECT_EQ(weak_energies.back(), 2'988'150'000ULL);
  EXPECT_DOUBLE_EQ(weak_yields.front(), 6.5e-10);
  EXPECT_DOUBLE_EQ(weak_yields.back(), 7e-11);

  EXPECT_EQ(main_energies.size() + weak_energies.size(), 510U);
}

} // namespace
