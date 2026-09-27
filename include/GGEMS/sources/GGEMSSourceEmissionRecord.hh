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
 * \brief Defines the shared radioactive emission descriptor.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::sources {

/*! \brief Describes one immutable radioactive emission for host/device use. */
struct GGEMSSourceEmissionRecord {
  /*! \brief Device particle-kind encoding for this emission population. */
  std::uint32_t particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};

  /*! \brief Index in the configuration snapshot energy-descriptor array. */
  std::uint32_t energy_distribution_record_index{0U};

  /*! \brief Monoenergetic emission energy in micro-eV; zero for a table. */
  std::uint64_t mono_energy_micro_eV{0ULL};
};

} // namespace ggems::core::sources
