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
 * \brief Tests the independently selected Ba-133 source contracts.
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
using ggems::core::radioactivity::builtins::BuildBa133Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildBa133Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Ba-133");
  // LNHB evaluated 3849.3 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 332579520.0L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 11U);
  // Independent selected LNHB values; see data/Ba-133/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // nuclear_gamma
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 1.35598L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 1.3476L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_53_1622_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.12106L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_79_6142_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 0.046471L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_80_9979_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 0.56750L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_160_6121_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.00187084L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_223_2368_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[6U].GetYieldPerDecay(), 0.000438265L,
                       1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_276_3989_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[7U].GetYieldPerDecay(), 0.0040351L, 1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_302_8508_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[8U].GetYieldPerDecay(), 0.00793991L, 1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_356_0129_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[9U].GetYieldPerDecay(), 0.0157948L, 1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_383_8485_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[10U].GetYieldPerDecay(), 0.00180753L,
                       1.0e-16L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 3.470497445L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesNuclearGamma) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 9U> expected_energies{
    {
      53'162'200'000ULL,
      79'614'200'000ULL,
      80'997'900'000ULL,
      160'612'100'000ULL,
      223'236'800'000ULL,
      276'398'900'000ULL,
      302'850'800'000ULL,
      356'012'900'000ULL,
      383'848'500'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 9U> expected_weights{
    {
      0.0214,
      0.0263,
      0.3331,
      0.00638,
      0.0045,
      0.0713,
      0.1831,
      0.6205,
      0.0894,
    },
  };

  ASSERT_EQ(energies.size(), 9U);
  EXPECT_EQ(distribution.GetTableCount(), 9U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.35598L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 267.42465171167716L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       267.42465171167716L, 6.9304513668864965e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesAtomicXray) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    {
      4'673'550'000ULL,
      30'625'400'000ULL,
      30'973'100'000ULL,
      35'053'000'000ULL,
      35'900'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 5U> expected_weights{
    {
      0.1587,
      0.338,
      0.624,
      0.1824,
      0.0445,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.3476L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 28.503652816117542L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       28.503652816117542L, 3.6452721508592367e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion531622Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      17'177'600'000ULL,
      47'447'900'000ULL,
      47'802'800'000ULL,
      48'150'300'000ULL,
      52'213'300'000ULL,
      53'018'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.1023,
      0.01252,
      0.00152,
      0.0009,
      0.00308,
      0.00074,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.12106L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 22.033389608458616L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       22.033389608458616L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion796142Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      43'629'600'000ULL,
      73'899'900'000ULL,
      74'254'800'000ULL,
      74'602'300'000ULL,
      78'665'300'000ULL,
      79'470'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0393,
      0.00479,
      0.00058,
      0.00034,
      0.00118,
      0.000281,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.046471L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 48.464903858320243L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       48.464903858320243L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion809979Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      45'013'300'000ULL,
      75'283'600'000ULL,
      75'638'500'000ULL,
      75'986'000'000ULL,
      80'049'000'000ULL,
      80'853'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.477,
      0.0576,
      0.00843,
      0.00603,
      0.01489,
      0.00355,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.56750L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 50.013160440528634L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       50.013160440528634L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion1606121Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      124'627'500'000ULL,
      154'897'800'000ULL,
      155'252'700'000ULL,
      155'600'200'000ULL,
      159'663'200'000ULL,
      160'468'100'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.001493,
      0.0001627,
      0.0000708,
      0.0000664,
      0.0000632,
      0.00001474,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00187084L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 130.98420390519767L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       130.98420390519767L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion2232368Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      187'252'400'000ULL,
      217'522'700'000ULL,
      217'877'600'000ULL,
      218'225'100'000ULL,
      222'288'100'000ULL,
      223'093'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000376,
      0.0000457,
      0.00000306,
      8.9E-7,
      0.00001017,
      0.000002445,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000438265L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 191.69851105381447L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       191.69851105381447L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion2763989Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      240'414'600'000ULL,
      270'684'900'000ULL,
      271'039'800'000ULL,
      271'387'300'000ULL,
      275'450'300'000ULL,
      276'255'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00328,
      0.00035,
      0.0001362,
      0.0001138,
      0.0001257,
      0.0000294,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0040351L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 246.29999357884563L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       246.29999357884563L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion3028508Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      266'866'600'000ULL,
      297'136'900'000ULL,
      297'491'800'000ULL,
      297'839'300'000ULL,
      301'902'300'000ULL,
      302'707'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00683,
      0.000828,
      0.0000465,
      0.00001091,
      0.0001809,
      0.0000436,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00793991L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 271.24025167199628L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       271.24025167199628L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion3560129Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      320'028'800'000ULL,
      350'299'100'000ULL,
      350'654'000'000ULL,
      351'001'500'000ULL,
      355'064'500'000ULL,
      355'869'400'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.01309,
      0.00144,
      0.000403,
      0.0003096,
      0.000447,
      0.0001052,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0157948L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 325.40726155316940L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       325.40726155316940L, 5.0168739801645279e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBa133Test, PreservesConversion3838485Kev) {
  auto const definition = BuildBa133Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      347'864'500'000ULL,
      378'134'800'000ULL,
      378'489'700'000ULL,
      378'837'200'000ULL,
      382'900'200'000ULL,
      383'705'100'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.001505,
      0.0001663,
      0.0000425,
      0.00003183,
      0.0000501,
      0.0000118,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00180753L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 353.12006454443356L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       353.12006454443356L, 5.0168739801645279e-8L);
}

} // namespace
