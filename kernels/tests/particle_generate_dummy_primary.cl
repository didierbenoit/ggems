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

__kernel void particle_generate_dummy_primary(
  __global GGEMSParticleState *particles, ulong global_particle_offset,
  uint particle_count, uint particle_type, ulong energy_micro_eV) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  ulong global_particle_id = global_particle_offset + (ulong)(particle_index);

  GGEMSParticleState particle;

  particle.global_particle_id = global_particle_id;
  particle.track_id = global_particle_id;
  particle.parent_track_id = GGEMS_INVALID_ID_U64;
  particle.time_ps = 0UL;

  particle.position_x_pm = 0L;
  particle.position_y_pm = 0L;
  particle.position_z_pm = 0L;

  particle.particle_type = particle_type;
  particle.status = GGEMS_PARTICLE_STATUS_ALIVE;
  particle.generation = 0U;
  particle.flags = 0U;

  particle.current_navigator_id = GGEMS_INVALID_ID_U32;
  particle.current_volume_id = GGEMS_INVALID_ID_U32;
  particle.material_id = GGEMS_INVALID_ID_U32;
  particle.region_id = GGEMS_INVALID_ID_U32;

  particle.direction_x = 0.0f;
  particle.direction_y = 0.0f;
  particle.direction_z = 1.0f;
  particle.direction_w = 0.0f;

  particle.energy_micro_eV = energy_micro_eV;

  particles[particle_index] = particle;
}
