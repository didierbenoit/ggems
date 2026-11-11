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
 * \brief Defines the immutable regular material-grid record transferred to
 * OpenCL.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"

namespace ggems::geometry {

/*!
 * \brief Stores one scalar 104-byte voxel occurrence with 8-byte alignment.
 *
 * Material labels remain uint16 values; the local palette maps them to normal
 * dense scene MaterialIDs. Original anatomical labels are not device assets.
 */
struct GGEMSVoxelizedVolumeRecord {
  /*! \brief World-frame center and local-to-World axes. */
  GGEMSBoxPoseRecord pose{};

  /*! \brief Number of cells along local X. */
  std::uint32_t dimension_x{0};

  /*! \brief Number of cells along local Y. */
  std::uint32_t dimension_y{0};

  /*! \brief Number of cells along local Z. */
  std::uint32_t dimension_z{0};

  /*! \brief Global physical VolumeID of the complete grid. */
  std::uint32_t volume_id{0};

  /*! \brief Cell width along local X in FP32 pm. */
  float spacing_x_pm{0.0F};

  /*! \brief Cell width along local Y in FP32 pm. */
  float spacing_y_pm{0.0F};

  /*! \brief Cell width along local Z in FP32 pm. */
  float spacing_z_pm{0.0F};

  /*! \brief Automatic outer-support surface half-band in FP32 pm. */
  float tolerance_pm{0.0F};

  /*! \brief Centered grid X half extent in FP32 pm. */
  float half_extent_x_pm{0.0F};

  /*! \brief Centered grid Y half extent in FP32 pm. */
  float half_extent_y_pm{0.0F};

  /*! \brief Centered grid Z half extent in FP32 pm. */
  float half_extent_z_pm{0.0F};

  /*! \brief Element offset in the concatenated dense MaterialID palettes. */
  std::uint32_t palette_offset{0};

  /*! \brief Element offset in the concatenated X-fastest ushort labels. */
  std::uint64_t material_label_offset{0};
};

} // namespace ggems::geometry
