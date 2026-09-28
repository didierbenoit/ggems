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

#include "particles/GGEMSParticleState.clh"

__kernel void particle_dummy_kill_alive(__global GGEMSParticleState *particles,
                                        __global uint *active_count,
                                        uint particle_count) {
  uint particle_index = get_global_id(0);

  if (particle_index == 0U) {
    active_count[0] = 0U;
  }

  if (particle_index >= particle_count) {
    return;
  }

  GGEMSParticleState particle = particles[particle_index];

  if (particle.status == GGEMS_PARTICLE_STATUS_ALIVE) {
    particle.status = GGEMS_PARTICLE_STATUS_KILLED;
    particle.time_ps += 1UL;
    particle.flags |= 1U;
  }

  particles[particle_index] = particle;
}
