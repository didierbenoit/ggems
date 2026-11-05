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
 * \brief Checks cursor reference intersections and bounded Cartesian lattice
 * density.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
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

auto ExpectLatticePositions(Grid::PlaneLines const &plane, std::size_t axis,
                            double tolerance) -> void {
  for (std::uint32_t line = 0U; line < plane.line_count[axis]; ++line) {
    double const coordinate =
      (static_cast<double>(plane.first_index[axis]) + line) *
      plane.minor_step_m;
    EXPECT_GE(coordinate, plane.minimum_m[axis] - tolerance);
    EXPECT_LE(coordinate, plane.maximum_m[axis] + tolerance);
  }
}

auto ExpectLatticeBounds(Grid::PlaneLines const &plane, std::size_t axis,
                         double world_half) -> void {
  double const tolerance = world_half * 1.0e-10;
  EXPECT_GE(plane.minimum_m[axis], -world_half - tolerance);
  EXPECT_LE(plane.maximum_m[axis], world_half + tolerance);
  ExpectLatticePositions(plane, axis, tolerance);
  if (plane.minimum_m[axis] <= 0.0 && plane.maximum_m[axis] >= 0.0) {
    EXPECT_LE(plane.first_index[axis], 0);
    EXPECT_GT(static_cast<std::int64_t>(plane.first_index[axis]) +
                plane.line_count[axis],
              0);
  }
}

// This oracle checks physical lattice values rather than duplicating the
// spacing selector or the frustum clipper.
auto ExpectLattice(Grid::PlaneLines const &plane,
                   std::array<double, 2> const &world_half) -> void {
  ASSERT_GT(plane.minor_step_m, 0.0);
  double const decade =
    std::pow(10.0, std::floor(std::log10(plane.minor_step_m)));
  double const normalized = plane.minor_step_m / decade;
  EXPECT_TRUE(std::abs(normalized - 1.0) < 1.0e-10 ||
              std::abs(normalized - 2.0) < 1.0e-10 ||
              std::abs(normalized - 5.0) < 1.0e-10);
  EXPECT_DOUBLE_EQ(plane.major_step_m, 5.0 * plane.minor_step_m);
  EXPECT_LE(plane.line_count[0] + plane.line_count[1], 256U);
  for (std::size_t axis = 0U; axis < 2U; ++axis) {
    ExpectLatticeBounds(plane, axis, world_half[axis]);
  }
}

TEST(GGEMSCartesianGrid,
     DefaultsAreExplicitAndHiddenPlanesStillHaveDiagnostics) {
  Grid grid;
  EXPECT_EQ(grid.visible, (std::array<bool, 3>{}));
  EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::XY);
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.SetView(Camera::ViewPlane::XY);
  grid.Update(camera, {1.0, 2.0, 3.0});
  EXPECT_GT(grid.GetPlanes()[0].minor_step_m, 0.0);
  EXPECT_EQ(grid.GetPlanes()[1].line_count, (std::array<std::uint32_t, 2>{}));
  EXPECT_EQ(grid.GetPlanes()[2].line_count, (std::array<std::uint32_t, 2>{}));
}

TEST(GGEMSCartesianGrid, AllPlanesRemainBoundedAcrossScaleZoomAndAspect) {
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
          grid.Update(camera, {scale, 2.0 * scale, 3.0 * scale});
          auto const &planes = grid.GetPlanes();
          ExpectLattice(planes[0], {scale, 2.0 * scale});
          ExpectLattice(planes[1], {scale, 3.0 * scale});
          ExpectLattice(planes[2], {2.0 * scale, 3.0 * scale});
        }
      }
    }
  }
}

TEST(GGEMSCartesianGrid, ZoomHysteresisResizeAndPanKeepOriginAnchored) {
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.FrameWorld({2.0, 2.0, 2.0});
  camera.SetView(Camera::ViewPlane::XY);
  Grid grid;
  grid.Update(camera, {2.0, 2.0, 2.0});
  double const initial_step = grid.GetPlanes()[0].minor_step_m;
  float const initial_zoom = camera.GetDiagnostics().zoom;
  for (float factor : {1.01F, 0.99F, 1.005F, 1.0F}) {
    camera.SetZoom(initial_zoom * factor);
    grid.Update(camera, {2.0, 2.0, 2.0});
    EXPECT_DOUBLE_EQ(grid.GetPlanes()[0].minor_step_m, initial_step);
  }
  camera.SetZoom(initial_zoom * 100.0F);
  grid.Update(camera, {2.0, 2.0, 2.0});
  EXPECT_LT(grid.GetPlanes()[0].minor_step_m, initial_step);
  camera.Pan(35.0F, -19.0F);
  camera.SetViewportSize(1200U, 900U);
  grid.Update(camera, {2.0, 2.0, 2.0});
  ExpectLattice(grid.GetPlanes()[0], {2.0, 2.0});
}

// Equivalent physical scenes must choose equivalent lattice steps even in
// decades where dividing a represented five-step by its decade rounds upward.
TEST(GGEMSCartesianGrid, PhysicalScaleCovarianceKeepsFiveStepsStable) {
  Camera large;
  large.SetViewportSize(800U, 600U);
  large.SetView(Camera::ViewPlane::XY);
  large.SetZoom(2.0e8F);
  Camera small;
  small.SetViewportSize(800U, 600U);
  small.SetView(Camera::ViewPlane::XY);
  small.SetZoom(2.0e17F);
  Grid large_grid;
  Grid small_grid;
  large_grid.Update(large, {1.0, 1.0, 1.0});
  small_grid.Update(small, {1.0e-9, 1.0e-9, 1.0e-9});
  double const large_step = large_grid.GetPlanes()[0].minor_step_m;
  double const small_step = small_grid.GetPlanes()[0].minor_step_m;
  ASSERT_GT(large_step, 0.0);
  EXPECT_NEAR(small_step / large_step, 1.0e-9, 1.0e-20);
  for (int frame = 0; frame < 20; ++frame) {
    large_grid.Update(large, {1.0, 1.0, 1.0});
    small_grid.Update(small, {1.0e-9, 1.0e-9, 1.0e-9});
    EXPECT_DOUBLE_EQ(large_grid.GetPlanes()[0].minor_step_m, large_step);
    EXPECT_DOUBLE_EQ(small_grid.GetPlanes()[0].minor_step_m, small_step);
  }
}

TEST(GGEMSCartesianGrid,
     GrazingProjectionFadesAndNeverEmitsDenseParallelLines) {
  Camera camera;
  camera.SetViewportSize(800U, 600U);
  camera.SetZoom(0.5F);
  Grid grid;
  camera.SetOrbitAngles(270.0F, 87.0F);
  grid.Update(camera, {2.0, 2.0, 2.0});
  EXPECT_GT(grid.GetPlanes()[0].opacity, 0.0F);
  EXPECT_LT(grid.GetPlanes()[0].opacity, 1.0F);
  EXPECT_LE(grid.GetPlanes()[0].line_count[0] +
              grid.GetPlanes()[0].line_count[1],
            256U);
  camera.SetOrbitAngles(270.0F, 90.0F);
  grid.Update(camera, {2.0, 2.0, 2.0});
  EXPECT_FLOAT_EQ(grid.GetPlanes()[0].opacity, 0.0F);
  EXPECT_EQ(grid.GetPlanes()[0].line_count, (std::array<std::uint32_t, 2>{}));
}

} // namespace
