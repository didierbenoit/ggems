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
 * \brief Tests the independently selected S-35 source contracts.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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
using ggems::core::radioactivity::builtins::BuildS35Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSS35Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildS35Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "S-35");
  // LNHB evaluated 87.25 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 7538400.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  // Independent selected LNHB values; see data/S-35/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_167_33_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 1.0L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 1.0L, 1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSS35Test, PreservesBetaMinus16733Kev) {
  auto const definition = BuildS35Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 167.33 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'492'000ULL);
  ASSERT_EQ(energies.size(), 335U);
  EXPECT_EQ(distribution.GetTableCount(), 335U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'746'000ULL, 180'000ULL);
  EXPECT_EQ(energies.back() + 249'746'000ULL, 167'330'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'492'000ULL));
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
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 48.27632511759676L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       48.27632511759676L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 83U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.49270056776883880L, 1.0e-12L);
    }
    if (index + 1U == 167U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.82330594023492082L, 1.0e-12L);
    }
    if (index + 1U == 251U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97459467734080944L, 1.0e-12L);
    }
  }
}

} // namespace
