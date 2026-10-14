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
 * \brief Declares a full solid Sphere authored by center, radius, and Material.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <optional>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Owns the host parameters of one full solid Sphere.
 *
 * Radius is a strictly positive integer number of picometers, including odd
 * values. World copies this value and checks strict containment. Per-particle
 * intersections and ownership belong to OpenCL Navigation.
 */
class GGEMSSphere {
public:
  /*! \brief Creates a Sphere at the origin without radius or Material. */
  GGEMSSphere() = default;

  /*!
   * \brief Sets the radius after central Units conversion.
   * \param[in] radius Strictly positive canonical length.
   * \return This Sphere for chained configuration.
   * \throws core::GGEMSRecoverable If radius is zero; the old radius remains.
   */
  auto SetRadius(units::Length radius) -> GGEMSSphere &;

  /*!
   * \brief Sets the global geometric center.
   * \param[in] center Signed integer-pm center.
   * \return This Sphere for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSSphere &;

  /*!
   * \brief Sets the owned Sphere medium.
   * \param[in] material Explicit medium, copied or moved into this Sphere.
   * \return This Sphere for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSSphere &;

  /*!
   * \brief Checks that radius and Material are configured.
   * \throws core::GGEMSRecoverable If either is absent.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the canonical radius.
   * \return Radius in pm, or zero before configuration.
   */
  [[nodiscard]] auto GetRadiusPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the global geometric center.
   * \return Signed integer-pm center.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Borrows the owned Sphere medium.
   * \return Material reference valid for this Sphere's lifetime.
   * \throws core::GGEMSRecoverable If no Material is configured.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

private:
  /*! \brief Global geometric center in pm. */
  Position3PM center_pm_{};

  /*! \brief Positive radius in pm; zero until configuration. */
  DistancePM radius_pm_{0};

  /*! \brief Explicit owned medium, absent until configuration. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
