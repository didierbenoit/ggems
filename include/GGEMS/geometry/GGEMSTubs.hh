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
 * \brief Declares a finite full-azimuth cylinder with an optional coaxial bore.
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
 * \brief Owns one finite circular cylinder or hollow tube in its local frame.
 *
 * The local axis is +Z, the caps are at +/- half length, and azimuth is always
 * the full 360 degrees. Zero inner radius gives a filled cylinder with no inner
 * wall. Radii are not diameters; length is the full cap-to-cap dimension.
 * World copies the complete value and admits its pose and FP32 surface bands.
 */
class GGEMSTubs {
public:
  /*! \brief Creates a centered tube without dimensions or Material. */
  GGEMSTubs() = default;

  /*!
   * \brief Sets both radii after central Units conversion.
   * \param[in] outer_radius Strictly positive outer radius.
   * \param[in] inner_radius Bore radius; zero selects a filled cylinder.
   * \return This tube for chained configuration.
   * \throws core::GGEMSRecoverable Unless 0 <= inner < outer; old radii remain.
   */
  auto SetRadii(units::Length outer_radius, units::Length inner_radius = {})
    -> GGEMSTubs &;

  /*!
   * \brief Sets the full distance between the planar caps.
   * \param[in] length Positive even integer-pm length, at least 2 pm.
   * \return This tube for chained configuration.
   * \throws core::GGEMSRecoverable If length is zero or odd; old length
   * remains.
   */
  auto SetLength(units::Length length) -> GGEMSTubs &;

  /*!
   * \brief Sets the global geometric center.
   * \param[in] center Signed integer-pm center.
   * \return This tube for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSTubs &;

  /*!
   * \brief Assigns the absolute active rotation Rz * Ry * Rx about the center.
   *
   * Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
   * Identity is the default; calls replace rather than accumulate the pose.
   * \param[in] angle_x Rotation about X.
   * \param[in] angle_y Rotation about Y.
   * \param[in] angle_z Rotation about Z.
   * \return This tube for chained configuration.
   * \throws core::GGEMSRecoverable If the basis is invalid; old rotation
   * remains.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSTubs &;

  /*!
   * \brief Sets the owned tube medium.
   * \param[in] material Explicit medium copied or moved into this tube.
   * \return This tube for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSTubs &;

  /*!
   * \brief Checks that dimensions and Material are configured.
   * \throws core::GGEMSRecoverable If a required parameter is absent.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the canonical outer radius.
   * \return Radius in pm, or zero before configuration.
   */
  [[nodiscard]] auto GetOuterRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the canonical bore radius.
   * \return Radius in pm; zero for a filled cylinder.
   */
  [[nodiscard]] auto GetInnerRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the distance from the center to either cap.
   * \return Half length in pm, or zero before configuration.
   */
  [[nodiscard]] auto GetHalfLengthPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the global geometric center.
   * \return Signed integer-pm center.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the immutable local-to-global rotation.
   * \return Validated orientation owned by this tube.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Borrows the owned tube medium.
   * \return Material reference valid for this tube's lifetime.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Global geometric center in pm. */
  Position3PM center_pm_{};

  /*! \brief Outer radius in pm; zero until configuration. */
  DistancePM outer_radius_pm_{0};

  /*! \brief Bore radius in pm; zero gives a filled cylinder. */
  DistancePM inner_radius_pm_{0};

  /*! \brief Distance to each cap in pm; zero until configuration. */
  DistancePM half_length_pm_{0};

  /*! \brief Proper local-to-global rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Explicit owned medium, absent until configuration. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
