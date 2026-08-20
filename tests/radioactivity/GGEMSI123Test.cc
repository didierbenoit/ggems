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

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::builtins::BuildI123Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 40U> k_expected_gamma_energies_milli_eV{{
    158'970'000ULL, 174'200'000ULL, 182'610'000ULL,   192'170'000ULL,
    197'220'000ULL, 198'230'000ULL, 206'790'000ULL,   207'800'000ULL,
    247'960'000ULL, 257'510'000ULL, 278'360'000ULL,   281'030'000ULL,
    295'170'000ULL, 329'380'000ULL, 330'700'000ULL,   343'730'000ULL,
    346'350'000ULL, 405'020'000ULL, 437'500'000ULL,   440'020'000ULL,
    454'760'000ULL, 505'330'000ULL, 528'960'000ULL,   538'540'000ULL,
    556'050'000ULL, 562'790'000ULL, 578'260'000ULL,   599'690'000ULL,
    610'050'000ULL, 624'570'000ULL, 628'260'000ULL,   687'950'000ULL,
    735'780'000ULL, 783'590'000ULL, 837'100'000ULL,   877'520'000ULL,
    894'800'000ULL, 909'120'000ULL, 1'036'630'000ULL, 1'068'120'000ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 40U> k_expected_gamma_line_yields{{
    0.8325,     0.0000083, 0.00018,   0.000199,  0.0000033,  0.000035,
    0.000033,   0.0000112, 0.000698,  0.000016,  0.000023,   0.000789,
    0.00001582, 0.000026,  0.0001164, 0.000044,  0.001257,   0.0000298,
    0.000007,   0.004229,  0.0000412, 0.00266,   0.0128,     0.003788,
    0.000029,   0.0000115, 0.0000126, 0.0000266, 0.000011,   0.000798,
    0.0000164,  0.000269,  0.000616,  0.000591,  0.00000582, 0.0000083,
    0.0000101,  0.0000141, 0.0000097, 0.0000142,
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 5U> k_expected_x_ray_energies_milli_eV{{
    4'078'000ULL,
    27'202'000ULL,
    27'472'600ULL,
    31'104'400ULL,
    31'762'300ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 5U> k_expected_x_ray_line_yields{{
    0.09,
    0.2469,
    0.4598,
    0.1316,
    0.0286,
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 13U> k_expected_auger_energies_milli_eV{{
    22'924ULL,
    24'926ULL,
    121'020ULL,
    295'783ULL,
    449'494ULL,
    541'188ULL,
    687'637ULL,
    3'084'750ULL,
    3'678'860ULL,
    4'295'240ULL,
    22'665'300ULL,
    26'505'600ULL,
    30'346'100ULL,
}};

// =============================================================================
// =============================================================================

constexpr std::array<double, 13U> k_expected_auger_line_yields{{
    7.26733,
    2.42736,
    0.83628,
    0.155577,
    1.86981,
    0.0720601,
    0.000271332,
    0.734409,
    0.207978,
    0.0147089,
    0.0807305,
    0.0355571,
    0.00374455,
}};

// =============================================================================
// =============================================================================

constexpr std::array<std::uint64_t, 36U>
    k_expected_conversion_energies_milli_eV{{
        127'180'000ULL, 142'390'000ULL, 150'800'000ULL, 154'360'000ULL,
        160'370'000ULL, 165'410'000ULL, 166'440'000ULL, 169'570'000ULL,
        174'980'000ULL, 176'010'000ULL, 177'980'000ULL, 187'550'000ULL,
        192'590'000ULL, 193'620'000ULL, 202'160'000ULL, 203'190'000ULL,
        216'150'000ULL, 225'710'000ULL, 243'330'000ULL, 246'450'000ULL,
        249'220'000ULL, 252'890'000ULL, 273'630'000ULL, 276'400'000ULL,
        298'900'000ULL, 314'540'000ULL, 326'080'000ULL, 341'720'000ULL,
        408'210'000ULL, 435'390'000ULL, 473'530'000ULL, 500'710'000ULL,
        506'730'000ULL, 533'910'000ULL, 592'800'000ULL, 619'980'000ULL,
    }};

// =============================================================================
// =============================================================================

constexpr std::array<double, 36U> k_expected_conversion_line_yields{{
    0.1372,      0.00000132, 0.000025,    0.01798,     0.0000235,   0.00000036,
    0.0000037,   0.00000024, 0.0000031,   0.00000104,  0.0000043,   0.000004,
    0.000000059, 0.00000063, 0.00000053,  0.00000017,  0.0000377,   0.00000084,
    0.0000059,   0.00000089, 0.0000286,   0.000000147, 0.000000133, 0.0000038,
    0.00000276,  0.00002615, 0.000000438, 0.00000329,  0.0000436,   0.00000614,
    0.0000215,   0.00000283, 0.00012,     0.000019,    0.00000335,  0.000000438,
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

TEST(GGEMSI123Test, BuildsExactIdentityAndOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildI123Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "I-123");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 47'604.24L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 4U);

  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[0U].GetYieldPerDecay()), 0.86195334,
              1.0e-12);

  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(static_cast<double>(emissions[1U].GetYieldPerDecay()), 0.9569,
              1.0e-12);

  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[2U].GetYieldPerDecay()),
              13.705816482, 1.0e-12);

  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(static_cast<double>(emissions[3U].GetYieldPerDecay()),
              0.155575455, 1.0e-12);

  EXPECT_NEAR(static_cast<double>(definition.GetTotalYieldPerDecay()),
              15.680245277, 1.0e-12);
}

// =============================================================================
// =============================================================================

TEST(GGEMSI123Test, PreservesFortyPromptLaraGammaLines) {
  GGEMSRadionuclideDefinition const definition = BuildI123Radionuclide();
  GGEMSRadionuclideEmission const &gamma = definition.GetEmissions()[0U];

  CheckReachableDiscreteChannel(gamma, GGEMSParticleType::Gamma,
                                k_expected_gamma_energies_milli_eV.size());

  auto const energies =
      gamma.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const yields = gamma.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_gamma_energies_milli_eV.size(); ++index) {
    EXPECT_EQ(energies[index], k_expected_gamma_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(yields[index], k_expected_gamma_line_yields[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSI123Test, PreservesLaraAndMirdAtomicRadiations) {
  GGEMSRadionuclideDefinition const definition = BuildI123Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 4U);

  GGEMSRadionuclideEmission const &x_rays = emissions[1U];
  CheckReachableDiscreteChannel(x_rays, GGEMSParticleType::Gamma,
                                k_expected_x_ray_energies_milli_eV.size());

  auto const x_ray_energies =
      x_rays.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const x_ray_yields = x_rays.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_x_ray_energies_milli_eV.size(); ++index) {
    EXPECT_EQ(x_ray_energies[index], k_expected_x_ray_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(x_ray_yields[index], k_expected_x_ray_line_yields[index]);
  }

  GGEMSRadionuclideEmission const &auger = emissions[2U];
  CheckReachableDiscreteChannel(auger, GGEMSParticleType::Electron,
                                k_expected_auger_energies_milli_eV.size());

  auto const auger_energies =
      auger.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const auger_yields = auger.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_auger_energies_milli_eV.size(); ++index) {
    EXPECT_EQ(auger_energies[index], k_expected_auger_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(auger_yields[index], k_expected_auger_line_yields[index]);
  }

  GGEMSRadionuclideEmission const &conversion = emissions[3U];
  CheckReachableDiscreteChannel(conversion, GGEMSParticleType::Electron,
                                k_expected_conversion_energies_milli_eV.size());

  auto const conversion_energies =
      conversion.GetEnergyDistribution().GetEnergyValuesMilliElectronVolt();
  auto const conversion_yields =
      conversion.GetEnergyDistribution().GetRelativeWeights();

  for (std::size_t index = 0U;
       index < k_expected_conversion_energies_milli_eV.size(); ++index) {
    EXPECT_EQ(conversion_energies[index],
              k_expected_conversion_energies_milli_eV[index]);
    EXPECT_DOUBLE_EQ(conversion_yields[index],
                     k_expected_conversion_line_yields[index]);
  }
}

} // namespace
