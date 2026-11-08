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
 * \brief Checks camera projection, interaction, and rejected-input state.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {

// =============================================================================
// =============================================================================

using Camera = ggems::ui::detail::GGEMSSceneCamera;
using Matrix4Rows = Camera::Matrix4Rows;

constexpr float k_tolerance{1.0e-5F};
constexpr float k_sin_sixty_degrees{0.8660254037844386F};
constexpr float k_sin_twenty_five_degrees{0.4226182617406994F};
constexpr float k_cos_twenty_five_degrees{0.9063077870366500F};

// =============================================================================
// =============================================================================

struct Point3 {
  float x{0.0F};
  float y{0.0F};
  float z{0.0F};
};

constexpr std::array k_world_corners{
  Point3{.x = -2, .y = -3, .z = -1}, Point3{.x = 2, .y = -3, .z = -1},
  Point3{.x = -2, .y = 3, .z = -1},  Point3{.x = 2, .y = 3, .z = -1},
  Point3{.x = -2, .y = -3, .z = 1},  Point3{.x = 2, .y = -3, .z = 1},
  Point3{.x = -2, .y = 3, .z = 1},   Point3{.x = 2, .y = 3, .z = 1},
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto TransformPoint(Matrix4Rows const &matrix,
                                  Point3 const &point) noexcept
  -> std::array<float, 4U> {
  return {
    (matrix.row_0[0] * point.x) + (matrix.row_0[1] * point.y) +
      (matrix.row_0[2] * point.z) + matrix.row_0[3],
    (matrix.row_1[0] * point.x) + (matrix.row_1[1] * point.y) +
      (matrix.row_1[2] * point.z) + matrix.row_1[3],
    (matrix.row_2[0] * point.x) + (matrix.row_2[1] * point.y) +
      (matrix.row_2[2] * point.z) + matrix.row_2[3],
    (matrix.row_3[0] * point.x) + (matrix.row_3[1] * point.y) +
      (matrix.row_3[2] * point.z) + matrix.row_3[3],
  };
}

// =============================================================================
// =============================================================================

auto ExpectRowNear(std::array<float, 4U> const &actual,
                   std::array<float, 4U> const &expected) -> void {
  for (std::size_t index = 0U; index < actual.size(); ++index) {
    EXPECT_NEAR(actual[index], expected[index], k_tolerance);
  }
}

// =============================================================================
// =============================================================================

auto ExpectMatrixNear(Matrix4Rows const &actual, Matrix4Rows const &expected)
  -> void {
  ExpectRowNear(actual.row_0, expected.row_0);
  ExpectRowNear(actual.row_1, expected.row_1);
  ExpectRowNear(actual.row_2, expected.row_2);
  ExpectRowNear(actual.row_3, expected.row_3);
}

// =============================================================================
// =============================================================================

class GGEMSSceneCameraProjection
    : public ::testing::TestWithParam<Camera::Projection> {};

INSTANTIATE_TEST_SUITE_P(BothProjections, GGEMSSceneCameraProjection,
                         ::testing::Values(Camera::Projection::Orthographic,
                                           Camera::Projection::Perspective));

// The three exact presets keep scientific axes fixed and explicitly define
// the six signed axes on the positive-height Vulkan viewport.
TEST_P(GGEMSSceneCameraProjection, ExactViewsProjectAllSignedWorldAxes) {
  struct View {
    Camera::ViewPlane plane;
    std::array<float, 3> right;
    std::array<float, 3> down;
    std::array<float, 3> forward;
  };
  std::array const views{
    View{
      .plane = Camera::ViewPlane::XY,
      .right = {1, 0, 0},
      .down = {0, 1, 0},
      .forward = {0, 0, -1},
    },
    View{
      .plane = Camera::ViewPlane::XZ,
      .right = {1, 0, 0},
      .down = {0, 0, -1},
      .forward = {0, -1, 0},
    },
    View{
      .plane = Camera::ViewPlane::YZ,
      .right = {0, 0, -1},
      .down = {0, 1, 0},
      .forward = {-1, 0, 0},
    },
  };
  std::array const axes{
    Point3{.x = 1},  Point3{.x = -1}, Point3{.y = 1},
    Point3{.y = -1}, Point3{.z = 1},  Point3{.z = -1},
  };
  auto const dot = [](std::array<float, 3> const &axis, Point3 point) -> float {
    return (axis[0] * point.x) + (axis[1] * point.y) + (axis[2] * point.z);
  };
  Camera camera{};
  camera.SetViewportSize(800U, 800U);
  camera.SetProjection(GetParam());
  camera.FrameWorld({2.0, 3.0, 1.0});

  for (auto const &view : views) {
    SCOPED_TRACE(static_cast<int>(view.plane));
    camera.Pan(61, -27);
    camera.SetView(view.plane);
    auto const state = camera.GetDiagnostics();
    EXPECT_EQ(state.projection, GetParam());
    EXPECT_EQ(state.target_m, (std::array<float, 3>{}));
    for (std::size_t component = 0; component < 3; ++component) {
      EXPECT_NEAR(state.right[component], view.right[component], k_tolerance);
      EXPECT_NEAR(state.up[component], view.down[component], k_tolerance);
      EXPECT_NEAR(state.forward[component], view.forward[component],
                  k_tolerance);
    }
    auto const matrix = camera.BuildWorldToClipMatrix();
    float const distance =
      GetParam() == Camera::Projection::Perspective ? state.distance_m : 1.0F;
    for (auto const &axis : axes) {
      auto const projected = TransformPoint(matrix, axis);
      float const expected_w = GetParam() == Camera::Projection::Perspective
                                 ? distance + dot(view.forward, axis)
                                 : 1.0F;
      EXPECT_NEAR(projected[3], expected_w, k_tolerance);
      EXPECT_NEAR(projected[0] / projected[3],
                  dot(view.right, axis) * state.zoom * distance / expected_w,
                  k_tolerance);
      EXPECT_NEAR(projected[1] / projected[3],
                  dot(view.down, axis) * state.zoom * distance / expected_w,
                  k_tolerance);
      EXPECT_GT(projected[2], 0.0F);
      EXPECT_LT(projected[2], projected[3]);
    }
  }
}

// The perspective divide must change apparent size with distance, while near
// and far map to Vulkan zero and one rather than an orthographic rescaling.
TEST(GGEMSSceneCamera, PerspectiveUsesCameraDepthAndVulkanNearFarPlanes) {
  Camera camera{};
  camera.SetViewportSize(1600U, 800U);
  camera.FrameWorld({2.0, 3.0, 1.0});
  camera.SetView(Camera::ViewPlane::YZ);
  camera.SetProjection(Camera::Projection::Perspective);
  auto const state = camera.GetDiagnostics();
  auto const matrix = camera.BuildWorldToClipMatrix();
  EXPECT_NEAR(state.vertical_fov_degrees, 45.0F, k_tolerance);
  EXPECT_GT(state.near_m, 0.0F);
  EXPECT_GT(state.far_m, state.near_m);

  auto const near_point =
    TransformPoint(matrix, Point3{.x = state.distance_m - state.near_m});
  auto const far_point =
    TransformPoint(matrix, Point3{.x = state.distance_m - state.far_m});
  EXPECT_NEAR(near_point[3], state.near_m, k_tolerance);
  EXPECT_NEAR(far_point[3], state.far_m, k_tolerance);
  EXPECT_NEAR(near_point[2] / near_point[3], 0.0F, k_tolerance);
  EXPECT_NEAR(far_point[2] / far_point[3], 1.0F, k_tolerance);

  auto const front = TransformPoint(matrix, Point3{.x = 1, .y = 1, .z = 1});
  auto const back = TransformPoint(matrix, Point3{.x = -1, .y = 1, .z = 1});
  EXPECT_NEAR(front[3], state.distance_m - 1.0F, k_tolerance);
  EXPECT_NEAR(back[3], state.distance_m + 1.0F, k_tolerance);
  EXPECT_GT(front[1] / front[3], back[1] / back[3]);
  EXPECT_LT(front[2] / front[3], back[2] / back[3]);
  EXPECT_NEAR(front[0] / front[1], -0.5F, k_tolerance);
}

// Centered strong zoom narrows the FOV after the eye reaches its safe distance.
TEST(GGEMSSceneCamera, PerspectiveHighZoomStaysOutsideWorld) {
  Camera camera{};
  camera.SetViewportSize(800U, 800U);
  camera.FrameWorld({2.0, 3.0, 1.0});
  camera.SetProjection(Camera::Projection::Perspective);
  auto const before = camera.GetDiagnostics();
  camera.ZoomBy(20);
  auto const after = camera.GetDiagnostics();
  EXPECT_GT(after.zoom, before.zoom);
  EXPECT_GT(after.distance_m, std::sqrt(14.0F));
  EXPECT_LT(after.vertical_fov_degrees, before.vertical_fov_degrees);
  EXPECT_GT(after.near_m, 0.0F);
  EXPECT_GT(after.far_m, after.near_m);
  EXPECT_EQ(after.target_m, before.target_m);
}

// Presets and reset reframe navigation but retain the selected projection.
// Returning to the default view must recover the same fitted matrix.
TEST_P(GGEMSSceneCameraProjection, ResetPreservesProjectionAndDefaultFraming) {
  Camera camera{};
  EXPECT_EQ(camera.GetDiagnostics().projection,
            Camera::Projection::Orthographic);
  camera.SetViewportSize(900U, 700U);
  camera.SetProjection(GetParam());
  camera.FrameWorld({2.0, 3.0, 1.0});
  auto const original = camera.BuildWorldToClipMatrix();
  camera.SetView(Camera::ViewPlane::XZ);
  camera.OrbitByPixels(51, 27);
  camera.Pan(-71, 43);
  camera.ZoomBy(5);
  camera.Reset();
  auto const reset = camera.GetDiagnostics();
  EXPECT_EQ(reset.projection, GetParam());
  EXPECT_NEAR(reset.yaw_degrees, 300.0F, 1.0e-4F);
  EXPECT_NEAR(reset.pitch_degrees, -25.0F, k_tolerance);
  EXPECT_EQ(reset.target_m, (std::array<float, 3>{}));
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), original);
}

// Focus fits only the selected bounds, including millimeter-sized objects near
// a World corner, while retaining orientation, projection, and World clipping.
TEST_P(GGEMSSceneCameraProjection, FrameBoundsFitsTinyOffAxisObjects) {
  for (double scale : {1.0e-6, 1.0, 1.0e5}) {
    SCOPED_TRACE(scale);
    for (auto const viewport :
         {std::array{1200U, 700U}, std::array{320U, 720U}}) {
      SCOPED_TRACE(viewport[0]);
      Camera camera{};
      camera.SetViewportSize(viewport[0], viewport[1]);
      camera.SetProjection(GetParam());
      camera.FrameWorld({2.0 * scale, 3.0 * scale, scale});
      camera.SetOrbitAngles(721.0F, 35.0F);
      auto const before = camera.GetDiagnostics();
      for (auto const center :
           {std::array{0.3, -0.1, 0.2}, std::array{1.995, 2.995, 0.995}}) {
        SCOPED_TRACE(center[0]);
        std::array<double, 3> minimum{};
        std::array<double, 3> maximum{};
        for (std::size_t axis = 0U; axis < center.size(); ++axis) {
          minimum[axis] = (center[axis] - 0.003) * scale;
          maximum[axis] = (center[axis] + 0.003) * scale;
        }
        camera.FrameBounds(minimum, maximum);
        auto const focused = camera.GetDiagnostics();
        EXPECT_EQ(focused.projection, before.projection);
        EXPECT_EQ(focused.yaw_degrees, before.yaw_degrees);
        EXPECT_EQ(focused.pitch_degrees, before.pitch_degrees);
        EXPECT_GT(focused.zoom, before.zoom * 100.0F);
        for (std::size_t axis = 0U; axis < center.size(); ++axis) {
          EXPECT_FLOAT_EQ(focused.target_m[axis],
                          static_cast<float>(center[axis] * scale));
        }
        auto const matrix = camera.BuildWorldToClipMatrix();
        auto const projected_center =
          TransformPoint(matrix, Point3{
                                   .x = static_cast<float>(center[0] * scale),
                                   .y = static_cast<float>(center[1] * scale),
                                   .z = static_cast<float>(center[2] * scale),
                                 });
        // FP32 translation cancellation at the corner is below 0.12 logical
        // pixels at the largest viewport in this fixture.
        EXPECT_NEAR(projected_center[0] / projected_center[3], 0.0F, 2.0e-4F);
        EXPECT_NEAR(projected_center[1] / projected_center[3], 0.0F, 2.0e-4F);
        float largest_extent = 0.0F;
        for (unsigned int corner = 0U; corner < 8U; ++corner) {
          auto const projected = TransformPoint(
            matrix, Point3{
                      .x = static_cast<float>((corner & 1U) != 0U ? maximum[0]
                                                                  : minimum[0]),
                      .y = static_cast<float>((corner & 2U) != 0U ? maximum[1]
                                                                  : minimum[1]),
                      .z = static_cast<float>((corner & 4U) != 0U ? maximum[2]
                                                                  : minimum[2]),
                    });
          ASSERT_GT(projected[3], 0.0F);
          EXPECT_LT(std::abs(projected[0] / projected[3]), 0.92F);
          EXPECT_LT(std::abs(projected[1] / projected[3]), 0.92F);
          EXPECT_GT(projected[2], 0.0F);
          EXPECT_LT(projected[2], projected[3]);
          largest_extent = std::max({
            largest_extent,
            std::abs(projected[0] / projected[3]),
            std::abs(projected[1] / projected[3]),
          });
        }
        EXPECT_GT(largest_extent, 0.25F);
        for (auto const &corner : k_world_corners) {
          auto const projected =
            TransformPoint(matrix, Point3{
                                     .x = corner.x * static_cast<float>(scale),
                                     .y = corner.y * static_cast<float>(scale),
                                     .z = corner.z * static_cast<float>(scale),
                                   });
          EXPECT_GT(projected[3], 0.0F);
          EXPECT_GT(projected[2], 0.0F);
          EXPECT_LT(projected[2], projected[3]);
        }
        if (GetParam() == Camera::Projection::Perspective) {
          EXPECT_GT(focused.near_m, 0.0F);
          EXPECT_GT(focused.far_m, focused.near_m);
          EXPECT_LE(focused.vertical_fov_degrees, 45.0F + k_tolerance);
        }
      }
    }
  }
}

// Focus changes the orbit center without changing subsequent controls or the
// full-World reference used by Reset and axial presets.
TEST_P(GGEMSSceneCameraProjection, FocusRetainsOrbitCenterAndFullWorldReset) {
  Camera camera{};
  camera.SetViewportSize(1200U, 700U);
  camera.SetProjection(GetParam());
  camera.FrameWorld({2.0, 3.0, 1.0});
  camera.SetOrbitAngles(719.0F, -32.0F);
  camera.FrameBounds({1.59, -2.41, 0.69}, {1.61, -2.39, 0.71});
  auto const focused = camera.GetDiagnostics();
  camera.OrbitByPixels(38.0F, -21.0F);
  auto const orbited = camera.GetDiagnostics();
  EXPECT_EQ(orbited.target_m, focused.target_m);
  EXPECT_EQ(orbited.zoom, focused.zoom);
  EXPECT_NEAR(orbited.yaw_degrees, focused.yaw_degrees - 7.6F, 1.0e-4F);
  EXPECT_NEAR(orbited.pitch_degrees, focused.pitch_degrees + 4.2F, k_tolerance);
  for (auto const projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    camera.SetProjection(projection);
    auto const switched = camera.GetDiagnostics();
    EXPECT_EQ(switched.target_m, orbited.target_m);
    EXPECT_EQ(switched.zoom, orbited.zoom);
    EXPECT_EQ(switched.yaw_degrees, orbited.yaw_degrees);
    EXPECT_EQ(switched.pitch_degrees, orbited.pitch_degrees);
  }
  camera.SetProjection(GetParam());
  camera.Pan(12.0F, -5.0F);
  camera.ZoomBy(2.0F);
  camera.SetViewportSize(320U, 720U);
  camera.Reset();
  Camera expected{};
  expected.SetViewportSize(320U, 720U);
  expected.SetProjection(GetParam());
  expected.FrameWorld({2.0, 3.0, 1.0});
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(),
                   expected.BuildWorldToClipMatrix());
  camera.FrameBounds({1.59, -2.41, 0.69}, {1.61, -2.39, 0.71});
  camera.SetView(Camera::ViewPlane::XY);
  expected.SetView(Camera::ViewPlane::XY);
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(),
                   expected.BuildWorldToClipMatrix());
}

// At either exact pole, outward pitch clamps and inward drag moves by the
// requested 0.2 degrees. Horizontal motion has a continuous screen orientation.
TEST_P(GGEMSSceneCameraProjection,
       PoleOrbitRemainsContinuousAndKeepsDragSigns) {
  Camera camera{};
  camera.SetProjection(GetParam());
  camera.FrameWorld({2.0, 3.0, 1.0});
  for (float pole : {-90.0F, 90.0F}) {
    SCOPED_TRACE(pole);
    camera.SetOrbitAngles(270.0F, pole);
    auto const before = camera.GetDiagnostics();
    EXPECT_FLOAT_EQ(before.forward[0], 0.0F);
    EXPECT_FLOAT_EQ(before.forward[2], 0.0F);
    EXPECT_FLOAT_EQ(before.forward[1], pole > 0 ? -1.0F : 1.0F);
    camera.OrbitByPixels(1.0F, 0.0F);
    auto const horizontal = camera.GetDiagnostics();
    EXPECT_NEAR(horizontal.yaw_degrees, 269.8F, 1.0e-4F);
    EXPECT_EQ(horizontal.forward, before.forward);
    EXPECT_GT(horizontal.right[0], 0.999F);

    camera.OrbitByPixels(0.0F, pole > 0 ? -1.0F : 1.0F);
    EXPECT_NEAR(camera.GetDiagnostics().pitch_degrees, pole, k_tolerance);
    camera.OrbitByPixels(0.0F, pole > 0 ? 1.0F : -1.0F);
    auto const inside = camera.GetDiagnostics();
    EXPECT_NEAR(std::abs(inside.pitch_degrees), 89.8F, 1.0e-4F);
    EXPECT_GT(std::abs(inside.forward[1]), 0.999F);
    EXPECT_GT(inside.right[0], 0.999F);
    auto const matrix = camera.BuildWorldToClipMatrix();
    for (float value : matrix.row_0) {
      EXPECT_TRUE(std::isfinite(value));
    }
    for (float value : matrix.row_1) {
      EXPECT_TRUE(std::isfinite(value));
    }
    for (float value : matrix.row_2) {
      EXPECT_TRUE(std::isfinite(value));
    }
    for (float value : matrix.row_3) {
      EXPECT_TRUE(std::isfinite(value));
    }
  }
}

// Panning follows the same pixel displacement at the target plane in either
// projection. Switching modes keeps that plane's scale, orbit, and target.
TEST_P(GGEMSSceneCameraProjection, PanAndProjectionSwitchKeepTargetPlaneScale) {
  Camera camera{};
  camera.SetViewportSize(1000U, 500U);
  camera.FrameWorld({2.0, 3.0, 1.0});
  camera.SetView(Camera::ViewPlane::XY);
  camera.SetProjection(GetParam());
  camera.Pan(100, 50);
  auto const origin = TransformPoint(camera.BuildWorldToClipMatrix(), Point3{});
  EXPECT_NEAR(origin[0] / origin[3], 0.2F, k_tolerance);
  EXPECT_NEAR(origin[1] / origin[3], 0.2F, k_tolerance);
  camera.ZoomBy(20);
  auto const before = camera.GetDiagnostics();
  Point3 const target{
    .x = before.target_m[0],
    .y = before.target_m[1],
    .z = before.target_m[2],
  };
  Point3 const offset{
    .x = target.x + (before.right[0] / before.zoom),
    .y = target.y + (before.right[1] / before.zoom),
    .z = target.z + (before.right[2] / before.zoom),
  };
  for (auto projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    camera.SetProjection(projection);
    auto const state = camera.GetDiagnostics();
    EXPECT_EQ(state.target_m, before.target_m);
    EXPECT_FLOAT_EQ(state.zoom, before.zoom);
    EXPECT_FLOAT_EQ(state.yaw_degrees, before.yaw_degrees);
    EXPECT_FLOAT_EQ(state.pitch_degrees, before.pitch_degrees);
    auto const matrix = camera.BuildWorldToClipMatrix();
    auto const center = TransformPoint(matrix, target);
    auto const edge = TransformPoint(matrix, offset);
    EXPECT_NEAR(center[0] / center[3], 0.0F, k_tolerance);
    EXPECT_NEAR(center[1] / center[3], 0.0F, k_tolerance);
    EXPECT_NEAR(edge[0] / edge[3], 0.5F, k_tolerance);
    EXPECT_NEAR(edge[1] / edge[3], 0.0F, k_tolerance);
  }
}

// Zoom may intentionally crop XY, but World must remain inside the depth
// interval even after a large pan followed by orbit and an aspect change.
TEST_P(GGEMSSceneCameraProjection, StrongNavigationKeepsWorldDepthVisible) {
  Camera camera{};
  camera.SetProjection(GetParam());
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    SCOPED_TRACE(scale);
    camera.SetViewportSize(900U, 700U);
    camera.FrameWorld({2.0 * scale, 3.0 * scale, scale});
    camera.Pan(1800, -1200);
    camera.OrbitByPixels(280, -60);
    camera.ZoomBy(35);
    camera.SetViewportSize(300U, 800U);
    auto const state = camera.GetDiagnostics();
    auto const matrix = camera.BuildWorldToClipMatrix();
    for (auto const &point : k_world_corners) {
      auto const corner =
        TransformPoint(matrix, Point3{
                                 .x = point.x * static_cast<float>(scale),
                                 .y = point.y * static_cast<float>(scale),
                                 .z = point.z * static_cast<float>(scale),
                               });
      EXPECT_GT(corner[3], 0.0F);
      EXPECT_GT(corner[2] / corner[3], 0.0F);
      EXPECT_LT(corner[2] / corner[3], 1.0F);
    }
    if (GetParam() == Camera::Projection::Perspective) {
      EXPECT_GT(state.near_m, 0.0F);
      EXPECT_GT(state.far_m, state.near_m);
      EXPECT_GT(state.vertical_fov_degrees, 0.0F);
      EXPECT_LE(state.vertical_fov_degrees, 45.0F + k_tolerance);
    }
  }
}

// Reset must undo prior pan, orbit, and zoom and recover the independently
// expected view.
TEST(GGEMSSceneCamera, ResetRestoresCanonicalView) {
  Camera camera{};
  camera.SetViewportSize(800U, 800U);

  camera.SetOrbitAngles(-20.0F, 15.0F);
  camera.SetZoom(1.7F);
  camera.Pan(37.0F, -19.0F);
  camera.Orbit(12.0F, 8.0F);
  camera.ZoomBy(2.0F);

  camera.Reset();

  Matrix4Rows const expected{
    .row_0 = {0.8F * k_sin_sixty_degrees, 0.0F, -0.4F, 0.0F},
    .row_1 =
      {
        0.4F * k_sin_twenty_five_degrees,
        0.8F * k_cos_twenty_five_degrees,
        0.8F * k_sin_sixty_degrees * k_sin_twenty_five_degrees,
        0.0F,
      },
    .row_2 =
      {
        -0.025F * k_cos_twenty_five_degrees,
        0.05F * k_sin_twenty_five_degrees,
        -0.05F * k_sin_sixty_degrees * k_cos_twenty_five_degrees,
        0.5F,
      },
    .row_3 = {0.0F, 0.0F, 0.0F, 1.0F},
  };

  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);

  auto const diagnostics = camera.GetDiagnostics();
  EXPECT_NEAR(diagnostics.yaw_degrees, 300.0F, 1.0e-4F);
  EXPECT_NEAR(diagnostics.pitch_degrees, -25.0F, k_tolerance);
  EXPECT_EQ(diagnostics.target_m, (std::array<float, 3>{}));
  EXPECT_FLOAT_EQ(diagnostics.zoom, 0.8F);
  EXPECT_EQ(diagnostics.viewport_width, 800U);
  EXPECT_EQ(diagnostics.viewport_height, 800U);
}

// =============================================================================
// =============================================================================

// Diagnostics must describe the current projection after interaction, including
// an unwrapped yaw, the panned target, and the actual viewing axes.
TEST(GGEMSSceneCamera, DiagnosticsMatchCurrentProjectionAfterInteraction) {
  Camera camera{};
  camera.SetViewportSize(1600U, 800U);
  camera.SetOrbitAngles(370.0F, 25.0F);
  camera.OrbitByPixels(50.0F, 0.0F);
  camera.Pan(12.0F, -17.0F);
  camera.ZoomBy(2.0F);

  auto const matrix = camera.BuildWorldToClipMatrix();
  auto const diagnostics = camera.GetDiagnostics();

  EXPECT_NEAR(diagnostics.yaw_degrees, 360.0F, 1.0e-4F);
  EXPECT_NEAR(diagnostics.pitch_degrees, 25.0F, k_tolerance);
  EXPECT_NEAR(diagnostics.zoom, 0.8F * 1.12F * 1.12F, k_tolerance);
  EXPECT_EQ(diagnostics.viewport_width, 1600U);
  EXPECT_EQ(diagnostics.viewport_height, 800U);

  for (std::size_t index = 0U; index < 3U; ++index) {
    EXPECT_NEAR(matrix.row_0[index],
                diagnostics.right[index] * diagnostics.zoom * 0.5F,
                k_tolerance);
    EXPECT_NEAR(matrix.row_1[index], diagnostics.up[index] * diagnostics.zoom,
                k_tolerance);
    EXPECT_NEAR(matrix.row_2[index], diagnostics.forward[index] * 0.05F,
                k_tolerance);
  }

  auto const target = TransformPoint(matrix, Point3{
                                               .x = diagnostics.target_m[0],
                                               .y = diagnostics.target_m[1],
                                               .z = diagnostics.target_m[2],
                                             });
  ExpectRowNear(target, {0.0F, 0.0F, 0.5F, 1.0F});

  auto const after_read = camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after_read.row_0, matrix.row_0);
  EXPECT_EQ(after_read.row_1, matrix.row_1);
  EXPECT_EQ(after_read.row_2, matrix.row_2);
  EXPECT_EQ(after_read.row_3, matrix.row_3);
}

// =============================================================================
// =============================================================================

// An axis-aligned view must preserve handedness and front-to-back depth
// ordering.
TEST(GGEMSSceneCamera, ZeroYawAndPitchUseExpectedViewBasisAndDepth) {
  Camera camera{};
  camera.SetViewportSize(800U, 800U);
  camera.SetOrbitAngles(0.0F, 0.0F);
  camera.SetZoom(1.0F);

  Matrix4Rows const expected{
    .row_0 = {0.0F, 0.0F, -1.0F, 0.0F},
    .row_1 = {0.0F, 1.0F, 0.0F, 0.0F},
    .row_2 = {-0.05F, 0.0F, 0.0F, 0.5F},
    .row_3 = {0.0F, 0.0F, 0.0F, 1.0F},
  };

  Matrix4Rows const matrix = camera.BuildWorldToClipMatrix();
  ExpectMatrixNear(matrix, expected);

  auto const target = TransformPoint(matrix, Point3{});
  auto const positive_x = TransformPoint(matrix, Point3{.x = 1.0F});
  auto const negative_x = TransformPoint(matrix, Point3{.x = -1.0F});

  EXPECT_NEAR(target[0], 0.0F, k_tolerance);
  EXPECT_NEAR(target[1], 0.0F, k_tolerance);
  EXPECT_NEAR(target[2], 0.5F, k_tolerance);
  EXPECT_NEAR(target[3], 1.0F, k_tolerance);

  EXPECT_NEAR(positive_x[2], 0.45F, k_tolerance);
  EXPECT_NEAR(negative_x[2], 0.55F, k_tolerance);
  EXPECT_LT(positive_x[2], target[2]);
  EXPECT_LT(target[2], negative_x[2]);
}

// =============================================================================
// =============================================================================

// All signed world axes must project with the intended screen orientation and
// depth.
TEST(GGEMSSceneCamera,
     CanonicalViewProjectsSixGlobalAxesWithPositiveYDownConvention) {
  Camera camera{};
  camera.SetViewportSize(800U, 800U);
  camera.Reset();

  Matrix4Rows const matrix = camera.BuildWorldToClipMatrix();

  auto const positive_x = TransformPoint(matrix, Point3{.x = 1.0F});
  auto const negative_x = TransformPoint(matrix, Point3{.x = -1.0F});
  auto const positive_y = TransformPoint(matrix, Point3{.y = 1.0F});
  auto const negative_y = TransformPoint(matrix, Point3{.y = -1.0F});
  auto const positive_z = TransformPoint(matrix, Point3{.z = 1.0F});
  auto const negative_z = TransformPoint(matrix, Point3{.z = -1.0F});

  EXPECT_NEAR(positive_x[0], 0.8F * k_sin_sixty_degrees, k_tolerance);
  EXPECT_NEAR(positive_x[1], 0.4F * k_sin_twenty_five_degrees, k_tolerance);
  EXPECT_NEAR(positive_x[2], 0.5F - (0.025F * k_cos_twenty_five_degrees),
              k_tolerance);

  EXPECT_NEAR(negative_x[0], -0.8F * k_sin_sixty_degrees, k_tolerance);
  EXPECT_NEAR(negative_x[1], -0.4F * k_sin_twenty_five_degrees, k_tolerance);
  EXPECT_NEAR(negative_x[2], 0.5F + (0.025F * k_cos_twenty_five_degrees),
              k_tolerance);

  EXPECT_NEAR(positive_y[0], 0.0F, k_tolerance);
  EXPECT_NEAR(positive_y[1], 0.8F * k_cos_twenty_five_degrees, k_tolerance);
  EXPECT_NEAR(positive_y[2], 0.5F + (0.05F * k_sin_twenty_five_degrees),
              k_tolerance);

  EXPECT_NEAR(negative_y[0], 0.0F, k_tolerance);
  EXPECT_NEAR(negative_y[1], -0.8F * k_cos_twenty_five_degrees, k_tolerance);
  EXPECT_NEAR(negative_y[2], 0.5F - (0.05F * k_sin_twenty_five_degrees),
              k_tolerance);

  EXPECT_NEAR(positive_z[0], -0.4F, k_tolerance);
  EXPECT_NEAR(positive_z[1],
              0.8F * k_sin_sixty_degrees * k_sin_twenty_five_degrees,
              k_tolerance);
  EXPECT_NEAR(positive_z[2],
              0.5F - (0.05F * k_sin_sixty_degrees * k_cos_twenty_five_degrees),
              k_tolerance);

  EXPECT_NEAR(negative_z[0], 0.4F, k_tolerance);
  EXPECT_NEAR(negative_z[1],
              -0.8F * k_sin_sixty_degrees * k_sin_twenty_five_degrees,
              k_tolerance);
  EXPECT_NEAR(negative_z[2],
              0.5F + (0.05F * k_sin_sixty_degrees * k_cos_twenty_five_degrees),
              k_tolerance);
}

// =============================================================================
// =============================================================================

// A wide viewport must correct horizontal aspect without altering vertical
// projection or depth.
TEST(GGEMSSceneCamera,
     SixteenByNineViewportScalesOnlyHorizontalClipCoordinate) {
  Camera square_camera{};
  square_camera.SetViewportSize(900U, 900U);
  square_camera.Reset();

  Camera wide_camera{};
  wide_camera.SetViewportSize(1600U, 900U);
  wide_camera.Reset();

  auto const square_positive_x =
    TransformPoint(square_camera.BuildWorldToClipMatrix(), Point3{.x = 1.0F});
  auto const wide_positive_x =
    TransformPoint(wide_camera.BuildWorldToClipMatrix(), Point3{.x = 1.0F});

  constexpr float k_sixteen_by_nine_inverse_aspect{9.0F / 16.0F};

  EXPECT_NEAR(wide_positive_x[0],
              square_positive_x[0] * k_sixteen_by_nine_inverse_aspect,
              k_tolerance);
  EXPECT_NEAR(wide_positive_x[1], square_positive_x[1], k_tolerance);
  EXPECT_NEAR(wide_positive_x[2], square_positive_x[2], k_tolerance);
}

// =============================================================================
// =============================================================================

// An asymmetric drag must move the expected view target without shifting its
// depth.
TEST(GGEMSSceneCamera, PanMovesTargetAlongBothViewAxes) {
  Camera camera{};
  camera.SetViewportSize(100U, 100U);
  camera.SetOrbitAngles(0.0F, 0.0F);
  camera.SetZoom(1.0F);

  camera.Pan(10.0F, 20.0F);

  Matrix4Rows const matrix = camera.BuildWorldToClipMatrix();
  auto const origin = TransformPoint(matrix, Point3{});
  auto const target =
    TransformPoint(matrix, Point3{.x = 0.0F, .y = -0.4F, .z = 0.2F});

  EXPECT_NEAR(origin[0], 0.2F, k_tolerance);
  EXPECT_NEAR(origin[1], 0.4F, k_tolerance);
  EXPECT_NEAR(origin[2], 0.5F, k_tolerance);

  EXPECT_NEAR(target[0], 0.0F, k_tolerance);
  EXPECT_NEAR(target[1], 0.0F, k_tolerance);
  EXPECT_NEAR(target[2], 0.5F, k_tolerance);
  EXPECT_NEAR(target[3], 1.0F, k_tolerance);
}

// =============================================================================
// =============================================================================

// Zoom must enlarge the screen projection while preserving scene depth.
TEST(GGEMSSceneCamera, ZoomScalesXYWithoutChangingDepth) {
  Camera camera{};
  camera.SetViewportSize(800U, 800U);
  camera.SetOrbitAngles(0.0F, 0.0F);
  camera.SetZoom(0.5F);

  Matrix4Rows const before_matrix = camera.BuildWorldToClipMatrix();
  auto const positive_y_before =
    TransformPoint(before_matrix, Point3{.y = 1.0F});
  auto const positive_x_before =
    TransformPoint(before_matrix, Point3{.x = 1.0F});

  camera.ZoomBy(1.0F);

  Matrix4Rows const after_matrix = camera.BuildWorldToClipMatrix();
  auto const positive_y_after = TransformPoint(after_matrix, Point3{.y = 1.0F});
  auto const positive_x_after = TransformPoint(after_matrix, Point3{.x = 1.0F});

  EXPECT_NEAR(positive_y_before[1], 0.5F, k_tolerance);
  EXPECT_NEAR(positive_y_after[1], 0.56F, k_tolerance);
  EXPECT_NEAR(positive_x_after[2], positive_x_before[2], k_tolerance);
}

// =============================================================================
// =============================================================================

// Positive and negative pitch limits must prevent the camera from flipping over
// its poles.
TEST(GGEMSSceneCamera, PitchIsClampedToExactNinetyDegreePoles) {
  Camera positive_over_limit{};
  positive_over_limit.SetOrbitAngles(0.0F, 100.0F);
  positive_over_limit.SetZoom(1.0F);

  Camera positive_limit{};
  positive_limit.SetOrbitAngles(0.0F, 90.0F);
  positive_limit.SetZoom(1.0F);

  ExpectMatrixNear(positive_over_limit.BuildWorldToClipMatrix(),
                   positive_limit.BuildWorldToClipMatrix());

  Camera negative_over_limit{};
  negative_over_limit.SetOrbitAngles(0.0F, -100.0F);
  negative_over_limit.SetZoom(1.0F);

  Camera negative_limit{};
  negative_limit.SetOrbitAngles(0.0F, -90.0F);
  negative_limit.SetZoom(1.0F);

  ExpectMatrixNear(negative_over_limit.BuildWorldToClipMatrix(),
                   negative_limit.BuildWorldToClipMatrix());
}

// =============================================================================
// =============================================================================

// Pixel drags must apply the negated angular sensitivity on both axes.
TEST(GGEMSSceneCamera, OrbitByPixelsAppliesTheOrbitPixelPolicy) {
  Camera by_pixels{};
  by_pixels.SetViewportSize(800U, 800U);
  by_pixels.SetOrbitAngles(0.0F, 0.0F);
  by_pixels.SetZoom(1.0F);
  by_pixels.OrbitByPixels(5.0F, -10.0F);

  auto const diagnostics = by_pixels.GetDiagnostics();
  EXPECT_NEAR(diagnostics.yaw_degrees, -1.0F, k_tolerance);
  EXPECT_NEAR(diagnostics.pitch_degrees, 2.0F, k_tolerance);

  Camera by_degrees{};
  by_degrees.SetViewportSize(800U, 800U);
  by_degrees.SetOrbitAngles(0.0F, 0.0F);
  by_degrees.SetZoom(1.0F);
  by_degrees.Orbit(-1.0F, 2.0F);

  ExpectMatrixNear(by_pixels.BuildWorldToClipMatrix(),
                   by_degrees.BuildWorldToClipMatrix());
}

// =============================================================================
// =============================================================================

// A minimized viewport must retain its last valid projection instead of
// dividing by zero.
TEST(GGEMSSceneCamera, ZeroExtentKeepsLastValidViewportExtent) {
  Camera camera{};
  camera.SetViewportSize(1600U, 900U);

  Matrix4Rows const expected = camera.BuildWorldToClipMatrix();

  camera.SetViewportSize(0U, 720U);
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);

  camera.SetViewportSize(1280U, 0U);
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);
}

// =============================================================================
// =============================================================================

// Rejected angular input must leave the complete previous view unchanged.
TEST(GGEMSSceneCamera, InvalidAngularInputsPreserveThePreviousView) {
  using ConversionFailure =
    std::bad_expected_access<ggems::units::UnitConversionError>;
  Camera camera{};
  camera.SetOrbitAngles(-20.0F, 15.0F);
  auto const expected = camera.BuildWorldToClipMatrix();
  auto const expect_unchanged = [&camera, &expected] -> void {
    auto const actual = camera.BuildWorldToClipMatrix();
    EXPECT_EQ(actual.row_0, expected.row_0);
    EXPECT_EQ(actual.row_1, expected.row_1);
    EXPECT_EQ(actual.row_2, expected.row_2);
    EXPECT_EQ(actual.row_3, expected.row_3);
  };
  std::array const invalid_values{
    std::numeric_limits<float>::quiet_NaN(),
    std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::max(),
    std::numeric_limits<float>::lowest(),
  };
  for (float const invalid : invalid_values) {
    SCOPED_TRACE(invalid);
    EXPECT_THROW(camera.SetOrbitAngles(invalid, 12.0F), ConversionFailure);
    expect_unchanged();
    EXPECT_THROW(camera.SetOrbitAngles(12.0F, invalid), ConversionFailure);
    expect_unchanged();
    EXPECT_THROW(camera.Orbit(invalid, 12.0F), ConversionFailure);
    expect_unchanged();
    EXPECT_THROW(camera.Orbit(12.0F, invalid), ConversionFailure);
    expect_unchanged();
    EXPECT_THROW(camera.OrbitByPixels(invalid, 5.0F), ConversionFailure);
    expect_unchanged();
    EXPECT_THROW(camera.OrbitByPixels(5.0F, invalid), ConversionFailure);
    expect_unchanged();
  }
}

} // namespace

// =============================================================================
// =============================================================================

// World corners stay visible across physical scales, orbit and viewport resize.
TEST_P(GGEMSSceneCameraProjection, FramesWorldAtSmallAndLargePhysicalScales) {
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    SCOPED_TRACE(scale);
    Camera camera{};
    camera.SetProjection(GetParam());
    camera.SetViewportSize(1200U, 700U);
    camera.FrameWorld({2.0 * scale, 3.0 * scale, scale});

    for (auto const viewport : {
           std::array{1200U, 700U},
           std::array{320U, 720U},
           std::array{30U, 1000U},
         }) {
      camera.SetViewportSize(viewport[0], viewport[1]);
      camera.Reset();
      camera.Orbit(37.0F, -12.0F);
      auto const matrix = camera.BuildWorldToClipMatrix();

      for (auto const &corner : k_world_corners) {
        auto const point =
          TransformPoint(matrix, Point3{
                                   .x = corner.x * static_cast<float>(scale),
                                   .y = corner.y * static_cast<float>(scale),
                                   .z = corner.z * static_cast<float>(scale),
                                 });
        EXPECT_GT(point[3], 0.0F);
        EXPECT_LT(std::abs(point[0] / point[3]), 1.0F);
        EXPECT_LT(std::abs(point[1] / point[3]), 1.0F);
        EXPECT_GT(point[2], 0.0F);
        EXPECT_LT(point[2], point[3]);
      }
    }
  }
}

// =============================================================================
// =============================================================================

// Pixel panning must remain responsive when World framing needs a tiny scale.
TEST(GGEMSSceneCamera, LargeWorldRetainsPixelPanAndReset) {
  Camera camera{};
  camera.SetViewportSize(1000U, 1000U);
  camera.FrameWorld({1.0e6, 2.0e6, 3.0e6});
  auto const framed = camera.BuildWorldToClipMatrix();
  camera.Pan(100.0F, 50.0F);
  auto const origin = TransformPoint(camera.BuildWorldToClipMatrix(), Point3{});
  EXPECT_NEAR(origin[0], 0.2F, k_tolerance);
  EXPECT_NEAR(origin[1], 0.1F, k_tolerance);
  camera.ZoomBy(3.0F);
  camera.Reset();
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), framed);
}

// =============================================================================
// =============================================================================

// The reported viewing frame remains orthonormal and right-handed near the
// observed yaw boundaries, with the same axes used by the actual projection.
TEST(GGEMSSceneCamera, BoundaryAnglesPreserveViewingBasisAndProjection) {
  auto const dot = [](auto const &first, auto const &second) -> double {
    return (static_cast<double>(first[0]) * second[0]) +
           (static_cast<double>(first[1]) * second[1]) +
           (static_cast<double>(first[2]) * second[2]);
  };

  for (float pitch : {-5.0F, 0.0F, 5.0F}) {
    SCOPED_TRACE(pitch);
    for (float yaw : {
           0.0F,
           90.0F,
           179.0F,
           180.0F,
           181.0F,
           270.0F,
           359.0F,
           360.0F,
           361.0F,
         }) {
      SCOPED_TRACE(yaw);
      Camera camera{};
      camera.SetViewportSize(1200U, 800U);
      camera.SetZoom(1.0F);
      camera.SetOrbitAngles(yaw, pitch);

      auto const state = camera.GetDiagnostics();
      auto const matrix = camera.BuildWorldToClipMatrix();
      double const yaw_rad = yaw * std::numbers::pi / 180.0;
      double const pitch_rad = pitch * std::numbers::pi / 180.0;
      std::array const expected_right{
        -std::sin(yaw_rad),
        0.0,
        -std::cos(yaw_rad),
      };
      std::array const expected_up{
        -std::cos(yaw_rad) * std::sin(pitch_rad),
        std::cos(pitch_rad),
        std::sin(yaw_rad) * std::sin(pitch_rad),
      };
      std::array const expected_forward{
        -std::cos(yaw_rad) * std::cos(pitch_rad),
        -std::sin(pitch_rad),
        std::sin(yaw_rad) * std::cos(pitch_rad),
      };

      EXPECT_NEAR(dot(state.right, state.right), 1.0, 2.0e-6);
      EXPECT_NEAR(dot(state.up, state.up), 1.0, 2.0e-6);
      EXPECT_NEAR(dot(state.forward, state.forward), 1.0, 2.0e-6);
      EXPECT_NEAR(dot(state.right, state.up), 0.0, 2.0e-6);
      EXPECT_NEAR(dot(state.right, state.forward), 0.0, 2.0e-6);
      EXPECT_NEAR(dot(state.up, state.forward), 0.0, 2.0e-6);

      std::array const right_cross_up{
        (state.right[1] * state.up[2]) - (state.right[2] * state.up[1]),
        (state.right[2] * state.up[0]) - (state.right[0] * state.up[2]),
        (state.right[0] * state.up[1]) - (state.right[1] * state.up[0]),
      };
      EXPECT_NEAR(-dot(right_cross_up, state.forward), 1.0, 2.0e-6);

      for (std::size_t index = 0; index < 3; ++index) {
        EXPECT_NEAR(state.right[index], expected_right[index], 2.0e-6);
        EXPECT_NEAR(state.up[index], expected_up[index], 2.0e-6);
        EXPECT_NEAR(state.forward[index], expected_forward[index], 2.0e-6);
        EXPECT_NEAR(matrix.row_0[index], expected_right[index] * (2.0 / 3.0),
                    5.0e-6);
        EXPECT_NEAR(matrix.row_1[index], expected_up[index], 5.0e-6);
        EXPECT_NEAR(matrix.row_2[index], expected_forward[index] * 0.05,
                    5.0e-6);
      }
    }
  }
}

// =============================================================================
// =============================================================================

// Positive and negative pixel updates cross 180/360 without reversing yaw or
// changing pitch, regardless of batching and zero-motion frames between drags.
TEST(GGEMSSceneCamera, PixelOrbitCrossesYawBoundariesWithoutReversal) {
  for (float pitch : {-5.0F, 0.0F, 5.0F}) {
    SCOPED_TRACE(pitch);
    for (float boundary : {180.0F, 360.0F}) {
      SCOPED_TRACE(boundary);
      for (float direction : {-1.0F, 1.0F}) {
        SCOPED_TRACE(direction);
        float const start = boundary + direction;
        for (int step_pixels : {1, 2, 5, 10}) {
          SCOPED_TRACE(step_pixels);
          Camera camera{};
          camera.SetOrbitAngles(start, pitch);
          auto const before = camera.GetDiagnostics();
          float previous_yaw = before.yaw_degrees;

          for (int step = 0; step < 10 / step_pixels; ++step) {
            camera.OrbitByPixels(direction * static_cast<float>(step_pixels),
                                 0.0F);
            auto const current = camera.GetDiagnostics();
            EXPECT_LT(direction * (current.yaw_degrees - previous_yaw), 0.0F);
            EXPECT_EQ(current.pitch_degrees, before.pitch_degrees);
            previous_yaw = current.yaw_degrees;

            if (step_pixels == 2 || step_pixels == 5) {
              camera.OrbitByPixels(0.0F, 0.0F);
            }
          }

          Camera direct{};
          direct.SetOrbitAngles(boundary - direction, pitch);
          EXPECT_NEAR(camera.GetDiagnostics().yaw_degrees, boundary - direction,
                      1.0e-3F);
          auto const actual = camera.BuildWorldToClipMatrix();
          auto const expected = direct.BuildWorldToClipMatrix();
          std::array const actual_rows{
            actual.row_0,
            actual.row_1,
            actual.row_2,
            actual.row_3,
          };
          std::array const expected_rows{
            expected.row_0,
            expected.row_1,
            expected.row_2,
            expected.row_3,
          };
          for (std::size_t row = 0; row < 4; ++row) {
            for (std::size_t col = 0; col < 4; ++col) {
              EXPECT_NEAR(actual_rows[row][col], expected_rows[row][col],
                          2.0e-5F);
            }
          }
        }
      }
    }
  }
}
