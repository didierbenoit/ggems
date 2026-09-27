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
 * \brief Implements physical particle and emission-yield admission.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <utility>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Recognizes particle kinds admitted for radioactive emission.
 *
 * \param[in] particle_type Candidate particle kind.
 * \return True for Gamma, Electron, Positron, Proton, Neutron, or Alpha.
 */
[[nodiscard]] constexpr auto
IsPhysicalParticleType(particles::GGEMSParticleType particle_type) noexcept
  -> bool {
  switch (particle_type) {
  case particles::GGEMSParticleType::Gamma:
  case particles::GGEMSParticleType::Electron:
  case particles::GGEMSParticleType::Positron:
  case particles::GGEMSParticleType::Proton:
  case particles::GGEMSParticleType::Neutron:
  case particles::GGEMSParticleType::Alpha:
    return true;
  case particles::GGEMSParticleType::Unknown:
  case particles::GGEMSParticleType::Aionino:
    return false;
  }

  return false;
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSRadionuclideEmission::GGEMSRadionuclideEmission(
  particles::GGEMSParticleType particle_type, long double yield_per_decay,
  sources::GGEMSEnergyDistribution energy_distribution)
    : particle_type_{particle_type}, yield_per_decay_{yield_per_decay},
      energy_distribution_{std::move(energy_distribution)} {
  if (!IsPhysicalParticleType(particle_type_)) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide emission particle type must be a physical particle.");
  }

  if (!(yield_per_decay_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide emission yield must be strictly positive.");
  }
}

} // namespace ggems::core::radioactivity
