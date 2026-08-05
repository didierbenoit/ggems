#include <array>
#include <cstddef>

#include <gtest/gtest.h>
#include <vulkan/vulkan.hpp>

#include "GGEMS/ui/GGEMSVulkanCamera.hh"

namespace {

// =============================================================================
// =============================================================================

using Camera = ggems::ui::GGEMSVulkanCamera;
using Matrix4Rows = Camera::Matrix4Rows;

constexpr float k_tolerance{1.0e-5F};
constexpr float k_sqrt_half{0.7071067811865475F};

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
  return {(matrix.row_0[0] * point.x) + (matrix.row_0[1] * point.y) +
              (matrix.row_0[2] * point.z) + matrix.row_0[3],
          (matrix.row_1[0] * point.x) + (matrix.row_1[1] * point.y) +
              (matrix.row_1[2] * point.z) + matrix.row_1[3],
          (matrix.row_2[0] * point.x) + (matrix.row_2[1] * point.y) +
              (matrix.row_2[2] * point.z) + matrix.row_2[3],
          (matrix.row_3[0] * point.x) + (matrix.row_3[1] * point.y) +
              (matrix.row_3[2] * point.z) + matrix.row_3[3]};
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

TEST(GGEMSVulkanCamera, ResetRestoresCanonicalView) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 800U, .height = 800U});

  camera.SetOrbitAngles(-20.0F, 15.0F);
  camera.SetZoom(1.7F);
  camera.Pan(37.0F, -19.0F);
  camera.Orbit(12.0F, 8.0F);
  camera.ZoomBy(2.0F);

  camera.Reset();

  Matrix4Rows const expected{
      .row_0 = {0.8F * k_sqrt_half, 0.0F, -0.8F * k_sqrt_half, 0.0F},
      .row_1 = {-0.4F, 0.8F * k_sqrt_half, -0.4F, 0.0F},
      .row_2 = {-0.025F, -0.05F * k_sqrt_half, -0.025F, 0.5F},
      .row_3 = {0.0F, 0.0F, 0.0F, 1.0F}};

  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);
}

TEST(GGEMSVulkanCamera, ZeroYawAndPitchUseExpectedViewBasisAndDepth) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 800U, .height = 800U});
  camera.SetOrbitAngles(0.0F, 0.0F);
  camera.SetZoom(1.0F);

  Matrix4Rows const expected{.row_0 = {0.0F, 0.0F, -1.0F, 0.0F},
                             .row_1 = {0.0F, 1.0F, 0.0F, 0.0F},
                             .row_2 = {-0.05F, 0.0F, 0.0F, 0.5F},
                             .row_3 = {0.0F, 0.0F, 0.0F, 1.0F}};

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

TEST(GGEMSVulkanCamera,
     CanonicalViewProjectsSixGlobalAxesWithPositiveYDownConvention) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 800U, .height = 800U});
  camera.Reset();

  Matrix4Rows const matrix = camera.BuildWorldToClipMatrix();

  auto const positive_x = TransformPoint(matrix, Point3{.x = 1.0F});
  auto const negative_x = TransformPoint(matrix, Point3{.x = -1.0F});
  auto const positive_y = TransformPoint(matrix, Point3{.y = 1.0F});
  auto const negative_y = TransformPoint(matrix, Point3{.y = -1.0F});
  auto const positive_z = TransformPoint(matrix, Point3{.z = 1.0F});
  auto const negative_z = TransformPoint(matrix, Point3{.z = -1.0F});

  EXPECT_NEAR(positive_x[0], 0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(positive_x[1], -0.4F, k_tolerance);
  EXPECT_NEAR(positive_x[2], 0.475F, k_tolerance);

  EXPECT_NEAR(negative_x[0], -0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(negative_x[1], 0.4F, k_tolerance);
  EXPECT_NEAR(negative_x[2], 0.525F, k_tolerance);

  EXPECT_NEAR(positive_y[0], 0.0F, k_tolerance);
  EXPECT_NEAR(positive_y[1], 0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(positive_y[2], 0.5F - (0.05F * k_sqrt_half), k_tolerance);

  EXPECT_NEAR(negative_y[0], 0.0F, k_tolerance);
  EXPECT_NEAR(negative_y[1], -0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(negative_y[2], 0.5F + (0.05F * k_sqrt_half), k_tolerance);

  EXPECT_NEAR(positive_z[0], -0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(positive_z[1], -0.4F, k_tolerance);
  EXPECT_NEAR(positive_z[2], 0.475F, k_tolerance);

  EXPECT_NEAR(negative_z[0], 0.8F * k_sqrt_half, k_tolerance);
  EXPECT_NEAR(negative_z[1], 0.4F, k_tolerance);
  EXPECT_NEAR(negative_z[2], 0.525F, k_tolerance);
}

TEST(GGEMSVulkanCamera,
     SixteenByNineViewportScalesOnlyHorizontalClipCoordinate) {
  Camera square_camera{};
  square_camera.SetViewportExtent(vk::Extent2D{.width = 900U, .height = 900U});
  square_camera.Reset();

  Camera wide_camera{};
  wide_camera.SetViewportExtent(vk::Extent2D{.width = 1600U, .height = 900U});
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

TEST(GGEMSVulkanCamera, PanMovesTargetAlongBothViewAxes) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 100U, .height = 100U});
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

TEST(GGEMSVulkanCamera, ZoomScalesXYWithoutChangingDepth) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 800U, .height = 800U});
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

TEST(GGEMSVulkanCamera, PitchIsClampedToEightyFiveDegrees) {
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

TEST(GGEMSVulkanCamera, ZeroExtentKeepsLastValidViewportExtent) {
  Camera camera{};
  camera.SetViewportExtent(vk::Extent2D{.width = 1600U, .height = 900U});

  Matrix4Rows const expected = camera.BuildWorldToClipMatrix();

  camera.SetViewportExtent(vk::Extent2D{.width = 0U, .height = 720U});
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);

  camera.SetViewportExtent(vk::Extent2D{.width = 1280U, .height = 0U});
  ExpectMatrixNear(camera.BuildWorldToClipMatrix(), expected);
}

} // namespace
