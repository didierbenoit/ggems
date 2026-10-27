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
 * \brief Defines the shared FP32 rigid pose for analytic shapes except Sphere.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

namespace ggems::geometry {

/*!
 * \brief Stores a scalar 48-byte pose matching GGEMSBoxPoseRecord.clh.
 *
 * The translation locates the local origin in World coordinates; for
 * Polyhedra it is not necessarily the geometric center. The axes are
 * local-to-World matrix columns. Device code subtracts the translation
 * and projects onto those columns to obtain local coordinates in FP32.
 * Box, Tubs, Ellipsoid, Trd, Trap, Cons, and Polyhedra share this layout;
 * Sphere stores its center directly in its scalar shape record.
 */
struct GGEMSBoxPoseRecord {
  /*! \brief World X coordinate of the local origin in FP32 pm. */
  float center_x_pm{0.0F};

  /*! \brief World Y coordinate of the local origin in FP32 pm. */
  float center_y_pm{0.0F};

  /*! \brief World Z coordinate of the local origin in FP32 pm. */
  float center_z_pm{0.0F};

  /*! \brief World X component of local X. */
  float axis_x_x{1.0F};

  /*! \brief World Y component of local X. */
  float axis_x_y{0.0F};

  /*! \brief World Z component of local X. */
  float axis_x_z{0.0F};

  /*! \brief World X component of local Y. */
  float axis_y_x{0.0F};

  /*! \brief World Y component of local Y. */
  float axis_y_y{1.0F};

  /*! \brief World Z component of local Y. */
  float axis_y_z{0.0F};

  /*! \brief World X component of local Z. */
  float axis_z_x{0.0F};

  /*! \brief World Y component of local Z. */
  float axis_z_y{0.0F};

  /*! \brief World Z component of local Z. */
  float axis_z_z{1.0F};
};

} // namespace ggems::geometry
