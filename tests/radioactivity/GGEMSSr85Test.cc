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
 * \brief Tests the independently selected Sr-85 source contracts.
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
using ggems::core::radioactivity::builtins::BuildSr85Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildSr85Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Sr-85");
  // LNHB evaluated 64.850 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 5603040.000L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 5U);
  // Independent selected LNHB values; see data/Sr-85/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // nuclear_gamma
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.0001342L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_129_826_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 3.60766E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_354_97_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 1.2637E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_717_81_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 3.9492E-9L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_868_98_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 8.8691E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 0.0001346660432L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesNuclearGamma) {
  auto const definition = BuildSr85Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      129'826'000'000ULL,
      354'970'000'000ULL,
      717'810'000'000ULL,
      868'980'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.000005,
      0.000005,
      0.0000032,
      0.000121,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0001342L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 818.68518628912072L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       818.68518628912072L, 6.8849080631732941e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesConversion129826Kev) {
  auto const definition = BuildSr85Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    {
      114'626'000'000ULL,
      127'761'000'000ULL,
      127'962'000'000ULL,
      128'022'000'000ULL,
      129'620'000'000ULL,
      129'811'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      3.18E-7,
      3.38E-8,
      1.515E-9,
      5.6E-10,
      6.04E-9,
      8.51E-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 3.60766E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 116.22026053175743L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       116.22026053175743L, 2.1313199943304062e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesConversion35497Kev) {
  auto const definition = BuildSr85Radionuclide();
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
      339'770'000'000ULL,
      352'900'000'000ULL,
      353'110'000'000ULL,
      353'170'000'000ULL,
      354'760'000'000ULL,
      354'960'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      1.12E-8,
      1.14E-9,
      2.6E-11,
      4.2E-11,
      2E-10,
      2.9E-11,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.2637E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 341.29855503679671L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       341.29855503679671L, 2.1320184862613678e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesConversion71781Kev) {
  auto const definition = BuildSr85Radionuclide();
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
      702'610'000'000ULL,
      715'740'000'000ULL,
      715'950'000'000ULL,
      716'010'000'000ULL,
      717'600'000'000ULL,
      717'800'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      3.49E-9,
      3.61E-10,
      1.25E-11,
      1.16E-11,
      6.5E-11,
      9.1E-12,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 3.9492E-9L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 704.17353160133698L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       704.17353160133698L, 2.1320184862613678e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSr85Test, PreservesConversion86898Kev) {
  auto const definition = BuildSr85Radionuclide();
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
      853'780'000'000ULL,
      866'910'000'000ULL,
      867'120'000'000ULL,
      867'180'000'000ULL,
      868'770'000'000ULL,
      868'970'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      7.86E-8,
      8.14E-9,
      1.82E-10,
      1.48E-10,
      1.42E-9,
      2.01E-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 8.8691E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 855.30922246902166L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       855.30922246902166L, 2.1320184862613678e-8L);
}

} // namespace
