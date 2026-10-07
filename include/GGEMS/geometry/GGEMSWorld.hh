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
 * \brief Declares the finite explicit GGEMS World and its exact point
 * membership.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Largest admitted World full size along one axis, in picometers.
 *
 * The value is the largest even integer below the signed 64-bit maximum. It
 * keeps every World half extent at or below 2^62 - 1 pm, so every World point
 * fits CoordinatePM and the displacement between any two World points fits
 * CoordinatePM without further overflow checks.
 */
inline constexpr DistancePM k_max_world_size_pm{0x7FFF'FFFF'FFFF'FFFEULL};

/*! \brief Classifies a canonical position relative to the closed World box. */
enum class GGEMSWorldRegion : std::uint8_t {
  /*! \brief Strictly inside the World on every axis. */
  Inside = 0U,

  /*! \brief On at least one World face, edge, or corner, and not outside. */
  Boundary = 1U,

  /*! \brief Strictly outside the World on at least one axis. */
  Outside = 2U,
};

/*!
 * \brief Finite explicit World: an axis-aligned box centered on the global
 * origin.
 *
 * The World frame is the canonical global frame of Position3PM and Direction3:
 * right-handed with X x Y = Z and default direction +Z. Faces lie at exactly
 * +/- half extent on the integer picometer lattice, so membership queries use
 * exact integer comparisons without any tolerance. The World medium is an
 * explicit owned Material value; exact Vacuum is admissible. The object is an
 * immutable validated host value with no occurrence or device record.
 */
class GGEMSWorld {
public:
  /*!
   * \brief Builds a World from its full sizes along the global axes and its
   * medium.
   *
   * \param[in] size_x Full extent along X.
   * \param[in] size_y Full extent along Y.
   * \param[in] size_z Full extent along Z.
   * \param[in] material Owned World medium; no default medium exists.
   * \throws GGEMSRecoverable If a size is zero, is an odd number of
   * picometers, or exceeds k_max_world_size_pm.
   */
  GGEMSWorld(units::Length size_x, units::Length size_y, units::Length size_z,
             core::materials::GGEMSMaterial material);

  /*!
   * \brief Returns the distances from the origin to the World faces.
   *
   * \return Half extents in picometers along X, Y, and Z.
   */
  [[nodiscard]] auto GetHalfExtentPM() const noexcept -> HalfExtent3PM;

  /*!
   * \brief Returns the World corner with the smallest coordinates.
   *
   * \return Position equal to the negated half extents.
   */
  [[nodiscard]] auto GetLowerCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the World corner with the largest coordinates.
   *
   * \return Position equal to the half extents.
   */
  [[nodiscard]] auto GetUpperCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Classifies a canonical global position against the closed World.
   *
   * \param[in] position Global position in picometers.
   * \return Outside if any coordinate magnitude exceeds its half extent,
   *         Boundary if none does and at least one equals it, Inside otherwise.
   */
  [[nodiscard]] auto Classify(Position3PM position) const noexcept
    -> GGEMSWorldRegion;

  /*!
   * \brief Checks closed-box membership.
   *
   * \param[in] position Global position in picometers.
   * \return True for Inside or Boundary positions.
   */
  [[nodiscard]] auto Contains(Position3PM position) const noexcept -> bool;

  /*!
   * \brief Returns the World medium.
   *
   * \return A borrowed reference to the owned Material; it must not outlive
   * this World.
   */
  [[nodiscard]] auto GetMaterial() const noexcept
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Validated half extents in picometers. */
  HalfExtent3PM half_extent_pm_;

  /*! \brief Owned World medium. */
  core::materials::GGEMSMaterial material_;
};

} // namespace ggems::geometry
