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
 * \brief Tests the independently selected Cu-64 source contracts.
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
using ggems::core::radioactivity::builtins::BuildCu64Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSCu64Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildCu64Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Cu-64");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 45721.4400L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 5U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_579_4_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.3848L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_plus_653_03_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.1751L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.004748L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // atomic_xray
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 0.16943L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1345_77_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 5.79655E-7L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 0.734078579655L,
                       1.0e-14L);
}

TEST(GGEMSCu64Test, PreservesBetaMinus5794Kev) {
  auto const definition = BuildCu64Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1159U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'912'000ULL);
  EXPECT_EQ(energies.front() - 249'956'000ULL, 1'992'000ULL);
  EXPECT_EQ(energies.back() + 249'956'000ULL, 579'400'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1159U);
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
                (static_cast<std::uint64_t>(index) * 499'912'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 189.67272830799277L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       189.67272830799277L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 289U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.41751408974448412L, 1.0e-12L);
    }
    if (index + 1U == 579U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.77201325560701206L, 1.0e-12L);
    }
    if (index + 1U == 869U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96374288628108418L, 1.0e-12L);
    }
  }
}

TEST(GGEMSCu64Test, PreservesBetaPlus65303Kev) {
  auto const definition = BuildCu64Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1307U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'640'000ULL);
  EXPECT_EQ(energies.front() - 249'820'000ULL, 520'000ULL);
  EXPECT_EQ(energies.back() + 249'820'000ULL, 653'030'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1307U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 277.82746429516288L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       277.82746429516288L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 326U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.21425815201750179L, 1.0e-12L);
    }
    if (index + 1U == 653U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.64564015789147611L, 1.0e-12L);
    }
    if (index + 1U == 980U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.93874167117044061L, 1.0e-12L);
    }
  }
}

TEST(GGEMSCu64Test, PreservesNuclearGamma) {
  auto const definition = BuildCu64Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            1'345'770'000'000ULL);
}

TEST(GGEMSCu64Test, PreservesAtomicXray) {
  auto const definition = BuildCu64Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    876'400'000ULL,
    7'460'930'000ULL,
    7'478'190'000ULL,
    8'296'700'000ULL,
  };
  constexpr std::array<double, 4U> expected_weights{
    0.00493,
    0.049,
    0.0956,
    0.0199,
  };
  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.16943L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 7.3772384819689547L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       7.3772384819689547L, 7.9106929004192352e-09L);
}

TEST(GGEMSCu64Test, PreservesConversion134577Kev) {
  auto const definition = BuildCu64Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    1'337'420'000'000ULL,
    1'344'740'000'000ULL,
    1'344'880'000'000ULL,
    1'344'900'000'000ULL,
  };
  constexpr std::array<double, 4U> expected_weights{
    5.28e-07,
    5.08e-08,
    3.85e-10,
    4.7e-10,
  };
  ASSERT_EQ(energies.size(), 4U);
  EXPECT_EQ(distribution.GetTableCount(), 4U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.79655E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1338.0725324546497L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1338.0725324546497L, 7.9662928581237804e-09L);
}

TEST(GGEMSCu64Test, ExcludesUnsupportedDiscreteEmissions) {
  auto const definition = BuildCu64Radionuclide();
  for (auto const &emission : definition.GetEmissions()) {
    auto const &distribution = emission.GetEnergyDistribution();
    // annihilation_photons
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  511'000'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 511'000'000'000ULL);
        }
      }
    }
  }
}

} // namespace
