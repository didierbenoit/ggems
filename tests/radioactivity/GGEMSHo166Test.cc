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
 * \brief Tests the independently selected Ho-166 source contracts.
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
using ggems::core::radioactivity::builtins::BuildHo166Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSHo166Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildHo166Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Ho-166");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 96508.800L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 26U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_23_4_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 3.39E-4L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_40_6_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 1.1E-6L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_191_4_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.00296L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_325_4_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 2.8E-5L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_393_8_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 0.00953L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1067_9_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 6.3E-5L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1773_2_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 0.502L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1853_8_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 0.484L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 0.0788853L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 0.18275L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_80_5776_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 0.44810L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_184_4124_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 0.0000048157L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_520_914_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 5.037E-8L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_674_125_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 0.0000015282L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_705_326_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 9.968E-7L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_785_903_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[15U].GetYieldPerDecay(), 6.6558E-7L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1263_406_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[16U].GetYieldPerDecay(), 3.1975E-8L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1379_447_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[17U].GetYieldPerDecay(), 0.0000160585L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1447_817_keV
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[18U].GetYieldPerDecay(), 3.0626E-8L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1528_394_keV
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[19U].GetYieldPerDecay(), 1.3122E-9L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1581_849_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[20U].GetYieldPerDecay(), 0.00000109564L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1662_426_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[21U].GetYieldPerDecay(), 6.53293E-7L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1732_61_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[22U].GetYieldPerDecay(), 7.065E-10L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1749_838_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[23U].GetYieldPerDecay(), 1.39659E-7L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1813_19_keV
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[24U].GetYieldPerDecay(), 7.6519E-10L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1830_414_keV
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[25U].GetYieldPerDecay(), 3.83217E-8L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 1.70868250744859L,
                       1.0e-14L);
}

TEST(GGEMSHo166Test, PreservesBetaMinus234Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 47U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 497'872'000ULL);
  EXPECT_EQ(energies.front() - 248'936'000ULL, 16'000ULL);
  EXPECT_EQ(energies.back() + 248'936'000ULL, 23'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 47U);
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
                (static_cast<std::uint64_t>(index) * 497'872'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 5.7956314798813588L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       5.7956314798813588L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 11U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55574309922473475L, 1.0e-12L);
    }
    if (index + 1U == 23U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86940933691136624L, 1.0e-12L);
    }
    if (index + 1U == 35U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98414367587407792L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus406Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 82U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 495'120'000ULL);
  EXPECT_EQ(energies.front() - 247'560'000ULL, 160'000ULL);
  EXPECT_EQ(energies.back() + 247'560'000ULL, 40'600'000'000ULL);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 495'120'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 10.148467627442519L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       10.148467627442519L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 20U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.56802905645035029L, 1.0e-12L);
    }
    if (index + 1U == 41U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.8740292273521223L, 1.0e-12L);
    }
    if (index + 1U == 61U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98312017500309734L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus1914Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 383U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'738'000ULL);
  EXPECT_EQ(energies.front() - 249'869'000ULL, 346'000ULL);
  EXPECT_EQ(energies.back() + 249'869'000ULL, 191'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 383U);
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
                (static_cast<std::uint64_t>(index) * 499'738'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 51.277252296247596L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       51.277252296247596L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 95U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.537426455575159L, 1.0e-12L);
    }
    if (index + 1U == 191U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.84985207767310378L, 1.0e-12L);
    }
    if (index + 1U == 287U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.9795456109938504L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus3254Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 651U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'846'000ULL);
  EXPECT_EQ(energies.front() - 249'923'000ULL, 254'000ULL);
  EXPECT_EQ(energies.back() + 249'923'000ULL, 325'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 651U);
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
                (static_cast<std::uint64_t>(index) * 499'846'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 103.42103634362208L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       103.42103634362208L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 162U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.44738057875410303L, 1.0e-12L);
    }
    if (index + 1U == 325U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.77401224340430819L, 1.0e-12L);
    }
    if (index + 1U == 488U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96010512686957039L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus3938Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 788U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'746'000ULL);
  EXPECT_EQ(energies.front() - 249'873'000ULL, 152'000ULL);
  EXPECT_EQ(energies.back() + 249'873'000ULL, 393'800'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 788U);
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
                (static_cast<std::uint64_t>(index) * 499'746'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 113.82483638048022L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       113.82483638048022L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 197U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.49710028132654521L, 1.0e-12L);
    }
    if (index + 1U == 394U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.82354836925167496L, 1.0e-12L);
    }
    if (index + 1U == 591U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97426109594826948L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus10679Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 2136U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'952'000ULL);
  EXPECT_EQ(energies.front() - 249'976'000ULL, 2'528'000ULL);
  EXPECT_EQ(energies.back() + 249'976'000ULL, 1'067'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 2136U);
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
                (static_cast<std::uint64_t>(index) * 499'952'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 363.15534935373313L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       363.15534935373313L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 534U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.41368060969649484L, 1.0e-12L);
    }
    if (index + 1U == 1068U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.74027501076090174L, 1.0e-12L);
    }
    if (index + 1U == 1602U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.94769120957315822L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus17732Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 3547U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'914'000ULL);
  EXPECT_EQ(energies.front() - 249'957'000ULL, 5'042'000ULL);
  EXPECT_EQ(energies.back() + 249'957'000ULL, 1'773'200'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 3547U);
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
                (static_cast<std::uint64_t>(index) * 499'914'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 599.20809137972753L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       599.20809137972753L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 886U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.41195682724001953L, 1.0e-12L);
    }
    if (index + 1U == 1773U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.75044592770586671L, 1.0e-12L);
    }
    if (index + 1U == 2660U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.95191558872391102L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesBetaMinus18538Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 3708U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'946'000ULL);
  EXPECT_EQ(energies.front() - 249'973'000ULL, 232'000ULL);
  EXPECT_EQ(energies.back() + 249'973'000ULL, 1'853'800'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 3708U);
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
                (static_cast<std::uint64_t>(index) * 499'946'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 671.91450820983152L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       671.91450820983152L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 927U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.34855619895045181L, 1.0e-12L);
    }
    if (index + 1U == 1854U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.7230390433856938L, 1.0e-12L);
    }
    if (index + 1U == 2781U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.95322218055069818L, 1.0e-12L);
    }
  }
}

TEST(GGEMSHo166Test, PreservesNuclearGamma) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 16U> expected_energies{
    80'577'600'000ULL,    184'412'400'000ULL,   520'914'000'000ULL,
    674'125'000'000ULL,   705'326'000'000ULL,   785'903'000'000ULL,
    1'263'406'000'000ULL, 1'379'447'000'000ULL, 1'447'817'000'000ULL,
    1'528'394'000'000ULL, 1'581'849'000'000ULL, 1'662'426'000'000ULL,
    1'732'610'000'000ULL, 1'749'838'000'000ULL, 1'813'190'000'000ULL,
    1'830'414'000'000ULL,
  };
  constexpr std::array<double, 16U> expected_weights{
    0.06605, 1.46e-05,  3.4e-06, 0.000193, 0.000134, 0.0001187,
    1.5e-05, 0.00905,   1.3e-05, 9e-07,    0.001789, 0.001164,
    5e-07,   0.0002584, 6e-07,   8.02e-05,
  };
  ASSERT_EQ(energies.size(), 16U);
  EXPECT_EQ(distribution.GetTableCount(), 16U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0788853L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 298.32639801509282L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       298.32639801509282L, 6.5196485648155206e-06L);
}

TEST(GGEMSHo166Test, PreservesAtomicXray) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    7'785'850'000ULL,  48'221'500'000ULL, 49'128'200'000ULL,
    55'739'000'000ULL, 57'326'300'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.0788, 0.0297, 0.0527, 0.0171, 0.00445,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.18275L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 31.972632585499316L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       31.972632585499316L, 5.8672674302011729e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion805776Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    23'092'100'000ULL, 70'826'300'000ULL, 71'313'300'000ULL,
    72'219'700'000ULL, 78'800'300'000ULL, 80'365'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.1104, 0.01038, 0.1222, 0.1262, 0.063, 0.01592,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.44810L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 61.051073273822809L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       61.051073273822809L, 8.1009317025542258e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1844124Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    126'926'900'000ULL, 174'661'100'000ULL, 175'148'100'000ULL,
    176'054'500'000ULL, 182'635'100'000ULL, 184'199'800'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.99e-06, 3.15e-07, 6.09e-07, 4.81e-07, 3.35e-07, 8.57e-08,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000048157L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 145.94882701580249L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       145.94882701580249L, 8.1009317025542258e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion520914Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    463'429'000'000ULL, 511'164'000'000ULL, 511'651'000'000ULL,
    512'557'000'000ULL, 519'138'000'000ULL, 520'702'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4.03e-08, 5e-09, 1.86e-09, 9.6e-10, 1.78e-09, 4.7e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.037E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 473.38752948183441L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       473.38752948183441L, 8.1009456723928442e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion674125Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    616'640'000'000ULL, 664'375'000'000ULL, 664'862'000'000ULL,
    665'768'000'000ULL, 672'349'000'000ULL, 673'913'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.25e-06, 1.58e-07, 3.96e-08, 1.89e-08, 4.88e-08, 1.29e-08,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000015282L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 625.69487586703315L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       625.69487586703315L, 8.1009456723928442e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion705326Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    647'842'000'000ULL, 695'576'000'000ULL, 696'063'000'000ULL,
    696'969'000'000ULL, 703'550'000'000ULL, 705'114'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    8.2e-07, 1.04e-07, 2.3e-08, 1.06e-08, 3.1e-08, 8.2e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 9.968E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 656.66096428571427L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       656.66096428571427L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion785903Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    728'420'000'000ULL, 776'154'000'000ULL, 776'641'000'000ULL,
    777'547'000'000ULL, 784'128'000'000ULL, 785'692'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.5e-07, 6.98e-08, 1.392e-08, 6.35e-09, 2.018e-08, 5.33e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 6.6558E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 737.05077792301449L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       737.05077792301449L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1263406Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'205'926'000'000ULL, 1'253'660'000'000ULL, 1'254'147'000'000ULL,
    1'255'053'000'000ULL, 1'261'634'000'000ULL, 1'263'198'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.7e-08, 3.4e-09, 3.4e-10, 1.47e-10, 8.6e-10, 2.28e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 3.1975E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1213.6470103205629L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1213.6470103205629L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1379447Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'321'968'000'000ULL, 1'369'702'000'000ULL, 1'370'189'000'000ULL,
    1'371'095'000'000ULL, 1'377'676'000'000ULL, 1'379'240'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.356e-05, 1.729e-06, 1.568e-07, 6.7e-08, 4.31e-07, 1.147e-07,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000160585L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1329.6875188965346L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1329.6875188965346L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1447817Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'390'338'000'000ULL, 1'438'072'000'000ULL, 1'438'559'000'000ULL,
    1'439'465'000'000ULL, 1'446'046'000'000ULL, 1'447'610'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.6e-08, 3.4e-09, 1.8e-10, 4.6e-11, 7.9e-10, 2.1e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 3.0626E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1397.8241771697251L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1397.8241771697251L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1528394Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'470'916'000'000ULL, 1'518'650'000'000ULL, 1'519'137'000'000ULL,
    1'520'043'000'000ULL, 1'526'624'000'000ULL, 1'528'188'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.11e-09, 1.42e-10, 1.13e-11, 4.8e-12, 3.48e-11, 9.3e-12,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.3122E-9L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1478.5598065081542L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1478.5598065081542L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1581849Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'524'372'000'000ULL, 1'572'106'000'000ULL, 1'572'593'000'000ULL,
    1'573'499'000'000ULL, 1'580'080'000'000ULL, 1'581'644'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    9.37e-07, 1.163e-07, 3.76e-09, 4.15e-09, 2.72e-08, 7.23e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00000109564L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1531.5513530447956L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1531.5513530447956L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1662426Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'604'950'000'000ULL, 1'652'684'000'000ULL, 1'653'171'000'000ULL,
    1'654'077'000'000ULL, 1'660'658'000'000ULL, 1'662'222'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5.59e-07, 6.93e-08, 2.142e-09, 2.421e-09, 1.613e-08, 4.3e-09,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 6.53293E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1612.1061025282684L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1612.1061025282684L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion173261Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'675'130'000'000ULL, 1'722'870'000'000ULL, 1'723'360'000'000ULL,
    1'724'260'000'000ULL, 1'730'840'000'000ULL, 1'732'410'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6e-10, 7.8e-11, 4.2e-12, 1.4e-12, 1.8e-11, 4.9e-12,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 7.065E-10L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1682.6013658881811L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1682.6013658881811L, 8.1019235610961911e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1749838Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'692'362'000'000ULL, 1'740'096'000'000ULL, 1'740'583'000'000ULL,
    1'741'489'000'000ULL, 1'748'070'000'000ULL, 1'749'634'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.194e-07, 1.491e-08, 4.7e-10, 4.86e-10, 3.47e-09, 9.23e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.39659E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1699.5539629096586L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1699.5539629096586L, 8.1008059740066525e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion181319Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'755'710'000'000ULL, 1'803'450'000'000ULL, 1'803'940'000'000ULL,
    1'804'840'000'000ULL, 1'811'420'000'000ULL, 1'812'990'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    6.5e-10, 8.4e-11, 4.4e-12, 1.49e-12, 2e-11, 5.3e-12,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 7.6519E-10L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1763.1765896051961L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1763.1765896051961L, 8.1019235610961911e-08L);
}

TEST(GGEMSHo166Test, PreservesConversion1830414Kev) {
  auto const definition = BuildHo166Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'772'940'000'000ULL, 1'820'674'000'000ULL, 1'821'161'000'000ULL,
    1'822'067'000'000ULL, 1'828'648'000'000ULL, 1'830'212'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.28e-08, 4.07e-09, 1.169e-10, 1.368e-10, 9.46e-10, 2.52e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 3.83217E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1780.0839236907548L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1780.0839236907548L, 8.1008059740066525e-08L);
}

} // namespace
