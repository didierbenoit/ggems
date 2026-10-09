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
 * \brief Tests the independently selected Cs-131 source contracts.
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
using ggems::core::radioactivity::builtins::BuildCs131Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSCs131Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildCs131Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Cs-131");
  // LNHB evaluated 9.681 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 836438.400L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  // Independent selected LNHB values; see data/Cs-131/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // atomic_xray
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.745264L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 0.745264L, 1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs131Test, PreservesAtomicXray) {
  auto const definition = BuildCs131Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB LARA absolute emission inventory; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 8U> expected_energies{
    {
      30'631'000'000ULL,
      30'978'000'000ULL,
      34'925'000'000ULL,
      34'993'000'000ULL,
      35'252'000'000ULL,
      35'266'000'000ULL,
      35'828'000'000ULL,
      35'980'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 8U> expected_weights{
    {
      0.214,
      0.396,
      0.037,
      0.072,
      0.000354,
      0.00052,
      0.0231,
      0.00229,
    },
  };

  ASSERT_EQ(energies.size(), 8U);
  EXPECT_EQ(distribution.GetTableCount(), 8U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.745264L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 31.632926758839821L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       31.632926758839821L, 1.0063288903236389e-8L);
}

} // namespace
