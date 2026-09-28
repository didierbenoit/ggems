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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoEmission(GGEMSParticleType particle_type,
                                    long double yield_per_decay,
                                    std::uint64_t energy_micro_eV)
  -> GGEMSRadionuclideEmission {
  return {particle_type, yield_per_decay,
          GGEMSEnergyDistribution::BuildMono(energy_micro_eV)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDiscreteEmission(GGEMSParticleType particle_type,
                                        long double yield_per_decay)
  -> GGEMSRadionuclideEmission {
  constexpr std::array<double, 3U> energies{4.0, 5.0, 6.0};
  constexpr std::array<double, 3U> weights{2.0, 3.0, 1.0};
  return {
    particle_type, yield_per_decay,
    GGEMSEnergyDistribution::BuildDiscreteLines(energies, weights, "MeV")};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSpectrumEmission(GGEMSParticleType particle_type,
                                        long double yield_per_decay)
  -> GGEMSRadionuclideEmission {
  constexpr std::array<double, 3U> centers{100.0, 102.0, 104.0};
  constexpr std::array<double, 3U> weights{1.0, 4.0, 1.0};
  return {
    particle_type, yield_per_decay,
    GGEMSEnergyDistribution::BuildRegularSpectrum(centers, weights, "keV")};
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest, OwnsEmissionsAndTotalYield) {
  std::string canonical_name{"Synthetic-Mixed"};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeDiscreteEmission(GGEMSParticleType::Alpha, 1.0L));
  emissions.push_back(
    MakeMonoEmission(GGEMSParticleType::Gamma, 0.3592L, 59'000'000'000ULL));
  emissions.push_back(
    MakeSpectrumEmission(GGEMSParticleType::Electron, 1.675L));

  GGEMSRadionuclideDefinition const definition{canonical_name, 4321.25L,
                                               emissions};

  canonical_name.clear();
  emissions.clear();

  EXPECT_EQ(definition.GetCanonicalName(), "Synthetic-Mixed");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 4321.25L);

  auto const stored_emissions = definition.GetEmissions();
  ASSERT_EQ(stored_emissions.size(), 3U);

  EXPECT_EQ(stored_emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_EQ(stored_emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(stored_emissions[2U].GetParticleType(),
            GGEMSParticleType::Electron);
  EXPECT_EQ(stored_emissions[0U].GetYieldPerDecay(), 1.0L);
  EXPECT_EQ(stored_emissions[1U].GetYieldPerDecay(), 0.3592L);
  EXPECT_EQ(stored_emissions[2U].GetYieldPerDecay(), 1.675L);

  long double const expected_total = 1.0L + 0.3592L + 1.675L;
  long double const tolerance =
    std::numeric_limits<long double>::epsilon() * expected_total * 8.0L;
  EXPECT_LE(std::abs(definition.GetTotalYieldPerDecay() - expected_total),
            tolerance);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest, RejectsInvalidNameHalfLifeAndEmissions) {
  std::vector<GGEMSRadionuclideEmission> valid_emissions;
  valid_emissions.push_back(
    MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1'000ULL));

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"", 1.0L, valid_emissions}),
               ggems::core::GGEMSRecoverable);

  for (long double half_life :
       {0.0L, -1.0L, std::numeric_limits<long double>::quiet_NaN(),
        -std::numeric_limits<long double>::infinity()}) {
    SCOPED_TRACE(static_cast<double>(half_life));
    EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic", half_life,
                                                    valid_emissions}),
                 ggems::core::GGEMSRecoverable);
  }

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic", 1.0L, {}}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     PreservesOrderAndDoesNotMergeMatchingParticleTypes) {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeSpectrumEmission(GGEMSParticleType::Electron, 0.75L));
  emissions.push_back(MakeDiscreteEmission(GGEMSParticleType::Electron, 1.25L));

  GGEMSRadionuclideDefinition const definition{"Synthetic-Electrons", 10.0L,
                                               std::move(emissions)};

  auto const stored = definition.GetEmissions();
  ASSERT_EQ(stored.size(), 2U);
  EXPECT_EQ(stored[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(stored[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(stored[0U].GetYieldPerDecay(), 0.75L);
  EXPECT_EQ(stored[1U].GetYieldPerDecay(), 1.25L);
  EXPECT_EQ(stored[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(stored[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     PreservesTinyPositiveYieldWithoutEnergyTicketQuantization) {
  constexpr long double tiny_yield{1.0e-12L};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(
    MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 2'000ULL));
  emissions.push_back(
    MakeMonoEmission(GGEMSParticleType::Electron, tiny_yield, 3'000ULL));

  GGEMSRadionuclideDefinition const definition{"Synthetic-Weak", 100.0L,
                                               std::move(emissions)};

  ASSERT_EQ(definition.GetEmissions().size(), 2U);
  EXPECT_EQ(definition.GetEmissions()[1U].GetYieldPerDecay(), tiny_yield);
  EXPECT_TRUE(definition.GetEmissions()[1U]
                .GetEnergyDistribution()
                .GetCumulativeTicketUpperBounds()
                .empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     FailedConstructionLeavesCallerOwnedInputsUnchanged) {
  std::string canonical_name{"Synthetic-Stable"};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(
    MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1'000ULL));

  EXPECT_THROW(
    ((void)GGEMSRadionuclideDefinition{canonical_name, 0.0L, emissions}),
    ggems::core::GGEMSRecoverable);

  EXPECT_EQ(canonical_name, "Synthetic-Stable");
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
}

} // namespace
