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
 * \brief Tests the independently selected Kr-81m source contracts.
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
using ggems::core::radioactivity::builtins::BuildKr81mRadionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildKr81mRadionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Kr-81m");
  // Evaluated 13.10 s, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 13.10L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 5U);
  // Independent selected reference values; see data/Kr-81m/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // nuclear_gamma
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.675199L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // atomic_xray
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 2.332874047764841L,
                       1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray_weak_1
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 1.0938661191E-9L,
                       1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // auger_electron
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 1.8289832671039L,
                       1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_electron
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 0.324775717L,
                       1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(),
                       5.1618320329626071191L, 1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesNuclearGamma) {
  auto const definition = BuildKr81mRadionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  // Selected absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 190'460'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesAtomicXray) {
  auto const definition = BuildKr81mRadionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Large families: boundary/central witnesses and independent total/mean;
  // deterministic reference validation separately compares every line.
  ASSERT_EQ(energies.size(), 53U);
  EXPECT_EQ(distribution.GetTableCount(), 53U);
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
    if (index == 0U) {
      EXPECT_EQ(energies[index], 17'589'400ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.000179323);
    }
    if (index == 26U) {
      EXPECT_EQ(energies[index], 1'660'680'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 4.12858E-8);
    }
    if (index == 52U) {
      EXPECT_EQ(energies[index], 14'267'300'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.0012089);
    }
    if (index > 0U) {
      EXPECT_GT(energies[index], energies[index - 1U]);
    }
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.332874047764841L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.96529577521253577L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.96529577521253577L, 1.7594177241660655e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesAtomicXrayWeak1) {
  auto const definition = BuildKr81mRadionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 11U> expected_energies{
    {
      48'500'000ULL,
      54'900'000ULL,
      167'100'000ULL,
      1'361'780'000ULL,
      1'368'810'000ULL,
      1'417'320'000ULL,
      1'542'010'000ULL,
      1'542'570'000ULL,
      1'591'070'000ULL,
      1'689'610'000ULL,
      1'690'750'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 11U> expected_weights{
    {
      1.35991E-14,
      2.11938E-10,
      8.57946E-11,
      2.5712E-10,
      2.45985E-10,
      2.02542E-10,
      2.37155E-12,
      2.25075E-12,
      2.23132E-12,
      3.36172E-11,
      5.00021E-11,
    },
  };

  ASSERT_EQ(energies.size(), 11U);
  EXPECT_EQ(distribution.GetTableCount(), 11U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0938661191E-9L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1.0530601776719295L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1.0530601776719295L, 4.3060273699462414e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesAugerElectron) {
  auto const definition = BuildKr81mRadionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 18U> expected_energies{
    {
      43'862'400ULL,
      56'258'500ULL,
      57'733'000ULL,
      65'478'000ULL,
      90'939'100ULL,
      98'252'400ULL,
      1'320'950'000ULL,
      1'407'150'000ULL,
      1'411'990'000ULL,
      1'511'590'000ULL,
      1'554'220'000ULL,
      1'670'360'000ULL,
      10'202'900'000ULL,
      10'798'000'000ULL,
      11'635'400'000ULL,
      12'337'100'000ULL,
      13'070'800'000ULL,
      13'878'600'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 18U> expected_weights{
    {
      0.0000807015,
      0.941951,
      0.0000312246,
      0.36977,
      0.00000563471,
      0.0639987,
      0.0000267465,
      0.331218,
      0.00000158831,
      0.0239096,
      3.63479E-8,
      0.000685276,
      0.00000635567,
      0.072487,
      0.00000194426,
      0.0230065,
      1.49206E-7,
      0.00180281,
    },
  };

  ASSERT_EQ(energies.size(), 18U);
  EXPECT_EQ(distribution.GetTableCount(), 18U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.8289832671039L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.91775232974060004L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.91775232974060004L, 5.8080715483427048e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSKr81mTest, PreservesConversionElectron) {
  auto const definition = BuildKr81mRadionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      176'180'000'000ULL,
      188'558'000'000ULL,
      188'732'000'000ULL,
      188'787'000'000ULL,
      190'253'000'000ULL,
      190'460'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.26913,
      0.0255162,
      0.0106526,
      0.0108225,
      0.00781308,
      0.000841337,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.324775717L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 178.35983656549052L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       178.35983656549052L, 2.0048929548263550e-8L);
}

} // namespace
