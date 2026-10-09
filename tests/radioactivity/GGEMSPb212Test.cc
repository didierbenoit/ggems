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
 * \brief Tests the independently selected Pb-212 source contracts.
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
using ggems::core::radioactivity::builtins::BuildPb212Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildPb212Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Pb-212");
  // LNHB evaluated 10.630 h, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 38268.000L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 10U);
  // Independent selected LNHB values; see data/Pb-212/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_153_7_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.0492L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_330_4_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.808L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_569_0_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 0.144L, 1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 0.471507L, 1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 0.4746L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_115_183_keV
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.0398531L, 1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_176_64_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[6U].GetYieldPerDecay(), 0.001085139L,
                       1.0e-16L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_238_632_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[7U].GetYieldPerDecay(), 0.376739L, 1.0e-16L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_300_089_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[8U].GetYieldPerDecay(), 0.01500419L, 1.0e-16L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_415_272_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[9U].GetYieldPerDecay(), 0.000025012L,
                       1.0e-16L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 2.380013441L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesBetaMinus1537Kev) {
  auto const definition = BuildPb212Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 153.7 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'024'000ULL);
  ASSERT_EQ(energies.size(), 308U);
  EXPECT_EQ(distribution.GetTableCount(), 308U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'512'000ULL, 608'000ULL);
  EXPECT_EQ(energies.back() + 249'512'000ULL, 153'700'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'024'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 40.588105383352776L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       40.588105383352776L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 77U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.54852258729516353L, 1.0e-12L);
    }
    if (index + 1U == 154U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.85689461879344817L, 1.0e-12L);
    }
    if (index + 1U == 231U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.98098745341454689L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesBetaMinus3304Kev) {
  auto const definition = BuildPb212Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 330.4 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'848'000ULL);
  ASSERT_EQ(energies.size(), 661U);
  EXPECT_EQ(distribution.GetTableCount(), 661U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'924'000ULL, 472'000ULL);
  EXPECT_EQ(energies.back() + 249'924'000ULL, 330'400'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'848'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 92.71614328335268L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       92.71614328335268L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 165U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.51357799139797964L, 1.0e-12L);
    }
    if (index + 1U == 330U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.83483931002210280L, 1.0e-12L);
    }
    if (index + 1U == 495U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97647069868508710L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesBetaMinus5690Kev) {
  auto const definition = BuildPb212Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 569.0 keV endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'998'000ULL);
  ASSERT_EQ(energies.size(), 1138U);
  EXPECT_EQ(distribution.GetTableCount(), 1138U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'999'000ULL, 2'276'000ULL);
  EXPECT_EQ(energies.back() + 249'999'000ULL, 569'000'000'000ULL);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.0L, 1.0e-12L);
  // Independent full-support piecewise-linear mean; unchanged 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 170.62948703251197L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       170.62948703251197L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 284U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.47378621585004986L, 1.0e-12L);
    }
    if (index + 1U == 569U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.81028827572796485L, 1.0e-12L);
    }
    if (index + 1U == 853U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.97161366030126616L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesNuclearGamma) {
  auto const definition = BuildPb212Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB LARA absolute emission inventory; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    {
      115'183'000'000ULL,
      176'640'000'000ULL,
      238'632'000'000ULL,
      300'089'000'000ULL,
      415'272'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 5U> expected_weights{
    {
      0.00586,
      0.000537,
      0.4326,
      0.03238,
      0.00013,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.471507L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 241.29630830507288L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       241.29630830507288L, 3.4944957511723042e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesAtomicXray) {
  auto const definition = BuildPb212Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Raw LNHB LARA absolute emission inventory; each line retains its energy and
  // absolute yield.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 5U> expected_energies{
    {
      12'564'550'000ULL,
      74'815'700'000ULL,
      77'108'800'000ULL,
      87'347'000'000ULL,
      90'075'700'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 5U> expected_weights{
    {
      0.1346,
      0.0994,
      0.1663,
      0.0569,
      0.0174,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.4746L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 60.026177686472819L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       60.026177686472819L, 9.0334854724258184e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesConversion115183Kev) {
  auto const definition = BuildPb212Radionuclide();
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
      24'657'000'000ULL,
      98'796'000'000ULL,
      99'472'000'000ULL,
      101'764'000'000ULL,
      111'955'000'000ULL,
      114'662'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0324,
      0.00511,
      0.000546,
      0.0000421,
      0.001342,
      0.000413,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0398531L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 39.141982741618594L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       39.141982741618594L, 1.2583553249239922e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesConversion17664Kev) {
  auto const definition = BuildPb212Radionuclide();
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
      86'114'000'000ULL,
      160'252'000'000ULL,
      160'929'000'000ULL,
      163'221'000'000ULL,
      173'412'000'000ULL,
      176'119'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.000884,
      0.000138,
      0.0000147,
      0.000001079,
      0.0000362,
      0.00001116,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.001085139L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 100.47037863259914L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       100.47037863259914L, 1.2583553249239922e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesConversion238632Kev) {
  auto const definition = BuildPb212Radionuclide();
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
      148'106'100'000ULL,
      222'244'500'000ULL,
      222'920'900'000ULL,
      225'213'400'000ULL,
      235'404'100'000ULL,
      238'111'200'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.307,
      0.0479,
      0.00507,
      0.000359,
      0.01255,
      0.00386,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.376739L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 162.44289948903618L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       162.44289948903618L, 1.2583567219078541e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesConversion300089Kev) {
  auto const definition = BuildPb212Radionuclide();
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
      209'563'000'000ULL,
      283'702'000'000ULL,
      284'378'000'000ULL,
      286'670'000'000ULL,
      296'861'000'000ULL,
      299'568'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.01224,
      0.001901,
      0.0001995,
      0.00001379,
      0.000497,
      0.0001529,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.01500419L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 223.83074837761985L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       223.83074837761985L, 1.2583553249239922e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPb212Test, PreservesConversion415272Kev) {
  auto const definition = BuildPb212Radionuclide();
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
      324'746'000'000ULL,
      398'884'000'000ULL,
      399'561'000'000ULL,
      401'853'000'000ULL,
      412'044'000'000ULL,
      414'751'000'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 6U> expected_weights{
    {
      0.0000204,
      0.0000032,
      3.2E-7,
      2.2E-8,
      8.2E-7,
      2.5E-7,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.000025012L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 339.01772413241644L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       339.01772413241644L, 1.2583553249239922e-7L);
}

} // namespace
