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
 * \brief Test probe running one generic Transport transaction from the World
 * on one ray per work-item, outside the production stream kernel.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "geometry/GGEMSAnalyticVolumeRecord.clh"
#include "geometry/GGEMSBoxRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"
#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSTransport.clh"

/*!
 * \brief Moves independent World-owned rays to their first boundary.
 *
 * Without Box occurrences the first boundary is the World exit, so the
 * reported status is GGEMS_NAVIGATION_RESOLVED (0), OUTSIDE_WORLD (1) or
 * UNRESOLVED (2) and the endpoint is the FP32 World-exit commit.
 *
 * \param[in] world Shared World record.
 * \param[in] volume Single analytic record (None for the World-only probe).
 * \param[in] positions_pm Start positions, three coordinates per ray.
 * \param[in] directions Stored binary32 directions, three components per ray.
 * \param[in] ray_count Number of rays.
 * \param[out] statuses Transport outcome per ray.
 * \param[out] endpoints_pm Final positions (unchanged on failure).
 * \param[out] exit_faces FP32 crossing mask per ray.
 */
__kernel void
world_navigation_probe(__global GGEMSWorldRecord const *world,
                       __global GGEMSAnalyticVolumeRecord const *volume,
                       __global float const *positions_pm,
                       __global float const *directions, uint ray_count,
                       __global uint *statuses, __global float *endpoints_pm,
                       __global uint *exit_faces) {
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
  particle.current_volume_id = GGEMS_VOLUME_ID_WORLD;
  particle.material_id = GGEMS_MATERIAL_ID_WORLD;

  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;
  uint previous_volume = GGEMS_VOLUME_ID_WORLD;
  statuses[ray] = GGEMS_TransportToNextBoundary(
    world, volume, 0, &particle, &event, &faces, &previous_volume);
  endpoints_pm[3U * ray] = particle.position_x_pm;
  endpoints_pm[3U * ray + 1U] = particle.position_y_pm;
  endpoints_pm[3U * ray + 2U] = particle.position_z_pm;
  exit_faces[ray] = faces;
}
