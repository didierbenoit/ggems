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
 * \brief Tests the independently selected Zn-65 source contracts.
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
using ggems::core::radioactivity::builtins::BuildZn65Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildZn65Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Zn-65");
  // LNHB evaluated 244.01 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 21082464.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);
  // Independent selected LNHB values; see data/Zn-65/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_plus_330_1_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.01421L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.5022523L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.40795L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_344_95_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 1.55E-7L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_770_64_keV
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 1.56E-8L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1115_539_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.0000916L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 0.9245040706L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesBetaPlus3301Kev) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 330.1 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'394'000ULL);
  ASSERT_EQ(energies.size(), 661U);
  EXPECT_EQ(distribution.GetTableCount(), 661U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'697'000ULL, 566'000ULL);
  EXPECT_EQ(energies.back() + 249'697'000ULL, 330'100'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'394'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 142.99620991244026L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       142.99620991244026L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 165U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.19416480357975388L, 1.0e-12L);
    }
    if (index + 1U == 330U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.63590413893996742L, 1.0e-12L);
    }
    if (index + 1U == 495U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.93755591535407264L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesNuclearGamma) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    {
      344'950'000'000ULL,
      770'640'000'000ULL,
      1'115'539'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 3U> expected_weights{
    {
      0.0000254,
      0.0000269,
      0.5022,
    },
  };

  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.5022523L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1115.4815572691255L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1115.4815572691255L, 5.3835019858777523e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesAtomicXray) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB selected photon law; see reference README; each line
  // retains its energy and absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      916'500'000ULL,
      8'027'920'000ULL,
      8'047'870'000ULL,
      8'941'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.01305,
      0.1176,
      0.2291,
      0.0482,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.40795L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 7.9195405662458635L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       7.9195405662458635L, 7.5735842645168304e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesConversion34495Kev) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    {
      335'970'000'000ULL,
      343'850'000'000ULL,
      344'020'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 3U> expected_weights{
    {
      1.41E-7,
      1.3E-8,
      1E-9,
    },
  };

  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.55E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 336.68283870967742L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       336.68283870967742L, 5.7228600442409515e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesConversion77064Kev) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 3U> expected_energies{
    {
      761'660'000'000ULL,
      769'540'000'000ULL,
      769'710'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 3U> expected_weights{
    {
      9.3E-9,
      9E-10,
      5.4E-9,
    },
  };

  ASSERT_EQ(energies.size(), 3U);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.56E-8L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 764.90115384615385L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       764.90115384615385L, 5.7228600442409515e-9L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSZn65Test, PreservesConversion1115539Kev) {
  auto const definition = BuildZn65Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent LNHB PenNuc shell records; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    {
      1'106'570'100'000ULL,
      1'114'452'900'000ULL,
      1'114'598'000'000ULL,
      1'114'617'900'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 4U> expected_weights{
    {
      0.0000834,
      0.000008,
      1E-7,
      1E-7,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0000916L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1107.2761040393013L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1107.2761040393013L, 7.5950978159904480e-9L);
}

} // namespace
