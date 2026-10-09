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
 * \brief Tests the independently selected Rb-81 source contracts.
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
using ggems::core::radioactivity::builtins::BuildRb81Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildRb81Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Rb-81");
  // Evaluated 4.571 h, converted exactly to seconds.
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 16455.600L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 6U);
  // Independent selected reference values; see data/Rb-81/reference/README.md.
  // Physical yields are not a categorical probability distribution.
  // beta_plus_aggregate
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  GGEMS_EXPECT_NEAR_LD(emissions[0U].GetYieldPerDecay(), 0.2723788548L,
                       1.0e-16L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[1U].GetYieldPerDecay(), 0.43301408L, 1.0e-16L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[2U].GetYieldPerDecay(), 5.3669557225502L,
                       1.0e-16L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // atomic_xray_weak_1
  EXPECT_EQ(emissions[3U].GetParticleType(), GGEMSParticleType::Gamma);
  GGEMS_EXPECT_NEAR_LD(emissions[3U].GetYieldPerDecay(), 4.64015E-10L,
                       1.0e-16L);
  EXPECT_EQ(emissions[3U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  // auger_electron
  EXPECT_EQ(emissions[4U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[4U].GetYieldPerDecay(), 4.21160990L, 1.0e-16L);
  EXPECT_EQ(emissions[4U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_electron
  EXPECT_EQ(emissions[5U].GetParticleType(), GGEMSParticleType::Electron);
  GGEMS_EXPECT_NEAR_LD(emissions[5U].GetYieldPerDecay(), 0.0028164287889456L,
                       1.0e-16L);
  EXPECT_EQ(emissions[5U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 10.2867749866031606L,
                       1.0e-14L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesBetaPlusAggregate) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Exact historical grid rule applied to the independent 1215.000 keV
  // endpoint.
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(),
            1'998'354'000ULL);
  ASSERT_EQ(energies.size(), 608U);
  EXPECT_EQ(distribution.GetTableCount(), 608U);
  ASSERT_EQ(weights.size(), energies.size());
  ASSERT_EQ(tickets.size(), energies.size());
  EXPECT_EQ(energies.front() - 999'177'000ULL, 768'000ULL);
  EXPECT_EQ(energies.back() + 999'177'000ULL, 1'215'000'000'000ULL);
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
                (static_cast<std::uint64_t>(index) * 1'998'354'000ULL));
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
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 433.32435601322845L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       433.32435601322845L, 0.005L);
  // Independently integrated conditional masses at three bin boundaries.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 152U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.30311257806229066L, 1.0e-12L);
    }
    if (index + 1U == 304U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.78258869258325461L, 1.0e-12L);
    }
    if (index + 1U == 456U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.99127287737604663L, 1.0e-12L);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesNuclearGamma) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Large families: boundary/central witnesses and independent total/mean;
  // deterministic reference validation separately compares every line.
  ASSERT_EQ(energies.size(), 61U);
  EXPECT_EQ(distribution.GetTableCount(), 61U);
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
      EXPECT_EQ(energies[index], 49'570'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.0006728);
    }
    if (index == 30U) {
      EXPECT_EQ(energies[index], 689'900'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.0003016);
    }
    if (index == 60U) {
      EXPECT_EQ(energies[index], 1'874'000'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.0001392);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.43301408L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 518.27087755298858L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       518.27087755298858L, 0.000025911875883287191L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesAtomicXray) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 31U> expected_energies{
    {
      19'381'000ULL,     125'056'000ULL,    174'000'000ULL,
      228'901'000ULL,    1'395'230'000ULL,  1'450'120'000ULL,
      1'458'290'000ULL,  1'466'440'000ULL,  1'521'340'000ULL,
      1'578'050'000ULL,  1'579'420'000ULL,  1'632'950'000ULL,
      1'645'960'000ULL,  1'659'970'000ULL,  1'660'680'000ULL,
      1'687'180'000ULL,  1'695'340'000ULL,  1'700'860'000ULL,
      1'715'580'000ULL,  1'806'950'000ULL,  1'808'320'000ULL,
      1'888'870'000ULL,  1'889'580'000ULL,  12'551'700'000ULL,
      12'606'600'000ULL, 14'064'900'000ULL, 14'073'000'000ULL,
      14'184'600'000ULL, 14'186'000'000ULL, 14'266'600'000ULL,
      14'267'300'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 31U> expected_weights{
    {
      4.94018,       0.000169016,   0.00000238688, 0.000013749,   0.00228041,
      0.000873421,   0.00000768248, 0.00000733419, 0.00000606677, 0.000913814,
      0.00889415,    0.0045142,     0.000084963,   1.00061E-7,    9.4893E-8,
      0.000186689,   0.000332065,   0.0000432297,  9.45062E-8,    0.00000109832,
      0.00000163435, 0.0000175062,  0.0000309432,  0.120808,      0.232955,
      0.016997,      0.0331478,     0.0000495543,  0.0000709997,  0.00149355,
      0.00287317,
    },
  };

  ASSERT_EQ(energies.size(), 31U);
  EXPECT_EQ(distribution.GetTableCount(), 31U);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 5.3669557225502L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.99627725208399770L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.99627725208399770L, 1.0293791669644415e-7L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesAtomicXrayWeak1) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[3U].GetEnergyDistribution();
  // Selected absolute emission inventory; not a compiled-table oracle.
  EXPECT_EQ(distribution.GetMonoEnergyMicroElectronVolt(), 54'900'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesAugerElectron) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[4U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Independent evaluated line energies.
  constexpr std::array<std::uint64_t, 9U> expected_energies{
    {
      56'290'100ULL,
      65'532'300ULL,
      99'937'900ULL,
      1'407'350'000ULL,
      1'512'240'000ULL,
      1'671'140'000ULL,
      10'798'000'000ULL,
      12'337'100'000ULL,
      13'878'600'000ULL,
    },
  };

  // Independent evaluated absolute line yields.
  constexpr std::array<double, 9U> expected_weights{
    {
      2.16,
      0.849417,
      0.161014,
      0.753876,
      0.0544987,
      0.00156238,
      0.172278,
      0.0546791,
      0.00428472,
    },
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 4.21160990L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 0.93399937366062797L,
                       1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       0.93399937366062797L, 2.9064315797202289e-8L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, PreservesConversionElectron) {
  auto const definition = BuildRb81Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[5U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independent MIRD explicit source rows; see the reference README.
  // Large families: boundary/central witnesses and independent total/mean;
  // deterministic reference validation separately compares every line.
  ASSERT_EQ(energies.size(), 312U);
  EXPECT_EQ(distribution.GetTableCount(), 312U);
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
      EXPECT_EQ(energies[index], 35'290'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.000760743);
    }
    if (index == 156U) {
      EXPECT_EQ(energies[index], 668'020'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 0.00000203161);
    }
    if (index == 311U) {
      EXPECT_EQ(energies[index], 1'874'000'000'000ULL);
      EXPECT_DOUBLE_EQ(weights[index], 3.5477E-11);
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.0028164287889456L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 216.49135300777875L, 1.0e-9L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       216.49135300777875L, 0.00013356980623135567L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, ExcludesNonParentSourceLines) {
  auto const definition = BuildRb81Radionuclide();
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
  // delayed_Kr81m_IT_evaluated
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Gamma) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 190'440'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 190'440'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_IT_MIRD
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Gamma) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 190'460'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 190'460'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_176_18
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 176'180'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 176'180'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_188_558
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 188'558'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 188'558'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_188_732
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 188'732'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 188'732'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_188_787
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 188'787'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 188'787'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_190_253
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 190'253'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 190'253'000'000ULL);
      }
    }
  }
  // delayed_Kr81m_conversion_190_46
  for (auto const &emission : definition.GetEmissions()) {
    if (emission.GetParticleType() != GGEMSParticleType::Electron) {
      continue;
    }
    auto const &law = emission.GetEnergyDistribution();
    if (law.GetType() == GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(law.GetMonoEnergyMicroElectronVolt(), 190'460'000'000ULL);
    } else if (law.GetType() == GGEMSEnergyDistributionType::DiscreteLines) {
      for (auto const line_energy : law.GetEnergyValuesMicroElectronVolt()) {
        EXPECT_NE(line_energy, 190'460'000'000ULL);
      }
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRb81Test, UsesOneAggregateContinuousBetaLaw) {
  auto const definition = BuildRb81Radionuclide();
  auto const emissions = definition.GetEmissions();
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.2723788548L);
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
