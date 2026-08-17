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
 * \brief OpenCL ABI probe for GGEMS random-engine state layouts.
 *
 * Reports state sizes, member offsets, array strides, and alignment offsets for comparison with the host-side ABI.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "random/GGEMSRandomTypes.clh"

/// \cond

// =============================================================================
// =============================================================================

typedef struct GGEMSJKissStateAlignmentProbe {
  uchar prefix;
  GGEMSJKissState state;
} GGEMSJKissStateAlignmentProbe;

// =============================================================================
// =============================================================================

typedef struct GGEMSPCG32StateAlignmentProbe {
  uchar prefix;
  GGEMSPCG32State state;
} GGEMSPCG32StateAlignmentProbe;

// =============================================================================
// =============================================================================

typedef struct GGEMSPhiloxStateAlignmentProbe {
  uchar prefix;
  GGEMSPhiloxState state;
} GGEMSPhiloxStateAlignmentProbe;

// =============================================================================
// =============================================================================

__kernel void random_state_abi_probe(__global ulong *layout,
                                     __global GGEMSJKissState *jkiss_states,
                                     __global GGEMSPCG32State *pcg32_states,
                                     __global GGEMSPhiloxState *philox_states) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSJKissState jkiss;
  __private uchar const *jkiss_base = (__private uchar const *)&jkiss;

  layout[0] = (ulong)(sizeof(GGEMSJKissState));
  layout[1] = (ulong)((__private uchar const *)&jkiss.x - jkiss_base);
  layout[2] = (ulong)((__private uchar const *)&jkiss.y - jkiss_base);
  layout[3] = (ulong)((__private uchar const *)&jkiss.z - jkiss_base);
  layout[4] = (ulong)((__private uchar const *)&jkiss.w - jkiss_base);
  layout[5] = (ulong)((__private uchar const *)&jkiss.c - jkiss_base);
  layout[6] = (ulong)((__global uchar const *)&jkiss_states[1] -
                      (__global uchar const *)&jkiss_states[0]);

  GGEMSJKissStateAlignmentProbe jkiss_alignment;
  layout[7] = (ulong)((__private uchar const *)&jkiss_alignment.state -
                      (__private uchar const *)&jkiss_alignment);

  GGEMSPCG32State pcg32;
  __private uchar const *pcg32_base = (__private uchar const *)&pcg32;

  layout[8] = (ulong)(sizeof(GGEMSPCG32State));
  layout[9] = (ulong)((__private uchar const *)&pcg32.state - pcg32_base);
  layout[10] = (ulong)((__private uchar const *)&pcg32.increment - pcg32_base);
  layout[11] = (ulong)((__global uchar const *)&pcg32_states[1] -
                       (__global uchar const *)&pcg32_states[0]);

  GGEMSPCG32StateAlignmentProbe pcg32_alignment;
  layout[12] = (ulong)((__private uchar const *)&pcg32_alignment.state -
                       (__private uchar const *)&pcg32_alignment);

  GGEMSPhiloxState philox;
  __private uchar const *philox_base = (__private uchar const *)&philox;

  layout[13] = (ulong)(sizeof(GGEMSPhiloxState));
  layout[14] =
      (ulong)((__private uchar const *)&philox.counter_0 - philox_base);
  layout[15] =
      (ulong)((__private uchar const *)&philox.counter_1 - philox_base);
  layout[16] =
      (ulong)((__private uchar const *)&philox.counter_2 - philox_base);
  layout[17] =
      (ulong)((__private uchar const *)&philox.counter_3 - philox_base);
  layout[18] = (ulong)((__private uchar const *)&philox.key_0 - philox_base);
  layout[19] = (ulong)((__private uchar const *)&philox.key_1 - philox_base);
  layout[20] = (ulong)((__global uchar const *)&philox_states[1] -
                       (__global uchar const *)&philox_states[0]);

  GGEMSPhiloxStateAlignmentProbe philox_alignment;
  layout[21] = (ulong)((__private uchar const *)&philox_alignment.state -
                       (__private uchar const *)&philox_alignment);
}
/// \endcond
