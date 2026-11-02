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
 * \brief Checks evaluated radionuclide lines and their quantized sampling law.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMSLongDoubleAssertions.hh"

namespace ggems::test {

// Keep expected evaluated data and frozen error budgets in the calling fixture.
inline auto ExpectRadionuclideLines(
  core::sources::GGEMSEnergyDistribution const &distribution,
  std::span<std::uint64_t const> expected_energies,
  std::span<double const> expected_weights, long double expected_weight_sum,
  long double expected_mean_keV, long double ticket_mean_tolerance_keV)
  -> void {
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  ASSERT_EQ(energies.size(), expected_energies.size());
  EXPECT_EQ(distribution.GetTableCount(), expected_energies.size());
  ASSERT_EQ(weights.size(), expected_weights.size());
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());

  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < energies.size(); ++index) {
    SCOPED_TRACE(index);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, expected_weight_sum, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / 1'000'000'000.0L,
                       expected_mean_keV, 1.0e-9L);
  // Largest-remainder allocation introduces at most one ticket error per line.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / 1'000'000'000.0L,
                       expected_mean_keV, ticket_mean_tolerance_keV);
}

} // namespace ggems::test
