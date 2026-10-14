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
 * \brief Defines the optional FP64 rigid pose accompanying local Box faces.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::geometry {

/*! \brief Scalar 96-byte pose; mirrors GGEMSBoxPoseRecord.clh. */
struct GGEMSBoxPoseRecord {
  /*! \brief Global center X coordinate in integer pm. */
  std::int64_t center_x_pm{0};

  /*! \brief Global center Y coordinate in integer pm. */
  std::int64_t center_y_pm{0};

  /*! \brief Global center Z coordinate in integer pm. */
  std::int64_t center_z_pm{0};

  /*! \brief World X component of local X. */
  double axis_x_x{1.0};

  /*! \brief World Y component of local X. */
  double axis_x_y{0.0};

  /*! \brief World Z component of local X. */
  double axis_x_z{0.0};

  /*! \brief World X component of local Y. */
  double axis_y_x{0.0};

  /*! \brief World Y component of local Y. */
  double axis_y_y{1.0};

  /*! \brief World Z component of local Y. */
  double axis_y_z{0.0};

  /*! \brief World X component of local Z. */
  double axis_z_x{0.0};

  /*! \brief World Y component of local Z. */
  double axis_z_y{0.0};

  /*! \brief World Z component of local Z. */
  double axis_z_z{1.0};
};

} // namespace ggems::geometry
