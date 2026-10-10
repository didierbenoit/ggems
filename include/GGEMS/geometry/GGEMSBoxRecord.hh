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
 * \brief Defines the immutable analytic Box occurrence transferred to OpenCL.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::geometry {

/*! \brief Snapshot-local physical volume identity of the World. */
inline constexpr std::uint32_t k_volume_id_world{0U};

/*! \brief Owner identity of a particle that has left the World. */
inline constexpr std::uint32_t k_volume_id_exterior{0xFFFFFFFFU};

/*!
 * \brief Device Box occurrence: lower and upper faces and snapshot identities.
 *
 * Mirrors kernels/geometry/GGEMSBoxRecord.clh field for field.
 */
struct GGEMSBoxRecord {
  /*! \brief Global X coordinate of the -X face in signed integer pm. */
  std::int64_t lower_x_pm{0};

  /*! \brief Global Y coordinate of the -Y face in signed integer pm. */
  std::int64_t lower_y_pm{0};

  /*! \brief Global Z coordinate of the -Z face in signed integer pm. */
  std::int64_t lower_z_pm{0};

  /*! \brief Global X coordinate of the +X face in signed integer pm. */
  std::int64_t upper_x_pm{0};

  /*! \brief Global Y coordinate of the +Y face in signed integer pm. */
  std::int64_t upper_y_pm{0};

  /*! \brief Global Z coordinate of the +Z face in signed integer pm. */
  std::int64_t upper_z_pm{0};

  /*! \brief Snapshot-local physical volume identity; never k_volume_id_world.
   */
  std::uint32_t volume_id{0U};

  /*! \brief Dense snapshot-local Material identity of the Box medium. */
  std::uint32_t material_id{0U};
};

} // namespace ggems::geometry
