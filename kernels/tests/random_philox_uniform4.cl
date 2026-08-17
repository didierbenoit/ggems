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
 * \brief OpenCL probe for Philox vector uniform sampling.
 *
 * Generates repeated four-value uniform blocks for each logical Philox stream.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "random/GGEMSPhilox.clh"

/// \cond

__kernel void random_philox_uniform4(__global GGEMSPhiloxState *states,
                                     __global float *values,
                                     uint particle_count,
                                     uint blocks_per_particle) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  for (uint block_index = 0U; block_index < blocks_per_particle;
       ++block_index) {
    uint output_index =
        (particle_index * blocks_per_particle + block_index) * 4U;

    float4 random_values = GGEMS_PhiloxUniform4(states, particle_index);

    values[output_index + 0U] = random_values.x;
    values[output_index + 1U] = random_values.y;
    values[output_index + 2U] = random_values.z;
    values[output_index + 3U] = random_values.w;
  }
}
/// \endcond
