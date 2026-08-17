#include <optional>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DefaultDirectionIsPositiveZ) {
  ggems::geometry::Direction3 const direction{};

  EXPECT_FLOAT_EQ(direction.x, 0.0F);
  EXPECT_FLOAT_EQ(direction.y, 0.0F);
  EXPECT_FLOAT_EQ(direction.z, 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, PositionStoresSignedPicometerCoordinates) {
  ggems::geometry::Position3PM const position =
      ggems::geometry::MakePositionPM(-10, 20, -30);

  EXPECT_EQ(position.x, -10);
  EXPECT_EQ(position.y, 20);
  EXPECT_EQ(position.z, -30);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DisplacementCanMovePosition) {
  ggems::geometry::Position3PM const position =
      ggems::geometry::MakePositionPM(100, 200, 300);

  ggems::geometry::Displacement3PM const displacement =
      ggems::geometry::MakeDisplacementPM(-10, 20, -30);

  ggems::geometry::Position3PM const moved = position + displacement;

  EXPECT_EQ(moved, ggems::geometry::MakePositionPM(90, 220, 270));
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DifferenceBetweenPositionsIsSignedDisplacement) {
  ggems::geometry::Position3PM const lhs =
      ggems::geometry::MakePositionPM(100, 200, 300);

  ggems::geometry::Position3PM const rhs =
      ggems::geometry::MakePositionPM(150, 150, 350);

  ggems::geometry::Displacement3PM const displacement = lhs - rhs;

  EXPECT_EQ(displacement, ggems::geometry::MakeDisplacementPM(-50, 50, -50));
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionIsNormalizedWhenCreated) {
  std::optional<ggems::geometry::Direction3> const direction =
      ggems::geometry::TryMakeDirection3(3.0F, 4.0F, 0.0F);

  ASSERT_TRUE(direction.has_value());

  EXPECT_FLOAT_EQ(direction->x, 0.6F);
  EXPECT_FLOAT_EQ(direction->y, 0.8F);
  EXPECT_FLOAT_EQ(direction->z, 0.0F);
  EXPECT_FLOAT_EQ(ggems::geometry::Norm(*direction), 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionRejectsZeroVector) {
  std::optional<ggems::geometry::Direction3> const direction =
      ggems::geometry::TryMakeDirection3(0.0F, 0.0F, 0.0F);

  EXPECT_FALSE(direction.has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionRejectsNonFiniteValues) {
  float infinity = std::numeric_limits<float>::infinity();

  std::optional<ggems::geometry::Direction3> const direction =
      ggems::geometry::TryMakeDirection3(1.0F, infinity, 0.0F);

  EXPECT_FALSE(direction.has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionHandlesLargeFiniteComponents) {
  auto const large = static_cast<double>(std::numeric_limits<float>::max());

  auto const direction = ggems::geometry::TryMakeDirection3(large, large, 0.0);

  ASSERT_TRUE(direction.has_value());
  EXPECT_NEAR(direction->x, 0.70710678F, 1.0e-6F);
  EXPECT_NEAR(direction->y, 0.70710678F, 1.0e-6F);
  EXPECT_FLOAT_EQ(direction->z, 0.0F);
  EXPECT_NEAR(ggems::geometry::Norm(*direction), 1.0F, 1.0e-6F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DotProductUsesNormalizedDirections) {
  std::optional<ggems::geometry::Direction3> const x_axis =
      ggems::geometry::TryMakeDirection3(1.0F, 0.0F, 0.0F);

  std::optional<ggems::geometry::Direction3> const y_axis =
      ggems::geometry::TryMakeDirection3(0.0F, 1.0F, 0.0F);

  ASSERT_TRUE(x_axis.has_value());
  ASSERT_TRUE(y_axis.has_value());

  EXPECT_FLOAT_EQ(ggems::geometry::Dot(*x_axis, *y_axis), 0.0F);
}
