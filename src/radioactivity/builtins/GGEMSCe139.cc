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
 * \brief Defines the compiled Ce-139 marginal source-emission laws.
 *
 * LNHB: LNHB/INEEL, 2008. Selected EC source emissions.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Ce-139/reference/.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

/*! \brief Evaluated Ce-139 parent half-life in seconds. */
constexpr long double k_half_life_seconds{11892182.400L};

// =============================================================================
// =============================================================================

/*! \brief nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.799L};

/*! \brief Selected monoenergy in canonical integer micro-eV. */
constexpr std::uint64_t k_nuclear_gamma_energy_micro_eV{165'857'500'000ULL};

// =============================================================================
// =============================================================================

/*! \brief atomic xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.9252L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    5'094'500'000ULL,
    33'034'400'000ULL,
    33'442'100'000ULL,
    37'868'700'000ULL,
    38'822'800'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.1219,
    0.228,
    0.419,
    0.1247,
    0.0316,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 165 8575 keV yield per parent decay. */
constexpr long double k_conversion_165_8575_keV_yield{0.199292L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_165_8575_keV_energies_micro_eV{
    {
      126'933'000'000ULL,
      159'591'300'000ULL,
      159'967'000'000ULL,
      160'374'900'000ULL,
      164'783'700'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_165_8575_keV_line_yields{
  {
    0.1715,
    0.02125,
    0.00144,
    0.000312,
    0.00479,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCe139Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(3U);

  emissions.emplace_back(particles::GGEMSParticleType::Gamma,
                         k_nuclear_gamma_yield,
                         sources::GGEMSEnergyDistribution::BuildMono(
                           k_nuclear_gamma_energy_micro_eV));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_165_8575_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_165_8575_keV_energies_micro_eV,
                           k_conversion_165_8575_keV_line_yields));

  return {"Ce-139", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
