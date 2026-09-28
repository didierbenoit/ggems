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

typedef struct GGEMSParticleStateAlignmentProbe {
  uchar prefix;
  GGEMSParticleState particle;
} GGEMSParticleStateAlignmentProbe;

__kernel void particle_state_abi_probe(__global ulong *layout,
                                       __global GGEMSParticleState *particles) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSParticleState private_particle;
  __private uchar const *base = (__private uchar const *)&private_particle;

#define GGEMS_WRITE_PARTICLE_OFFSET(INDEX, FIELD)                              \
  layout[INDEX] =                                                              \
    (ulong)((__private uchar const *)&private_particle.FIELD - base)

  layout[0] = (ulong)(sizeof(GGEMSParticleState));
  GGEMS_WRITE_PARTICLE_OFFSET(1, global_particle_id);
  GGEMS_WRITE_PARTICLE_OFFSET(2, track_id);
  GGEMS_WRITE_PARTICLE_OFFSET(3, parent_track_id);
  GGEMS_WRITE_PARTICLE_OFFSET(4, time_ps);
  GGEMS_WRITE_PARTICLE_OFFSET(5, position_x_pm);
  GGEMS_WRITE_PARTICLE_OFFSET(6, position_y_pm);
  GGEMS_WRITE_PARTICLE_OFFSET(7, position_z_pm);
  GGEMS_WRITE_PARTICLE_OFFSET(8, particle_type);
  GGEMS_WRITE_PARTICLE_OFFSET(9, status);
  GGEMS_WRITE_PARTICLE_OFFSET(10, generation);
  GGEMS_WRITE_PARTICLE_OFFSET(11, flags);
  GGEMS_WRITE_PARTICLE_OFFSET(12, current_navigator_id);
  GGEMS_WRITE_PARTICLE_OFFSET(13, current_volume_id);
  GGEMS_WRITE_PARTICLE_OFFSET(14, material_id);
  GGEMS_WRITE_PARTICLE_OFFSET(15, region_id);
  GGEMS_WRITE_PARTICLE_OFFSET(16, direction_x);
  GGEMS_WRITE_PARTICLE_OFFSET(17, direction_y);
  GGEMS_WRITE_PARTICLE_OFFSET(18, direction_z);
  GGEMS_WRITE_PARTICLE_OFFSET(19, direction_w);
  GGEMS_WRITE_PARTICLE_OFFSET(20, energy_micro_eV);

#undef GGEMS_WRITE_PARTICLE_OFFSET

  layout[21] = (ulong)((__global uchar const *)&particles[1] -
                       (__global uchar const *)&particles[0]);

  GGEMSParticleStateAlignmentProbe alignment_probe;
  __private uchar const *alignment_base =
    (__private uchar const *)&alignment_probe;
  layout[22] = (ulong)((__private uchar const *)&alignment_probe.particle -
                       alignment_base);

  particles[1].track_id = 7UL;
  particles[1].direction_w = 0.5f;
  particles[1].energy_micro_eV = 511000000000UL;
}
