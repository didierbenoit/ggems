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
 * \brief Checks cursor reference intersections and shared physical grid
 * spacing.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace {

using Camera = ggems::ui::detail::GGEMSSceneCamera;
using Grid = ggems::ui::detail::GGEMSCartesianGrid;

// Exact axial views give an independent signed-coordinate reference in both
// projections; pixel coordinates use the same logical extent as the camera.
TEST(GGEMSSceneCameraRay, AxialCenterAndOffsetHaveKnownWorldCoordinates) {
  for (auto projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    for (auto plane : {
           Camera::ViewPlane::XY,
           Camera::ViewPlane::XZ,
           Camera::ViewPlane::YZ,
         }) {
      Camera camera;
      camera.SetViewportSize(800U, 600U);
      camera.SetProjection(projection);
      camera.SetView(plane);
      camera.SetZoom(0.5F);
      auto const center = camera.IntersectReferencePlane(400.0, 300.0, plane);
      ASSERT_TRUE(center.has_value());
      for (double coordinate : *center) {
        EXPECT_NEAR(coordinate, 0.0, 1.0e-6);
      }
      auto const offset = camera.IntersectReferencePlane(550.0, 450.0, plane);
      ASSERT_TRUE(offset.has_value());
      std::array<double, 3> expected{};
      switch (plane) {
      case Camera::ViewPlane::XY:
        expected = {1.0, 1.0, 0.0};
        break;
      case Camera::ViewPlane::XZ:
        expected = {1.0, 0.0, -1.0};
        break;
      case Camera::ViewPlane::YZ:
        expected = {0.0, 1.0, -1.0};
        break;
      }
      for (std::size_t axis = 0U; axis < 3U; ++axis) {
        EXPECT_NEAR((*offset)[axis], expected[axis], 1.0e-6);
      }
    }
  }
}

TEST(GGEMSSceneCameraRay, PanResizeAndLogicalPixelScalingPreserveKnownPoint) {
  for (auto projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    Camera camera;
    camera.SetViewportSize(800U, 600U);
    camera.SetProjection(projection);
    camera.SetView(Camera::ViewPlane::YZ);
    camera.SetZoom(0.5F);
    camera.Pan(30.0F, -15.0F);
    auto const point =
      camera.IntersectReferencePlane(550.0, 300.0, Camera::ViewPlane::YZ);
    ASSERT_TRUE(point.has_value());
    EXPECT_NEAR((*point)[0], 0.0, 1.0e-7);
    EXPECT_NEAR((*point)[1], 0.1, 1.0e-7);
    EXPECT_NEAR((*point)[2], -0.8, 1.0e-7);
    camera.SetViewportSize(1600U, 1200U);
    auto const resized =
      camera.IntersectReferencePlane(1100.0, 600.0, Camera::ViewPlane::YZ);
    ASSERT_TRUE(resized.has_value());
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      EXPECT_NEAR((*resized)[axis], (*point)[axis], 1.0e-7);
    }
  }
}

TEST(GGEMSSceneCameraRay,
     RejectsOutsideInvalidParallelAndClippedIntersections) {
  Camera camera;
  camera.SetViewportSize(200U, 200U);
  camera.SetView(Camera::ViewPlane::YZ);
  for (auto const pixel : {
         std::array{-1.0, 100.0},
         std::array{200.0, 100.0},
         std::array{100.0, -1.0},
         std::array{100.0, 200.0},
         std::array{std::numeric_limits<double>::quiet_NaN(), 100.0},
         std::array{100.0, std::numeric_limits<double>::infinity()},
       }) {
    EXPECT_FALSE(camera.BuildScreenRay(pixel[0], pixel[1]).has_value());
    EXPECT_FALSE(
      camera.IntersectReferencePlane(pixel[0], pixel[1], Camera::ViewPlane::YZ)
        .has_value());
  }
  for (auto projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    camera.Reset();
    camera.SetProjection(projection);
    camera.SetView(Camera::ViewPlane::YZ);
    EXPECT_FALSE(
      camera.IntersectReferencePlane(100.0, 100.0, Camera::ViewPlane::XY)
        .has_value());
    camera.SetOrbitAngles(0.0F, 45.0F);
    camera.Pan(0.0F, 10000.0F);
    EXPECT_FALSE(
      camera.IntersectReferencePlane(100.0, 100.0, Camera::ViewPlane::YZ)
        .has_value());
  }
  camera.SetProjection(Camera::Projection::Perspective);
  camera.SetView(Camera::ViewPlane::YZ);
  camera.Pan(20.0F, 0.0F);
  EXPECT_FALSE(
    camera.IntersectReferencePlane(50.0, 100.0, Camera::ViewPlane::XY)
      .has_value());
}

TEST(GGEMSSceneCameraRay,
     PhysicalScaleAndRayDirectionMatchIndependentGeometry) {
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    for (auto projection :
         {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
      Camera camera;
      camera.SetViewportSize(800U, 600U);
      camera.FrameWorld({2.0 * scale, 3.0 * scale, 4.0 * scale});
      camera.SetProjection(projection);
      camera.SetView(Camera::ViewPlane::YZ);
      auto const point =
        camera.IntersectReferencePlane(550.0, 450.0, Camera::ViewPlane::YZ);
      ASSERT_TRUE(point.has_value());
      double const offset = std::sqrt(29.0) * scale / (2.0 * 0.85);
      EXPECT_NEAR((*point)[0], 0.0, scale * 2.0e-6);
      EXPECT_NEAR((*point)[1], offset, scale * 2.0e-6);
      EXPECT_NEAR((*point)[2], -offset, scale * 2.0e-6);
      auto const ray = camera.BuildScreenRay(550.0, 450.0);
      ASSERT_TRUE(ray.has_value());
      EXPECT_NEAR(
        std::hypot(ray->direction[0], ray->direction[1], ray->direction[2]),
        1.0, 1.0e-12);
      EXPECT_LT(ray->direction[0], 0.0);
      if (projection == Camera::Projection::Orthographic) {
        EXPECT_NEAR(ray->direction[1], 0.0, 1.0e-12);
        EXPECT_NEAR(ray->direction[2], 0.0, 1.0e-12);
      } else {
        EXPECT_GT(ray->direction[1], 0.0);
        EXPECT_LT(ray->direction[2], 0.0);
      }
    }
  }
}

// In addition to the independent axial oracles, verify that general plane rays
// invert the actual render projection after navigation at every physical scale.
TEST(GGEMSSceneCameraRay, FreeViewPlanePointsRoundTripThroughSharedProjection) {
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    for (auto projection :
         {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
      Camera camera;
      camera.SetViewportSize(1000U, 700U);
      camera.FrameWorld({2.0 * scale, 3.0 * scale, 4.0 * scale});
      camera.SetProjection(projection);
      camera.Pan(30.0F, -15.0F);
      auto const matrix = camera.BuildWorldToClipMatrix();
      std::array const rows{
        matrix.row_0,
        matrix.row_1,
        matrix.row_2,
        matrix.row_3,
      };
      for (auto plane : {
             Camera::ViewPlane::XY,
             Camera::ViewPlane::XZ,
             Camera::ViewPlane::YZ,
           }) {
        std::array<double, 3> expected{};
        switch (plane) {
        case Camera::ViewPlane::XY:
          expected = {0.1 * scale, -0.15 * scale, 0.0};
          break;
        case Camera::ViewPlane::XZ:
          expected = {0.1 * scale, 0.0, 0.2 * scale};
          break;
        case Camera::ViewPlane::YZ:
          expected = {0.0, -0.15 * scale, 0.2 * scale};
          break;
        }
        std::array<double, 4> clip{};
        for (std::size_t row = 0U; row < 4U; ++row) {
          clip[row] = rows[row][3];
          for (std::size_t axis = 0U; axis < 3U; ++axis) {
            clip[row] += rows[row][axis] * expected[axis];
          }
        }
        auto const actual = camera.IntersectReferencePlane(
          ((clip[0] / clip[3]) + 1.0) * 500.0,
          ((clip[1] / clip[3]) + 1.0) * 350.0, plane);
        ASSERT_TRUE(actual.has_value());
        for (std::size_t axis = 0U; axis < 3U; ++axis) {
          EXPECT_NEAR((*actual)[axis], expected[axis], scale * 2.0e-6);
        }
      }
      auto const after = camera.BuildWorldToClipMatrix();
      EXPECT_EQ(after.row_0, matrix.row_0);
      EXPECT_EQ(after.row_1, matrix.row_1);
      EXPECT_EQ(after.row_2, matrix.row_2);
      EXPECT_EQ(after.row_3, matrix.row_3);
    }
  }
}

// The allowed physical intervals are observable in both ImGui and the shader.
// Accept the adjacent decade at a binary32 rounding boundary.
auto ExpectPhysicalStep(float step) -> void {
  ASSERT_GT(step, 0.0F);
  double const decade = std::pow(10.0, std::floor(std::log10(step)));
  EXPECT_TRUE(step == static_cast<float>(decade) ||
              step == static_cast<float>(2.0 * decade) ||
              step == static_cast<float>(5.0 * decade) ||
              step == static_cast<float>(10.0 * decade));
}

TEST(GGEMSCartesianGrid, DefaultsAreHiddenAndSpacingIsAvailableWithoutShowing) {
  Grid grid;
  EXPECT_EQ(grid.visible, (std::array<bool, 3U>{}));
  EXPECT_EQ(grid.polar_visible, (std::array<bool, 3U>{}));
  EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::XY);
  EXPECT_FLOAT_EQ(grid.GetMinorStepM(), 0.0F);
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.SetZoom(0.5F);
  grid.Update(camera);
  EXPECT_FLOAT_EQ(grid.GetMinorStepM(), 0.2F);
  EXPECT_EQ(grid.visible, (std::array<bool, 3U>{}));
  EXPECT_EQ(grid.polar_visible, (std::array<bool, 3U>{}));
}

// All six choices survive spacing updates and camera navigation independently.
TEST(GGEMSCartesianGrid, CartesianAndPolarVisibilitySurviveCameraUpdates) {
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.FrameWorld({2.0, 3.0, 4.0});
  for (unsigned cartesian = 0U; cartesian < 8U; ++cartesian) {
    for (unsigned polar = 0U; polar < 8U; ++polar) {
      Grid grid;
      for (std::size_t plane = 0U; plane < 3U; ++plane) {
        grid.visible[plane] = (cartesian & (1U << plane)) != 0U;
        grid.polar_visible[plane] = (polar & (1U << plane)) != 0U;
      }
      auto const cartesian_before = grid.visible;
      auto const polar_before = grid.polar_visible;
      for (auto projection :
           {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
        camera.SetProjection(projection);
        for (auto plane : {Camera::ViewPlane::XY, Camera::ViewPlane::XZ,
                           Camera::ViewPlane::YZ}) {
          camera.SetView(plane);
          camera.Pan(35.0F, -19.0F);
          camera.ZoomBy(2.0F);
          camera.OrbitByPixels(10.0F, 5.0F);
          grid.Update(camera);
          EXPECT_EQ(grid.visible, cartesian_before);
          EXPECT_EQ(grid.polar_visible, polar_before);
          ExpectPhysicalStep(grid.GetMinorStepM());
        }
      }
      camera.Reset();
      grid.Update(camera);
      EXPECT_EQ(grid.visible, cartesian_before);
      EXPECT_EQ(grid.polar_visible, polar_before);
    }
  }
}

TEST(GGEMSCartesianGrid, SharedIntervalUsesReadablePhysicalStepsAcrossScales) {
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    for (auto projection :
         {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
      for (auto const viewport :
           {std::array{1200U, 600U}, std::array{300U, 900U}}) {
        Camera camera;
        camera.SetViewportSize(viewport[0], viewport[1]);
        camera.FrameWorld({scale, 2.0 * scale, 3.0 * scale});
        camera.SetProjection(projection);
        Grid grid;
        for (float zoom_factor : {1.0F, 0.1F, 100.0F, 10000.0F}) {
          camera.Reset();
          camera.SetZoom(camera.GetDiagnostics().zoom * zoom_factor);
          grid.Update(camera);
          float const step = grid.GetMinorStepM();
          ExpectPhysicalStep(step);
          double const nominal_pixels =
            0.5 * camera.GetDiagnostics().zoom * viewport[1] * step;
          EXPECT_GE(nominal_pixels, 16.0);
          EXPECT_LE(nominal_pixels, 72.0);
        }
      }
    }
  }
}

TEST(GGEMSCartesianGrid,
     HysteresisRetainsStepThroughPanOrbitAndPlaneSelection) {
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.FrameWorld({2.0, 2.0, 2.0});
  Grid grid;
  grid.Update(camera);
  float const initial_step = grid.GetMinorStepM();
  float const initial_zoom = camera.GetDiagnostics().zoom;
  for (float factor : {1.01F, 0.99F, 1.005F, 1.0F}) {
    camera.SetZoom(initial_zoom * factor);
    grid.Update(camera);
    EXPECT_FLOAT_EQ(grid.GetMinorStepM(), initial_step);
  }
  for (auto plane :
       {Camera::ViewPlane::XY, Camera::ViewPlane::XZ, Camera::ViewPlane::YZ}) {
    grid.reference_plane = plane;
    auto const index = static_cast<std::size_t>(plane);
    grid.visible[index] = !grid.visible[index];
    camera.SetView(plane);
    camera.Pan(35.0F, -19.0F);
    camera.Orbit(25.0F, -10.0F);
    grid.Update(camera);
    EXPECT_FLOAT_EQ(grid.GetMinorStepM(), initial_step);
    EXPECT_EQ(grid.reference_plane, plane);
  }
  camera.SetZoom(initial_zoom * 100.0F);
  grid.Update(camera);
  EXPECT_LT(grid.GetMinorStepM(), initial_step);
  float const zoomed_step = grid.GetMinorStepM();
  camera.SetViewportSize(8000U, 6000U);
  grid.Update(camera);
  EXPECT_LT(grid.GetMinorStepM(), zoomed_step);
}

TEST(GGEMSCartesianGrid, PhysicalScaleCovarianceRetainsExactShaderFloat) {
  Camera large;
  large.SetViewportSize(800U, 600U);
  large.SetZoom(2.0e8F);
  Camera small;
  small.SetViewportSize(800U, 600U);
  small.SetZoom(2.0e17F);
  Grid large_grid;
  Grid small_grid;
  large_grid.Update(large);
  small_grid.Update(small);
  float const large_step = large_grid.GetMinorStepM();
  float const small_step = small_grid.GetMinorStepM();
  ASSERT_GT(large_step, 0.0F);
  EXPECT_FLOAT_EQ(small_step / large_step, 1.0e-9F);
  for (int frame = 0; frame < 20; ++frame) {
    large_grid.Update(large);
    small_grid.Update(small);
    EXPECT_FLOAT_EQ(large_grid.GetMinorStepM(), large_step);
    EXPECT_FLOAT_EQ(small_grid.GetMinorStepM(), small_step);
  }
}

TEST(GGEMSCartesianGrid, BothProjectionsPublishKnownIntervalsAtTheTargetScale) {
  std::array const zooms{0.1F, 0.2F, 0.5F, 1.0F, 2.0F, 5.0F, 10.0F};
  std::array const expected{1.0F, 0.5F, 0.2F, 0.1F, 0.05F, 0.02F, 0.01F};
  for (auto projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    for (std::size_t index = 0U; index < zooms.size(); ++index) {
      Camera camera;
      camera.SetViewportSize(800U, 600U);
      camera.SetProjection(projection);
      camera.SetZoom(zooms[index]);
      Grid grid;
      grid.Update(camera);
      EXPECT_FLOAT_EQ(grid.GetMinorStepM(), expected[index]);
    }
  }
}

} // namespace
