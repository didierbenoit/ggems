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
TEST(GGEMSSceneCamera, PitchIsClampedToEightyFiveDegrees) {
  Camera positive_over_limit{};
  positive_over_limit.SetOrbitAngles(0.0F, 90.0F);
  positive_over_limit.SetZoom(1.0F);

  Camera positive_limit{};
  positive_limit.SetOrbitAngles(0.0F, 85.0F);
  positive_limit.SetZoom(1.0F);

  ExpectMatrixNear(positive_over_limit.BuildWorldToClipMatrix(),
                   positive_limit.BuildWorldToClipMatrix());

  Camera negative_over_limit{};
  negative_over_limit.SetOrbitAngles(0.0F, -90.0F);
  negative_over_limit.SetZoom(1.0F);

  Camera negative_limit{};
  negative_limit.SetOrbitAngles(0.0F, -85.0F);
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
TEST(GGEMSSceneCamera, FramesWorldAtSmallAndLargePhysicalScales) {
  for (double scale : {1.0e-9, 1.0, 1.0e6}) {
    SCOPED_TRACE(scale);
    Camera camera{};
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

      for (float x : {-2.0F, 2.0F}) {
        for (float y : {-3.0F, 3.0F}) {
          for (float z : {-1.0F, 1.0F}) {
            auto const point =
              TransformPoint(matrix, Point3{
                                       .x = x * static_cast<float>(scale),
                                       .y = y * static_cast<float>(scale),
                                       .z = z * static_cast<float>(scale),
                                     });
            EXPECT_LT(std::abs(point[0]), 1.0F);
            EXPECT_LT(std::abs(point[1]), 1.0F);
            EXPECT_GT(point[2], 0.0F);
            EXPECT_LT(point[2], 1.0F);
          }
        }
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
