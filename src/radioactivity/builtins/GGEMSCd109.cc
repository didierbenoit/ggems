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
 * \brief Defines the compiled Cd-109 marginal source-emission laws.
 *
 * LNHB: M.-M. Be and E. Schonfeld, LNHB/PTB, 2014. Selected EC source
 * emissions.
 *
 * Delayed daughter-isomer emissions and inseparable prompt/delayed totals
 * are excluded.
 *
 * Prompt capture K X rays use the evaluated P_K times omega_K and relative
 * line probabilities; mixed atomic totals are not selected.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Cd-109/reference/.
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

/*! \brief Evaluated Cd-109 parent half-life in seconds. */
constexpr long double k_half_life_seconds{39908160.0L};

// =============================================================================
// =============================================================================

/*! \brief atomic xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.6747720000000000000000000000L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_atomic_xray_energies_micro_eV{
  {
    21'990'600'000ULL,
    22'163'170'000ULL,
    25'000'200'000ULL,
    25'484'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_atomic_xray_line_yields{
  {
    0.1929010863824971708789136175,
    0.3636212749905695963787250094,
    0.1007230931723877781969068276,
    0.01752654545454545454545454545,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCd109Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(1U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  return {"Cd-109", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
