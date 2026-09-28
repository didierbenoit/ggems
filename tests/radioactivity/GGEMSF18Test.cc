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

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, BuildsExactIdentity) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "F-18");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 6'584.04L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, UsesDeclaredThreeGroupSourceModel) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();

  auto const emissions = definition.GetEmissions();

  // The known LNHB K-Auger marginal awaits a supported conditional energy law.
  ASSERT_EQ(emissions.size(), 3U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.9686L);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.00229L);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.00020L);

  constexpr long double k_expected_total_yield{0.97109L};
  long double const tolerance =
    std::numeric_limits<long double>::epsilon() * k_expected_total_yield * 8.0L;
  EXPECT_LE(
    std::abs(definition.GetTotalYieldPerDecay() - k_expected_total_yield),
    tolerance);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, PreservesExactEnergyDistributionsAndTabulatedSpectrum) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);

  auto const &positron_energy = emissions[0U].GetEnergyDistribution();

  EXPECT_EQ(positron_energy.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(positron_energy.GetTableCount(), 1268U);
  EXPECT_EQ(positron_energy.GetRegularBinWidthMicroElectronVolt(),
            499'920'000ULL);

  auto const centers = positron_energy.GetEnergyValuesMicroElectronVolt();
  auto const weights = positron_energy.GetRelativeWeights();
  auto const tickets = positron_energy.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(centers.size(), 1268U);
  ASSERT_EQ(weights.size(), centers.size());
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 249'960'000ULL, 1'440'000ULL);
  EXPECT_EQ(centers.back() + 249'960'000ULL, 633'900'000'000ULL);

  long double weight_sum{0.0L};
  long double weighted_center_sum{0.0L};
  std::uint64_t previous_ticket{0ULL};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    double const weight = weights[index];
    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_GT(weight, 0.0);
    weight_sum += static_cast<long double>(weight);
    weighted_center_sum += static_cast<long double>(weight) *
                           static_cast<long double>(centers[index]);

    EXPECT_GT(tickets[index], previous_ticket);
    previous_ticket = tickets[index];
  }

  EXPECT_NEAR(static_cast<double>(weight_sum), 1.0, 1.0e-12);
  EXPECT_EQ(previous_ticket, 4'294'967'296ULL);

  long double const mean_energy_keV =
    weighted_center_sum / weight_sum /
    static_cast<long double>(ggems::units::operator""_keV(1ULL).value);
  EXPECT_NEAR(static_cast<double>(mean_energy_keV), 250.50, 0.01);

  auto const &electron_energy = emissions[1U].GetEnergyDistribution();
  EXPECT_EQ(electron_energy.GetType(), GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(electron_energy.GetMonoEnergyMicroElectronVolt(), 14'300'000ULL);

  auto const &gamma_energy = emissions[2U].GetEnergyDistribution();
  EXPECT_EQ(gamma_energy.GetType(), GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(gamma_energy.GetMonoEnergyMicroElectronVolt(), 525'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, OmitsAnnihilationPhotonsAndPlaceholders) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);

  std::size_t oxygen_x_ray_count{0U};

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Aionino);

    auto const &energy = emission.GetEnergyDistribution();

    if (energy.GetType() != GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Electron);
      continue;
    }

    std::uint64_t const mono_energy = energy.GetMonoEnergyMicroElectronVolt();
    EXPECT_FALSE(emission.GetParticleType() == GGEMSParticleType::Gamma &&
                 mono_energy == 511'000'000'000ULL);

    if (emission.GetParticleType() == GGEMSParticleType::Gamma &&
        mono_energy == 525'000'000ULL) {
      ++oxygen_x_ray_count;
    }
  }
  EXPECT_EQ(oxygen_x_ray_count, 1U);
}

} // namespace
