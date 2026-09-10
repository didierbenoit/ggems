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
using ggems::core::radioactivity::builtins::BuildI125Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

constexpr std::uint64_t k_expected_gamma_energy_micro_eV{35'492'200'000ULL};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 5U> k_expected_x_ray_energies_micro_eV{{
    4'078'800'000ULL,
    27'202'000'000ULL,
    27'472'600'000ULL,
    31'058'900'000ULL,
    31'762'300'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 5U> k_expected_x_ray_line_yields{{
    0.147,
    0.393,
    0.732,
    0.209,
    0.0454,
}};

// =============================================================================
// =============================================================================
constexpr std::array<std::uint64_t, 13U> k_expected_auger_energies_micro_eV{{
    22'927'000ULL,
    24'922'000ULL,
    120'899'000ULL,
    299'843'000ULL,
    449'679'000ULL,
    541'735'000ULL,
    690'006'000ULL,
    3'088'170'000ULL,
    3'682'760'000ULL,
    4'299'940'000ULL,
    22'665'300'000ULL,
    26'505'600'000ULL,
    30'346'100'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 13U> k_expected_auger_line_yields{{
    12.198,
    4.08498,
    1.41044,
    0.275991,
    3.12454,
    0.120544,
    0.000457128,
    1.2264,
    0.347461,
    0.0245935,
    0.130975,
    0.057687,
    0.00607509,
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 6U> k_expected_conversion_energies_micro_eV{
    {
        3'678'400'000ULL,
        30'553'000'000ULL,
        30'880'200'000ULL,
        31'150'800'000ULL,
        34'722'400'000ULL,
        35'398'500'000ULL,
    }};

// =============================================================================
// =============================================================================

constexpr std::array<double, 6U> k_expected_conversion_line_yields{{
    0.776,
    0.0936,
    0.0172,
    0.0159,
    0.0256,
    0.00548,
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

TEST(GGEMSI125Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildI125Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "I-125");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 5'131'123.2L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 4U);

  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[0U].GetYieldPerDecay()), 0.0663,
              1.0e-12);
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[1U].GetYieldPerDecay()), 1.5264,
              1.0e-12);
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[2U].GetYieldPerDecay()),
              23.008143718, 1.0e-12);
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()), 0.93378,
              1.0e-12);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              25.534623718, 1.0e-12);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI125Test, PreservesSingleEvaluatedGammaAndCompactXRayGroups) {
  GGEMSRadionuclideDefinition const definition = BuildI125Radionuclide();
  auto const emissions = definition.GetEmissions();

  GGEMSRadionuclideEmission const &gamma = emissions[0U];
  EXPECT_EQ(gamma.GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(gamma.GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(gamma.GetEnergyDistribution().GetMonoEnergyMicroElectronVolt(),
            k_expected_gamma_energy_micro_eV);

  GGEMSRadionuclideEmission const &x_rays = emissions[1U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma,
                                k_expected_x_ray_energies_micro_eV.size());
  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();
  for (std::size_t index = 0U;
       index < k_expected_x_ray_energies_micro_eV.size(); ++index) {
    EXPECT_EQ(x_ray_energies[index], k_expected_x_ray_energies_micro_eV[index]);
    EXPECT_DOUBLE_EQ(x_ray_yields[index], k_expected_x_ray_line_yields[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSI125Test, PreservesMirdAugerAndLaraConversionElectrons) {
  GGEMSRadionuclideDefinition const definition = BuildI125Radionuclide();
  auto const emissions = definition.GetEmissions();

  GGEMSRadionuclideEmission const &auger = emissions[2U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron,
                                k_expected_auger_energies_micro_eV.size());
  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();
  for (std::size_t index = 0U;
       index < k_expected_auger_energies_micro_eV.size(); ++index) {
    EXPECT_EQ(auger_energies[index], k_expected_auger_energies_micro_eV[index]);
    EXPECT_DOUBLE_EQ(auger_yields[index], k_expected_auger_line_yields[index]);
  }

  GGEMSRadionuclideEmission const &conversion = emissions[3U];
  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron,
                                k_expected_conversion_energies_micro_eV.size());
  auto const conversion_energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const conversion_yields =
      conversion.GetEnergyDistribution().GetRelativeWeights();
  for (std::size_t index = 0U;
       index < k_expected_conversion_energies_micro_eV.size(); ++index) {
    EXPECT_EQ(conversion_energies[index],
              k_expected_conversion_energies_micro_eV[index]);
    EXPECT_DOUBLE_EQ(conversion_yields[index],
                     k_expected_conversion_line_yields[index]);
  }
}

} // namespace
