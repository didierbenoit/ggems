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
 * \brief Tests the independently selected Zr-89 source contracts.
 *
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
using ggems::core::radioactivity::builtins::BuildZr89Radionuclide;
using ggems::core::sources::GGEMSEnergyDistributionType;

TEST(GGEMSZr89Test, PreservesIdentityAndOrderedMarginalYields) {
  auto const definition = BuildZr89Radionuclide();
  EXPECT_EQ(definition.GetCanonicalName(), "Zr-89");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 282312.00L);
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);
  // Selected absolute LNHB values; these are independent marginal yields.
  // beta_plus_901_8_keV
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.228L);
  EXPECT_EQ(emissions[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  // nuclear_gamma
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.01048L);
  EXPECT_EQ(emissions[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  // conversion_1744_72_keV
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 2.39377E-7L);
  EXPECT_EQ(emissions[2U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  GGEMS_EXPECT_NEAR_LD(definition.GetTotalYieldPerDecay(), 0.238480239377L,
                       1.0e-14L);
}

TEST(GGEMSZr89Test, PreservesBetaPlus9018Kev) {
  auto const definition = BuildZr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[0U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(energies.size(), 902U);
  EXPECT_EQ(distribution.GetRegularBinWidthMicroElectronVolt(), 999'778'000ULL);
  EXPECT_EQ(energies.front() - 499'889'000ULL, 244'000ULL);
  EXPECT_EQ(energies.back() + 499'889'000ULL, 901'800'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 902U);
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
                (static_cast<std::uint64_t>(index) * 999'778'000ULL));
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
  // Independent 60-digit full-support integral; existing 0.005 keV budget.
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 392.12786228674531L, 0.005L);
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       392.12786228674531L, 0.005L);
  // Independently integrated retained-support cumulative masses.
  long double cumulative{0.0L};
  for (std::size_t index = 0U; index < weights.size(); ++index) {
    cumulative += static_cast<long double>(weights[index]);
    if (index + 1U == 225U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.19768885049939536L, 1.0e-12L);
    }
    if (index + 1U == 451U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.63145862523980389L, 1.0e-12L);
    }
    if (index + 1U == 676U) {
      GGEMS_EXPECT_NEAR_LD(cumulative, 0.93337504367091795L, 1.0e-12L);
    }
  }
}

TEST(GGEMSZr89Test, PreservesNuclearGamma) {
  auto const definition = BuildZr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[1U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 4U> expected_energies{
    1'620'810'000'000ULL,
    1'657'560'000'000ULL,
    1'713'100'000'000ULL,
    1'744'720'000'000ULL,
  };
  constexpr std::array<double, 4U> expected_weights{
    0.00074,
    0.00106,
    0.00745,
    0.00123,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 0.01048L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1704.6768702290076L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1704.6768702290076L, 1.1640018022060394e-07L);
}

TEST(GGEMSZr89Test, PreservesConversion174472Kev) {
  auto const definition = BuildZr89Radionuclide();
  auto const &distribution =
    definition.GetEmissions()[2U].GetEnergyDistribution();
  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const tickets = distribution.GetCumulativeTicketUpperBounds();
  // Independently selected LNHB line energies and absolute intensities.
  constexpr std::array<std::uint64_t, 6U> expected_energies{
    1'727'700'000'000ULL, 1'742'370'000'000ULL, 1'742'580'000'000ULL,
    1'742'660'000'000ULL, 1'744'480'000'000ULL, 1'744'720'000'000ULL,
  };
  constexpr std::array<double, 6U> expected_weights{
    2.12e-07, 2.23e-08, 3e-10, 3.15e-10, 3.9e-09, 5.62e-10,
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
  GGEMS_EXPECT_NEAR_LD(weight_sum, 2.39377E-7L, 1.0e-14L);
  GGEMS_EXPECT_NEAR_LD(moment / weight_sum / keV, 1729.4183131211437L, 1.0e-9L);
  // At most one ticket of error per line from largest-remainder allocation.
  GGEMS_EXPECT_NEAR_LD(ticket_moment / 4'294'967'296.0L / keV,
                       1729.4183131211437L, 2.4776665329933167e-08L);
}

TEST(GGEMSZr89Test, ExcludesUnsupportedDiscreteEmissions) {
  auto const definition = BuildZr89Radionuclide();
  for (auto const &emission : definition.GetEmissions()) {
    auto const &distribution = emission.GetEnergyDistribution();
    // unseparated_atomic_2_0165
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  2'016'500'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 2'016'500'000ULL);
        }
      }
    }
    // unseparated_atomic_14_8829
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  14'882'900'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 14'882'900'000ULL);
        }
      }
    }
    // unseparated_atomic_14_9585
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  14'958'500'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 14'958'500'000ULL);
        }
      }
    }
    // unseparated_atomic_16_7813
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  16'781'300'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 16'781'300'000ULL);
        }
      }
    }
    // unseparated_atomic_17_0259
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  17'025'900'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 17'025'900'000ULL);
        }
      }
    }
    // annihilation_photons
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  511'000'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 511'000'000'000ULL);
        }
      }
    }
    // delayed_Y89m_gamma
    if (emission.GetParticleType() == GGEMSParticleType::Gamma) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  908'970'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 908'970'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_891_932
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  891'932'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 891'932'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_906_598
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  906'598'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 906'598'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_906_815
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  906'815'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 906'815'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_906_89
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  906'890'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 906'890'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_908_705
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  908'705'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 908'705'000'000ULL);
        }
      }
    }
    // delayed_Y89m_conversion_908_95
    if (emission.GetParticleType() == GGEMSParticleType::Electron) {
      if (distribution.GetType() == GGEMSEnergyDistributionType::Mono) {
        EXPECT_NE(distribution.GetMonoEnergyMicroElectronVolt(),
                  908'950'000'000ULL);
      } else if (distribution.GetType() ==
                 GGEMSEnergyDistributionType::DiscreteLines) {
        for (auto energy : distribution.GetEnergyValuesMicroElectronVolt()) {
          EXPECT_NE(energy, 908'950'000'000ULL);
        }
      }
    }
  }
}

} // namespace
