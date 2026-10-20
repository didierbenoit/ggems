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
 * \brief Exercises the shared FP32 movement transaction without integer
 * conversion.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSDiagnosticProjection.clh"
#include "transport/GGEMSTransport.clh"

__kernel void
diagnostic_projection_probe(__global GGEMSParticleState *particles,
                            __global float const *parameters,
                            __global uint *success, uint count) {
  uint index = (uint)get_global_id(0);
  if (index >= count) {
    return;
  }

  GGEMSParticleState particle = particles[index];
  success[index] = GGEMS_TransportMoveToBoundary(parameters[index], &particle);
  particles[index] = particle;

  if (index == 0U) {
    success[count] = as_uint(GGEMS_DIAGNOSTIC_PROJECTION_DISTANCE_PM);
  }
}
