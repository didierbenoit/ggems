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

#define GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR 0U

// =============================================================================
// =============================================================================

__kernel void random_uniform24_vector4_stream(__global GGEMSRandomState *states,
                                              __global float *values,
                                              uint worker_count,
                                              uint samples_per_worker,
                                              uint layout, uint lanes_used) {
  uint const worker_index = (uint)get_global_id(0);

  if (worker_index >= worker_count) {
    return;
  }

  bool const worker_major = layout == GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR;
  ulong const output_stride = worker_major ? 1UL : (ulong)worker_count;
  ulong output_index = worker_major
                         ? (ulong)worker_index * (ulong)samples_per_worker
                         : (ulong)worker_index;

  uint const calls_per_worker = samples_per_worker / lanes_used;

  for (uint call_index = 0U; call_index < calls_per_worker; ++call_index) {
    float4 const random_values = GGEMS_RndmUniform4(states, worker_index);

    for (uint lane_index = 0U; lane_index < lanes_used; ++lane_index) {
      values[output_index] = random_values[lane_index];
      output_index += output_stride;
    }
  }
}
