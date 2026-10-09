// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Tests the independently selected Ra-223 source contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMSLongDoubleAssertions.hh"

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace {

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::builtins::BuildRa223Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSRa223Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildRa223Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Ra-223");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 987552.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 30U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // alpha
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0082057L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.358478714L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.7268L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_4_47_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 0.3264L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // conversion_9_9_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 0.1561L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_14_37_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.0997L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_31_87_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 0.0021114L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_69_5_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 0.0005147L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_103_2_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 0.000581L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_104_04_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 0.001864L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_106_78_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 0.00253209L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_110_856_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 0.0031136L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_122_319_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 0.090846L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_144_27_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 0.154198L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_154_208_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 0.223528L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_158_635_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[15U].GetYieldPerDecay(), 0.024704L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_179_54_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[16U].GetYieldPerDecay(), 0.0032559L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_221_32_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[17U].GetYieldPerDecay(), 0.000024239L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_249_49_keV
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[18U].GetYieldPerDecay(), 0.0002525L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_251_6_keV
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[19U].GetYieldPerDecay(), 0.0003088L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_269_463_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[20U].GetYieldPerDecay(), 0.112209L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_288_18_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[21U].GetYieldPerDecay(), 0.000058633L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_323_871_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[22U].GetYieldPerDecay(), 0.019186L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_328_38_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[23U].GetYieldPerDecay(), 0.000055015L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_334_01_keV
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[24U].GetYieldPerDecay(), 0.00010058L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_338_282_keV
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[25U].GetYieldPerDecay(), 0.01224667L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_342_78_keV
  EXPECT_EQ(emissions[26U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[26U].GetYieldPerDecay(), 0.000055588L);
  EXPECT_EQ(emissions[26U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_371_676_keV
  EXPECT_EQ(emissions[27U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[27U].GetYieldPerDecay(), 0.001662017L);
  EXPECT_EQ(emissions[27U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_372_86_keV
  EXPECT_EQ(emissions[28U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[28U].GetYieldPerDecay(), 0.0000104340L);
  EXPECT_EQ(emissions[28U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_445_033_keV
  EXPECT_EQ(emissions[29U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[29U].GetYieldPerDecay(), 0.00262479L);
  EXPECT_EQ(emissions[29U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 3.3317273700L,
                       1.0e-14L);
}

TEST(GGEMSRa223Test, PreservesAlpha) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 26U> expected_energies{
    5'014'300'000'000ULL, 5'026'100'000'000ULL, 5'035'900'000'000ULL,
    5'056'500'000'000ULL, 5'086'000'000'000ULL, 5'112'500'000'000ULL,
    5'137'100'000'000ULL, 5'151'980'000'000ULL, 5'173'100'000'000ULL,
    5'211'100'000'000ULL, 5'237'120'000'000ULL, 5'259'140'000'000ULL,
    5'283'650'000'000ULL, 5'288'190'000'000ULL, 5'339'370'000'000ULL,
    5'366'370'000'000ULL, 5'432'830'000'000ULL, 5'434'600'000'000ULL,
    5'481'700'000'000ULL, 5'502'120'000'000ULL, 5'539'430'000'000ULL,
    5'606'990'000'000ULL, 5'715'840'000'000ULL, 5'747'140'000'000ULL,
    5'857'520'000'000ULL, 5'871'630'000'000ULL,
  };
  constexpr std::array<double, 26U> expected_weights{
    4.4e-06, 6.3e-06, 4e-06,   2e-06,   3e-06,   6e-06,   1.7e-05,
    0.00021, 0.00026, 5.3e-05, 0.00041, 0.00042, 0.00093, 0.0016,
    0.0013,  0.0013,  0.005,   0.016,   8e-05,   0.0074,  0.106,
    0.258,   0.496,   0.1,     0.0032,  0.01,
  };
  ASSERT_EQ(energies.size(), 26U);
  EXPECT_EQ(distribution.GetTableCount(), 26U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0082057L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 5664.3745956306338L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       5664.3745956306338L, 5.1909300888180733e-06L);
}

TEST(GGEMSRa223Test, PreservesNuclearGamma) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 82U> expected_energies{
    4'470'000'000ULL,   9'900'000'000ULL,   14'370'000'000ULL,
    31'870'000'000ULL,  69'500'000'000ULL,  70'900'000'000ULL,
    102'200'000'000ULL, 103'200'000'000ULL, 104'040'000'000ULL,
    106'780'000'000ULL, 108'500'000'000ULL, 110'856'000'000ULL,
    114'700'000'000ULL, 122'319'000'000ULL, 131'600'000'000ULL,
    138'300'000'000ULL, 144'270'000'000ULL, 147'200'000'000ULL,
    154'208'000'000ULL, 158'635'000'000ULL, 165'800'000'000ULL,
    175'650'000'000ULL, 177'300'000'000ULL, 179'540'000'000ULL,
    199'300'000'000ULL, 221'320'000'000ULL, 247'200'000'000ULL,
    249'490'000'000ULL, 251'600'000'000ULL, 255'200'000'000ULL,
    255'700'000'000ULL, 260'400'000'000ULL, 269'463'000'000ULL,
    270'300'000'000ULL, 286'000'000'000ULL, 288'180'000'000ULL,
    323'871'000'000ULL, 328'380'000'000ULL, 334'010'000'000ULL,
    338'282'000'000ULL, 342'780'000'000ULL, 355'500'000'000ULL,
    355'700'000'000ULL, 361'890'000'000ULL, 362'900'000'000ULL,
    368'560'000'000ULL, 371'676'000'000ULL, 372'860'000'000ULL,
    376'260'000'000ULL, 383'350'000'000ULL, 387'700'000'000ULL,
    390'100'000'000ULL, 430'600'000'000ULL, 432'450'000'000ULL,
    445'033'000'000ULL, 487'500'000'000ULL, 490'800'000'000ULL,
    500'000'000'000ULL, 510'000'000'000ULL, 523'200'000'000ULL,
    527'611'000'000ULL, 532'900'000'000ULL, 537'600'000'000ULL,
    541'990'000'000ULL, 545'800'000'000ULL, 574'100'000'000ULL,
    579'600'000'000ULL, 584'300'000'000ULL, 594'000'000'000ULL,
    598'721'000'000ULL, 609'310'000'000ULL, 619'100'000'000ULL,
    623'680'000'000ULL, 631'700'000'000ULL, 641'700'000'000ULL,
    646'100'000'000ULL, 696'900'000'000ULL, 711'300'000'000ULL,
    718'400'000'000ULL, 728'400'000'000ULL, 732'800'000'000ULL,
    737'200'000'000ULL,
  };
  constexpr std::array<double, 82U> expected_weights{
    6.4e-08,  0.000158, 0.000185, 1.05e-06, 7e-05,   3.6e-05,  8e-06,   6e-05,
    0.000194, 0.000233, 6e-05,    0.00058,  0.0001,  0.01238,  6e-05,   1.7e-05,
    0.0336,   6e-05,    0.0584,   0.00713,  5.4e-05, 0.00017,  0.00047, 0.00154,
    3e-05,    0.00036,  9.7e-05,  0.00038,  0.00055, 0.00048,  5.5e-05, 6.7e-05,
    0.1423,   7e-06,    1.1e-05,  0.00161,  0.0406,  0.00203,  0.001,   0.0285,
    0.00226,  4.3e-05,  2.8e-05,  0.00028,  0.00016, 9e-05,    0.00499, 0.00051,
    0.00013,  7e-05,    0.00016,  4.6e-05,  0.0002,  0.000356, 0.0128,  0.00011,
    1.7e-05,  1.4e-05,  4e-06,    1.4e-05,  0.00073, 1.4e-05,  2.1e-05, 1.4e-05,
    1.1e-05,  1.1e-05,  1.4e-05,  1.4e-05,  1.4e-05, 0.00092,  0.00057, 3.6e-05,
    9e-05,    4e-06,    1.7e-05,  4e-06,    7e-06,   3.7e-05,  1.4e-05, 2.8e-06,
    6e-06,    2.8e-06,
  };
  ASSERT_EQ(energies.size(), 82U);
  EXPECT_EQ(distribution.GetTableCount(), 82U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.358478714L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 253.46606180801018L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       253.46606180801018L, 1.3990363797008992e-05L);
}

TEST(GGEMSRa223Test, PreservesAtomicXray) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    13'697'500'000ULL, 81'070'000'000ULL, 83'780'000'000ULL,
    94'854'700'000ULL, 97'896'700'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.221, 0.1486, 0.245, 0.085, 0.0272,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.7268L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 63.739253219592733L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       63.739253219592733L, 9.902076965570449e-08L);
}

TEST(GGEMSRa223Test, PreservesConversion447Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 860'000'000ULL);
}

TEST(GGEMSRa223Test, PreservesConversion99Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 2U> expected_energies{
    6'290'000'000ULL,
    9'277'000'000ULL,
  };
  constexpr std::array<double, 2U> expected_weights{
    0.119,
    0.0371,
  };
  ASSERT_EQ(energies.size(), 2U);
  EXPECT_EQ(distribution.GetTableCount(), 2U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.1561L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 6.9999147982062784L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       6.9999147982062784L, 2.3909302651882171e-09L);
}

TEST(GGEMSRa223Test, PreservesConversion1437Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 2U> expected_energies{
    10'760'000'000ULL,
    13'747'000'000ULL,
  };
  constexpr std::array<double, 2U> expected_weights{
    0.076,
    0.0237,
  };
  ASSERT_EQ(energies.size(), 2U);
  EXPECT_EQ(distribution.GetTableCount(), 2U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0997L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 11.470049147442326L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       11.470049147442326L, 2.3909302651882171e-09L);
}

TEST(GGEMSRa223Test, PreservesConversion3187Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    13'822'000'000ULL, 14'542'000'000ULL, 17'260'000'000ULL,
    28'260'000'000ULL, 31'247'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    2.14e-05, 0.00076, 0.00078, 0.00042, 0.00013,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0021114L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 19.296116699820026L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       19.296116699820026L, 2.128536982834339e-08L);
}

TEST(GGEMSRa223Test, PreservesConversion695Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    51'450'000'000ULL, 52'170'000'000ULL, 54'890'000'000ULL,
    65'890'000'000ULL, 68'880'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.00035, 3.9e-05, 2.7e-06, 9.3e-05, 3e-05,
  };
  ASSERT_EQ(energies.size(), 5U);
  EXPECT_EQ(distribution.GetTableCount(), 5U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0005147L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 55.147664659024677L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       55.147664659024677L, 2.1291190594434739e-08L);
}

TEST(GGEMSRa223Test, PreservesConversion1032Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    4'800'000'000ULL,  85'150'000'000ULL, 85'870'000'000ULL,
    88'590'000'000ULL, 99'590'000'000ULL, 102'580'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0003, 5e-05, 9e-05, 7e-05, 5.4e-05, 1.7e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000581L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 46.039277108433737L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       46.039277108433737L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion10404Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    5'643'000'000ULL,  85'992'000'000ULL,  86'712'000'000ULL,
    89'430'000'000ULL, 100'430'000'000ULL, 103'417'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.001, 0.00016, 0.00029, 0.00019, 0.00017, 5.4e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.001864L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 45.170342274678113L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       45.170342274678113L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion10678Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    8'383'000'000ULL,  88'732'000'000ULL,  89'452'000'000ULL,
    92'170'000'000ULL, 103'170'000'000ULL, 106'157'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00204, 0.000335, 3.73e-05, 2.49e-06, 8.9e-05, 2.83e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00253209L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 24.714356914643634L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       24.714356914643634L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion110856Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    12'459'000'000ULL, 92'808'000'000ULL,  93'528'000'000ULL,
    96'246'000'000ULL, 107'246'000'000ULL, 110'233'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000211, 6.56e-05, 0.00121, 0.00087, 0.000577, 0.00018,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0031136L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 92.28636812692703L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       92.28636812692703L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion122319Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    23'922'000'000ULL,  104'271'000'000ULL, 104'991'000'000ULL,
    107'709'000'000ULL, 118'709'000'000ULL, 121'696'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0728, 0.01184, 0.0016, 0.000285, 0.00328, 0.001041,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.090846L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 40.627258888668734L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       40.627258888668734L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion14427Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    45'873'000'000ULL,  126'222'000'000ULL, 126'942'000'000ULL,
    129'660'000'000ULL, 140'660'000'000ULL, 143'647'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.124, 0.0201, 0.00255, 0.000339, 0.00547, 0.001739,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.154198L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 62.336628056135616L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       62.336628056135616L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion154208Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    55'811'000'000ULL,  136'160'000'000ULL, 136'880'000'000ULL,
    139'598'000'000ULL, 150'598'000'000ULL, 153'585'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.1805, 0.0293, 0.00328, 0.000208, 0.00777, 0.00247,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.223528L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 71.985960121327082L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       71.985960121327082L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion158635Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    60'238'000'000ULL,  140'587'000'000ULL, 141'307'000'000ULL,
    144'025'000'000ULL, 155'025'000'000ULL, 158'012'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0198, 0.0032, 0.00045, 8e-05, 0.000891, 0.000283,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.024704L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 76.932708103950773L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       76.932708103950773L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion17954Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    81'140'000'000ULL,  161'490'000'000ULL, 162'210'000'000ULL,
    164'930'000'000ULL, 175'930'000'000ULL, 178'920'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00249, 0.000402, 0.000126, 5.1e-05, 0.000142, 4.49e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0032559L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 100.99297828557388L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       100.99297828557388L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion22132Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    122'920'000'000ULL, 203'270'000'000ULL, 203'990'000'000ULL,
    206'710'000'000ULL, 217'710'000'000ULL, 220'700'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.95e-05, 2.43e-06, 6.6e-07, 5.2e-07, 8.6e-07, 2.69e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000024239L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 139.42850777672345L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       139.42850777672345L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion24949Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    151'093'000'000ULL, 231'442'000'000ULL, 232'162'000'000ULL,
    234'880'000'000ULL, 245'880'000'000ULL, 248'867'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00019, 2.7e-05, 1.4e-05, 6e-06, 1.18e-05, 3.7e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0002525L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 172.03304514851484L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       172.03304514851484L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion2516Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    153'200'000'000ULL, 233'550'000'000ULL, 234'270'000'000ULL,
    236'990'000'000ULL, 247'990'000'000ULL, 250'980'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00022, 3.9e-05, 2e-05, 8e-06, 1.65e-05, 5.3e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0003088L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 177.51230246113991L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       177.51230246113991L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion269463Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    171'066'000'000ULL, 251'415'000'000ULL, 252'135'000'000ULL,
    254'853'000'000ULL, 265'853'000'000ULL, 268'840'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0906, 0.01454, 0.00176, 0.000157, 0.00391, 0.001242,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.112209L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 187.25153714051459L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       187.25153714051459L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion28818Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    189'783'000'000ULL, 270'132'000'000ULL, 270'852'000'000ULL,
    273'570'000'000ULL, 284'570'000'000ULL, 287'557'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.75e-05, 6.09e-06, 1.383e-06, 1.019e-06, 2.01e-06, 6.31e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000058633L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 205.79855530162195L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       205.79855530162195L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion323871Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    225'474'000'000ULL, 305'823'000'000ULL, 306'543'000'000ULL,
    309'261'000'000ULL, 320'261'000'000ULL, 323'248'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0155, 0.00248, 0.0003, 2.8e-05, 0.000666, 0.000212,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.019186L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 241.62059053476494L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       241.62059053476494L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion32838Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    229'983'000'000ULL, 310'332'000'000ULL, 311'052'000'000ULL,
    313'770'000'000ULL, 324'770'000'000ULL, 327'757'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.47e-05, 5.81e-06, 1.2e-06, 8.61e-07, 1.86e-06, 5.84e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000055015L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 245.79059670998819L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       245.79059670998819L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion33401Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    235'610'000'000ULL, 315'960'000'000ULL, 316'680'000'000ULL,
    319'400'000'000ULL, 330'400'000'000ULL, 333'390'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.46e-05, 8.7e-06, 1.8e-05, 7.57e-06, 8.9e-06, 2.81e-06,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00010058L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 274.49435175979318L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       274.49435175979318L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion338282Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    239'885'000'000ULL, 320'234'000'000ULL, 320'954'000'000ULL,
    323'672'000'000ULL, 334'672'000'000ULL, 337'659'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00992, 0.001587, 0.0001761, 1.017e-05, 0.00042, 0.0001334,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.01224667L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 255.84818177022817L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       255.84818177022817L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion34278Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[26U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    244'383'000'000ULL, 324'732'000'000ULL, 325'452'000'000ULL,
    328'170'000'000ULL, 339'170'000'000ULL, 342'157'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.52e-05, 5.9e-06, 1.19e-06, 8.4e-07, 1.87e-06, 5.88e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000055588L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 260.13559214218895L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       260.13559214218895L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion371676Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[27U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    273'279'000'000ULL, 353'628'000'000ULL, 354'348'000'000ULL,
    357'066'000'000ULL, 368'066'000'000ULL, 371'053'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.001347, 0.000215, 2.38e-05, 1.357e-06, 5.68e-05, 1.806e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.001662017L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 289.20415371322918L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       289.20415371322918L, 1.3758870011568069e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion37286Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[28U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    274'460'000'000ULL, 354'810'000'000ULL, 355'530'000'000ULL,
    358'250'000'000ULL, 369'250'000'000ULL, 372'240'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8.5e-06, 1.117e-06, 2.122e-07, 1.474e-07, 3.48e-07, 1.094e-07,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000104340L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 290.08092217749663L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       290.08092217749663L, 1.3759708201885222e-07L);
}

TEST(GGEMSRa223Test, PreservesConversion445033Kev) {
  auto const definition = BuildRa223Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[29U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    346'636'000'000ULL, 426'985'000'000ULL, 427'705'000'000ULL,
    430'423'000'000ULL, 441'423'000'000ULL, 444'410'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00213, 0.000338, 3.7e-05, 2.09e-06, 8.93e-05, 2.84e-05,
  };
  ASSERT_EQ(energies.size(), 6U);
  EXPECT_EQ(distribution.GetTableCount(), 6U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GT(weights[index], 0.0);
    EXPECT_GT(tickets[index], previous_ticket);
    EXPECT_EQ(energies[index], expected_energies[index]);
    EXPECT_DOUBLE_EQ(weights[index], expected_weights[index]);
    weight_sum += static_cast<long double>(weights[index]);
    moment += static_cast<long double>(weights[index]) *
              static_cast<long double>(energies[index]);
    ticket_moment +=
      static_cast<long double>(tickets[index] - previous_ticket) *
      static_cast<long double>(energies[index]);
    previous_ticket = tickets[index];
  }
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);
  long double const keV =
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00262479L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 362.47493969803298L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       362.47493969803298L, 1.3758870011568069e-07L);
}

} // namespace
