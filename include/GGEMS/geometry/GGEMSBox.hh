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
 *
 * \brief Declares the finite analytic Box volume placed by its center in the
 * World.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <optional>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Finite axis-aligned analytic Box: one physical volume with its own
 * explicit Material, placed by its geometric center in the World frame.
 *
 * The object is a host authoring value. A default-constructed Box is centered
 * on the global origin with no size and no Material; SetSize(), SetPosition()
 * and SetMaterial() may be called in any order. Validate() checks that the
 * Box is complete and is called by GGEMSWorld::AddVolume, which copies the
 * Box. Every full size is an even number of picometers, at least 2 pm, so the
 * local faces are exact lattice coordinates. Identity rotation keeps the exact
 * global axis-aligned path. The owning World checks strict containment and
 * compiles the pose and face records. Per-particle location, intersection and
 * boundary selection are computed by OpenCL Navigation.
 */
class GGEMSBox {
public:
  /*! \brief Creates a Box centered on the origin with no size and no Material.
   */
  GGEMSBox() = default;

  /*!
   * \brief Sets the full sizes along the local Box axes.
   *
   * \param[in] size_x Full extent along X.
   * \param[in] size_y Full extent along Y.
   * \param[in] size_z Full extent along Z.
   * \return This Box for chained configuration.
   * \throws GGEMSRecoverable If a size is zero or an odd number of picometers;
   * the previous size is kept.
   */
  auto SetSize(units::Length size_x, units::Length size_y, units::Length size_z)
    -> GGEMSBox &;

  /*!
   * \brief Sets the global position of the geometric center.
   *
   * \param[in] center Global center in picometers.
   * \return This Box for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSBox &;

  /*!
   * \brief Assigns the absolute active rotation Rz * Ry * Rx about the center.
   *
   * Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
   * The default is identity; this setter never accumulates rotations.
   *
   * \param[in] angle_x Rotation about X.
   * \param[in] angle_y Rotation about Y.
   * \param[in] angle_z Rotation about Z.
   * \return This Box for chained configuration.
   * \throws GGEMSRecoverable If an angle or basis is invalid; the old pose
   * stays.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSBox &;

  /*!
   * \brief Returns the immutable local-to-global FP64 rotation.
   * \return Validated orientation owned by this Box.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Sets the owned Box medium.
   *
   * \param[in] material Owned Box medium; no default medium exists.
   * \return This Box for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSBox &;

  /*!
   * \brief Checks that the Box is complete.
   *
   * \throws GGEMSRecoverable If the size or the Material is not set.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the geometric center.
   *
   * \return Center in picometers; the origin before SetPosition().
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the distances from the center to the faces.
   *
   * \return Half extents in picometers along X, Y, and Z; zero before
   * SetSize().
   */
  [[nodiscard]] auto GetHalfExtentPM() const noexcept -> HalfExtent3PM;

  /*!
   * \brief Returns the Box medium.
   *
   * \return A borrowed reference to the owned Material; it must not outlive
   * this Box.
   * \throws GGEMSRecoverable If no Material has been set.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Geometric center in picometers; the origin until SetPosition(). */
  Position3PM center_pm_{};

  /*! \brief Validated half extents in picometers; zero until SetSize(). */
  HalfExtent3PM half_extent_pm_{};

  /*! \brief Proper local-to-global rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Owned Box medium, absent until SetMaterial(). */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
