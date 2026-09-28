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

#include "radioactivity/GGEMSRadioactiveTimeSampling.clh"
#include "random/GGEMSRandom.clh"

__kernel void
radioactive_time_random_probe(__global GGEMSRandomState *random_states,
                              __global uint *raw_values,
                              __global ulong *sampled_time, ulong time_start_ps,
                              ulong time_stop_ps, float scaled_decay) {
  if (get_global_id(0) != 0U) {
    return;
  }

  uint const time_word = GGEMS_RndmUInt32(random_states, 0U);
  raw_values[0] = time_word;
  sampled_time[0] = GGEMS_SampleRadioactiveTimeFromRaw(
    time_start_ps, time_stop_ps, scaled_decay, time_word);
  raw_values[1] = GGEMS_RndmUInt32(random_states, 0U);
}
