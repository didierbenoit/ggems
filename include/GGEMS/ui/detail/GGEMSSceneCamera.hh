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
 * \brief Maps scene coordinates in meters to an orthographic clip volume.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>

namespace ggems::ui::detail {

/*! \brief Maintains orbit, pan, and zoom for scene presentation. */
class GGEMSSceneCamera {
public:
  /*! \brief Stores the four rows of the world-to-clip transform. */
  struct Matrix4Rows {
    /*! \brief Homogeneous row producing horizontal clip coordinates. */
    std::array<float, 4> row_0{1.0F, 0.0F, 0.0F, 0.0F};

    /*! \brief Homogeneous row producing vertical clip coordinates. */
    std::array<float, 4> row_1{0.0F, 1.0F, 0.0F, 0.0F};

    /*! \brief Homogeneous row producing Vulkan clip depth. */
    std::array<float, 4> row_2{0.0F, 0.0F, 1.0F, 0.0F};

    /*! \brief Homogeneous row preserving the orthographic w coordinate. */
    std::array<float, 4> row_3{0.0F, 0.0F, 0.0F, 1.0F};
  };

  /*! \brief Initializes the camera at its default view. */
  GGEMSSceneCamera() noexcept;

  /*!
   * \brief Updates the aspect ratio for a nonzero viewport extent.
   *
   * A zero dimension leaves the previous extent unchanged.
   *
   * \param[in] width Width in logical pixels.
   * \param[in] height Height in logical pixels.
   */
  auto SetViewportSize(std::uint32_t width, std::uint32_t height) noexcept
    -> void;

  /*!
   * \brief Sets yaw and clamps pitch to the supported orbit range.
   *
   * \param[in] yaw_degrees Yaw angle in degrees.
   * \param[in] pitch_degrees Pitch angle in degrees, clamped to [-85, 85].
   */
  auto SetOrbitAngles(float yaw_degrees, float pitch_degrees) noexcept -> void;

  /*!
   * \brief Sets the orthographic scale with a minimum of 0.05.
   *
   * \param[in] zoom Requested vertical scale applied to world meters.
   */
  auto SetZoom(float zoom) noexcept -> void;

  /*!
   * \brief Adds angular motion while retaining the pitch limit.
   *
   * \param[in] delta_yaw_degrees Yaw increment in degrees.
   * \param[in] delta_pitch_degrees Pitch increment in degrees.
   */
  auto Orbit(float delta_yaw_degrees, float delta_pitch_degrees) noexcept
    -> void;

  /*!
   * \brief Converts a drag to orbit motion at 0.20 degrees per pixel.
   *
   * \param[in] delta_x_pixels Horizontal drag in logical pixels.
   * \param[in] delta_y_pixels Vertical drag in logical pixels.
   */
  auto OrbitByPixels(float delta_x_pixels, float delta_y_pixels) noexcept
    -> void;

  /*!
   * \brief Moves the target in the camera plane using the current zoom.
   *
   * \param[in] delta_x_pixels Horizontal displacement in logical pixels.
   * \param[in] delta_y_pixels Vertical displacement in logical pixels.
   */
  auto Pan(float delta_x_pixels, float delta_y_pixels) noexcept -> void;

  /*!
   * \brief Applies multiplicative zoom from a mouse-wheel delta.
   *
   * \param[in] wheel_delta Wheel steps; positive values increase the scale.
   */
  auto ZoomBy(float wheel_delta) noexcept -> void;

  /*! \brief Restores the origin target, default orbit, and default zoom. */
  auto Reset() noexcept -> void;

  /*!
   * \brief Builds the orthographic transform for the current view.
   *
   * \return Rows mapping world meters to homogeneous clip coordinates.
   */
  [[nodiscard]] auto BuildWorldToClipMatrix() const noexcept -> Matrix4Rows;

private:
  /*! \brief Stores a camera direction or target coordinate triple. */
  struct Vector3 {
    /*! \brief X component in the enclosing vector space. */
    float x{0.0F};

    /*! \brief Y component in the enclosing vector space. */
    float y{0.0F};

    /*! \brief Z component in the enclosing vector space. */
    float z{0.0F};
  };

  /*!
   * \brief Computes the scalar product of two camera vectors.
   *
   * \param[in] first First vector.
   * \param[in] second Second vector.
   * \return Scalar product.
   */
  [[nodiscard]] static auto Dot(Vector3 const &first,
                                Vector3 const &second) noexcept -> float;

  /*!
   * \brief Computes the oriented cross product of two camera vectors.
   *
   * \param[in] first First vector.
   * \param[in] second Second vector.
   * \return Vector perpendicular to both inputs.
   */
  [[nodiscard]] static auto Cross(Vector3 const &first,
                                  Vector3 const &second) noexcept -> Vector3;

  /*!
   * \brief Normalizes a camera vector unless its length is negligible.
   *
   * \param[in] vec Vector to normalize.
   * \return Unit vector, or zero when the length is at most 1e-6.
   */
  [[nodiscard]] static auto Normalize(Vector3 const &vec) noexcept -> Vector3;

  /*! \brief Stores the orthonormal viewing axes in world space. */
  struct CameraBasis {
    /*! \brief Unit direction toward screen right. */
    Vector3 right{};

    /*! \brief Unit direction defining the camera vertical axis. */
    Vector3 up{};

    /*! \brief Unit direction from the camera toward its target. */
    Vector3 forward{};
  };

  /*!
   * \brief Builds camera axes from the current yaw and pitch.
   *
   * \return World-space right, up, and forward directions.
   */
  [[nodiscard]] auto BuildCameraBasis() const noexcept -> CameraBasis;

  /*! \brief Current viewport width in logical pixels. */
  std::uint32_t viewport_width_{1U};

  /*! \brief Current viewport height in logical pixels. */
  std::uint32_t viewport_height_{1U};

  /*! \brief Camera target in world-space meters. */
  Vector3 target_m_{.x = 0.0F, .y = 0.0F, .z = 0.0F};

  /*! \brief Orbit yaw in radians. */
  float yaw_radians_{};

  /*! \brief Orbit pitch in radians, limited away from the poles. */
  float pitch_radians_{};

  /*! \brief Orthographic vertical scale applied to world meters. */
  float zoom_{};

  /*! \brief World-meter to clip-depth scale around depth 0.5. */
  float depth_scale_{0.05F};
};

} // namespace ggems::ui::detail
