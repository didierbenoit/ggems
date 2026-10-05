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
 * \brief Tests the independently selected Y-90 source contracts.
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
using ggems::core::radioactivity::builtins::BuildY90Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildY90Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Y-90");
  // Evaluated 64.041 h, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 230547.600L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 4U);
  // Independent selected reference values; see data/Y-90/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_92_4_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[0U].GetYieldPerDecay(), 0.000000014L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_518_0_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[1U].GetYieldPerDecay(), 0.00017L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_2278_7_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_NEAR(emissions[2U].GetYieldPerDecay(), 0.99983L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_NEAR(emissions[3U].GetYieldPerDecay(), 1.4E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.000000028L, 1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, PreservesBetaMinus924Kev) {
  auto const definition = BuildY90Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 92.4 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'458'000ULL);
  ASSERT_EQ(energies.size(), 185U);
  EXPECT_EQ(distribution.GetTableCount(), 185U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'729'000ULL, 270'000ULL);
  EXPECT_EQ(energies.back() + 249'729'000ULL, 92'400'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'458'000ULL));
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
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 23.804216847335653L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 23.804216847335653L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 46U) {
      EXPECT_NEAR(cumulative, 0.56028927064420182L, 1.0e-12L);
    }
    if (index + 1U == 92U) {
      EXPECT_NEAR(cumulative, 0.86126387325000385L, 1.0e-12L);
    }
    if (index + 1U == 138U) {
      EXPECT_NEAR(cumulative, 0.98115088984628361L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, PreservesBetaMinus5180Kev) {
  auto const definition = BuildY90Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 518 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 1036U);
  EXPECT_EQ(distribution.GetTableCount(), 1036U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 2'072'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 518'000'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'998'000ULL));
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
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 183.3576808802562L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 183.3576808802562L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 259U) {
      EXPECT_NEAR(cumulative, 0.38406625242984292L, 1.0e-12L);
    }
    if (index + 1U == 518U) {
      EXPECT_NEAR(cumulative, 0.71943454915995924L, 1.0e-12L);
    }
    if (index + 1U == 777U) {
      EXPECT_NEAR(cumulative, 0.94509794423263022L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, PreservesBetaMinus22787Kev) {
  auto const definition = BuildY90Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 2278.7 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'934'000ULL);
  ASSERT_EQ(energies.size(), 4558U);
  EXPECT_EQ(distribution.GetTableCount(), 4558U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'967'000ULL, 828'000ULL);
  EXPECT_EQ(energies.back() + 249'967'000ULL, 2'278'700'000'000ULL);
  long double weight_sum{0.0L};
  long double moment{0.0L};
  long double ticket_moment{0.0L};
  std::uint64_t previous_ticket{0ULL};
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    EXPECT_TRUE(std::isfinite(weights[index]));
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'934'000ULL));
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
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 924.8620224363169L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 924.8620224363169L,
              0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 1139U) {
      EXPECT_NEAR(cumulative, 0.29246385854673314L, 1.0e-12L);
    }
    if (index + 1U == 2279U) {
      EXPECT_NEAR(cumulative, 0.64598543507349929L, 1.0e-12L);
    }
    if (index + 1U == 3418U) {
      EXPECT_NEAR(cumulative, 0.92039935633233424L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, PreservesNuclearGamma) {
  auto const definition = BuildY90Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  // Selected absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(),
            2'186'254'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSY90Test, ExcludesNonParentSourceLines) {
  auto const definition = BuildY90Radionuclide();
  // annihilation_photons
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Gamma) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 511'000'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 511'000'000'000ULL);
      }
    }
  }
}

} // namespace
