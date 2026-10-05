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
 * \brief Tests the independently selected Re-186 source contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
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
using ggems::core::radioactivity::builtins::BuildRe186Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSRe186Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildRe186Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Re-186");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 321287.0400L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 15U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_minus_159_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 2.7E-7L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_302_keV
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 6.27E-4L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_932_3_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.2149L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // beta_minus_1069_5_keV
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[3U].GetYieldPerDecay(), 0.7087L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[4U].GetYieldPerDecay(), 0.1008513924L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[5U].GetYieldPerDecay(), 0.14922L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_122_33_keV
  EXPECT_EQ(emissions[6U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[6U].GetYieldPerDecay(), 0.010921L);
  EXPECT_EQ(emissions[6U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_137_157_keV
  EXPECT_EQ(emissions[7U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[7U].GetYieldPerDecay(), 0.12157L);
  EXPECT_EQ(emissions[7U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_143_keV
  EXPECT_EQ(emissions[8U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[8U].GetYieldPerDecay(), 1.370E-8L);
  EXPECT_EQ(emissions[8U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_296_93_keV
  EXPECT_EQ(emissions[9U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[9U].GetYieldPerDecay(), 5.019E-8L);
  EXPECT_EQ(emissions[9U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_333_39_keV
  EXPECT_EQ(emissions[10U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[10U].GetYieldPerDecay(), 4.195E-8L);
  EXPECT_EQ(emissions[10U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_476_39_keV
  EXPECT_EQ(emissions[11U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[11U].GetYieldPerDecay(), 3.910E-10L);
  EXPECT_EQ(emissions[11U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_630_32_keV
  EXPECT_EQ(emissions[12U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[12U].GetYieldPerDecay(), 0.0000039495L);
  EXPECT_EQ(emissions[12U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_767_478_keV
  EXPECT_EQ(emissions[13U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[13U].GetYieldPerDecay(), 0.0000028328L);
  EXPECT_EQ(emissions[13U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_773_32_keV
  EXPECT_EQ(emissions[14U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[14U].GetYieldPerDecay(), 4.84E-9L);
  EXPECT_EQ(emissions[14U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_NEAR(definition.GetTotalYieldPerDecay(), 1.3067965557710L, 1.0e-14L);
}

TEST(GGEMSRe186Test, PreservesBetaMinus159Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 319U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 498'432'000ULL);
  EXPECT_EQ(energies.front() - 249'216'000ULL, 192'000ULL);
  EXPECT_EQ(energies.back() + 249'216'000ULL, 159'000'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 319U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 498'432'000ULL));
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
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 48.442440725701353L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 48.442440725701353L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 79U) {
      EXPECT_NEAR(cumulative, 0.46549310161517626L, 1.0e-12L);
    }
    if (index + 1U == 159U) {
      EXPECT_NEAR(cumulative, 0.79767373694795096L, 1.0e-12L);
    }
    if (index + 1U == 239U) {
      EXPECT_NEAR(cumulative, 0.96767737943414434L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe186Test, PreservesBetaMinus302Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 605U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'172'000ULL);
  EXPECT_EQ(energies.front() - 249'586'000ULL, 940'000ULL);
  EXPECT_EQ(energies.back() + 249'586'000ULL, 302'000'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 605U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'172'000ULL));
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
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 84.259923309014752L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 84.259923309014752L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 151U) {
      EXPECT_NEAR(cumulative, 0.517001649977475L, 1.0e-12L);
    }
    if (index + 1U == 302U) {
      EXPECT_NEAR(cumulative, 0.83651358740895865L, 1.0e-12L);
    }
    if (index + 1U == 453U) {
      EXPECT_NEAR(cumulative, 0.97673191233803724L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe186Test, PreservesBetaMinus9323Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 1865U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'892'000ULL);
  EXPECT_EQ(energies.front() - 249'946'000ULL, 1'420'000ULL);
  EXPECT_EQ(energies.back() + 249'946'000ULL, 932'300'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 1865U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'892'000ULL));
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
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 308.2248847697017L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 308.2248847697017L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 466U) {
      EXPECT_NEAR(cumulative, 0.41451205205882002L, 1.0e-12L);
    }
    if (index + 1U == 932U) {
      EXPECT_NEAR(cumulative, 0.76669005296577319L, 1.0e-12L);
    }
    if (index + 1U == 1398U) {
      EXPECT_NEAR(cumulative, 0.96153199172541515L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe186Test, PreservesBetaMinus10695Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 2140U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'766'000ULL);
  EXPECT_EQ(energies.front() - 249'883'000ULL, 760'000ULL);
  EXPECT_EQ(energies.back() + 249'883'000ULL, 1'069'500'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 2140U);
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
    EXPECT_EQ(energies[index],
              energies.front() +
                (static_cast<std::uint64_t>(index) * 499'766'000ULL));
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
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  EXPECT_NEAR(moment / weight_sum / keV, 361.45083520905683L, 0.005L);
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 361.45083520905683L,
              0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 535U) {
      EXPECT_NEAR(cumulative, 0.4002780585134153L, 1.0e-12L);
    }
    if (index + 1U == 1070U) {
      EXPECT_NEAR(cumulative, 0.75668286952002239L, 1.0e-12L);
    }
    if (index + 1U == 1605U) {
      EXPECT_NEAR(cumulative, 0.95935072476524674L, 1.0e-12L);
    }
  }
}

TEST(GGEMSRe186Test, PreservesNuclearGamma) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 9U> expected_energies{
    122'330'000'000ULL, 137'157'000'000ULL, 143'000'000'000ULL,
    296'930'000'000ULL, 333'390'000'000ULL, 476'390'000'000ULL,
    630'320'000'000ULL, 767'478'000'000ULL, 773'320'000'000ULL,
  };
  constexpr std::array<double, 9U> expected_weights{
    0.00603, 0.0942,   7.4e-09,  5.3e-07, 6.2e-07,
    1.5e-08, 0.000293, 0.000327, 2.2e-07,
  };
  ASSERT_EQ(energies.size(), 9U);
  EXPECT_EQ(distribution.GetTableCount(), 9U);
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
  EXPECT_NEAR(weight_sum, 0.1008513924L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 139.75048276229847L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 139.75048276229847L,
              1.3651337864100933e-06L);
}

TEST(GGEMSRe186Test, PreservesAtomicXray) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 10U> expected_energies{
    9'532'100'000ULL,  10'160'850'000ULL, 57'982'300'000ULL, 59'318'900'000ULL,
    61'487'300'000ULL, 63'001'100'000ULL, 67'287'000'000ULL, 69'270'700'000ULL,
    71'449'000'000ULL, 73'584'300'000ULL,
  };
  constexpr std::array<double, 10U> expected_weights{
    0.02,   0.0299, 0.01736, 0.0302, 0.0113,
    0.0194, 0.01,   0.00274, 0.0065, 0.00182,
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
  EXPECT_NEAR(weight_sum, 0.14922L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 44.70242860876558L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 44.70242860876558L,
              1.5013314953446389e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion12233Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[6U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    52'800'000'000ULL,  110'230'000'000ULL, 110'790'000'000ULL,
    112'120'000'000ULL, 120'060'000'000ULL, 122'030'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.00353, 0.000377, 0.00283, 0.00238, 0.001411, 0.000393,
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
  EXPECT_NEAR(weight_sum, 0.010921L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 93.918547752037355L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 93.918547752037355L,
              9.7713192760944361e-08L);
}

TEST(GGEMSRe186Test, PreservesConversion137157Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[7U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    63'286'000'000ULL,  124'189'000'000ULL, 124'772'000'000ULL,
    126'286'000'000ULL, 134'699'000'000ULL, 136'823'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    0.0408, 0.00462, 0.0316, 0.0246, 0.01552, 0.00443,
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
  EXPECT_NEAR(weight_sum, 0.12157L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 106.12739286008062L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 106.12739286008062L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion143Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[8U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    69'129'000'000ULL,  130'032'000'000ULL, 130'615'000'000ULL,
    132'129'000'000ULL, 140'542'000'000ULL, 142'666'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    1.03e-08, 1.5e-09, 6.7e-10, 4.3e-10, 6.2e-10, 1.8e-10,
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
  EXPECT_NEAR(weight_sum, 1.370E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 84.979572262773729L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 84.979572262773729L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion29693Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[9U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    223'062'000'000ULL, 283'965'000'000ULL, 284'548'000'000ULL,
    286'062'000'000ULL, 294'475'000'000ULL, 296'599'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.2e-08, 4.1e-09, 6.2e-09, 3.5e-09, 3.4e-09, 9.9e-10,
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
  EXPECT_NEAR(weight_sum, 5.019E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 246.3140687387926L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 246.3140687387926L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion33339Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[10U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    259'519'000'000ULL, 320'422'000'000ULL, 321'005'000'000ULL,
    322'519'000'000ULL, 330'932'000'000ULL, 333'056'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.8e-08, 3.7e-09, 4.5e-09, 2.4e-09, 2.6e-09, 7.5e-10,
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
  EXPECT_NEAR(weight_sum, 4.195E-8L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 280.83138736591178L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 280.83138736591178L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion47639Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[11U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    402'519'000'000ULL, 463'422'000'000ULL, 464'005'000'000ULL,
    465'519'000'000ULL, 473'932'000'000ULL, 476'056'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.9e-10, 3.9e-11, 2.6e-11, 1.16e-11, 1.9e-11, 5.4e-12,
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
  EXPECT_NEAR(weight_sum, 3.910E-10L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 419.03715805626598L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 419.03715805626598L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion63032Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[12U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    556'452'000'000ULL, 617'355'000'000ULL, 617'938'000'000ULL,
    619'452'000'000ULL, 627'865'000'000ULL, 629'989'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    3.08e-06, 4.19e-07, 1.76e-07, 7.03e-08, 1.58e-07, 4.62e-08,
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
  EXPECT_NEAR(weight_sum, 0.0000039495L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 570.49161422964937L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 570.49161422964937L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion767478Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[13U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    693'609'000'000ULL, 754'512'000'000ULL, 755'095'000'000ULL,
    756'609'000'000ULL, 765'022'000'000ULL, 767'146'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.26e-06, 3.09e-07, 9.55e-08, 3.49e-08, 1.03e-07, 3.04e-08,
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
  EXPECT_NEAR(weight_sum, 0.0000028328L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 706.48696660547864L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 706.48696660547864L,
              1.0373000225424766e-07L);
}

TEST(GGEMSRe186Test, PreservesConversion77332Kev) {
  auto const definition = BuildRe186Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[14U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 2U> expected_energies{
    699'452'000'000ULL,
    761'248'000'000ULL,
  };
  constexpr std::array<double, 2U> expected_weights{
    4.2e-09,
    6.4e-10,
  };
  ASSERT_EQ(energies.size(), 2U);
  EXPECT_EQ(distribution.GetTableCount(), 2U);
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
  EXPECT_NEAR(weight_sum, 4.84E-9L, 1.0e-14L);
  EXPECT_NEAR(moment / weight_sum / keV, 707.62337190082644L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  EXPECT_NEAR(ticket_moment / 4'294'967'296.0L / keV, 707.62337190082644L,
              2.9776004910469054e-08L);
}

} // namespace
