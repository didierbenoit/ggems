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
 * \brief Builds proper local-to-global rotations with the Source basis
 * convention.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Stores a right-handed local-to-global orthonormal basis in FP64.
 *
 * Columns are local axes expressed in World coordinates, as in SourceFrame.
 * The transpose maps World-relative vectors into the local frame.
 */
struct GGEMSRotation3D {
  /*! \brief Local X axis in World coordinates. */
  detail::NormalizedVector3D axis_x{.x = 1.0, .y = 0.0, .z = 0.0};

  /*! \brief Local Y axis in World coordinates. */
  detail::NormalizedVector3D axis_y{.x = 0.0, .y = 1.0, .z = 0.0};

  /*! \brief Local Z axis in World coordinates. */
  detail::NormalizedVector3D axis_z{.x = 0.0, .y = 0.0, .z = 1.0};

  /*!
   * \brief Tests the stored basis for exact identity.
   * \return True when every component equals its identity value.
   */
  [[nodiscard]] auto IsIdentity() const noexcept -> bool;
};

/*!
 * \brief Builds the active right-handed rotation Rz * Ry * Rx.
 *
 * Fixed World axes X, Y, then Z act on a local vector. Assignment is absolute.
 *
 * \param[in] angle_x Rotation about X.
 * \param[in] angle_y Rotation about Y.
 * \param[in] angle_z Rotation about Z.
 * \return Proper FP64 basis, with exact cardinal entries when recognized.
 * \throws core::GGEMSRecoverable If an angle or resulting basis is invalid.
 */
[[nodiscard]] auto BuildRotationXYZ(units::Angle angle_x, units::Angle angle_y,
                                    units::Angle angle_z) -> GGEMSRotation3D;

} // namespace ggems::geometry
