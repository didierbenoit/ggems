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

#include "transport/GGEMSDiagnosticProjection.clh"

__kernel void diagnostic_projection_probe(
  __global float const *components, __global long *scaled_components,
  __global uint *scale_success, uint component_count,
  __global long const *positions, __global long const *displacements,
  __global long *endpoints, __global uint *addition_success,
  uint addition_count, __global ulong *distance_pm) {
  uint index = (uint)(get_global_id(0));

  if (index == 0U) {
    distance_pm[0] = GGEMS_DIAGNOSTIC_PROJECTION_DISTANCE_PM;
  }

  if (index < component_count) {
    long scaled = scaled_components[index];

    uint success =
      GGEMS_TryScaleDiagnosticProjectionComponent(components[index], &scaled);

    scaled_components[index] = scaled;
    scale_success[index] = success;
  }

  if (index < addition_count) {
    long endpoint = endpoints[index];

    uint success = GGEMS_TryAddDiagnosticProjectionDisplacement(
      positions[index], displacements[index], &endpoint);

    endpoints[index] = endpoint;
    addition_success[index] = success;
  }
}
