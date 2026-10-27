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
 * \brief Authors a finite planar Trap with a separate rigid pose.
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
 * \brief Owns a general planar-faced trapezoid.
 *
 * All dimension setters take full positive even-pm sizes, as for Box.
 * End planes are at local Z = +/- half length; pose is Rz * Ry * Rx.
 * World copies this value and checks its packed FP32 bands and containment.
 * Face centers are +/- half length * tan(theta) * (cos(phi), sin(phi)).
 * At either end, a local Y offset v has X center v * tan(skew).
 * X half-width varies linearly between the negative-Y and positive-Y edges.
 * The four side faces must be planar within the local FP32 surface band;
 * incompatible end skews or width gradients are rejected by Validate().
 */
class GGEMSTrap {
public:
  /*! \brief Creates an unconfigured solid at the origin with identity pose. */
  GGEMSTrap() = default;

  /*!
   * \brief Sets the full dimensions of the local negative-Z end.
   * \param[in] height_y Full height along local Y.
   * \param[in] width_at_negative_y Full X width at the negative-Y edge.
   * \param[in] width_at_positive_y Full X width at the positive-Y edge.
   * \param[in] skew End-face skew strictly between -90 and +90 degrees.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable For zero/odd sizes or invalid skew.
   */
  auto SetNegativeZFace(units::Length height_y,
                        units::Length width_at_negative_y,
                        units::Length width_at_positive_y,
                        units::Angle skew = {}) -> GGEMSTrap &;

  /*!
   * \brief Sets the full dimensions of the local positive-Z end.
   * \param[in] height_y Full height along local Y.
   * \param[in] width_at_negative_y Full X width at the negative-Y edge.
   * \param[in] width_at_positive_y Full X width at the positive-Y edge.
   * \param[in] skew End-face skew strictly between -90 and +90 degrees.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable For zero/odd sizes or invalid skew.
   */
  auto SetPositiveZFace(units::Length height_y,
                        units::Length width_at_negative_y,
                        units::Length width_at_positive_y,
                        units::Angle skew = {}) -> GGEMSTrap &;

  /*!
   * \brief Sets the full distance between the end planes.
   * \param[in] length Positive even integer-pm cap-to-cap length.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the length is zero or odd.
   */
  auto SetLength(units::Length length) -> GGEMSTrap &;

  /*!
   * \brief Sets the center-line inclination independently from rigid pose.
   * \param[in] theta Signed tilt strictly between -90 and +90 degrees.
   * \param[in] phi Azimuth measured from local +X toward +Y.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If theta is outside the admitted interval.
   */
  auto SetInclination(units::Angle theta, units::Angle phi) -> GGEMSTrap &;

  /*!
   * \brief Sets the global position of the local frame origin.
   * \param[in] center Signed integer-pm position.
   * \return This solid for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSTrap &;

  /*!
   * \brief Replaces the absolute right-handed Rz * Ry * Rx rotation.
   * \param[in] angle_x Fixed-axis X rotation.
   * \param[in] angle_y Fixed-axis Y rotation.
   * \param[in] angle_z Fixed-axis Z rotation.
   * \return This solid for chained configuration.
   * \throws core::GGEMSRecoverable If the proper basis cannot be formed.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSTrap &;

  /*!
   * \brief Sets the explicit owned medium.
   * \param[in] material Material copied or moved into this solid.
   * \return This solid for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSTrap &;

  /*!
   * \brief Checks configured dimensions, planar side faces and Material.
   * \throws core::GGEMSRecoverable If the solid is incomplete or nonplanar.
   */
  auto Validate() const -> void;

  /*!
   * \brief Returns the negative-Z end half sizes in canonical pm.
   * \return Y half-height, negative-Y X half-width, positive-Y X half-width.
   */
  [[nodiscard]] auto GetNegativeZHalfSizePM() const noexcept
    -> std::array<DistancePM, 3U>;

  /*!
   * \brief Returns the positive-Z end half sizes in canonical pm.
   * \return Y half-height, negative-Y X half-width, positive-Y X half-width.
   */
  [[nodiscard]] auto GetPositiveZHalfSizePM() const noexcept
    -> std::array<DistancePM, 3U>;

  /*!
   * \brief Returns the authored negative-Z skew angle.
   * \return Signed skew of the negative-Z face in integer picoradians.
   */
  [[nodiscard]] auto GetNegativeZSkew() const noexcept -> units::Angle;

  /*!
   * \brief Returns the authored positive-Z skew angle.
   * \return Signed skew of the positive-Z face in integer picoradians.
   */
  [[nodiscard]] auto GetPositiveZSkew() const noexcept -> units::Angle;

  /*!
   * \brief Returns the authored inclination polar angle.
   * \return Signed center-line tilt in integer picoradians.
   */
  [[nodiscard]] auto GetInclinationTheta() const noexcept -> units::Angle;

  /*!
   * \brief Returns the authored inclination azimuth.
   * \return Center-line azimuth from local +X toward +Y in integer picoradians.
   */
  [[nodiscard]] auto GetInclinationPhi() const noexcept -> units::Angle;

  /*!
   * \brief Returns the distance from the local origin to either end plane.
   * \return Half length in integer pm; zero before SetLength().
   */
  [[nodiscard]] auto GetHalfLengthPM() const noexcept -> DistancePM;

  /*!
   * \brief Returns the local frame origin in the World.
   * \return Signed integer-pm World position; the origin by default.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Borrows the owned local-to-World proper rotation.
   * \return Proper rotation, initially identity; it must not outlive this solid.
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
  std::array<DistancePM, 3U> negative_half_size_pm_{};

  /*! \brief Positive-Z face half sizes in pm; zero until configuration. */
  std::array<DistancePM, 3U> positive_half_size_pm_{};

  /*! \brief Canonical distance to either end plane in pm. */
  DistancePM half_length_pm_{0};

  /*! \brief Negative-Z face skew in integer picoradians; zero by default. */
  units::Angle negative_z_skew_{};

  /*! \brief Positive-Z face skew in integer picoradians; zero by default. */
  units::Angle positive_z_skew_{};

  /*! \brief Signed center-line tilt in integer picoradians; zero by default. */
  units::Angle inclination_theta_{};

  /*! \brief Local center-line azimuth in integer picoradians; zero by default. */
  units::Angle inclination_phi_{};

  /*! \brief Global local-frame origin in canonical pm. */
  Position3PM center_pm_{};

  /*! \brief Absolute proper rotation; identity by default. */
  GGEMSRotation3D rotation_{};

  /*! \brief Explicit owned medium, absent until configuration. */
  std::optional<core::materials::GGEMSMaterial> material_;
};

} // namespace ggems::geometry
