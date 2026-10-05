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
 * \brief Tests the independently selected Re-188 source contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace {

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::builtins::BuildRe188Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSRe188Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildRe188Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Re-188");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 61218.000L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 41U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_97_96_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.98E-5L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_100_22_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 5.9E-5L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_155_44_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 2.1E-5L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_163_32_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 5.1E-4L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_171_81_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 7.9E-4L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_179_37_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.00102L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_183_5_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 2.14E-6L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_277_54_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 2.99E-5L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_295_46_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 2.36E-4L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_312_8_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 3.8E-4L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_355_05_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 0.00181L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_390_72_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 1.28E-5L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_416_08_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 2.3E-5L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_434_9_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 5.5E-6L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_642_31_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 1.8E-4L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_657_9_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[15U].GetYieldPerDecay(), 0.0044L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_662_85_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[16U].GetYieldPerDecay(), 4.2E-4L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_676_88_keV
  EXPECT_EQ(emissions[17U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[17U].GetYieldPerDecay(), 9.2E-6L);
  EXPECT_EQ(emissions[17U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_706_6_keV
  EXPECT_EQ(emissions[18U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[18U].GetYieldPerDecay(), 2.4E-5L);
  EXPECT_EQ(emissions[18U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_815_55_keV
  EXPECT_EQ(emissions[19U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[19U].GetYieldPerDecay(), 2.41E-4L);
  EXPECT_EQ(emissions[19U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1034_02_keV
  EXPECT_EQ(emissions[20U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[20U].GetYieldPerDecay(), 0.0063L);
  EXPECT_EQ(emissions[20U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1487_38_keV
  EXPECT_EQ(emissions[21U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[21U].GetYieldPerDecay(), 0.0165L);
  EXPECT_EQ(emissions[21U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1965_36_keV
  EXPECT_EQ(emissions[22U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[22U].GetYieldPerDecay(), 0.256L);
  EXPECT_EQ(emissions[22U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_2120_4_keV
  EXPECT_EQ(emissions[23U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[23U].GetYieldPerDecay(), 0.711L);
  EXPECT_EQ(emissions[23U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[24U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[24U].GetYieldPerDecay(), 0.1943678L);
  EXPECT_EQ(emissions[24U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[25U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[25U].GetYieldPerDecay(), 0.0772L);
  EXPECT_EQ(emissions[25U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_155_keV
  EXPECT_EQ(emissions[26U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[26U].GetYieldPerDecay(), 0.0000484L);
  EXPECT_EQ(emissions[26U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_155_041_keV
  EXPECT_EQ(emissions[27U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[27U].GetYieldPerDecay(), 0.1246L);
  EXPECT_EQ(emissions[27U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_312_001_keV
  EXPECT_EQ(emissions[28U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[28U].GetYieldPerDecay(), 0.0000326L);
  EXPECT_EQ(emissions[28U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_322_93_keV
  EXPECT_EQ(emissions[29U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[29U].GetYieldPerDecay(), 0.00001207L);
  EXPECT_EQ(emissions[29U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_453_34_keV
  EXPECT_EQ(emissions[30U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[30U].GetYieldPerDecay(), 0.00002150L);
  EXPECT_EQ(emissions[30U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_477_992_keV
  EXPECT_EQ(emissions[31U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[31U].GetYieldPerDecay(), 0.0002617L);
  EXPECT_EQ(emissions[31U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_514_88_keV
  EXPECT_EQ(emissions[32U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[32U].GetYieldPerDecay(), 0.000001257L);
  EXPECT_EQ(emissions[32U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_632_981_keV
  EXPECT_EQ(emissions[33U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[33U].GetYieldPerDecay(), 0.0001691L);
  EXPECT_EQ(emissions[33U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_634_98_keV
  EXPECT_EQ(emissions[34U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[34U].GetYieldPerDecay(), 0.00002014L);
  EXPECT_EQ(emissions[34U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_672_535_keV
  EXPECT_EQ(emissions[35U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[35U].GetYieldPerDecay(), 0.000004664L);
  EXPECT_EQ(emissions[35U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_825_2_keV
  EXPECT_EQ(emissions[36U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[36U].GetYieldPerDecay(), 0.000002828L);
  EXPECT_EQ(emissions[36U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_829_47_keV
  EXPECT_EQ(emissions[37U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[37U].GetYieldPerDecay(), 0.00001221L);
  EXPECT_EQ(emissions[37U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_931_345_keV
  EXPECT_EQ(emissions[38U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[38U].GetYieldPerDecay(), 0.00003190L);
  EXPECT_EQ(emissions[38U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1132_31_keV
  EXPECT_EQ(emissions[39U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[39U].GetYieldPerDecay(), 0.000003209L);
  EXPECT_EQ(emissions[39U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1209_79_keV
  EXPECT_EQ(emissions[40U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[40U].GetYieldPerDecay(), 2.106E-7L);
  EXPECT_EQ(emissions[40U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.3967829286L, 1.0e-14L);
}

TEST(GGEMSRe188Test, PreservesBetaMinus9796Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 196U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'794'000ULL);
  EXPECT_EQ(energies.front() - 249'897'000ULL, 376'000ULL);
  EXPECT_EQ(energies.back() + 249'897'000ULL, 97'960'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 196U);
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
                (static_cast<std::uint64_t>(index) * 499'794'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 25.289954828714503L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 25.289954828714503L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 49U) {
      EXPECT_NEAR(cumulative, 0.560961655872659L, 1.0e-12L);
    }
    if (index + 1U == 98U) {
      EXPECT_NEAR(cumulative, 0.86422278786429541L, 1.0e-12L);
    }
    if (index + 1U == 147U) {
      EXPECT_NEAR(cumulative, 0.98239413553932831L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus10022Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 201U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'606'000ULL);
  EXPECT_EQ(energies.front() - 249'303'000ULL, 194'000ULL);
  EXPECT_EQ(energies.back() + 249'303'000ULL, 100'220'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 201U);
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
                (static_cast<std::uint64_t>(index) * 498'606'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 25.898028648136751L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 25.898028648136751L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 50U) {
      EXPECT_NEAR(cumulative, 0.55835629814759702L, 1.0e-12L);
    }
    if (index + 1U == 100U) {
      EXPECT_NEAR(cumulative, 0.86192276209560281L, 1.0e-12L);
    }
    if (index + 1U == 150U) {
      EXPECT_NEAR(cumulative, 0.98153556514317264L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus15544Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 311U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'806'000ULL);
  EXPECT_EQ(energies.front() - 249'903'000ULL, 334'000ULL);
  EXPECT_EQ(energies.back() + 249'903'000ULL, 155'440'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 311U);
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
                (static_cast<std::uint64_t>(index) * 499'806'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 41.081015614823542L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 41.081015614823542L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 77U) {
      EXPECT_NEAR(cumulative, 0.54414554537890669L, 1.0e-12L);
    }
    if (index + 1U == 155U) {
      EXPECT_NEAR(cumulative, 0.85499350535557561L, 1.0e-12L);
    }
    if (index + 1U == 233U) {
      EXPECT_NEAR(cumulative, 0.98064559964424802L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus16332Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 327U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'448'000ULL);
  EXPECT_EQ(energies.front() - 249'724'000ULL, 504'000ULL);
  EXPECT_EQ(energies.back() + 249'724'000ULL, 163'320'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 327U);
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
                (static_cast<std::uint64_t>(index) * 499'448'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 43.298560429356002L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 43.298560429356002L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 81U) {
      EXPECT_NEAR(cumulative, 0.54262742580247258L, 1.0e-12L);
    }
    if (index + 1U == 163U) {
      EXPECT_NEAR(cumulative, 0.85398819714417795L, 1.0e-12L);
    }
    if (index + 1U == 245U) {
      EXPECT_NEAR(cumulative, 0.98044315028233398L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus17181Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 344U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'446'000ULL);
  EXPECT_EQ(energies.front() - 249'723'000ULL, 576'000ULL);
  EXPECT_EQ(energies.back() + 249'723'000ULL, 171'810'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 344U);
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
                (static_cast<std::uint64_t>(index) * 499'446'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 45.70159570096552L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 45.70159570096552L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 86U) {
      EXPECT_NEAR(cumulative, 0.54464324592705571L, 1.0e-12L);
    }
    if (index + 1U == 172U) {
      EXPECT_NEAR(cumulative, 0.85411354757411662L, 1.0e-12L);
    }
    if (index + 1U == 258U) {
      EXPECT_NEAR(cumulative, 0.98039294142206879L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus17937Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 359U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'636'000ULL);
  EXPECT_EQ(energies.front() - 249'818'000ULL, 676'000ULL);
  EXPECT_EQ(energies.back() + 249'818'000ULL, 179'370'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 359U);
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
                (static_cast<std::uint64_t>(index) * 499'636'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 47.853387199114898L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 47.853387199114898L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 89U) {
      EXPECT_NEAR(cumulative, 0.539500465179925L, 1.0e-12L);
    }
    if (index + 1U == 179U) {
      EXPECT_NEAR(cumulative, 0.85193611897031107L, 1.0e-12L);
    }
    if (index + 1U == 269U) {
      EXPECT_NEAR(cumulative, 0.98003138724675731L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus1835Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 368U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'640'000ULL);
  EXPECT_EQ(energies.front() - 249'320'000ULL, 480'000ULL);
  EXPECT_EQ(energies.back() + 249'320'000ULL, 183'500'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 368U);
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
                (static_cast<std::uint64_t>(index) * 498'640'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 49.03262988582842L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 49.03262988582842L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 92U) {
      EXPECT_NEAR(cumulative, 0.54213372369465607L, 1.0e-12L);
    }
    if (index + 1U == 184U) {
      EXPECT_NEAR(cumulative, 0.85255270065016675L, 1.0e-12L);
    }
    if (index + 1U == 276U) {
      EXPECT_NEAR(cumulative, 0.98008485641156318L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus27754Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 556U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'172'000ULL);
  EXPECT_EQ(energies.front() - 249'586'000ULL, 368'000ULL);
  EXPECT_EQ(energies.back() + 249'586'000ULL, 277'540'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 556U);
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
                (static_cast<std::uint64_t>(index) * 499'172'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 76.779689918785508L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 76.779689918785508L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 139U) {
      EXPECT_NEAR(cumulative, 0.52257655427290661L, 1.0e-12L);
    }
    if (index + 1U == 278U) {
      EXPECT_NEAR(cumulative, 0.84032924932618092L, 1.0e-12L);
    }
    if (index + 1U == 417U) {
      EXPECT_NEAR(cumulative, 0.97767120670175467L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus29546Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 591U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'932'000ULL);
  EXPECT_EQ(energies.front() - 249'966'000ULL, 188'000ULL);
  EXPECT_EQ(energies.back() + 249'966'000ULL, 295'460'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 591U);
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
                (static_cast<std::uint64_t>(index) * 499'932'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 82.249705037454916L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 82.249705037454916L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 147U) {
      EXPECT_NEAR(cumulative, 0.51686609916900883L, 1.0e-12L);
    }
    if (index + 1U == 295U) {
      EXPECT_NEAR(cumulative, 0.83731732780370161L, 1.0e-12L);
    }
    if (index + 1U == 443U) {
      EXPECT_NEAR(cumulative, 0.97711288631864646L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus3128Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 626U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'680'000ULL);
  EXPECT_EQ(energies.front() - 249'840'000ULL, 320'000ULL);
  EXPECT_EQ(energies.back() + 249'840'000ULL, 312'800'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 626U);
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
                (static_cast<std::uint64_t>(index) * 499'680'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 87.595280546600335L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 87.595280546600335L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 156U) {
      EXPECT_NEAR(cumulative, 0.51422996081815886L, 1.0e-12L);
    }
    if (index + 1U == 313U) {
      EXPECT_NEAR(cumulative, 0.83591237004917018L, 1.0e-12L);
    }
    if (index + 1U == 469U) {
      EXPECT_NEAR(cumulative, 0.97658125176047272L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus35505Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 711U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'366'000ULL);
  EXPECT_EQ(energies.front() - 249'683'000ULL, 774'000ULL);
  EXPECT_EQ(energies.back() + 249'683'000ULL, 355'050'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 711U);
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
                (static_cast<std::uint64_t>(index) * 499'366'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 100.83211794829985L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 100.83211794829985L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 177U) {
      EXPECT_NEAR(cumulative, 0.50563587461534709L, 1.0e-12L);
    }
    if (index + 1U == 355U) {
      EXPECT_NEAR(cumulative, 0.83009689176011281L, 1.0e-12L);
    }
    if (index + 1U == 533U) {
      EXPECT_NEAR(cumulative, 0.97567068592330441L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus39072Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 782U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'640'000ULL);
  EXPECT_EQ(energies.front() - 249'820'000ULL, 1'520'000ULL);
  EXPECT_EQ(energies.back() + 249'820'000ULL, 390'720'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 782U);
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
                (static_cast<std::uint64_t>(index) * 499'640'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 112.23194973393088L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 112.23194973393088L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 195U) {
      EXPECT_NEAR(cumulative, 0.49963438943640609L, 1.0e-12L);
    }
    if (index + 1U == 391U) {
      EXPECT_NEAR(cumulative, 0.82648821642824599L, 1.0e-12L);
    }
    if (index + 1U == 586U) {
      EXPECT_NEAR(cumulative, 0.97473637067437224L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus41608Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 833U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'494'000ULL);
  EXPECT_EQ(energies.front() - 249'747'000ULL, 1'498'000ULL);
  EXPECT_EQ(energies.back() + 249'747'000ULL, 416'080'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 833U);
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
                (static_cast<std::uint64_t>(index) * 499'494'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 120.45662305578871L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 120.45662305578871L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 208U) {
      EXPECT_NEAR(cumulative, 0.49554908604380754L, 1.0e-12L);
    }
    if (index + 1U == 416U) {
      EXPECT_NEAR(cumulative, 0.82295403129851696L, 1.0e-12L);
    }
    if (index + 1U == 624U) {
      EXPECT_NEAR(cumulative, 0.97406296874276466L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus4349Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 870U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'884'000ULL);
  EXPECT_EQ(energies.front() - 249'942'000ULL, 920'000ULL);
  EXPECT_EQ(energies.back() + 249'942'000ULL, 434'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 870U);
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
                (static_cast<std::uint64_t>(index) * 499'884'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 126.62254168307514L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 126.62254168307514L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 217U) {
      EXPECT_NEAR(cumulative, 0.49169849586165987L, 1.0e-12L);
    }
    if (index + 1U == 435U) {
      EXPECT_NEAR(cumulative, 0.82135369595000451L, 1.0e-12L);
    }
    if (index + 1U == 652U) {
      EXPECT_NEAR(cumulative, 0.97372248591398736L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus64231Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1285U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'852'000ULL);
  EXPECT_EQ(energies.front() - 249'926'000ULL, 180'000ULL);
  EXPECT_EQ(energies.back() + 249'926'000ULL, 642'310'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1285U);
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
                (static_cast<std::uint64_t>(index) * 499'852'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 197.74116621833954L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 197.74116621833954L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 321U) {
      EXPECT_NEAR(cumulative, 0.45808683781100423L, 1.0e-12L);
    }
    if (index + 1U == 642U) {
      EXPECT_NEAR(cumulative, 0.79882662482473965L, 1.0e-12L);
    }
    if (index + 1U == 963U) {
      EXPECT_NEAR(cumulative, 0.9692313977142657L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus6579Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1316U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'924'000ULL);
  EXPECT_EQ(energies.front() - 249'962'000ULL, 16'000ULL);
  EXPECT_EQ(energies.back() + 249'962'000ULL, 657'900'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1316U);
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
                (static_cast<std::uint64_t>(index) * 499'924'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 203.29822012368493L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 203.29822012368493L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 329U) {
      EXPECT_NEAR(cumulative, 0.45605518850393023L, 1.0e-12L);
    }
    if (index + 1U == 658U) {
      EXPECT_NEAR(cumulative, 0.79769444886205254L, 1.0e-12L);
    }
    if (index + 1U == 987U) {
      EXPECT_NEAR(cumulative, 0.96912106508147344L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus66285Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1326U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'886'000ULL);
  EXPECT_EQ(energies.front() - 249'943'000ULL, 1'164'000ULL);
  EXPECT_EQ(energies.back() + 249'943'000ULL, 662'850'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1326U);
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
                (static_cast<std::uint64_t>(index) * 499'886'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 205.06824236570176L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 205.06824236570176L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 331U) {
      EXPECT_NEAR(cumulative, 0.45469244208966253L, 1.0e-12L);
    }
    if (index + 1U == 663U) {
      EXPECT_NEAR(cumulative, 0.79721069663503163L, 1.0e-12L);
    }
    if (index + 1U == 994U) {
      EXPECT_NEAR(cumulative, 0.96889118307292721L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus67688Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[17U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1354U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'910'000ULL);
  EXPECT_EQ(energies.front() - 249'955'000ULL, 1'860'000ULL);
  EXPECT_EQ(energies.back() + 249'955'000ULL, 676'880'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1354U);
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
                (static_cast<std::uint64_t>(index) * 499'910'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 210.09952671708714L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 210.09952671708714L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 338U) {
      EXPECT_NEAR(cumulative, 0.45262169709368294L, 1.0e-12L);
    }
    if (index + 1U == 677U) {
      EXPECT_NEAR(cumulative, 0.79584834909369651L, 1.0e-12L);
    }
    if (index + 1U == 1015U) {
      EXPECT_NEAR(cumulative, 0.96861595649024201L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus7066Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[18U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1414U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'716'000ULL);
  EXPECT_EQ(energies.front() - 249'858'000ULL, 1'576'000ULL);
  EXPECT_EQ(energies.back() + 249'858'000ULL, 706'600'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1414U);
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
                (static_cast<std::uint64_t>(index) * 499'716'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 224.91710620540397L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 224.91710620540397L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 353U) {
      EXPECT_NEAR(cumulative, 0.45329804280934355L, 1.0e-12L);
    }
    if (index + 1U == 707U) {
      EXPECT_NEAR(cumulative, 0.7712029793631473L, 1.0e-12L);
    }
    if (index + 1U == 1060U) {
      EXPECT_NEAR(cumulative, 0.95646850360887659L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus81555Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[19U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1632U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'724'000ULL);
  EXPECT_EQ(energies.front() - 249'862'000ULL, 432'000ULL);
  EXPECT_EQ(energies.back() + 249'862'000ULL, 815'550'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1632U);
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
                (static_cast<std::uint64_t>(index) * 499'724'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 260.90130121573321L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 260.90130121573321L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 408U) {
      EXPECT_NEAR(cumulative, 0.43383970221481011L, 1.0e-12L);
    }
    if (index + 1U == 816U) {
      EXPECT_NEAR(cumulative, 0.78309467388802456L, 1.0e-12L);
    }
    if (index + 1U == 1224U) {
      EXPECT_NEAR(cumulative, 0.96614325348914254L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus103402Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[20U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 2069U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'768'000ULL);
  EXPECT_EQ(energies.front() - 249'884'000ULL, 8'000ULL);
  EXPECT_EQ(energies.back() + 249'884'000ULL, 1'034'020'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 2069U);
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
                (static_cast<std::uint64_t>(index) * 499'768'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 344.32626354473786L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 344.32626354473786L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 517U) {
      EXPECT_NEAR(cumulative, 0.40707907474030508L, 1.0e-12L);
    }
    if (index + 1U == 1034U) {
      EXPECT_NEAR(cumulative, 0.76511632022433484L, 1.0e-12L);
    }
    if (index + 1U == 1551U) {
      EXPECT_NEAR(cumulative, 0.96235000168923357L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus148738Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[21U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 2975U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'958'000ULL);
  EXPECT_EQ(energies.front() - 249'979'000ULL, 4'950'000ULL);
  EXPECT_EQ(energies.back() + 249'979'000ULL, 1'487'380'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 2975U);
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
                (static_cast<std::uint64_t>(index) * 499'958'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 526.97048068763047L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 526.97048068763047L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 743U) {
      EXPECT_NEAR(cumulative, 0.3639154520161742L, 1.0e-12L);
    }
    if (index + 1U == 1487U) {
      EXPECT_NEAR(cumulative, 0.73600524999905781L, 1.0e-12L);
    }
    if (index + 1U == 2231U) {
      EXPECT_NEAR(cumulative, 0.95639327560554277L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus196536Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[22U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 3931U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'964'000ULL);
  EXPECT_EQ(energies.front() - 249'982'000ULL, 1'516'000ULL);
  EXPECT_EQ(energies.back() + 249'982'000ULL, 1'965'360'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 3931U);
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
                (static_cast<std::uint64_t>(index) * 499'964'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 731.72328352659156L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 731.72328352659156L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 982U) {
      EXPECT_NEAR(cumulative, 0.3353622321261065L, 1.0e-12L);
    }
    if (index + 1U == 1965U) {
      EXPECT_NEAR(cumulative, 0.70639283357618321L, 1.0e-12L);
    }
    if (index + 1U == 2948U) {
      EXPECT_NEAR(cumulative, 0.94650209091263549L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesBetaMinus21204Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[23U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 4241U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'976'000ULL);
  EXPECT_EQ(energies.front() - 249'988'000ULL, 1'784'000ULL);
  EXPECT_EQ(energies.back() + 249'988'000ULL, 2'120'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 4241U);
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
                (static_cast<std::uint64_t>(index) * 499'976'000ULL));
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
  EXPECT_NEAR(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 798.7072182203151L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 798.7072182203151L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 1060U) {
      EXPECT_NEAR(cumulative, 0.32895505873571784L, 1.0e-12L);
    }
    if (index + 1U == 2120U) {
      EXPECT_NEAR(cumulative, 0.69904582819402183L, 1.0e-12L);
    }
    if (index + 1U == 3180U) {
      EXPECT_NEAR(cumulative, 0.94376987667116274L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe188Test, PreservesNuclearGamma) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[24U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 52U> expected_energies{
    155'000'000'000ULL,   155'041'000'000ULL,   312'001'000'000ULL,
    322'930'000'000ULL,   453'340'000'000ULL,   477'992'000'000ULL,
    486'087'000'000ULL,   514'880'000'000ULL,   557'710'000'000ULL,
    623'800'000'000ULL,   632'981'000'000ULL,   634'980'000'000ULL,
    672'535'000'000ULL,   810'490'000'000ULL,   825'200'000'000ULL,
    829'470'000'000ULL,   845'070'000'000ULL,   931'345'000'000ULL,
    979'250'000'000ULL,   984'100'000'000ULL,   1'017'700'000'000ULL,
    1'071'400'000'000ULL, 1'096'800'000'000ULL, 1'132'310'000'000ULL,
    1'149'700'000'000ULL, 1'150'500'000'000ULL, 1'174'570'000'000ULL,
    1'191'840'000'000ULL, 1'209'790'000'000ULL, 1'302'400'000'000ULL,
    1'304'860'000'000ULL, 1'308'030'000'000ULL, 1'322'910'000'000ULL,
    1'331'950'000'000ULL, 1'457'540'000'000ULL, 1'463'000'000'000ULL,
    1'530'500'000'000ULL, 1'549'260'000'000ULL, 1'574'570'000'000ULL,
    1'610'400'000'000ULL, 1'652'490'000'000ULL, 1'669'970'000'000ULL,
    1'785'950'000'000ULL, 1'802'040'000'000ULL, 1'807'600'000'000ULL,
    1'809'540'000'000ULL, 1'864'910'000'000ULL, 1'867'200'000'000ULL,
    1'936'900'000'000ULL, 1'940'910'000'000ULL, 1'956'960'000'000ULL,
    2'022'530'000'000ULL,
  };
  constexpr std::array<double, 52U> expected_weights{
    5.9e-05, 0.152,    0.00043,  0.000162, 0.00073,  0.0102,  0.00079,  5.4e-05,
    9.5e-06, 2.4e-05,  0.0128,   0.00148,  0.00112,  9.2e-06, 0.000176, 0.0041,
    6.5e-05, 0.0055,   1.04e-05, 3.4e-06,  0.000147, 6.7e-06, 6.4e-06,  0.00083,
    0.00015, 0.00015,  0.00018,  0.000134, 3e-05,    5.7e-05, 2.8e-05,  0.00065,
    0.00011, 1.74e-05, 0.000186, 8e-06,    5.5e-06,  1.6e-05, 6.3e-06,  0.00098,
    3.5e-05, 0.000104, 0.000195, 0.00036,  8.6e-06,  4e-06,   5e-05,    4.6e-06,
    2.1e-06, 1.85e-05, 0.00015,  1.52e-05,
  };
  ASSERT_EQ(energies.size(), 52U);
  EXPECT_EQ(distribution.GetTableCount(), 52U);
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
  EXPECT_NEAR(weight_sum, 0.1943678L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 280.63909142872433L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 280.63909142872433L,
              2.2611547021031378e-05L);
}

TEST(GGEMSRe188Test, PreservesAtomicXray) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[25U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    10'370'000'000ULL, 61'487'300'000ULL, 63'001'100'000ULL,
    71'449'000'000ULL, 73'584'300'000ULL,
  };
  constexpr std::array<double, 5U> expected_weights{
    0.03, 0.0136, 0.0235, 0.0079, 0.0022,
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
  EXPECT_NEAR(weight_sum, 0.0772L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 43.448001165803106L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 43.448001165803106L,
              7.4591130785644054e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion155Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[26U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    81'130'000'000ULL,
    142'930'000'000ULL,
    152'540'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    1.91e-05,
    2.21e-05,
    7.2e-06,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.0000484L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 119.97157024793388L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 119.97157024793388L,
              5.0879308789968491e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion155041Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[27U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    81'170'200'000ULL,
    142'966'400'000ULL,
    152'583'200'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    0.0491,
    0.057,
    0.0185,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.1246L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 120.04278346709471L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 120.04278346709471L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion312001Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[28U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    238'130'000'000ULL,
    299'926'000'000ULL,
    309'543'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    2.3e-05,
    9.3e-06,
    3e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.0000326L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 256.41609509202453L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 256.41609509202453L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion32293Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[29U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    249'059'000'000ULL,
    310'855'000'000ULL,
    320'472'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    8e-06,
    3.08e-06,
    9.9e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.00001207L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 270.68539188069593L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 270.68539188069593L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion45334Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[30U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    379'469'000'000ULL,
    441'265'000'000ULL,
    450'882'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    1.58e-05,
    4.36e-06,
    1.34e-06,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.00002150L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 396.45151069767439L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 396.45151069767439L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion477992Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[31U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    404'121'000'000ULL,
    465'917'000'000ULL,
    475'534'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    0.000197,
    5.18e-05,
    1.29e-05,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.0002617L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 419.87285517768436L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 419.87285517768436L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion51488Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[32U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    441'010'000'000ULL,
    502'810'000'000ULL,
    512'420'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    9.6e-07,
    2.27e-07,
    7e-08,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.000001257L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 456.14707239459028L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 456.14707239459028L,
              5.0879308789968491e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion632981Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[33U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    559'111'000'000ULL,
    620'907'000'000ULL,
    630'524'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    0.000132,
    2.85e-05,
    8.6e-06,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.0001691L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 573.15794145476048L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 573.15794145476048L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion63498Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[34U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    561'110'000'000ULL,
    622'910'000'000ULL,
    632'520'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    1.57e-05,
    3.34e-06,
    1.1e-06,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.00002014L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 575.25910625620656L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 575.25910625620656L,
              5.0879308789968491e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion672535Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[35U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    598'665'000'000ULL,
    660'461'000'000ULL,
    670'078'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    3.9e-06,
    5.9e-07,
    1.74e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.000004664L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 609.1464541166381L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 609.1464541166381L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion8252Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[36U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    751'300'000'000ULL,
    813'100'000'000ULL,
    822'700'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    2.3e-06,
    3.7e-07,
    1.58e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.000002828L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 763.37468175388972L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 763.37468175388972L,
              5.0872323870658874e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion82947Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[37U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    755'599'000'000ULL,
    817'395'000'000ULL,
    827'012'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    1.02e-05,
    1.56e-06,
    4.5e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.00001221L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 766.12624078624083L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 766.12624078624083L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion931345Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[38U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    857'476'000'000ULL,
    919'272'000'000ULL,
    928'889'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    2.59e-05,
    4.62e-06,
    1.38e-06,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.00003190L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 869.51510532915358L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 869.51510532915358L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion113231Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[39U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    1'058'439'000'000ULL,
    1'120'235'000'000ULL,
    1'129'852'000'000ULL,
  };
  constexpr std::array<double, 3U> expected_weights{
    2.67e-06,
    4.15e-07,
    1.24e-07,
  };
  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  EXPECT_NEAR(weight_sum, 0.000003209L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1069.1901847927704L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1069.1901847927704L,
              5.0881404265761379e-08L);
}

TEST(GGEMSRe188Test, PreservesConversion120979Kev) {
  auto const definition = BuildRe188Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[40U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 2U> expected_energies{
    1'135'919'000'000ULL,
    1'197'715'000'000ULL,
  };
  constexpr std::array<double, 2U> expected_weights{
    1.86e-07,
    2.46e-08,
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
  EXPECT_NEAR(weight_sum, 2.106E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 1143.1373361823362L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 1143.1373361823362L,
              2.9776004910469054e-08L);
}

} // namespace
