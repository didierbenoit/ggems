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
 * \brief Test probe running one production Transport transaction per ray
 * over the World and its Box occurrences, outside the stream kernel.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "geometry/GGEMSBoxRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"
#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSTransport.clh"

/*! \brief Layout probe: a Box record preceded by one byte. */
typedef struct BoxAlignmentProbe {
  uchar prefix;
  GGEMSBoxRecord record;
} BoxAlignmentProbe;

/*!
 * \brief Reports the device layout of GGEMSBoxRecord and transfers values in
 * both directions.
 *
 * \param[in,out] boxes Two records: the second one is read back field by
 * field, the first one receives every field of the second one transformed.
 * \param[out] layout Size, the eight field offsets, alignment, stride and the
 * eight values of the second record.
 */
__kernel void box_record_abi_probe(__global GGEMSBoxRecord *boxes,
                                   __global ulong *layout) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSBoxRecord record;
  BoxAlignmentProbe alignment;
  __private uchar const *base = (__private uchar const *)&record;

  layout[0] = sizeof(record);
  layout[1] = (ulong)((__private uchar const *)&record.lower_x_pm - base);
  layout[2] = (ulong)((__private uchar const *)&record.lower_y_pm - base);
  layout[3] = (ulong)((__private uchar const *)&record.lower_z_pm - base);
  layout[4] = (ulong)((__private uchar const *)&record.upper_x_pm - base);
  layout[5] = (ulong)((__private uchar const *)&record.upper_y_pm - base);
  layout[6] = (ulong)((__private uchar const *)&record.upper_z_pm - base);
  layout[7] = (ulong)((__private uchar const *)&record.volume_id - base);
  layout[8] = (ulong)((__private uchar const *)&record.material_id - base);
  layout[9] = (ulong)((__private uchar const *)&alignment.record -
                      (__private uchar const *)&alignment);
  layout[10] = (ulong)((__global uchar const *)&boxes[1] -
                       (__global uchar const *)&boxes[0]);
  layout[11] = (ulong)boxes[1].lower_x_pm;
  layout[12] = (ulong)boxes[1].lower_y_pm;
  layout[13] = (ulong)boxes[1].lower_z_pm;
  layout[14] = (ulong)boxes[1].upper_x_pm;
  layout[15] = (ulong)boxes[1].upper_y_pm;
  layout[16] = (ulong)boxes[1].upper_z_pm;
  layout[17] = (ulong)boxes[1].volume_id;
  layout[18] = (ulong)boxes[1].material_id;

  boxes[0].lower_x_pm = -boxes[1].lower_x_pm;
  boxes[0].lower_y_pm = -boxes[1].lower_y_pm;
  boxes[0].lower_z_pm = -boxes[1].lower_z_pm;
  boxes[0].upper_x_pm = -boxes[1].upper_x_pm;
  boxes[0].upper_y_pm = -boxes[1].upper_y_pm;
  boxes[0].upper_z_pm = -boxes[1].upper_z_pm;
  boxes[0].volume_id = boxes[1].volume_id + 100U;
  boxes[0].material_id = boxes[1].material_id + 34U;
}

/*!
 * \brief Locates each ray start and runs one production Transport
 * transaction from it.
 *
 * \param[in] world Shared World record.
 * \param[in] boxes Box occurrences.
 * \param[in] box_count Number of Box occurrences.
 * \param[in] positions_pm Start positions, three coordinates per ray.
 * \param[in] directions Stored binary32 directions, three components per ray.
 * \param[in] ray_count Number of rays.
 * \param[out] start_owners Owner volume at the start per ray.
 * \param[out] statuses Transport status per ray.
 * \param[out] events Boundary event per ray.
 * \param[out] exit_faces Exact crossing mask per ray.
 * \param[out] endpoints_pm Committed boundary positions (unchanged on failure).
 * \param[out] arrival_owners Owner after the transaction per ray.
 */
__kernel void box_navigation_probe(
  __global GGEMSWorldRecord const *world, __global GGEMSBoxRecord const *boxes,
  uint box_count, __global long const *positions_pm,
  __global float const *directions, uint ray_count, __global uint *start_owners,
  __global uint *statuses, __global uint *events, __global uint *exit_faces,
  __global long *endpoints_pm, __global uint *arrival_owners) {
  uint const ray = (uint)get_global_id(0);
  if (ray >= ray_count) {
    return;
  }

  GGEMSParticleState particle;
  particle.position_x_pm = positions_pm[3U * ray];
  particle.position_y_pm = positions_pm[3U * ray + 1U];
  particle.position_z_pm = positions_pm[3U * ray + 2U];
  particle.direction_x = directions[3U * ray];
  particle.direction_y = directions[3U * ray + 1U];
  particle.direction_z = directions[3U * ray + 2U];
  particle.status = GGEMS_PARTICLE_STATUS_ALIVE;

  ulong const owner = GGEMS_NavigationLocate(
    boxes, box_count,
    (long3)(particle.position_x_pm, particle.position_y_pm,
            particle.position_z_pm),
    (float3)(particle.direction_x, particle.direction_y, particle.direction_z));
  particle.current_volume_id = (uint)(owner >> 32U);
  particle.material_id = (uint)(owner & 0xFFFFFFFFUL);

  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;
  uint const status = GGEMS_TransportToNextBoundary(world, boxes, box_count,
                                                    &particle, &event, &faces);

  start_owners[ray] = (uint)(owner >> 32U);
  statuses[ray] = status;
  events[ray] = event;
  exit_faces[ray] = faces;
  endpoints_pm[3U * ray] = particle.position_x_pm;
  endpoints_pm[3U * ray + 1U] = particle.position_y_pm;
  endpoints_pm[3U * ray + 2U] = particle.position_z_pm;
  arrival_owners[ray] = particle.current_volume_id;
}
