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
 * \brief Tests the independently selected Tb-161 source contracts.
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
using ggems::core::radioactivity::builtins::BuildTb161Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildTb161Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Tb-161");
  // Evaluated 6.89 d, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 595296.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);
  // Independent selected reference values; see data/Tb-161/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_minus_aggregate
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 1.0L, 1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.5289922536L,
                       1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 12.948165964509L,
                       1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // auger_electron
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 10.963662513L,
                       1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_electron
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 1.419970563955529L,
                       1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_electron_weak_1
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 1.19259E-10L,
                       1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 26.860791295183788L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesBetaMinusAggregate) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 593.1000 keV
  // endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 499'662'000ULL);
  ASSERT_EQ(energies.size(), 1187U);
  EXPECT_EQ(distribution.GetTableCount(), 1187U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 249'831'000ULL, 1'206'000ULL);
  EXPECT_EQ(energies.back() + 249'831'000ULL, 593'100'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 499'662'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 154.85611172631553L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       154.85611172631553L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 296U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.53493743005842153L, 1.0e-12L);
    }
    if (index + 1U == 593U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.87635948041188180L, 1.0e-12L);
    }
    if (index + 1U == 890U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.99275734479452942L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesNuclearGamma) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Large families: boundary/central witnesses and independent total/mean;
  // deterministic reference validation separately compares every line.
  ASSERT_EQ(energies.size(), 34U);
  EXPECT_EQ(distribution.GetTableCount(), 34U);
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
      EXPECT_EQ(energies[index], 18'150'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.000203058);
    }
    if (index == 17U) {
      EXPECT_EQ(energies[index], 138'300'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.000007956);
    }
    if (index == 33U) {
      EXPECT_EQ(energies[index], 550'249'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.0003621);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.5289922536L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 45.317330556690783L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       45.317330556690783L, 0.0000042123243903577328L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesAtomicXray) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
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
      EXPECT_EQ(energies[index], 13'770'600ULL);
      EXPECT_DOUBLE_EQ(weights[index], 12.4748);
    }
    if (index == 26U) {
      EXPECT_EQ(energies[index], 7'752'910'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.00000133561);
    }
    if (index == 52U) {
      EXPECT_EQ(energies[index], 53'832'900'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.00120281);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 12.948165964509L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.96942139919280685L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.96942139919280685L, 6.6422935457192361e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesAugerElectron) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 15U> expected_energies{
    {
      18'168'600ULL,
      42'970'000ULL,
      128'230'000ULL,
      161'806'000ULL,
      212'183'000ULL,
      684'692'000ULL,
      1'021'550'000ULL,
      1'204'940'000ULL,
      1'395'930'000ULL,
      5'250'450'000ULL,
      6'527'700'000ULL,
      7'845'060'000ULL,
      37'064'000'000ULL,
      43'972'700'000ULL,
      50'888'700'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 15U> expected_weights{
    {
      0.715769,
      0.00557095,
      5.71555,
      0.288891,
      0.999455,
      0.378751,
      1.83604,
      0.101328,
      0.00464869,
      0.651571,
      0.230873,
      0.0206148,
      0.00935738,
      0.0046807,
      0.000561993,
    },
  };

  ASSERT_EQ(energies.size(), 15U);
  EXPECT_EQ(distribution.GetTableCount(), 15U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 10.963662513L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.81538057194452607L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.81538057194452607L, 1.7776327853314579e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesConversionElectron) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Large families: boundary/central witnesses and independent total/mean;
  // deterministic reference validation separately compares every line.
  ASSERT_EQ(energies.size(), 198U);
  EXPECT_EQ(distribution.GetTableCount(), 198U);
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
      EXPECT_EQ(energies[index], 3'330'700'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.182539);
    }
    if (index == 99U) {
      EXPECT_EQ(energies[index], 130'519'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 9.22536E-7);
    }
    if (index == 197U) {
      EXPECT_EQ(energies[index], 550'249'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 4.80345E-8);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 1.419970563955529L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 27.667017061175739L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       27.667017061175739L, 0.000025213289283385873L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, PreservesConversionElectronWeak1) {
  auto const definition = BuildTb161Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  // Selected absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 212'800'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTb161Test, UsesOneAggregateContinuousBetaLaw) {
  auto const definition = BuildTb161Radionuclide();
  auto const emissions = definition.GetEmissions();
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
  std::size_t continuous_count{0U};
  for (auto const &emission : emissions) {
    if (emission.GetEnergyDistribution().GetType() ==
        GGEMSEnergyDistributionType::RegularSpectrum) {
      ++continuous_count;
    }
  }
  EXPECT_EQ(continuous_count, 1U);
}

} // namespace
