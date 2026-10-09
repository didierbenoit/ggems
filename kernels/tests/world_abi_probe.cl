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
 * \brief Probes the World, counter, and accepted-interval device layouts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "geometry/GGEMSWorldRecord.clh"
#include "observer/GGEMSObserverRecord.clh"
#include "transport/GGEMSTransportCounters.clh"

typedef struct WorldAlignmentProbe {
  uchar prefix;
  GGEMSWorldRecord record;
} WorldAlignmentProbe;

typedef struct CounterAlignmentProbe {
  uchar prefix;
  GGEMSTransportCounters record;
} CounterAlignmentProbe;

typedef struct ObserverAlignmentProbe {
  uchar prefix;
  GGEMSObserverRecord record;
} ObserverAlignmentProbe;

__kernel void world_abi_probe(__global GGEMSWorldRecord const *worlds,
                              __global GGEMSTransportCounters *counts,
                              __global GGEMSObserverRecord *observations,
                              __global ulong *layout) {
  GGEMSWorldRecord world;
  GGEMSTransportCounters counter;
  GGEMSObserverRecord observer;
  WorldAlignmentProbe world_alignment;
  CounterAlignmentProbe counter_alignment;
  ObserverAlignmentProbe observer_alignment;
  layout[0] = sizeof(world);
  layout[1] = (ulong)((__private uchar *)&world.half_extent_y_pm -
                      (__private uchar *)&world);
  layout[2] = (ulong)((__private uchar *)&world.half_extent_z_pm -
                      (__private uchar *)&world);
  layout[3] = (ulong)((__private uchar *)&world_alignment.record -
                      (__private uchar *)&world_alignment);
  layout[4] =
    (ulong)((__global uchar *)&worlds[1] - (__global uchar *)&worlds[0]);
  layout[5] = sizeof(counter);
  layout[6] = (ulong)((__private uchar *)&counter.outside_world_count -
                      (__private uchar *)&counter);
  layout[7] = (ulong)((__private uchar *)&counter.unresolved_geometry_count -
                      (__private uchar *)&counter);
  layout[8] = (ulong)((__private uchar *)&counter.escaped_world_count -
                      (__private uchar *)&counter);
  layout[9] = (ulong)((__private uchar *)&counter_alignment.record -
                      (__private uchar *)&counter_alignment);
  layout[10] =
    (ulong)((__global uchar *)&counts[1] - (__global uchar *)&counts[0]);
  layout[11] = sizeof(observer);
  layout[12] = (ulong)((__private uchar *)&observer_alignment.record -
                       (__private uchar *)&observer_alignment);
  layout[13] = (ulong)((__global uchar *)&observations[1] -
                       (__global uchar *)&observations[0]);
  layout[14] = (ulong)worlds[1].half_extent_x_pm;
  layout[15] = (ulong)worlds[1].half_extent_y_pm;
  layout[16] = (ulong)worlds[1].half_extent_z_pm;

  counts[1].outside_world_count = 7U;
  counts[1].unresolved_geometry_count = 11U;
  counts[1].escaped_world_count = 13U;

  observations[1].energy_micro_eV = 9223372036854775806UL;
}
