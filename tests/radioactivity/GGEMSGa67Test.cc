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
 * \brief Tests the independently selected Ga-67 source contracts.
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
using ggems::core::radioactivity::builtins::BuildGa67Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildGa67Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Ga-67");
  // LNHB evaluated 3.2613 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 281776.3200L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 12U);
  // Independent raw-evidence inventory in data/Ga-67/reference/reference.json.
  // Physical yields are not a categorical probability distribution.
  // nuclear_gamma
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.859899L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.5883L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_91_263_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.00280875L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_93_307_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 0.3252078L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_184_577_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 0.0035407L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_208_939_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.0002135254L,
                       1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_300_232_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[6U].GetYieldPerDecay(), 0.000645600L,
                       1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_393_528_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[7U].GetYieldPerDecay(), 0.0000885200L,
                       1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_494_143_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[8U].GetYieldPerDecay(), 7.65457E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_703_11_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[9U].GetYieldPerDecay(), 5.92053E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_794_4_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[10U].GetYieldPerDecay(), 2.82516E-7L,
                       1.0e-16L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_887_676_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[11U].GetYieldPerDecay(), 5.28567E-7L,
                       1.0e-16L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 1.7807055311453L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesNuclearGamma) {
  auto const definition = BuildGa67Radionuclide();
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
      91'263'000'000ULL,
      93'307'000'000ULL,
      184'577'000'000ULL,
      208'939'000'000ULL,
      300'232'000'000ULL,
      393'528'000'000ULL,
      494'143'000'000ULL,
      703'110'000'000ULL,
      794'400'000'000ULL,
      887'676'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 10U> expected_weights{
    {
      0.0309,
      0.381,
      0.2096,
      0.0237,
      0.166,
      0.0459,
      0.000666,
      0.000113,
      0.000528,
      0.001492,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.859899L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 176.83825758606534L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       176.83825758606534L, 0.0000018543935140430927L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesAtomicXray) {
  auto const definition = BuildGa67Radionuclide();
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
      1'034'850'000ULL,
      8'615'870'000ULL,
      8'638'960'000ULL,
      9'611'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0175,
      0.17,
      0.33,
      0.0708,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.5883L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 8.5230721995580486L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       8.5230721995580486L, 8.0871620982885361e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion91263Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      81'604'000'000ULL,
      90'069'000'000ULL,
      90'220'000'000ULL,
      90'243'000'000ULL,
      91'198'000'000ULL,
      91'262'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0025,
      0.000244,
      0.0000136,
      0.0000111,
      0.0000386,
      0.00000145,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.00280875L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 82.552059813084112L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       82.552059813084112L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion93307Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      83'648'000'000ULL,
      92'113'000'000ULL,
      92'264'000'000ULL,
      92'287'000'000ULL,
      93'242'000'000ULL,
      93'306'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.285,
      0.0254,
      0.00394,
      0.00577,
      0.00495,
      0.0001478,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.3252078L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 84.717232725660332L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       84.717232725660332L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion184577Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      174'918'000'000ULL,
      183'383'000'000ULL,
      183'534'000'000ULL,
      183'557'000'000ULL,
      184'512'000'000ULL,
      184'576'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.00316,
      0.000306,
      0.000013,
      0.0000119,
      0.000048,
      0.0000018,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0035407L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 175.84521735814952L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       175.84521735814952L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion208939Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      199'280'000'000ULL,
      207'745'000'000ULL,
      207'896'000'000ULL,
      207'919'000'000ULL,
      208'874'000'000ULL,
      208'938'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000191,
      0.00001889,
      0.000000486,
      0.000000228,
      0.00000281,
      0.0000001114,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0002135254L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 200.18900641890848L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       200.18900641890848L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion300232Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      290'574'000'000ULL,
      299'039'000'000ULL,
      299'190'000'000ULL,
      299'213'000'000ULL,
      300'168'000'000ULL,
      300'232'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000578,
      0.0000568,
      0.000001275,
      0.00000076,
      0.00000843,
      0.000000335,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000645600L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 291.47622411710037L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       291.47622411710037L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion393528Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      383'870'000'000ULL,
      392'335'000'000ULL,
      392'486'000'000ULL,
      392'509'000'000ULL,
      393'464'000'000ULL,
      393'528'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000793,
      0.00000782,
      0.0000001331,
      0.0000000688,
      0.000001152,
      0.0000000461,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000885200L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 384.76736753953909L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       384.76736753953909L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion494143Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      484'486'000'000ULL,
      492'951'000'000ULL,
      493'102'000'000ULL,
      493'125'000'000ULL,
      494'080'000'000ULL,
      494'144'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000686,
      0.0000000676,
      0.00000000099,
      0.000000000559,
      0.00000000991,
      0.000000000398,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 7.65457E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 485.38025462828088L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       485.38025462828088L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion70311Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      693'450'000'000ULL,
      701'920'000'000ULL,
      702'070'000'000ULL,
      702'090'000'000ULL,
      703'040'000'000ULL,
      703'110'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000000531,
      0.00000000522,
      0.0000000000583,
      0.0000000000363,
      0.00000000076,
      0.0000000000307,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.92053E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 694.33867956078257L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       694.33867956078257L, 1.3594864106178284e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion7944Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      784'746'000'000ULL,
      793'211'000'000ULL,
      793'362'000'000ULL,
      793'385'000'000ULL,
      794'340'000'000ULL,
      794'404'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000253,
      0.0000000248,
      0.00000000043,
      0.000000000438,
      0.0000000037,
      0.000000000148,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.82516E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 785.64629572130428L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       785.64629572130428L, 1.3592070138454437e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGa67Test, PreservesConversion887676Kev) {
  auto const definition = BuildGa67Radionuclide();
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
      878'023'000'000ULL,
      886'488'000'000ULL,
      886'639'000'000ULL,
      886'662'000'000ULL,
      887'617'000'000ULL,
      887'681'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000000474,
      0.0000000464,
      0.000000000567,
      0.000000000527,
      0.0000000068,
      0.000000000273,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.28567E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 878.91236664415296L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       878.91236664415296L, 1.3592070138454437e-8L);
}

} // namespace
