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
 * over the World and its single Box, outside the stream kernel.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "geometry/GGEMSAnalyticVolumeRecord.clh"
#include "geometry/GGEMSBoxRecord.clh"
#include "geometry/GGEMSBoxPoseRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"
#include "navigation/GGEMSAnalyticNavigation.clh"
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
 * field, the first one receives transformed extents and owner IDs.
 * \param[out] layout Size, the five field offsets, alignment, stride and the
 * five values of the second record.
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
  layout[1] = (ulong)((__private uchar const *)&record.half_extent_x_pm - base);
  layout[2] = (ulong)((__private uchar const *)&record.half_extent_y_pm - base);
  layout[3] = (ulong)((__private uchar const *)&record.half_extent_z_pm - base);
  layout[4] = (ulong)((__private uchar const *)&record.volume_id - base);
  layout[5] = (ulong)((__private uchar const *)&record.material_id - base);
  layout[6] = (ulong)((__private uchar const *)&alignment.record -
                      (__private uchar const *)&alignment);
  layout[7] = (ulong)((__global uchar const *)&boxes[1] -
                      (__global uchar const *)&boxes[0]);
  layout[8] = (ulong)as_uint(boxes[1].half_extent_x_pm);
  layout[9] = (ulong)as_uint(boxes[1].half_extent_y_pm);
  layout[10] = (ulong)as_uint(boxes[1].half_extent_z_pm);
  layout[11] = (ulong)boxes[1].volume_id;
  layout[12] = (ulong)boxes[1].material_id;
  boxes[0].half_extent_x_pm = boxes[1].half_extent_x_pm + 1U;
  boxes[0].half_extent_y_pm = boxes[1].half_extent_y_pm + 2U;
  boxes[0].half_extent_z_pm = boxes[1].half_extent_z_pm + 3U;
  boxes[0].volume_id = boxes[1].volume_id + 100U;
  boxes[0].material_id = boxes[1].material_id + 34U;
}

/*!
 * \brief Locates each ray start and runs one production Transport
 * transaction from it.
 *
 * \param[in] world Shared World record.
 * \param[in] volume Single analytic Box record.
 * \param[in] pose Box placement, including identity orientation.
 * \param[in] positions_pm Start positions, three coordinates per ray.
 * \param[in] directions Stored binary32 directions, three components per ray.
 * \param[in] ray_count Number of rays.
 * \param[out] start_owners Owner volume at the start per ray.
 * \param[out] statuses Transport status per ray.
 * \param[out] events Boundary event per ray.
 * \param[out] exit_faces Local FP32 crossing mask per ray.
 * \param[out] endpoints_pm Committed boundary positions (unchanged on failure).
 * \param[out] arrival_owners Owner after the transaction per ray.
 */
__kernel void box_navigation_probe(
  __global GGEMSWorldRecord const *world,
  __global GGEMSAnalyticVolumeRecord const *volume,
  __global GGEMSBoxPoseRecord const *pose, __global float const *positions_pm,
  __global float const *directions, uint ray_count, __global uint *start_owners,
  __global uint *statuses, __global uint *events, __global uint *exit_faces,
  __global float *endpoints_pm, __global uint *arrival_owners) {
  uint const ray = (uint)get_global_id(0);
  if (ray >= ray_count) {
    return;
  }

  // Failed location has no owner; preserve the input endpoint explicitly.
  start_owners[ray] = GGEMS_VOLUME_ID_EXTERIOR;
  statuses[ray] = GGEMS_NAVIGATION_UNRESOLVED;
  events[ray] = GGEMS_NAVIGATION_EVENT_NONE;
  exit_faces[ray] = 0U;
  endpoints_pm[3U * ray] = positions_pm[3U * ray];
  endpoints_pm[3U * ray + 1U] = positions_pm[3U * ray + 1U];
  endpoints_pm[3U * ray + 2U] = positions_pm[3U * ray + 2U];
  arrival_owners[ray] = GGEMS_VOLUME_ID_EXTERIOR;

  GGEMSParticleState particle;
  particle.position_x_pm = positions_pm[3U * ray];
  particle.position_y_pm = positions_pm[3U * ray + 1U];
  particle.position_z_pm = positions_pm[3U * ray + 2U];
  particle.direction_x = directions[3U * ray];
  particle.direction_y = directions[3U * ray + 1U];
  particle.direction_z = directions[3U * ray + 2U];
  particle.status = GGEMS_PARTICLE_STATUS_ALIVE;

  ulong owner = 0UL;
  uint const locate_status = GGEMS_AnalyticLocate(
    world, volume, pose,
    (float3)(particle.position_x_pm, particle.position_y_pm,
             particle.position_z_pm),
    (float3)(particle.direction_x, particle.direction_y, particle.direction_z),
    &owner);
  if (locate_status != GGEMS_NAVIGATION_RESOLVED) {
    statuses[ray] = locate_status;
    return;
  }
  particle.current_volume_id = (uint)(owner >> 32U);
  particle.material_id = (uint)(owner & 0xFFFFFFFFUL);

  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;
  uint previous_volume = GGEMS_VOLUME_ID_WORLD;

  uint const status = GGEMS_TransportToNextBoundary(
    world, volume, pose, &particle, &event, &faces, &previous_volume);

  start_owners[ray] = (uint)(owner >> 32U);
  statuses[ray] = status;
  events[ray] = event;
  exit_faces[ray] = faces;
  endpoints_pm[3U * ray] = particle.position_x_pm;
  endpoints_pm[3U * ray + 1U] = particle.position_y_pm;
  endpoints_pm[3U * ray + 2U] = particle.position_z_pm;
  arrival_owners[ray] = particle.current_volume_id;
}
