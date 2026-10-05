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
 * \brief Tests the independently selected At-211 source contracts.
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
using ggems::core::radioactivity::builtins::BuildAt211Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSAt211Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildAt211Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "At-211");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 25977.600L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 9U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // alpha
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.417854L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.0025028L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.61860471L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_149_72_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 0.0000014736L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_222_69_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 3.8096E-7L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_669_77_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.00000197984L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_687_2_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 0.00013114L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_742_74_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 4.8889E-7L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_892_46_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 2.0341E-8L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.039096993631L, 1.0e-14L);
}

TEST(GGEMSAt211Test, PreservesAlpha) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    4'993'400'000'000ULL,
    5'140'300'000'000ULL,
    5'211'900'000'000ULL,
    5'869'000'000'000ULL,
  };
  constexpr std::array<double, 4U> expected_weights{
    4e-06,
    1.1e-05,
    3.9e-05,
    0.4178,
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
  EXPECT_NEAR(weight_sum, 0.417854L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 5868.911105314296L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 5868.911105314296L,
              8.1646604633331298e-07L);
}

TEST(GGEMSAt211Test, PreservesNuclearGamma) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    149'720'000'000ULL, 222'690'000'000ULL, 669'770'000'000ULL,
    687'200'000'000ULL, 742'740'000'000ULL, 892'460'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    5e-07, 4e-07, 3.8e-05, 0.00245, 1.25e-05, 1.4e-06,
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
  EXPECT_NEAR(weight_sum, 0.0025028L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 687.14595253316281L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 687.14595253316281L,
              1.0385957936048508e-06L);
}

TEST(GGEMSAt211Test, PreservesAtomicXray) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 10U> expected_energies{
    12'564'500'000ULL, 12'935'500'000ULL, 74'815'700'000ULL, 76'864'000'000ULL,
    77'108'800'000ULL, 79'293'000'000ULL, 87'347'000'000ULL, 89'808'700'000ULL,
    90'075'700'000ULL, 92'621'300'000ULL,
  };
  constexpr std::array<double, 10U> expected_weights{
    1.36e-06, 0.186,   9.8e-07, 0.1266,  1.64e-06,
    0.2108,   5.6e-07, 0.0726,  1.7e-07, 0.0226,
  };
  ASSERT_EQ(energies.size(), 10U);
  EXPECT_EQ(distribution.GetTableCount(), 10U);
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
  EXPECT_NEAR(weight_sum, 0.61860471L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 60.564659930777118L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 60.564659930777118L,
              1.8739676272869109e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion14972Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    59'190'000'000ULL,  133'330'000'000ULL, 134'010'000'000ULL,
    136'300'000'000ULL, 146'490'000'000ULL, 149'200'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.15e-06, 1.8e-07, 4.5e-08, 2e-08, 6e-08, 1.86e-08,
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
  EXPECT_NEAR(weight_sum, 0.0000014736L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 76.268234256243218L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 76.268234256243218L,
              1.2674251741170881e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion22269Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    132'160'000'000ULL, 206'300'000'000ULL, 206'980'000'000ULL,
    209'270'000'000ULL, 219'460'000'000ULL, 222'170'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.04e-07, 4.72e-08, 8.8e-09, 2.6e-09, 1.404e-08, 4.32e-09,
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
  EXPECT_NEAR(weight_sum, 3.8096E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 147.83840508189837L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 147.83840508189837L,
              1.2674251741170881e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion66977Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    579'240'000'000ULL, 653'380'000'000ULL, 654'060'000'000ULL,
    656'350'000'000ULL, 666'540'000'000ULL, 669'250'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.62e-06, 2.47e-07, 2.57e-08, 2.24e-09, 6.5e-08, 1.99e-08,
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
  EXPECT_NEAR(weight_sum, 0.00000197984L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 593.31885455390329L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 593.31885455390329L,
              1.2674251741170881e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion6872Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    594'100'000'000ULL, 670'300'000'000ULL, 671'000'000'000ULL,
    673'400'000'000ULL, 683'800'000'000ULL, 686'600'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.000107, 1.66e-05, 1.72e-06, 1.3e-07, 4.34e-06, 1.35e-06,
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
  EXPECT_NEAR(weight_sum, 0.00013114L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 608.75357633063902L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 608.75357633063902L,
              1.3022100722789764e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion74274Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    652'210'000'000ULL, 726'350'000'000ULL, 727'030'000'000ULL,
    729'320'000'000ULL, 739'510'000'000ULL, 742'220'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    4e-07, 6.1e-08, 6.4e-09, 5.9e-10, 1.6e-08, 4.9e-09,
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
  EXPECT_NEAR(weight_sum, 4.8889E-7L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 666.29237415369505L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 666.29237415369505L,
              1.2674251741170881e-07L);
}

TEST(GGEMSAt211Test, PreservesConversion89246Kev) {
  auto const definition = BuildAt211Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    801'930'000'000ULL, 876'070'000'000ULL, 876'750'000'000ULL,
    879'040'000'000ULL, 889'230'000'000ULL, 891'940'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.64e-08, 2.46e-09, 4.55e-10, 9.7e-11, 7.1e-10, 2.19e-10,
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
  EXPECT_NEAR(weight_sum, 2.0341E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 816.95395949068381L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 816.95395949068381L,
              1.2674251741170881e-07L);
}

TEST(GGEMSAt211Test, ExcludesUnsupportedDiscreteEmissions) {
  auto const definition = BuildAt211Radionuclide();
  for (auto const &emission : definition.GetEmissions()) {
    auto const &distribution = emission.GetEnergyDistribution();
    // upper_limit_alpha
    if (emission.GetParticleType() == GGEMSParticleType::Alpha) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  4'895'400'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 4'895'400'000'000ULL);
        }
      }
    }
  }
}

} // namespace
