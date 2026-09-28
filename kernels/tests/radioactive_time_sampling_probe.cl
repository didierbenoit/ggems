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

#include "random/GGEMSRandomCommon.clh"
#include "radioactivity/GGEMSRadioactiveTimeSampling.clh"

typedef struct GGEMSRadioactiveTimeSamplingResult {
  ulong time_ps;
  uint ticket;
  float relative;
} GGEMSRadioactiveTimeSamplingResult;

__kernel void radioactive_time_sampling_probe(
  __global ulong const *starts, __global ulong const *stops,
  __global float const *scaled_decays, __global uint const *raw_words,
  __global GGEMSRadioactiveTimeSamplingResult *results, uint sample_count) {
  uint const index = (uint)(get_global_id(0));
  if (index >= sample_count) {
    return;
  }

  float const uniform = GGEMS_UIntToUniform01(raw_words[index]);
  float const relative =
    GGEMS_ComputeRadioactiveTimeRelative(uniform, scaled_decays[index]);

  results[index].time_ps = GGEMS_SampleRadioactiveTimeFromRaw(
    starts[index], stops[index], scaled_decays[index], raw_words[index]);
  results[index].ticket = GGEMS_QuantizeRadioactiveTimeRelative(relative);
  results[index].relative = relative;
}
