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
 * \brief Defines a particle kind, yield, and energy law for one emission group.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {

/*! \brief Owns one marginal particle emission law per parent decay. */
class GGEMSRadionuclideEmission {
public:
  /*!
   * \brief Constructs one independent marginal emission population.
   * \param[in] particle_type Physical particle kind; Unknown and Aionino are
   * rejected.
   * \param[in] yield_per_decay Positive finite expected particles per parent
   * decay.
   * \param[in] energy_distribution Owned conditional energy law.
   * \throws GGEMSRecoverable If particle kind is not physical or yield fails
   * the positive comparison. Finite yield is a caller precondition.
   */
  GGEMSRadionuclideEmission(
    particles::GGEMSParticleType particle_type, long double yield_per_decay,
    sources::GGEMSEnergyDistribution energy_distribution);

  /*!
   * \brief Reads the physical particle kind of this group.
   *
   * \return Configured emitted particle kind.
   */
  [[nodiscard]] auto GetParticleType() const noexcept
    -> particles::GGEMSParticleType {
    return particle_type_;
  }

  /*!
   * \brief Reads the marginal group yield.
   *
   * \return Expected particles per parent decay; not a categorical probability.
   */
  [[nodiscard]] auto GetYieldPerDecay() const noexcept -> long double {
    return yield_per_decay_;
  }

  /*!
   * \brief Borrows the energy law conditional on this group.
   *
   * \return Reference valid until this emission is assigned or destroyed.
   */
  [[nodiscard]] auto GetEnergyDistribution() const noexcept
    -> sources::GGEMSEnergyDistribution const & {
    return energy_distribution_;
  }

private:
  /*! \brief Physical particle kind emitted by this group. */
  particles::GGEMSParticleType particle_type_;

  /*! \brief Expected particles of this group per parent decay. */
  long double yield_per_decay_;

  /*! \brief Owned energy law conditional on this emission group. */
  sources::GGEMSEnergyDistribution energy_distribution_;
};

} // namespace ggems::core::radioactivity
