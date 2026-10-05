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
 * \brief Tests the independently selected Sr-89 source contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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
using ggems::core::radioactivity::builtins::BuildSr89Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSSr89Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildSr89Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Sr-89");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 4369248.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_586_1_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 9.64E-5L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1495_1_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.9999036L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.00086L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.0008600L, 1.0e-14L);
}

TEST(GGEMSSr89Test, PreservesBetaMinus5861Kev) {
  auto const definition = BuildSr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1173U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'658'000ULL);
  EXPECT_EQ(energies.front() - 249'829'000ULL, 1'166'000ULL);
  EXPECT_EQ(energies.back() + 249'829'000ULL, 586'100'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1173U);
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
                (static_cast<std::uint64_t>(index) * 499'658'000ULL));
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
  EXPECT_NEAR(moment / weight_sum / keV, 209.55650819108234L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 209.55650819108234L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 293U) {
      EXPECT_NEAR(cumulative, 0.37799056124293101L, 1.0e-12L);
    }
    if (index + 1U == 586U) {
      EXPECT_NEAR(cumulative, 0.71323314038420138L, 1.0e-12L);
    }
    if (index + 1U == 879U) {
      EXPECT_NEAR(cumulative, 0.94277103613723379L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSr89Test, PreservesBetaMinus14951Kev) {
  auto const definition = BuildSr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 2991U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'866'000ULL);
  EXPECT_EQ(energies.front() - 249'933'000ULL, 794'000ULL);
  EXPECT_EQ(energies.back() + 249'933'000ULL, 1'495'100'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 2991U);
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
                (static_cast<std::uint64_t>(index) * 499'866'000ULL));
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
  EXPECT_NEAR(moment / weight_sum / keV, 580.94092578468621L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 580.94092578468621L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 747U) {
      EXPECT_NEAR(cumulative, 0.32417399455203255L, 1.0e-12L);
    }
    if (index + 1U == 1495U) {
      EXPECT_NEAR(cumulative, 0.66951562482103855L, 1.0e-12L);
    }
    if (index + 1U == 2243U) {
      EXPECT_NEAR(cumulative, 0.92803457072680473L, 1.0e-12L);
    }
  }
}

TEST(GGEMSSr89Test, PreservesAtomicXray) {
  auto const definition = BuildSr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 14'958'500'000ULL);
}

TEST(GGEMSSr89Test, ExcludesUnsupportedDiscreteEmissions) {
  auto const definition = BuildSr89Radionuclide();
  for (auto const &emission : definition.GetEmissions()) {
    auto const &distribution = emission.GetEnergyDistribution();
    // delayed_Y89m_gamma
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  909'000'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 909'000'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_891_96
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  891'960'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 891'960'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_906_8
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  906'800'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 906'800'000'000ULL);
        }
      }
    }
  }
}

} // namespace
