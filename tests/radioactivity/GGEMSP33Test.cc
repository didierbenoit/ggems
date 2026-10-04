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
 * \brief Tests the selected P-33 nuclear and spectrum contracts.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::builtins::BuildP33Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSP33Test, BuildsExactIdentityAndSingleBetaMinusEmission) {
  GGEMSRadionuclideDefinition const definition = BuildP33Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "P-33");
  // LNHB 2026: 25.38 d multiplied by exact 86400 s/d.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 2'192'832.0L);

  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
  EXPECT_EQ(definition.GetTotalYieldPerDecay(), 1.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSP33Test, PreservesSelectedBetaShapeSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildP33Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const centers = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();

  EXPECT_EQ(distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // Grid derived from the LNHB endpoint: ceil(endpoint / 0.5 keV) bins,
  // largest even integer-meV width leaving a positive lower edge.
  EXPECT_EQ(distribution.GetTableCount(), 497U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);

  ASSERT_EQ(centers.size(), 497U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 249'999'000ULL, 994'000ULL);
  EXPECT_EQ(centers.back() + 249'999'000ULL, 248'500'000'000ULL);

  long double weight_sum{0.0L};
  long double weighted_center_sum{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    EXPECT_EQ(centers[index],
              centers.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
    double const weight = weights[index];
    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_GT(weight, 0.0);
    weight_sum += static_cast<long double>(weight);
    weighted_center_sum += static_cast<long double>(weight) *
                           static_cast<long double>(centers[index]);

    EXPECT_GT(tickets[index], previous_ticket);
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);

  long double const mean_energy_keV =
    weighted_center_sum / weight_sum /
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  // Independent full-support integral of the selected BetaShape 2.4 CSV:
  // validation/radioactivity/data/P-33/reference/. The 0.005 keV
  // representation budget is the existing H-3/C-14 budget, fixed in advance.
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 75.9074988656789, 0.005);
}

} // namespace
