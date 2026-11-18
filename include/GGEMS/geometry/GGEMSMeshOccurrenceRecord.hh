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
 * \brief Defines a placed mesh record for common Navigation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"

namespace ggems::geometry {

/*! \brief Scalar 60-byte mesh placement in the scene auxiliary buffer. */
struct GGEMSMeshOccurrenceRecord {
  /*! \brief Local-to-World unit axes and translation in FP32 pm. */
  GGEMSBoxPoseRecord pose;

  /*! \brief Host-prepared closed navigation half-band in FP32 pm. */
  float tolerance_pm;

  /*! \brief Dense homogeneous scene Material identity. */
  std::uint32_t material_id;

  /*! \brief Shared BVH header byte offset from the auxiliary buffer start. */
  std::uint32_t asset_offset;
};

} // namespace ggems::geometry
