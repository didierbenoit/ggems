#include <array>
#include <limits>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceFrame.hh"

namespace {

// =============================================================================
// =============================================================================

using Direction3 = ggems::geometry::Direction3;
using GGEMSSourceFrame = ggems::core::sources::GGEMSSourceFrame;

auto ExpectAxis(Direction3 actual, Direction3 expected) -> void {
  EXPECT_NEAR(actual.x, expected.x,
              ggems::core::sources::k_source_frame_float_tolerance);
  EXPECT_NEAR(actual.y, expected.y,
              ggems::core::sources::k_source_frame_float_tolerance);
  EXPECT_NEAR(actual.z, expected.z,
              ggems::core::sources::k_source_frame_float_tolerance);
}

// =============================================================================
// =============================================================================

auto ExpectFramesEqual(GGEMSSourceFrame const &actual,
                       GGEMSSourceFrame const &expected) -> void {
  ExpectAxis(actual.axis_x, expected.axis_x);
  ExpectAxis(actual.axis_y, expected.axis_y);
  ExpectAxis(actual.axis_z, expected.axis_z);
}

// =============================================================================
// =============================================================================

auto ExpectOrthonormalRightHanded(GGEMSSourceFrame const &frame) -> void {
  double const tolerance = ggems::core::sources::k_source_frame_float_tolerance;

  EXPECT_NEAR(ggems::geometry::Norm(frame.axis_x), 1.0, tolerance);
  EXPECT_NEAR(ggems::geometry::Norm(frame.axis_y), 1.0, tolerance);
  EXPECT_NEAR(ggems::geometry::Norm(frame.axis_z), 1.0, tolerance);
  EXPECT_NEAR(ggems::geometry::Dot(frame.axis_x, frame.axis_y), 0.0, tolerance);
  EXPECT_NEAR(ggems::geometry::Dot(frame.axis_y, frame.axis_z), 0.0, tolerance);
  EXPECT_NEAR(ggems::geometry::Dot(frame.axis_z, frame.axis_x), 0.0, tolerance);

  Direction3 const cross{.x = (frame.axis_x.y * frame.axis_y.z) -
                              (frame.axis_x.z * frame.axis_y.y),
                         .y = (frame.axis_x.z * frame.axis_y.x) -
                              (frame.axis_x.x * frame.axis_y.z),
                         .z = (frame.axis_x.x * frame.axis_y.y) -
                              (frame.axis_x.y * frame.axis_y.x)};

  ExpectAxis(cross, frame.axis_z);
}

// =============================================================================
// =============================================================================

struct CardinalCase {
  char const *name;
  std::array<double, 3U> direction;
  std::array<double, 3U> up;
  GGEMSSourceFrame expected;
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSSourceFrame, BuildsSixCardinalRightHandedFrames) {
  std::array<CardinalCase, 6U> const cases{{
      {.name = "+X",
       .direction = {1.0, 0.0, 0.0},
       .up = {0.0, 0.0, 1.0},
       .expected = {.axis_x = {.x = 0.0F, .y = 1.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 0.0F, .z = 1.0F},
                    .axis_z = {.x = 1.0F, .y = 0.0F, .z = 0.0F}}},
      {.name = "-X",
       .direction = {-1.0, 0.0, 0.0},
       .up = {0.0, 0.0, 1.0},
       .expected = {.axis_x = {.x = 0.0F, .y = -1.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 0.0F, .z = 1.0F},
                    .axis_z = {.x = -1.0F, .y = 0.0F, .z = 0.0F}}},
      {.name = "+Y",
       .direction = {0.0, 1.0, 0.0},
       .up = {0.0, 0.0, 1.0},
       .expected = {.axis_x = {.x = -1.0F, .y = 0.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 0.0F, .z = 1.0F},
                    .axis_z = {.x = 0.0F, .y = 1.0F, .z = 0.0F}}},
      {.name = "-Y",
       .direction = {0.0, -1.0, 0.0},
       .up = {0.0, 0.0, 1.0},
       .expected = {.axis_x = {.x = 1.0F, .y = 0.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 0.0F, .z = 1.0F},
                    .axis_z = {.x = 0.0F, .y = -1.0F, .z = 0.0F}}},
      {.name = "+Z",
       .direction = {0.0, 0.0, 1.0},
       .up = {0.0, 1.0, 0.0},
       .expected = {.axis_x = {.x = 1.0F, .y = 0.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 1.0F, .z = 0.0F},
                    .axis_z = {.x = 0.0F, .y = 0.0F, .z = 1.0F}}},
      {.name = "-Z",
       .direction = {0.0, 0.0, -1.0},
       .up = {0.0, 1.0, 0.0},
       .expected = {.axis_x = {.x = -1.0F, .y = 0.0F, .z = 0.0F},
                    .axis_y = {.x = 0.0F, .y = 1.0F, .z = 0.0F},
                    .axis_z = {.x = 0.0F, .y = 0.0F, .z = -1.0F}}},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.name);
    auto const frame = ggems::core::sources::BuildSourceFrame(
        test_case.direction, test_case.up);
    ExpectFramesEqual(frame, test_case.expected);
    ExpectOrthonormalRightHanded(frame);
  }
}

// =============================================================================
// =============================================================================
TEST(GGEMSSourceFrame, BuildsNaturalCtFrame) {
  auto const frame =
      ggems::core::sources::BuildSourceFrame({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0});

  ExpectAxis(frame.axis_x, {.x = 0.0F, .y = 1.0F, .z = 0.0F});
  ExpectAxis(frame.axis_y, {.x = 0.0F, .y = 0.0F, .z = 1.0F});
  ExpectAxis(frame.axis_z, {.x = 1.0F, .y = 0.0F, .z = 0.0F});
}

// =============================================================================
// =============================================================================
TEST(GGEMSSourceFrame, BuildsRightHandedDiagonalFrame) {
  auto const frame =
      ggems::core::sources::BuildSourceFrame({1.0, 1.0, 1.0}, {0.0, 0.0, 1.0});

  constexpr float k_inverse_sqrt_three{std::numbers::inv_sqrt3_v<float>};
  EXPECT_NEAR(frame.axis_z.x, k_inverse_sqrt_three, 1.0e-6F);
  EXPECT_NEAR(frame.axis_z.y, k_inverse_sqrt_three, 1.0e-6F);
  EXPECT_NEAR(frame.axis_z.z, k_inverse_sqrt_three, 1.0e-6F);
  ExpectOrthonormalRightHanded(frame);
}

// =============================================================================
// =============================================================================
TEST(GGEMSSourceFrame, ProjectsNonOrthogonalUpOntoSourcePlane) {
  auto const frame =
      ggems::core::sources::BuildSourceFrame({1.0, 0.0, 0.0}, {1.0, 0.0, 2.0});

  ExpectAxis(frame.axis_x, {.x = 0.0F, .y = 1.0F, .z = 0.0F});
  ExpectAxis(frame.axis_y, {.x = 0.0F, .y = 0.0F, .z = 1.0F});
  ExpectAxis(frame.axis_z, {.x = 1.0F, .y = 0.0F, .z = 0.0F});
}

// =============================================================================
// =============================================================================
TEST(GGEMSSourceFrame, RejectsInvalidDirectionAndUp) {
  double const nan = std::numeric_limits<double>::quiet_NaN();
  double const infinity = std::numeric_limits<double>::infinity();

  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {0.0, 0.0, 0.0}, {0.0, 1.0, 0.0})),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {nan, 0.0, 1.0}, {0.0, 1.0, 0.0})),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {0.0, 0.0, 1.0}, {0.0, 0.0, 0.0})),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {0.0, 0.0, 1.0}, {0.0, infinity, 0.0})),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceFrame, RejectsExactAndNearParallelExplicitUp) {
  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {0.0, 0.0, 1.0}, {0.0, 0.0, 2.0})),
               ggems::core::GGEMSExceptionBase);

  EXPECT_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
                   {0.0, 0.0, 1.0}, {0.001, 0.0, 1.0})),
               ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(static_cast<void>(ggems::core::sources::BuildSourceFrame(
      {0.0, 0.0, 1.0}, {0.002, 0.0, 1.0})));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceFrame, HandlesValuesThatOverflowedFloatSquaredNorm) {
  auto const large = static_cast<double>(std::numeric_limits<float>::max());

  auto const frame = ggems::core::sources::BuildSourceFrame({large, large, 0.0},
                                                            {0.0, 0.0, 1.0});

  ExpectOrthonormalRightHanded(frame);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceFrame,
     AutomaticUpUsesPreferredAndFallbackAxesDeterministically) {
  auto const transverse =
      ggems::core::sources::BuildSourceFrameWithAutomaticUp({1.0, 0.0, 0.0});
  auto const transverse_explicit =
      ggems::core::sources::BuildSourceFrame({1.0, 0.0, 0.0}, {0.0, 0.0, 1.0});
  ExpectFramesEqual(transverse, transverse_explicit);

  std::array<double, 3U> const near_z{1.0e-4, 0.0, 1.0};
  auto const axial =
      ggems::core::sources::BuildSourceFrameWithAutomaticUp(near_z);
  auto const axial_explicit =
      ggems::core::sources::BuildSourceFrame(near_z, {0.0, 1.0, 0.0});
  auto const axial_repeated =
      ggems::core::sources::BuildSourceFrameWithAutomaticUp(near_z);

  ExpectFramesEqual(axial, axial_explicit);
  ExpectFramesEqual(axial, axial_repeated);
}
