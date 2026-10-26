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
 * \brief Declares a complete solid ellipsoid with an authored rigid pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <optional>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Owns a complete ellipsoid with positive local X/Y/Z semi-axes.
 *
 * The local surface is (x/a)^2 + (y/b)^2 + (z/c)^2 = 1. Semi-axes are radii,
 * not full dimensions; there are no cut planes. Equal semi-axes give a sphere.
 * World copies the complete value and admits its pose and FP32 surface bands.
 */
class GGEMSEllipsoid {
public:
  /*! \brief Creates a centered ellipsoid without semi-axes or Material. */
  GGEMSEllipsoid() = default;

  /*!
   * \brief Sets all three positive semi-axes after central Units conversion.
   * \param[in] axis_x Local X semi-axis, including odd integer-pm values.
   * \param[in] axis_y Local Y semi-axis.
   * \param[in] axis_z Local Z semi-axis.
   * \return This ellipsoid for chained configuration.
   * \throws core::GGEMSRecoverable If any semi-axis is zero; old axes remain.
   */
  auto SetSemiAxes(units::Length axis_x, units::Length axis_y,
                   units::Length axis_z) -> GGEMSEllipsoid &;

  /*!
   * \brief Sets the global geometric center.
   * \param[in] center Signed integer-pm center.
   * \return This ellipsoid for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSEllipsoid &;

  /*!
   * \brief Assigns the absolute active rotation Rz * Ry * Rx about the center.
   *
   * Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
   * Identity is the default; calls replace rather than accumulate the pose.
   * \param[in] angle_x Rotation about X.
   * \param[in] angle_y Rotation about Y.
   * \param[in] angle_z Rotation about Z.
   * \return This ellipsoid for chained configuration.
   * \throws core::GGEMSRecoverable If the basis is invalid; old rotation
   * remains.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSEllipsoid &;

  /*!
   * \brief Sets the owned ellipsoid medium.
   * \param[in] material Explicit medium copied or moved into this ellipsoid.
   * \return This ellipsoid for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSEllipsoid &;

  /*!
   * \brief Checks that semi-axes and Material are configured.
   * \throws core::GGEMSRecoverable If a required parameter is absent.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the canonical local semi-axes.
   * \return X/Y/Z semi-axes in pm, or zeros before configuration.
   */
  [[nodiscard]] auto GetSemiAxesPM() const noexcept -> HalfExtent3PM;

  /*!
   * \brief Returns the global geometric center.
   * \return Signed integer-pm center.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the immutable local-to-global rotation.
   * \return Validated orientation owned by this ellipsoid.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Borrows the owned ellipsoid medium.
   * \return Material reference valid for this ellipsoid's lifetime.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Global geometric center in pm. */
  Position3PM center_pm_{};

  /*! \brief Local X/Y/Z semi-axes in pm; zeros until configuration. */
  HalfExtent3PM semi_axes_pm_{};

  /*! \brief Proper local-to-global rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Explicit owned medium, absent until configuration. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
