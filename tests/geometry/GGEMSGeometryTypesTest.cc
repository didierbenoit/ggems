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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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
  auto const position = ggems::geometry::MakePositionPM(-10, 20, -30);

  EXPECT_EQ(position.x, -10);
  EXPECT_EQ(position.y, 20);
  EXPECT_EQ(position.z, -30);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DisplacementCanMovePosition) {
  auto const position = ggems::geometry::MakePositionPM(100, 200, 300);
  auto const displacement = ggems::geometry::MakeDisplacementPM(-10, 20, -30);

  auto const moved = position + displacement;

  EXPECT_EQ(moved, ggems::geometry::MakePositionPM(90, 220, 270));
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DifferenceBetweenPositionsIsSignedDisplacement) {
  auto const lhs = ggems::geometry::MakePositionPM(100, 200, 300);
  auto const rhs = ggems::geometry::MakePositionPM(150, 150, 350);
  auto const displacement = lhs - rhs;

  EXPECT_EQ(displacement, ggems::geometry::MakeDisplacementPM(-50, 50, -50));
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionIsNormalizedWhenCreated) {
  auto const direction = ggems::geometry::TryMakeDirection3(3.0F, 4.0F, 0.0F);

  ASSERT_TRUE(direction.has_value());
  EXPECT_FLOAT_EQ(direction->x, 0.6F);
  EXPECT_FLOAT_EQ(direction->y, 0.8F);
  EXPECT_FLOAT_EQ(direction->z, 0.0F);
  EXPECT_FLOAT_EQ(ggems::geometry::Norm(*direction), 1.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionRejectsZeroVector) {
  auto const direction = ggems::geometry::TryMakeDirection3(0.0F, 0.0F, 0.0F);

  EXPECT_FALSE(direction.has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSGeometryTypes, DirectionRejectsNonFiniteValues) {
  float const infinity = std::numeric_limits<float>::infinity();
  auto const direction =
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

TEST(GGEMSGeometryTypes, DotComputesScalarProduct) {
  ggems::geometry::Direction3 const lhs{
    .x = 1.0F,
    .y = 2.0F,
    .z = 3.0F,
  };

  ggems::geometry::Direction3 const rhs{
    .x = 4.0F,
    .y = -5.0F,
    .z = 6.0F,
  };

  EXPECT_FLOAT_EQ(ggems::geometry::Dot(lhs, rhs), 12.0F);
}
