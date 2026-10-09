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
 * \brief Defines the compiled Sn-113 marginal source-emission laws.
 *
 * LNHB: INEEL, 2004. Selected EC source emissions.
 *
 * Delayed daughter-isomer emissions and inseparable prompt/delayed totals
 * are excluded.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Sn-113/reference/.
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

/*! \brief Evaluated Sn-113 parent half-life in seconds. */
constexpr long double k_half_life_seconds{9943776.00L};

// =============================================================================
// =============================================================================

/*! \brief nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.02111034L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_nuclear_gamma_energies_micro_eV{
  {
    255'134'000'000ULL,
    382'900'000'000ULL,
    638'030'000'000ULL,
    646'830'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_nuclear_gamma_line_yields{
  {
    0.0211,
    6E-7,
    0.0000097,
    4E-8,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 255 134 keV yield per parent decay. */
constexpr long double k_conversion_255_134_keV_yield{0.00097945L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_255_134_keV_energies_micro_eV{
    {
      227'194'000'000ULL,
      250'896'000'000ULL,
      251'196'000'000ULL,
      251'404'000'000ULL,
      254'517'000'000ULL,
      255'072'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_255_134_keV_line_yields{
  {
    0.000836,
    0.0000931,
    0.0000128,
    0.00001002,
    0.0000227,
    0.00000483,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildSn113Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(2U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_255_134_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_255_134_keV_energies_micro_eV,
                           k_conversion_255_134_keV_line_yields));

  return {"Sn-113", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
