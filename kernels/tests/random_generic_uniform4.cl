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

#include "random/GGEMSRandom.clh"

__kernel void random_generic_uniform4(__global GGEMSRandomState *states,
                                      __global float *values,
                                      uint particle_count,
                                      uint blocks_per_particle) {
  uint const particle_index = (uint)get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  for (uint block_index = 0U; block_index < blocks_per_particle;
       ++block_index) {
    float4 const random_values = GGEMS_RndmUniform4(states, particle_index);

    size_t const output_block =
      (size_t)particle_index * blocks_per_particle + block_index;

    vstore4(random_values, output_block, values);
  }
}
