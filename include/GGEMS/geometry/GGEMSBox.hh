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
 * \brief Declares the finite analytic Box occurrence placed in the World.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Finite axis-aligned analytic Box: one physical occurrence with its
 * own explicit Material, placed by its lower corner in the World frame.
 *
 * Faces lie at the lower corner and at lower corner plus full size on the
 * integer picometer lattice; any positive integer size is admitted. The
 * object is an immutable validated host authoring value; per-particle
 * location, intersection and boundary selection are computed by OpenCL
 * Navigation. Strict containment in the World is checked when the Box is
 * attached to a Run with its World.
 */
class GGEMSBox {
public:
  /*!
   * \brief Builds a Box from its full sizes, lower corner and medium.
   *
   * \param[in] size_x Full extent along X.
   * \param[in] size_y Full extent along Y.
   * \param[in] size_z Full extent along Z.
   * \param[in] lower_corner Global position of the (-X,-Y,-Z) corner.
   * \param[in] material Owned Box medium; no default medium exists.
   * \throws GGEMSRecoverable If a size is zero or the upper corner does not
   * fit the signed 64-bit picometer lattice.
   */
  GGEMSBox(units::Length size_x, units::Length size_y, units::Length size_z,
           Position3PM lower_corner, core::materials::GGEMSMaterial material);

  /*!
   * \brief Returns the Box corner with the smallest coordinates.
   *
   * \return Lower corner in picometers.
   */
  [[nodiscard]] auto GetLowerCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the Box corner with the largest coordinates.
   *
   * \return Lower corner plus full sizes in picometers.
   */
  [[nodiscard]] auto GetUpperCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Checks strict containment of the Box in a World.
   *
   * Every Box face must lie strictly inside the World so that no Box face
   * coincides with a World face.
   *
   * \param[in] world Validated World.
   * \return True when the closed Box lies in the open World interior.
   */
  [[nodiscard]] auto IsStrictlyInside(GGEMSWorld const &world) const noexcept
    -> bool;

  /*!
   * \brief Builds the immutable device-visible occurrence record.
   *
   * \param[in] volume_id Snapshot-local physical volume identity.
   * \param[in] material_id Dense snapshot-local Material identity.
   * \return Record carrying the faces and identities.
   */
  [[nodiscard]] auto BuildRecord(std::uint32_t volume_id,
                                 std::uint32_t material_id) const noexcept
    -> GGEMSBoxRecord;

  /*!
   * \brief Returns the Box medium.
   *
   * \return A borrowed reference to the owned Material; it must not outlive
   * this Box.
   */
  [[nodiscard]] auto GetMaterial() const noexcept
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Validated lower corner in picometers. */
  Position3PM lower_corner_pm_;

  /*! \brief Validated upper corner in picometers. */
  Position3PM upper_corner_pm_;

  /*! \brief Owned Box medium. */
  core::materials::GGEMSMaterial material_;
};

} // namespace ggems::geometry
