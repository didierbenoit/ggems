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
 * \brief Maps scene meters to orthographic or perspective clip coordinates.
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
  /*! \brief Selects the projection without changing target-plane scale. */
  enum class Projection : std::uint8_t {
    /*! \brief Parallel rays, the default projection. */
    Orthographic,
    /*! \brief Perspective rays with an automatic distance and FOV. */
    Perspective,
  };

  /*! \brief Selects an exact anatomical plane without changing World axes. */
  enum class ViewPlane : std::uint8_t {
    /*! \brief From +Z; screen right +X and screen up -Y. */
    XY,
    /*! \brief From +Y; screen right +X and screen up +Z. */
    XZ,
    /*! \brief From +X; screen right -Z and screen up -Y. */
    YZ,
  };

  /*! \brief Stores the four rows of the world-to-clip transform. */
  struct Matrix4Rows {
    /*! \brief Homogeneous row producing horizontal clip coordinates. */
    std::array<float, 4> row_0{1.0F, 0.0F, 0.0F, 0.0F};

    /*! \brief Homogeneous row producing vertical clip coordinates. */
    std::array<float, 4> row_1{0.0F, 1.0F, 0.0F, 0.0F};

    /*! \brief Homogeneous row producing Vulkan clip depth. */
    std::array<float, 4> row_2{0.0F, 0.0F, 1.0F, 0.0F};

    /*! \brief Produces one for orthographic or camera depth for perspective. */
    std::array<float, 4> row_3{0.0F, 0.0F, 0.0F, 1.0F};
  };

  /*! \brief Reports the current camera state and world-space viewing axes. */
  struct Diagnostics {
    /*! \brief Current unwrapped orbit yaw in degrees. */
    float yaw_degrees{};

    /*! \brief Current clamped orbit pitch in degrees. */
    float pitch_degrees{};

    /*! \brief Camera target in world-space meters, ordered X, Y, Z. */
    std::array<float, 3> target_m{};

    /*! \brief Active projection mode. */
    Projection projection{Projection::Orthographic};

    /*! \brief Vertical clip scale per meter at the camera target plane. */
    float zoom{};

    /*! \brief Actual perspective vertical FOV; zero in orthographic mode. */
    float vertical_fov_degrees{};

    /*! \brief Perspective camera-to-target distance; zero in orthographic. */
    float distance_m{};

    /*! \brief Positive perspective near distance; zero in orthographic. */
    float near_m{};

    /*! \brief Positive perspective far distance; zero in orthographic. */
    float far_m{};

    /*! \brief Current projection width in logical pixels. */
    std::uint32_t viewport_width{};

    /*! \brief Current projection height in logical pixels. */
    std::uint32_t viewport_height{};

    /*! \brief Unit direction toward screen right, ordered X, Y, Z. */
    std::array<float, 3> right{};

    /*! \brief Unit clip +Y direction; screen up is its negative in Vulkan. */
    std::array<float, 3> up{};

    /*! \brief Unit direction toward the target, ordered X, Y, Z. */
    std::array<float, 3> forward{};
  };

  /*! \brief Initializes the camera at its default view. */
  GGEMSSceneCamera() noexcept;

  /*!
   * \brief Changes projection while preserving target, orbit, and scale.
   *
   * Perspective uses at most 45 degrees vertically. Distance keeps World in
   * front of the camera; closer zoom narrows the actual FOV instead of entering
   * World. The diagnostics report these automatically coupled parameters.
   *
   * \param[in] projection Requested projection mode.
   */
  auto SetProjection(Projection projection) noexcept -> void;

  /*!
   * \brief Recenters and frames World along an exact anatomical direction.
   *
   * Preserves the projection. The ViewPlane documentation defines screen signs
   * for the existing positive-height Vulkan viewport; World axes never change.
   *
   * \param[in] plane Plane to view from its documented positive normal.
   */
  auto SetView(ViewPlane plane) noexcept -> void;

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
   * \param[in] pitch_degrees Pitch angle in degrees, clamped to [-90, 90].
   * \throws std::bad_expected_access If an angle is not representable in Units.
   */
  auto SetOrbitAngles(float yaw_degrees, float pitch_degrees) -> void;

  /*!
   * \brief Sets target-plane vertical scale with a scene-relative minimum.
   *
   * \param[in] zoom Requested vertical scale applied to world meters.
   */
  auto SetZoom(float zoom) noexcept -> void;

  /*!
   * \brief Frames the finite World and adapts the depth range to its size.
   *
   * \param[in] half_extent_m Positive World half extents in meters.
   */
  auto FrameWorld(std::array<double, 3> const &half_extent_m) noexcept -> void;

  /*!
   * \brief Adds angular motion while retaining the pitch limit.
   *
   * \param[in] delta_yaw_degrees Yaw increment in degrees.
   * \param[in] delta_pitch_degrees Pitch increment in degrees.
   * \throws std::bad_expected_access If an increment is not representable in
   * Units.
   */
  auto Orbit(float delta_yaw_degrees, float delta_pitch_degrees) -> void;

  /*!
   * \brief Converts a drag to orbit motion at 0.20 degrees per pixel.
   *
   * \param[in] delta_x_pixels Horizontal drag in logical pixels.
   * \param[in] delta_y_pixels Vertical drag in logical pixels.
   * \throws std::bad_expected_access If the angular motion is not representable
   * in Units.
   */
  auto OrbitByPixels(float delta_x_pixels, float delta_y_pixels) -> void;

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

  /*! \brief Resets navigation and framing while preserving projection. */
  auto Reset() noexcept -> void;

  /*!
   * \brief Builds the current projection with Vulkan depth in [0, 1].
   *
   * Loaded World bounds define depth independently of navigation. Orthographic
   * depth stays centered on World; perspective planes enclose its sphere.
   *
   * \return Rows mapping world meters to homogeneous clip coordinates.
   */
  [[nodiscard]] auto BuildWorldToClipMatrix() const noexcept -> Matrix4Rows;

  /*!
   * \brief Reads the current camera state without changing the view.
   *
   * \return Current angles, target, scale, projection extent, and viewing axes.
   */
  [[nodiscard]] auto GetDiagnostics() const noexcept -> Diagnostics;

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

  /*! \brief Stores perspective distances measured along the viewing axis. */
  struct PerspectiveDepth {
    /*! \brief Camera-to-target distance in meters. */
    float distance_m{};

    /*! \brief Positive near-plane distance in meters. */
    float near_m{};

    /*! \brief Far-plane distance enclosing World in meters. */
    float far_m{};
  };

  /*!
   * \brief Keeps World ahead of the eye and between positive depth planes.
   *
   * \param[in] basis Current orthonormal camera axes.
   * \return Automatic target, near-plane, and far-plane distances in meters.
   */
  [[nodiscard]] auto
  BuildPerspectiveDepth(CameraBasis const &basis) const noexcept
    -> PerspectiveDepth;

  /*! \brief Current viewport width in logical pixels. */
  std::uint32_t viewport_width_{1U};

  /*! \brief Current viewport height in logical pixels. */
  std::uint32_t viewport_height_{1U};

  /*! \brief Camera target in world-space meters. */
  Vector3 target_m_{.x = 0.0F, .y = 0.0F, .z = 0.0F};

  /*! \brief Orbit yaw in radians. */
  float yaw_radians_{};

  /*! \brief Orbit pitch in radians, including both exact poles. */
  float pitch_radians_{};

  /*! \brief Selected projection, preserved by navigation resets. */
  Projection projection_{Projection::Orthographic};

  /*! \brief Vertical clip scale per meter at the target plane. */
  float zoom_{};

  /*! \brief World-meter to clip-depth scale around depth 0.5. */
  float depth_scale_{0.05F};

  /*! \brief World bounding-sphere radius in meters, or zero before loading. */
  float world_radius_m_{0.0F};
};

} // namespace ggems::ui::detail
