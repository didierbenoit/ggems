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
 * \brief Authors a finite planar Trd with a separate rigid pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <optional>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Owns a tapered solid with rectangular end faces.
 *
 * All dimension setters take full positive even-pm sizes, as for Box.
 * End planes are at local Z = +/- half length; pose is Rz * Ry * Rx.
 * World copies this value and checks its packed FP32 bands and direct-child
 * containment. Pose coordinates and fixed rotation axes belong to the parent
 * selected by GGEMSWorld::AddVolume; parent zero selects the World frame.
 */
class GGEMSTrd {
public:
  /*! \brief Creates an unconfigured solid at the origin with identity pose. */
  GGEMSTrd() = default;

  /*!
   * \brief Sets the full dimensions of the local negative-Z end.
   * \param[in] size_x Full X size of this rectangular end.
   * \param[in] size_y Full Y size of this rectangular end.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable For zero/odd sizes.
   */
  auto SetNegativeZFace(units::Length size_x, units::Length size_y)
    -> GGEMSTrd &;

  /*!
   * \brief Sets the full dimensions of the local positive-Z end.
   * \param[in] size_x Full X size of this rectangular end.
   * \param[in] size_y Full Y size of this rectangular end.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable For zero/odd sizes.
   */
  auto SetPositiveZFace(units::Length size_x, units::Length size_y)
    -> GGEMSTrd &;

  /*!
   * \brief Sets the full distance between the end planes.
   * \param[in] length Positive even integer-pm cap-to-cap length.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the length is zero or odd.
   */
  auto SetLength(units::Length length) -> GGEMSTrd &;

  /*!
   * \brief Sets the parent-frame position of the local frame origin.
   * \param[in] center Signed integer-pm position.
   * \return This solid for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSTrd &;

  /*!
   * \brief Replaces the absolute right-handed Rz * Ry * Rx rotation.
   * \param[in] angle_x Fixed-axis X rotation.
   * \param[in] angle_y Fixed-axis Y rotation.
   * \param[in] angle_z Fixed-axis Z rotation.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the proper basis cannot be formed.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSTrd &;

  /*!
   * \brief Sets the explicit owned medium.
   * \param[in] material Material copied or moved into this solid.
   * \return This solid for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSTrd &;

  /*!
   * \brief Checks configured dimensions and Material.
   * \throws core::GGEMSRecoverable If the solid is incomplete.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the negative-Z end half sizes in canonical pm.
   * \return X and Y half sizes, in that order.
   */
  [[nodiscard]] auto GetNegativeZHalfSizePM() const noexcept
    -> std::array<DistancePM, 2U>;

  /*!
   * \brief Returns the positive-Z end half sizes in canonical pm.
   * \return X and Y half sizes, in that order.
   */
  [[nodiscard]] auto GetPositiveZHalfSizePM() const noexcept
    -> std::array<DistancePM, 2U>;

  /*!
   * \brief Returns the distance from the local origin to either end plane.
   * \return Half length in integer pm; zero before SetLength().
   */
  [[nodiscard]] auto GetHalfLengthPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the local frame origin in the parent frame.
   * \return Signed integer-pm parent-frame position; the origin by default.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Borrows the owned local-to-parent proper rotation.
   * \return Proper rotation, initially identity; it must not outlive this
   * solid.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Borrows the owned medium.
   * \return Material reference; it must not outlive this solid.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Negative-Z face half sizes in pm; zero until configuration. */
  std::array<DistancePM, 2U> negative_half_size_pm_{};

  /*! \brief Positive-Z face half sizes in pm; zero until configuration. */
  std::array<DistancePM, 2U> positive_half_size_pm_{};

  /*! \brief Canonical distance to either end plane in pm. */
  DistancePM half_length_pm_{0};

  /*! \brief Global local-frame origin in canonical pm. */
  Position3PM center_pm_{};

  /*! \brief Absolute proper rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Explicit owned medium, absent until configuration. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
