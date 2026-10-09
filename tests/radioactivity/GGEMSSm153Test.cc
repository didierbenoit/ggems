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
 * \brief Tests the independently selected Sm-153 source contracts.
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
using ggems::core::radioactivity::builtins::BuildSm153Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSSm153Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildSm153Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Sm-153");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 166626.72000L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 35U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_43_8_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 4.4E-7L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_47_2_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 9.8E-6L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_88_9_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 1.43E-5L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_94_5_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 1.41E-4L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_101_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 2.41E-4L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_106_1_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 7.6E-5L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_113_4_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 2.21E-4L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_125_7_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 8.5E-5L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_149_9_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 9.0E-6L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_171_1_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 6.48E-4L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_173_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 5.65E-4L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_222_6_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 2.27E-5L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_537_9_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 2.16E-4L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_634_7_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 0.304L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_656_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 4.2E-4L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_704_4_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[15U].GetYieldPerDecay(), 0.492L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_710_2_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[16U].GetYieldPerDecay(), 0.0062L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_807_6_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[17U].GetYieldPerDecay(), 0.195L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[18U].GetYieldPerDecay(), 0.35466842L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[19U].GetYieldPerDecay(), 0.6937L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_19_81296_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[20U].GetYieldPerDecay(), 0.0033817L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_54_1936_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[21U].GetYieldPerDecay(), 0.0003463L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_68_2574_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[22U].GetYieldPerDecay(), 0.000010125L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_69_673_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[23U].GetYieldPerDecay(), 0.24801L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_75_42213_keV
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[24U].GetYieldPerDecay(), 0.0012704L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_83_36717_keV
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[25U].GetYieldPerDecay(), 0.007227L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_89_48595_keV
  EXPECT_EQ(emissions[26U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[26U].GetYieldPerDecay(), 0.004095L);
  EXPECT_EQ(emissions[26U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_96_8824_keV
  EXPECT_EQ(emissions[27U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[27U].GetYieldPerDecay(), 0.00016449L);
  EXPECT_EQ(emissions[27U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_97_431_keV
  EXPECT_EQ(emissions[28U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[28U].GetYieldPerDecay(), 0.0023219L);
  EXPECT_EQ(emissions[28U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_103_18012_keV
  EXPECT_EQ(emissions[29U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[29U].GetYieldPerDecay(), 0.49324L);
  EXPECT_EQ(emissions[29U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_118_1105_keV
  EXPECT_EQ(emissions[30U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[30U].GetYieldPerDecay(), 4.156E-7L);
  EXPECT_EQ(emissions[30U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_151_6244_keV
  EXPECT_EQ(emissions[31U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[31U].GetYieldPerDecay(), 0.0000094694L);
  EXPECT_EQ(emissions[31U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_166_5546_keV
  EXPECT_EQ(emissions[32U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[32U].GetYieldPerDecay(), 0.0000024129L);
  EXPECT_EQ(emissions[32U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_172_3032_keV
  EXPECT_EQ(emissions[33U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[33U].GetYieldPerDecay(), 2.5978E-7L);
  EXPECT_EQ(emissions[33U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_172_85307_keV
  EXPECT_EQ(emissions[34U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[34U].GetYieldPerDecay(), 0.00027618L);
  EXPECT_EQ(emissions[34U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 2.80859331268L,
                       1.0e-14L);
}

TEST(GGEMSSm153Test, PreservesBetaMinus438Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 88U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 497'726'000ULL);
  EXPECT_EQ(energies.front() - 248'863'000ULL, 112'000ULL);
  EXPECT_EQ(energies.back() + 248'863'000ULL, 43'800'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 88U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 497'726'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 10.961473359822076L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       10.961473359822076L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 22U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.57761445175539095L, 1.0e-12L);
    }
    if (index + 1U == 44U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.87340267292960894L, 1.0e-12L);
    }
    if (index + 1U == 66U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98414866102720722L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus472Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 95U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 496'842'000ULL);
  EXPECT_EQ(energies.front() - 248'421'000ULL, 10'000ULL);
  EXPECT_EQ(energies.back() + 248'421'000ULL, 47'200'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 95U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 496'842'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 11.834280573563756L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       11.834280573563756L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 23U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.56330578462910885L, 1.0e-12L);
    }
    if (index + 1U == 47U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.8687686743516585L, 1.0e-12L);
    }
    if (index + 1U == 71U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98350766143989476L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus889Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 178U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'438'000ULL);
  EXPECT_EQ(energies.front() - 249'719'000ULL, 36'000ULL);
  EXPECT_EQ(energies.back() + 249'719'000ULL, 88'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 178U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'438'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 22.767924969787888L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       22.767924969787888L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 44U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.56059256737347429L, 1.0e-12L);
    }
    if (index + 1U == 89U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86589765469732438L, 1.0e-12L);
    }
    if (index + 1U == 133U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98203857214570001L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus945Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 190U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 497'368'000ULL);
  EXPECT_EQ(energies.front() - 248'684'000ULL, 80'000ULL);
  EXPECT_EQ(energies.back() + 248'684'000ULL, 94'500'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 190U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 497'368'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 24.265739671639803L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       24.265739671639803L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 47U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55947199054926222L, 1.0e-12L);
    }
    if (index + 1U == 95U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86502313499294636L, 1.0e-12L);
    }
    if (index + 1U == 142U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98189674808539806L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus101Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 203U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 497'536'000ULL);
  EXPECT_EQ(energies.front() - 248'768'000ULL, 192'000ULL);
  EXPECT_EQ(energies.back() + 248'768'000ULL, 101'000'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 203U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 497'536'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 26.01693163547397L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       26.01693163547397L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 50U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55598653169890788L, 1.0e-12L);
    }
    if (index + 1U == 101U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.862040089531246L, 1.0e-12L);
    }
    if (index + 1U == 152U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98198766166844664L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus1061Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 213U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'122'000ULL);
  EXPECT_EQ(energies.front() - 249'061'000ULL, 14'000ULL);
  EXPECT_EQ(energies.back() + 249'061'000ULL, 106'100'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 213U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 498'122'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 27.397946590408793L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       27.397946590408793L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 53U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55889707535790822L, 1.0e-12L);
    }
    if (index + 1U == 106U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86132297192499374L, 1.0e-12L);
    }
    if (index + 1U == 159U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98133291257024113L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus1134Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 227U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'558'000ULL);
  EXPECT_EQ(energies.front() - 249'779'000ULL, 334'000ULL);
  EXPECT_EQ(energies.back() + 249'779'000ULL, 113'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 227U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'558'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 29.385359153369549L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       29.385359153369549L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 56U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55343991919824831L, 1.0e-12L);
    }
    if (index + 1U == 113U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86028644636741403L, 1.0e-12L);
    }
    if (index + 1U == 170U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98162801317044313L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus1257Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 252U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'808'000ULL);
  EXPECT_EQ(energies.front() - 249'404'000ULL, 384'000ULL);
  EXPECT_EQ(energies.back() + 249'404'000ULL, 125'700'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 252U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 498'808'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 32.75986584238661L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       32.75986584238661L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 63U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55587154856014631L, 1.0e-12L);
    }
    if (index + 1U == 126U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86014812690822517L, 1.0e-12L);
    }
    if (index + 1U == 189U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98148979604909981L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus1499Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 300U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'666'000ULL);
  EXPECT_EQ(energies.front() - 249'833'000ULL, 200'000ULL);
  EXPECT_EQ(energies.back() + 249'833'000ULL, 149'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 300U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'666'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 39.505896403415825L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       39.505896403415825L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 75U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.54978859371142008L, 1.0e-12L);
    }
    if (index + 1U == 150U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.85642272583003776L, 1.0e-12L);
    }
    if (index + 1U == 225U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98075761044024301L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus1711Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 343U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'832'000ULL);
  EXPECT_EQ(energies.front() - 249'416'000ULL, 624'000ULL);
  EXPECT_EQ(energies.back() + 249'416'000ULL, 171'100'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 343U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 498'832'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 45.523359552130721L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       45.523359552130721L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 85U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.54091508177505565L, 1.0e-12L);
    }
    if (index + 1U == 171U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.85198706505450172L, 1.0e-12L);
    }
    if (index + 1U == 257U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97995361783195001L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus173Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 347U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'558'000ULL);
  EXPECT_EQ(energies.front() - 249'279'000ULL, 374'000ULL);
  EXPECT_EQ(energies.back() + 249'279'000ULL, 173'000'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 347U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 498'558'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 46.067729996097704L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       46.067729996097704L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 86U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.54049226811940221L, 1.0e-12L);
    }
    if (index + 1U == 173U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.85171212653199957L, 1.0e-12L);
    }
    if (index + 1U == 260U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97989861037769443L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus2226Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 446U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'102'000ULL);
  EXPECT_EQ(energies.front() - 249'551'000ULL, 508'000ULL);
  EXPECT_EQ(energies.back() + 249'551'000ULL, 222'600'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 446U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'102'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 60.555817847129653L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       60.555817847129653L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 111U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.53033683203084436L, 1.0e-12L);
    }
    if (index + 1U == 223U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.84551790966884188L, 1.0e-12L);
    }
    if (index + 1U == 334U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97832821157419381L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus5379Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1076U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'906'000ULL);
  EXPECT_EQ(energies.front() - 249'953'000ULL, 1'144'000ULL);
  EXPECT_EQ(energies.back() + 249'953'000ULL, 537'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1076U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'906'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 176.62032464157352L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       176.62032464157352L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 269U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.43315232881913629L, 1.0e-12L);
    }
    if (index + 1U == 538U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.75666942322471131L, 1.0e-12L);
    }
    if (index + 1U == 807U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.95404323921480827L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus6347Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1270U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'762'000ULL);
  EXPECT_EQ(energies.front() - 249'881'000ULL, 2'260'000ULL);
  EXPECT_EQ(energies.back() + 249'881'000ULL, 634'700'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1270U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'762'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 198.26827661393557L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       198.26827661393557L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 317U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.4488744582320639L, 1.0e-12L);
    }
    if (index + 1U == 635U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.79242451060453134L, 1.0e-12L);
    }
    if (index + 1U == 952U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96776256808247296L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus656Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1313U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'618'000ULL);
  EXPECT_EQ(energies.front() - 249'809'000ULL, 1'566'000ULL);
  EXPECT_EQ(energies.back() + 249'809'000ULL, 656'000'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1313U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'618'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 217.82287120968726L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       217.82287120968726L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 328U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.42709840314374087L, 1.0e-12L);
    }
    if (index + 1U == 656U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.75043448128170442L, 1.0e-12L);
    }
    if (index + 1U == 984U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.95159735785339294L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus7044Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1409U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'928'000ULL);
  EXPECT_EQ(energies.front() - 249'964'000ULL, 1'448'000ULL);
  EXPECT_EQ(energies.back() + 249'964'000ULL, 704'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1409U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'928'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 223.97628875906673L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       223.97628875906673L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 352U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.437895602042563L, 1.0e-12L);
    }
    if (index + 1U == 704U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.78452658021505939L, 1.0e-12L);
    }
    if (index + 1U == 1056U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96615318796765381L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus7102Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1421U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'788'000ULL);
  EXPECT_EQ(energies.front() - 249'894'000ULL, 1'252'000ULL);
  EXPECT_EQ(energies.back() + 249'894'000ULL, 710'200'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1421U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'788'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 226.1405320415081L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       226.1405320415081L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 355U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.4369852306318045L, 1.0e-12L);
    }
    if (index + 1U == 710U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.78392130455463493L, 1.0e-12L);
    }
    if (index + 1U == 1065U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96602834331296539L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesBetaMinus8076Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1616U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'752'000ULL);
  EXPECT_EQ(energies.front() - 249'876'000ULL, 768'000ULL);
  EXPECT_EQ(energies.back() + 249'876'000ULL, 807'600'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1616U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'752'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 263.01498212261987L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       263.01498212261987L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 404U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.42259888339302282L, 1.0e-12L);
    }
    if (index + 1U == 808U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.77448071764168569L, 1.0e-12L);
    }
    if (index + 1U == 1212U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96418599385674786L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSm153Test, PreservesNuclearGamma) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 61U> expected_energies{
    19'812'960'000ULL,  54'193'600'000ULL,  68'257'400'000ULL,
    69'673'000'000ULL,  75'422'130'000ULL,  83'367'170'000ULL,
    89'485'950'000ULL,  96'882'400'000ULL,  97'431'000'000ULL,
    103'180'120'000ULL, 118'110'500'000ULL, 151'624'400'000ULL,
    166'554'600'000ULL, 172'303'200'000ULL, 172'853'070'000ULL,
    412'050'000'000ULL, 424'400'000'000ULL, 436'900'000'000ULL,
    443'200'000'000ULL, 462'000'000'000ULL, 463'600'000'000ULL,
    485'000'000'000ULL, 487'750'000'000ULL, 509'150'000'000ULL,
    521'300'000'000ULL, 531'400'000'000ULL, 533'200'000'000ULL,
    539'100'000'000ULL, 542'700'000'000ULL, 545'750'000'000ULL,
    554'940'000'000ULL, 574'100'000'000ULL, 578'750'000'000ULL,
    584'550'000'000ULL, 587'600'000'000ULL, 590'960'000'000ULL,
    596'700'000'000ULL, 598'300'000'000ULL, 598'540'000'000ULL,
    603'600'000'000ULL, 604'030'000'000ULL, 609'500'000'000ULL,
    609'950'000'000ULL, 615'510'000'000ULL, 615'800'000'000ULL,
    617'900'000'000ULL, 630'500'000'000ULL, 634'800'000'000ULL,
    636'500'000'000ULL, 657'210'000'000ULL, 657'550'000'000ULL,
    662'400'000'000ULL, 677'000'000'000ULL, 682'000'000'000ULL,
    694'100'000'000ULL, 701'800'000'000ULL, 706'800'000'000ULL,
    713'900'000'000ULL, 719'000'000'000ULL, 760'500'000'000ULL,
    763'800'000'000ULL,
  };
  constexpr std::array<double, 61U> expected_weights{
    1.05e-06, 1.9e-05,  1.3e-05,  0.04691,  0.00169,   0.00193,  0.00158,
    7e-05,    0.00767,  0.2919,   2.3e-06,  0.0001033, 6.1e-06,  4e-06,
    0.000736, 1.91e-05, 1.95e-05, 1.58e-05, 4.1e-06,   1.58e-05, 0.000127,
    3.8e-06,  3.6e-06,  1.9e-05,  6.7e-05,  0.000544,  0.000294, 0.000207,
    2.34e-05, 9e-06,    4.7e-05,  1.6e-06,  3.4e-05,   1.07e-05, 4.8e-06,
    1.22e-05, 9.9e-05,  2e-05,    2e-05,    4.9e-05,   4.9e-05,  0.000129,
    0.000129, 5e-06,    5e-06,    6.7e-06,  9.9e-07,   5e-06,    1.95e-05,
    3.7e-06,  3.7e-06,  7e-07,    4.4e-07,  1.5e-06,   2e-07,    2.9e-07,
    2.3e-07,  2.31e-06, 2.5e-07,  3.2e-07,  4.4e-07,
  };
  ASSERT_EQ(energies.size(), 61U);
  EXPECT_EQ(distribution.GetTableCount(), 61U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.35466842L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 101.02415343057045L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       101.02415343057045L, 1.0567601864993572e-05L);
}

TEST(GGEMSSm153Test, PreservesAtomicXray) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    6'483'000'000ULL,  40'902'400'000ULL, 41'542'700'000ULL,
    47'105'100'000ULL, 48'380'000'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.1088, 0.166, 0.3, 0.0945, 0.0244,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.6937L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 36.888947311517946L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       36.888947311517946L, 4.9774527385830885e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion1981296Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    11'760'960'000ULL, 12'195'860'000ULL, 12'836'060'000ULL,
    18'375'760'000ULL, 19'579'540'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    1.07e-05, 0.00109, 0.00152, 0.00061, 0.000151,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0033817L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 13.926683328503415L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       13.926683328503415L, 1.0102025069296361e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion541936Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    5'675'700'000ULL,  46'142'700'000ULL, 46'577'600'000ULL,
    47'217'800'000ULL, 52'757'500'000ULL, 53'961'300'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000118, 1.49e-05, 7.4e-05, 8.8e-05, 4.1e-05, 1.04e-05,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0003463L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 33.737865001443836L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       33.737865001443836L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion682574Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    19'739'500'000ULL, 60'206'500'000ULL, 60'641'400'000ULL,
    61'281'600'000ULL, 66'821'300'000ULL, 68'025'100'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8.4e-06, 8.3e-07, 2.3e-07, 3e-07, 2.9e-07, 7.5e-08,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000010125L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 26.922988296296296L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       26.922988296296296L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion69673Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    21'154'000'000ULL, 61'621'000'000ULL, 62'055'900'000ULL,
    62'696'100'000ULL, 68'235'800'000ULL, 69'439'580'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.205, 0.0268, 0.00424, 0.00266, 0.00737, 0.00194,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.24801L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 28.448498339583082L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       28.448498339583082L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion7542213Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    26'903'130'000ULL, 67'370'130'000ULL, 67'805'030'000ULL,
    68'445'230'000ULL, 73'984'930'000ULL, 75'188'710'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00103, 0.000125, 2.57e-05, 3.75e-05, 4.14e-05, 1.08e-05,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0012704L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 34.883335733627206L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       34.883335733627206L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion8336717Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    34'848'170'000ULL, 75'315'170'000ULL, 75'750'070'000ULL,
    76'390'270'000ULL, 81'929'970'000ULL, 83'133'750'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00444, 0.000523, 0.000799, 0.00084, 0.000498, 0.000127,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.007227L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 51.220001764217521L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       51.220001764217521L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion8948595Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[26U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    40'966'950'000ULL, 81'433'950'000ULL, 81'868'850'000ULL,
    82'509'050'000ULL, 88'048'750'000ULL, 89'252'530'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00332, 0.00043, 0.0001, 7.6e-05, 0.000134, 3.5e-05,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.004095L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 48.939393492063495L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       48.939393492063495L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion968824Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[27U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    48'364'800'000ULL, 88'831'800'000ULL, 89'266'700'000ULL,
    89'906'900'000ULL, 95'446'600'000ULL, 96'650'400'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000103, 1.18e-05, 1.78e-05, 1.81e-05, 1.1e-05, 2.79e-06,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00016449L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 64.23259168338501L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       64.23259168338501L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion97431Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[28U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    48'912'000'000ULL, 89'379'000'000ULL, 89'813'900'000ULL,
    90'454'100'000ULL, 95'993'800'000ULL, 97'197'580'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00195, 0.000199, 4.13e-05, 5.22e-05, 6.31e-05, 1.63e-05,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0023219L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 55.660178484861532L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       55.660178484861532L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion10318012Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[29U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    54'661'120'000ULL, 95'128'120'000ULL,  95'563'020'000ULL,
    96'203'220'000ULL, 101'742'920'000ULL, 102'946'700'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.414, 0.054, 0.00587, 0.00231, 0.01349, 0.00357,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.49324L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 61.40993884397048L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       61.40993884397048L, 6.8454176023602482e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion1181105Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[30U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    69'591'500'000ULL,  110'058'500'000ULL, 110'493'400'000ULL,
    111'133'600'000ULL, 116'673'300'000ULL, 117'877'100'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.5e-07, 3.7e-08, 6.6e-09, 8.1e-09, 1.1e-08, 2.9e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 4.156E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 76.236472545717035L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       76.236472545717035L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion1516244Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[31U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    103'106'700'000ULL, 143'573'700'000ULL, 144'008'600'000ULL,
    144'648'800'000ULL, 150'188'500'000ULL, 151'392'300'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8.01e-06, 8.64e-07, 1.29e-07, 1.55e-07, 2.47e-07, 6.44e-08,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000094694L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 109.59260785477433L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       109.59260785477433L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion1665546Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[32U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    118'037'800'000ULL, 158'504'800'000ULL, 158'939'700'000ULL,
    159'579'900'000ULL, 165'119'600'000ULL, 166'323'400'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.6e-06, 1.62e-07, 2.49e-07, 2.2e-07, 1.45e-07, 3.69e-08,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000024129L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 132.33101096605745L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       132.33101096605745L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion1723032Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[33U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    123'787'000'000ULL, 164'254'000'000ULL, 164'688'900'000ULL,
    165'329'100'000ULL, 170'868'800'000ULL, 172'072'600'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.2e-07, 2.41e-08, 3.3e-09, 3.9e-09, 6.72e-09, 1.76e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.5978E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 130.22944172761567L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       130.22944172761567L, 6.8454203963279716e-08L);
}

TEST(GGEMSSm153Test, PreservesConversion17285307Kev) {
  auto const definition = BuildSm153Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[34U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    124'334'200'000ULL, 164'801'200'000ULL, 165'236'100'000ULL,
    165'876'300'000ULL, 171'416'000'000ULL, 172'619'780'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000216, 2.62e-05, 1.164e-05, 9.12e-06, 1.05e-05, 2.72e-06,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00027618L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 133.53433449779129L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       133.53433449779129L, 6.8454176023602482e-08L);
}

} // namespace
