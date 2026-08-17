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
 * \brief OpenCL probe for host/kernel random-stream agreement.
 *
 * Produces raw and uniform samples from paired random states for direct comparison with the host stream implementation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "random/GGEMSRandom.clh"

/// \cond

__kernel void
random_host_stream_probe(__global GGEMSRandomState *raw_states,
                         __global GGEMSRandomState *uniform_states,
                         __global uint *raw_values,
                         __global float *uniform_values, uint sample_count) {
  if (get_global_id(0) != 0U) {
    return;
  }

  for (uint sample_index = 0U; sample_index < sample_count; ++sample_index) {
    raw_values[sample_index] = GGEMS_RndmUInt32(raw_states, 0U);
    uniform_values[sample_index] = GGEMS_RndmUniform(uniform_states, 0U);
  }
}
/// \endcond
