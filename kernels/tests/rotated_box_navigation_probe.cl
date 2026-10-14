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
 * \brief Tests rotated queries and accepted common Transport transactions.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#ifndef GGEMS_ENABLE_ROTATED_BOX

/*! \brief This focused probe always requests the rotated scene variant. */
#define GGEMS_ENABLE_ROTATED_BOX 1
#endif

#include "geometry/GGEMSBoxRecord.clh"
#include "geometry/GGEMSBoxPoseRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"
#include "navigation/GGEMSRotatedBoxNavigation.clh"
#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSTransport.clh"

/*!
 * \brief Records up to three events plus the unrounded parameter and length.
 * \param[in] world Shared finite World.
 * \param[in] box Single local Box.
 * \param[in] pose Immutable placement.
 * \param[in] input One fully initialized particle.
 * \param[out] trace Initial owner, final status, count, then nine values per
 * event: event, faces, departure, arrival, Material, XYZ and particle status.
 * \param[out] parameters Alternating parameter and norm-aware length per event.
 * \param[out] layout Pose size, twelve offsets, Particle size and Box size.
 */
__kernel void rotated_box_navigation_probe(
  __global GGEMSWorldRecord const *world, __global GGEMSBoxRecord const *box,
  __global GGEMSBoxPoseRecord const *pose,
  __global GGEMSParticleState const *input, __global long *trace,
  __global double *parameters, __global ulong *layout) {
  if (get_global_id(0) != 0U) {
    return;
  }

  layout[0] = sizeof(GGEMSBoxPoseRecord);
  layout[1] = __builtin_offsetof(GGEMSBoxPoseRecord, center_x_pm);
  layout[2] = __builtin_offsetof(GGEMSBoxPoseRecord, center_y_pm);
  layout[3] = __builtin_offsetof(GGEMSBoxPoseRecord, center_z_pm);
  layout[4] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_x_x);
  layout[5] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_x_y);
  layout[6] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_x_z);
  layout[7] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_y_x);
  layout[8] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_y_y);
  layout[9] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_y_z);
  layout[10] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_z_x);
  layout[11] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_z_y);
  layout[12] = __builtin_offsetof(GGEMSBoxPoseRecord, axis_z_z);
  layout[13] = sizeof(GGEMSParticleState);
  layout[14] = sizeof(GGEMSBoxRecord);

  GGEMSParticleState particle = *input;
  float3 direction =
    (float3)(particle.direction_x, particle.direction_y, particle.direction_z);
  ulong owner;
  uint status = GGEMS_RotatedBoxLocate(world, box, pose,
                                       (long3)(particle.position_x_pm,
                                               particle.position_y_pm,
                                               particle.position_z_pm),
                                       direction, &owner);
  trace[1] = status;
  trace[2] = 0L;

  if (status != GGEMS_NAVIGATION_RESOLVED) {
    return;
  }

  particle.current_volume_id = (uint)(owner >> 32U);
  particle.material_id = (uint)owner;
  trace[0] = particle.current_volume_id;
  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;

  for (uint index = 0U; index < 3U; ++index) {
    uint departure = particle.current_volume_id;
    GGEMSBoundaryWitness witness;
    double parameter;
    status = GGEMS_RotatedBoxFindBoundary(
      world, box, pose,
      (long3)(particle.position_x_pm, particle.position_y_pm,
              particle.position_z_pm),
      direction, departure, event, &witness, &parameter);
    trace[1] = status;

    if (status != GGEMS_NAVIGATION_RESOLVED) {
      return;
    }

    parameters[2U * index] = parameter;
    parameters[2U * index + 1U] =
      parameter * length(GGEMS_RotatedBoxDirection(direction));
    status = GGEMS_TransportToNextBoundary(world, box, 1U, pose, &particle,
                                           &event, &faces);
    trace[1] = status;

    if (status != GGEMS_NAVIGATION_RESOLVED) {
      return;
    }

    uint offset = 3U + 9U * index;
    trace[2] = index + 1U;
    trace[offset] = event;
    trace[offset + 1U] = faces;
    trace[offset + 2U] = departure;
    trace[offset + 3U] = particle.current_volume_id;
    trace[offset + 4U] = particle.material_id;
    trace[offset + 5U] = particle.position_x_pm;
    trace[offset + 6U] = particle.position_y_pm;
    trace[offset + 7U] = particle.position_z_pm;
    trace[offset + 8U] = particle.status;
    if (event == GGEMS_NAVIGATION_EVENT_WORLD_EXIT) {
      break;
    }
  }

  trace[30] = particle.direction_x == input->direction_x &&
              particle.direction_y == input->direction_y &&
              particle.direction_z == input->direction_z &&
              particle.energy_micro_eV == input->energy_micro_eV &&
              particle.time_ps == input->time_ps;
}
