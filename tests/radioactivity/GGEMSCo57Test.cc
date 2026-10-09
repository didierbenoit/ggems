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
 * \brief Tests the independently selected Co-57 source contracts.
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
using ggems::core::radioactivity::builtins::BuildCo57Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildCo57Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Co-57");
  // LNHB evaluated 271.81 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 23484384.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 12U);
  // Independent raw-evidence inventory in data/Co-57/reference/reference.json.
  // Physical yields are not a categorical probability distribution.
  // nuclear_gamma
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 1.055677L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.5885L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_14_41295_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.784623L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_122_06065_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 0.02017715L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_136_47356_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 0.01593619L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_230_27_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 1.6648E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_339_67_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[6U].GetYieldPerDecay(), 6.33053E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_352_34_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[7U].GetYieldPerDecay(), 4.77934E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_366_74_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[8U].GetYieldPerDecay(), 2.32566E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_569_94_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[9U].GetYieldPerDecay(), 7.6554E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_692_01_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[10U].GetYieldPerDecay(), 5.80152E-7L,
                       1.0e-16L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_706_415_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[11U].GetYieldPerDecay(), 2.41978E-8L,
                       1.0e-16L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 2.4649141719071L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesNuclearGamma) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB LARA absolute photon inventory; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 10U> expected_energies{
    {
      14'412'950'000ULL,
      122'060'650'000ULL,
      136'473'560'000ULL,
      230'270'000'000ULL,
      339'670'000'000ULL,
      352'340'000'000ULL,
      366'740'000'000ULL,
      569'940'000'000ULL,
      692'010'000'000ULL,
      706'415'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 10U> expected_weights{
    {
      0.0918,
      0.8549,
      0.1071,
      0.000004,
      0.000038,
      0.000032,
      0.000013,
      0.00015,
      0.00159,
      0.00005,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.055677L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 115.12996443135542L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       115.12996443135542L, 0.0000016112928271129727L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesAtomicXray) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB LARA absolute photon inventory; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      705'400'000ULL,
      6'390'910'000ULL,
      6'403'910'000ULL,
      7'083'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.013,
      0.1712,
      0.335,
      0.0693,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.5885L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 6.3542389158878505L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       6.3542389158878505L, 6.0397891163825989e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion1441295Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      7'300'950'000ULL,
      13'566'850'000ULL,
      13'691'850'000ULL,
      13'704'850'000ULL,
      14'371'330'000ULL,
      14'412'550'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.702,
      0.0666,
      0.004,
      0.001662,
      0.00993,
      0.000431,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.784623L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 7.9723422416243215L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       7.9723422416243215L, 1.0034790432453156e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion12206065Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      114'948'790'000ULL,
      121'214'690'000ULL,
      121'339'690'000ULL,
      121'352'690'000ULL,
      122'019'170'000ULL,
      122'060'390'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.01812,
      0.001701,
      0.000056,
      0.0000412,
      0.0002479,
      0.00001105,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.02017715L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 115.59860215692008L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       115.59860215692008L, 1.0034790432453156e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion13647356Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      129'361'740'000ULL,
      135'627'640'000ULL,
      135'752'640'000ULL,
      135'765'640'000ULL,
      136'432'120'000ULL,
      136'473'340'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.01426,
      0.001271,
      0.0000812,
      0.0001155,
      0.0002003,
      0.00000819,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.01593619L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 130.03297895661385L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       130.03297895661385L, 1.0034790432453156e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion23027Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      223'158'000'000ULL,
      229'424'000'000ULL,
      229'549'000'000ULL,
      229'562'000'000ULL,
      230'228'000'000ULL,
      230'270'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000015,
      0.0000000014,
      0.000000000026,
      0.000000000013,
      0.0000000002,
      0.000000000009,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.6648E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 223.78869593945219L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       223.78869593945219L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion33967Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      332'558'000'000ULL,
      338'824'000'000ULL,
      338'949'000'000ULL,
      338'962'000'000ULL,
      339'628'000'000ULL,
      339'670'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000057,
      0.0000000054,
      0.000000000077,
      0.0000000000436,
      0.00000000075,
      0.0000000000347,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 6.33053E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 333.19233879627772L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       333.19233879627772L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion35234Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      345'228'000'000ULL,
      351'494'000'000ULL,
      351'619'000'000ULL,
      351'632'000'000ULL,
      352'298'000'000ULL,
      352'340'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000043,
      0.0000000041,
      0.000000000056,
      0.0000000000309,
      0.00000000058,
      0.0000000000265,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 4.77934E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 345.86690511242138L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       345.86690511242138L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion36674Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      359'628'000'000ULL,
      365'894'000'000ULL,
      366'019'000'000ULL,
      366'032'000'000ULL,
      366'698'000'000ULL,
      366'740'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000021,
      0.0000000019,
      0.000000000034,
      0.00000000003,
      0.00000000028,
      0.0000000000126,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.32566E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 360.24649217856436L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       360.24649217856436L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion56994Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      562'828'000'000ULL,
      569'094'000'000ULL,
      569'219'000'000ULL,
      569'232'000'000ULL,
      569'898'000'000ULL,
      569'940'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000069,
      0.0000000065,
      0.000000000062,
      0.00000000004,
      0.00000000091,
      0.000000000042,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 7.6554E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 563.45649499699558L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       563.45649499699558L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion69201Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      684'898'000'000ULL,
      691'164'000'000ULL,
      691'289'000'000ULL,
      691'302'000'000ULL,
      691'968'000'000ULL,
      692'010'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000523,
      0.0000000491,
      0.000000000464,
      0.000000000388,
      0.00000000688,
      0.00000000032,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.80152E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 685.52547041464995L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       685.52547041464995L, 1.0035349225997925e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCo57Test, PreservesConversion706415Kev) {
  auto const definition = BuildCo57Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB PenNuc shell records; each line retains its energy and absolute
  // yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      699'308'000'000ULL,
      705'574'000'000ULL,
      705'699'000'000ULL,
      705'712'000'000ULL,
      706'378'000'000ULL,
      706'420'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000000218,
      0.00000000204,
      0.0000000000266,
      0.00000000003,
      0.000000000288,
      0.0000000000132,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.41978E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 699.93924742745208L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       699.93924742745208L, 1.0035349225997925e-8L);
}

} // namespace
