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
 * \brief Tests the independently selected Cs-134 source contracts.
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
using ggems::core::radioactivity::builtins::BuildCs134Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildCs134Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Cs-134");
  // Evaluated 2.0644 y, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 65146117.981947724800L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 17U);
  // Independent selected reference values; see data/Cs-134/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_89_06_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.2727L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_415_64_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.02498L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_658_39_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.7019L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1454_26_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 0.0006L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 2.228254L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.009430L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_242_76_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[6U].GetYieldPerDecay(), 0.000021049L,
                       1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_326_585_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[7U].GetYieldPerDecay(), 0.0000062967L,
                       1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_475_365_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[8U].GetYieldPerDecay(), 0.000169122L,
                       1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_563_246_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[9U].GetYieldPerDecay(), 0.00059543L, 1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_569_33_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[10U].GetYieldPerDecay(), 0.00143769L,
                       1.0e-16L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_604_72_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[11U].GetYieldPerDecay(), 0.0057957L, 1.0e-16L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_795_86_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[12U].GetYieldPerDecay(), 0.00258205L,
                       1.0e-16L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_801_95_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[13U].GetYieldPerDecay(), 0.000258442L,
                       1.0e-16L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1038_605_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[14U].GetYieldPerDecay(), 0.0000205331L,
                       1.0e-16L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1167_967_keV
  EXPECT_EQ(emissions[15U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[15U].GetYieldPerDecay(), 0.0000233512L,
                       1.0e-16L);
  EXPECT_EQ(emissions[15U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1365_194_keV
  EXPECT_EQ(emissions[16U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[16U].GetYieldPerDecay(), 0.0000286996L,
                       1.0e-16L);
  EXPECT_EQ(emissions[16U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 3.2488023636L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesBetaMinus8906Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 89.06 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 497'540'000ULL);
  ASSERT_EQ(energies.size(), 179U);
  EXPECT_EQ(distribution.GetTableCount(), 179U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 248'770'000ULL, 340'000ULL);
  EXPECT_EQ(energies.back() + 248'770'000ULL, 89'060'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 497'540'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 22.75476427493706L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       22.75476427493706L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 44U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.55924402196302152L, 1.0e-12L);
    }
    if (index + 1U == 89U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.86361709788879709L, 1.0e-12L);
    }
    if (index + 1U == 134U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98226527516211044L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesBetaMinus41564Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 415.64 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'566'000ULL);
  ASSERT_EQ(energies.size(), 832U);
  EXPECT_EQ(distribution.GetTableCount(), 832U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'783'000ULL, 1'088'000ULL);
  EXPECT_EQ(energies.back() + 249'783'000ULL, 415'640'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'566'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 123.44346372913043L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       123.44346372913043L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 208U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.48096141174738306L, 1.0e-12L);
    }
    if (index + 1U == 416U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.81375518725309867L, 1.0e-12L);
    }
    if (index + 1U == 624U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97225798984788658L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesBetaMinus65839Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 658.39 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'916'000ULL);
  ASSERT_EQ(energies.size(), 1317U);
  EXPECT_EQ(distribution.GetTableCount(), 1317U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'958'000ULL, 628'000ULL);
  EXPECT_EQ(energies.back() + 249'958'000ULL, 658'390'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'916'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 209.93182849187855L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       209.93182849187855L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 329U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.43626369423161421L, 1.0e-12L);
    }
    if (index + 1U == 658U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.78382753336583328L, 1.0e-12L);
    }
    if (index + 1U == 987U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.96593251922660801L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesBetaMinus145426Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 1454.26 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'916'000ULL);
  ASSERT_EQ(energies.size(), 2909U);
  EXPECT_EQ(distribution.GetTableCount(), 2909U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'958'000ULL, 4'356'000ULL);
  EXPECT_EQ(energies.back() + 249'958'000ULL, 1'454'260'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'916'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 535.4502144567747L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       535.4502144567747L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 727U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.36147767430683806L, 1.0e-12L);
    }
    if (index + 1U == 1454U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.69981926980477002L, 1.0e-12L);
    }
    if (index + 1U == 2181U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.93618611145940865L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesNuclearGamma) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 12U> expected_energies{
    {
      242'760'000'000ULL,
      326'585'000'000ULL,
      475'365'000'000ULL,
      563'246'000'000ULL,
      569'330'000'000ULL,
      604'720'000'000ULL,
      795'860'000'000ULL,
      801'950'000'000ULL,
      847'000'000'000ULL,
      1'038'605'000'000ULL,
      1'167'967'000'000ULL,
      1'365'194'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 12U> expected_weights{
    {
      0.000241,
      0.000171,
      0.01479,
      0.08342,
      0.15368,
      0.9763,
      0.8547,
      0.08694,
      0.000003,
      0.009909,
      0.01791,
      0.03019,
    },
  };

  ASSERT_EQ(energies.size(), 12U);
  EXPECT_EQ(distribution.GetTableCount(), 12U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.228254L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 697.57953987292293L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       697.57953987292293L, 0.0000031361443681478500L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesAtomicXray) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    {
      4'882'400'000ULL,
      31'817'400'000ULL,
      32'193'900'000ULL,
      36'445'700'000ULL,
      37'331'700'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 5U> expected_weights{
    {
      0.001058,
      0.002378,
      0.00438,
      0.001289,
      0.000325,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.009430L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 29.792995567338282L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       29.792995567338282L, 3.7875957025587559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion24276Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      205'305'000'000ULL,
      236'757'000'000ULL,
      237'122'000'000ULL,
      237'499'000'000ULL,
      241'732'000'000ULL,
      242'585'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000174,
      0.00000198,
      5.1E-7,
      4.1E-7,
      6E-7,
      1.49E-7,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000021049L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 210.96380041807212L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       210.96380041807212L, 5.2179558372497559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion326585Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      289'144'000'000ULL,
      320'596'000'000ULL,
      320'961'000'000ULL,
      321'338'000'000ULL,
      325'571'000'000ULL,
      326'424'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000053,
      6.2E-7,
      1E-7,
      7E-8,
      1.66E-7,
      4.07E-8,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000062967L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 294.30538580526307L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       294.30538580526307L, 5.2179558372497559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion475365Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      437'927'000'000ULL,
      469'379'000'000ULL,
      469'744'000'000ULL,
      470'121'000'000ULL,
      474'354'000'000ULL,
      475'207'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000142,
      0.0000161,
      0.00000322,
      0.0000022,
      0.0000045,
      0.000001102,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000169122L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 443.15789130923239L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       443.15789130923239L, 5.2179558372497559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion563246Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      525'805'100'000ULL,
      557'256'900'000ULL,
      557'622'100'000ULL,
      557'998'700'000ULL,
      562'232'000'000ULL,
      563'084'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000503,
      0.0000577,
      0.00000947,
      0.00000624,
      0.00001527,
      0.00000375,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00059543L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 530.86530804628588L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       530.86530804628588L, 5.2178440785408020e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion56933Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      531'890'000'000ULL,
      563'342'000'000ULL,
      563'707'000'000ULL,
      564'084'000'000ULL,
      568'317'000'000ULL,
      569'170'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.001237,
      0.0001495,
      0.00000775,
      0.00000238,
      0.00003289,
      0.00000817,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00143769L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 536.43057488053753L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       536.43057488053753L, 5.2179558372497559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion60472Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      567'281'700'000ULL,
      598'733'500'000ULL,
      599'098'700'000ULL,
      599'475'300'000ULL,
      603'708'600'000ULL,
      604'560'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00491,
      0.000565,
      0.0000842,
      0.0000546,
      0.000146,
      0.0000359,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0057957L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 572.26188869851787L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       572.26188869851787L, 5.2178440785408020e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion79586Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      758'427'100'000ULL,
      789'878'900'000ULL,
      790'244'100'000ULL,
      790'620'700'000ULL,
      794'854'000'000ULL,
      795'706'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.002205,
      0.0002573,
      0.00002632,
      0.00001624,
      0.0000619,
      0.00001529,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00258205L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 763.18208914505916L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       763.18208914505916L, 5.2178440785408020e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion80195Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      764'512'000'000ULL,
      795'964'000'000ULL,
      796'329'000'000ULL,
      796'706'000'000ULL,
      800'939'000'000ULL,
      801'792'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0002208,
      0.00002573,
      0.000002608,
      0.000001608,
      0.00000617,
      0.000001526,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000258442L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 769.25445872574891L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       769.25445872574891L, 5.2179558372497559e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion1038605Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'001'173'100'000ULL,
      1'032'624'900'000ULL,
      1'032'990'100'000ULL,
      1'033'366'700'000ULL,
      1'037'600'000'000ULL,
      1'038'452'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000177,
      0.00000212,
      9.41E-8,
      4.06E-8,
      4.63E-7,
      1.154E-7,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000205331L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1005.6608066901734L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1005.6608066901734L, 5.2178440785408020e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion1167967Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[15U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'130'527'400'000ULL,
      1'161'979'200'000ULL,
      1'162'344'400'000ULL,
      1'162'721'000'000ULL,
      1'166'954'300'000ULL,
      1'167'806'600'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000201,
      0.000002355,
      1.444E-7,
      8.79E-8,
      5.32E-7,
      1.319E-7,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000233512L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1135.0577632584193L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1135.0577632584193L, 5.2178440785408020e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSCs134Test, PreservesConversion1365194Kev) {
  auto const definition = BuildCs134Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[16U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB absolute emission records; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      1'327'758'100'000ULL,
      1'359'209'900'000ULL,
      1'359'575'100'000ULL,
      1'359'951'700'000ULL,
      1'364'185'000'000ULL,
      1'365'037'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00002476,
      0.000002901,
      1.455E-7,
      9.03E-8,
      6.43E-7,
      1.598E-7,
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
    EXPECT_GE(weights[index], 0.0);
    if (weights[index] > 0.0) {
      EXPECT_GT(tickets[index], previous_ticket);
    } else {
      EXPECT_EQ(tickets[index], previous_ticket);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000286996L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1332.2235922103444L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1332.2235922103444L, 5.2178440785408020e-8L);
}

} // namespace
