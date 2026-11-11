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
 * \brief Exercises the common mixed-scene locator and voxel record device ABI.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "navigation/GGEMSOccurrenceNavigation.clh"
#include "geometry/GGEMSAnalyticVolumeRecord.clh"
#include "geometry/GGEMSBoxRecord.clh"
#include "geometry/GGEMSVoxelizedVolumeRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"

/*! \brief Publishes exact owner/status results from the production locator. */
__kernel void voxelized_locate_probe(
  __global GGEMSWorldRecord const *world,
  __global GGEMSAnalyticVolumeRecord const *analytic,
  __global void const *auxiliary,
  __global GGEMSVoxelizedVolumeRecord const *voxels,
  __global ushort const *labels, __global uint const *palette,
  __global float const *positions, __global uint *results) {
  size_t const index = get_global_id(0);
  float3 const position = vload3(index, positions);
  float3 const direction = (float3)(0.0F, 0.0F, 1.0F);
  ulong owner = ~0UL;
  uint const status =
    GGEMS_SceneLocate(world, analytic, auxiliary, voxels, labels, palette,
                      position, direction, &owner);
  results[4U * index] = status;
  results[(4U * index) + 1U] = (uint)(owner >> 32U);
  results[(4U * index) + 2U] = (uint)owner;
  GGEMSBoundaryWitness witness;
  results[(4U * index) + 3U] =
    status == GGEMS_NAVIGATION_RESOLVED
      ? GGEMS_SceneFindBoundary(
          world, analytic, auxiliary, position, direction, (uint)(owner >> 32U),
          GGEMS_NAVIGATION_EVENT_NONE, GGEMS_VOLUME_ID_WORLD, &witness)
      : ~0U;
}

/*! \brief Reports every voxel field offset, alignment, stride, and scalar
 * reads. */
__kernel void
voxelized_abi_probe(__global GGEMSVoxelizedVolumeRecord const *records,
                    __global ulong *layout) {
  layout[0] = sizeof(GGEMSVoxelizedVolumeRecord);
  layout[1] = __alignof(GGEMSVoxelizedVolumeRecord);
  layout[2] =
    (ulong)((__global uchar *)&records[1] - (__global uchar *)&records[0]);
  layout[3] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, pose);
  layout[4] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, dimension_x);
  layout[5] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, dimension_y);
  layout[6] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, dimension_z);
  layout[7] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, volume_id);
  layout[8] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, spacing_x_pm);
  layout[9] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, spacing_y_pm);
  layout[10] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, spacing_z_pm);
  layout[11] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, tolerance_pm);
  layout[12] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, half_extent_x_pm);
  layout[13] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, half_extent_y_pm);
  layout[14] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, half_extent_z_pm);
  layout[15] = __builtin_offsetof(GGEMSVoxelizedVolumeRecord, palette_offset);
  layout[16] =
    __builtin_offsetof(GGEMSVoxelizedVolumeRecord, material_label_offset);
  layout[17] = records[1].dimension_x;
  layout[18] = records[1].dimension_y;
  layout[19] = records[1].dimension_z;
  layout[20] = records[1].volume_id;
  layout[21] = records[1].palette_offset;
  layout[22] = records[1].material_label_offset;
  layout[23] = as_uint(records[1].pose.center_x_pm);
  layout[24] = as_uint(records[1].spacing_z_pm);
}
