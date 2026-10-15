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
 * \brief Checks authored rotations, copied poses, and conservative containment.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <expected>
#include <limits>
#include <numbers>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
using namespace ggems::units;
using ggems::core::materials::GGEMSMaterial;
using ggems::geometry::GGEMSBox;
using ggems::geometry::GGEMSWorld;

TEST(GGEMSRotatedBox, IdentityAndAbsoluteAssignment) {
  GGEMSBox box;
  EXPECT_TRUE(box.GetRotation().IsIdentity());
  EXPECT_EQ(&box.SetRotation(0_deg, 0_deg, 90_deg), &box);
  auto const first = box.GetRotation();
  box.SetRotation(0_deg, 0_deg, 90_deg);
  EXPECT_DOUBLE_EQ(box.GetRotation().axis_x.x, first.axis_x.x);
  EXPECT_DOUBLE_EQ(box.GetRotation().axis_x.y, 1.0);
  EXPECT_DOUBLE_EQ(box.GetRotation().axis_y.x, -1.0);
  EXPECT_DOUBLE_EQ(box.GetRotation().axis_z.z, 1.0);
  box.SetRotation(0_deg, 0_deg, 0_deg);
  EXPECT_TRUE(box.GetRotation().IsIdentity());
}

TEST(GGEMSRotatedBox, ActiveFixedAxisXYZOrder) {
  auto const rotation =
    ggems::geometry::BuildRotationXYZ(90_deg, 90_deg, 90_deg);
  EXPECT_DOUBLE_EQ(rotation.axis_x.z, -1.0);
  EXPECT_DOUBLE_EQ(rotation.axis_y.y, 1.0);
  EXPECT_DOUBLE_EQ(rotation.axis_z.x, 1.0);
  EXPECT_DOUBLE_EQ(rotation.axis_x.x, 0.0);
  EXPECT_DOUBLE_EQ(rotation.axis_y.z, 0.0);
  EXPECT_DOUBLE_EQ(rotation.axis_z.y, 0.0);
}

TEST(GGEMSRotatedBox, ThirtyFortyFiveNegativeAndRadians) {
  auto const thirty = ggems::geometry::BuildRotationXYZ(0_deg, 0_deg, 30_deg);
  // Noncardinal rotations evaluate the canonical picoradian angle.
  EXPECT_NEAR(thirty.axis_x.x, std::cos(ToRadians(30_deg)), 2.0e-16);
  EXPECT_NEAR(thirty.axis_x.y, std::sin(ToRadians(30_deg)), 2.0e-16);
  auto const negative =
    ggems::geometry::BuildRotationXYZ(0_deg, 0_deg, -45.0_deg);
  auto const radians = ggems::geometry::BuildRotationXYZ(
    0_rad, 0_rad, MakeRadians(-std::numbers::pi_v<long double> / 4.0L));
  EXPECT_NEAR(negative.axis_x.x, std::cos(ToRadians(-45.0_deg)), 2.0e-16);
  EXPECT_NEAR(negative.axis_x.y, std::sin(ToRadians(-45.0_deg)), 2.0e-16);
  EXPECT_DOUBLE_EQ(negative.axis_x.x, radians.axis_x.x);
  EXPECT_DOUBLE_EQ(negative.axis_y.x, radians.axis_y.x);
}

TEST(GGEMSRotatedBox, InvalidAnglesKeepThePreviousRotation) {
  GGEMSBox box;
  box.SetRotation(0_deg, 0_deg, 30_deg);
  double const previous = box.GetRotation().axis_x.x;
  for (auto const invalid : {std::numeric_limits<long double>::infinity(),
                             std::numeric_limits<long double>::quiet_NaN()}) {
    EXPECT_THROW(box.SetRotation(MakeRadians(invalid), 0_rad, 0_rad),
                 std::bad_expected_access<UnitConversionError>);
    EXPECT_DOUBLE_EQ(box.GetRotation().axis_x.x, previous);
  }
}

TEST(GGEMSRotatedBox, IndependentlyRoundedQuarterTurnsRemainExact) {
  for (int turn = -16; turn <= 16; ++turn) {
    SCOPED_TRACE(turn);
    auto const rotation = ggems::geometry::BuildRotationXYZ(
      0_deg, 0_deg, MakeDegrees(static_cast<long double>(turn) * 90.0L));
    std::array<double, 4U> constexpr cosine{1.0, 0.0, -1.0, 0.0};
    std::array<double, 4U> constexpr sine{0.0, 1.0, 0.0, -1.0};
    auto const quadrant = static_cast<std::size_t>(((turn % 4) + 4) % 4);
    EXPECT_EQ(rotation.axis_x.x, cosine[quadrant]);
    EXPECT_EQ(rotation.axis_x.y, sine[quadrant]);
    EXPECT_EQ(rotation.axis_y.x, -sine[quadrant]);
    EXPECT_EQ(rotation.axis_y.y, cosine[quadrant]);
    EXPECT_EQ(rotation.IsIdentity(), quadrant == 0U);
  }
  for (long double const degrees : {-90'000'000.0L, 90'000'000.0L}) {
    EXPECT_TRUE(
      ggems::geometry::BuildRotationXYZ(MakeDegrees(degrees), 0_deg, 0_deg)
        .IsIdentity());
  }
  for (std::int64_t const ticks : {std::numeric_limits<std::int64_t>::min(),
                                   std::numeric_limits<std::int64_t>::max()}) {
    EXPECT_NO_THROW(
      (void)ggems::geometry::BuildRotationXYZ(0_deg, 0_deg, Angle{ticks}));
  }
}

TEST(GGEMSRotatedBox, PicoradianNeighborsAreNotSnappedToCardinalAngles) {
  for (long double const degrees : {-360.0L, -90.0L, 0.0L, 90.0L, 360.0L}) {
    auto const cardinal = MakeDegrees(degrees);
    auto const exact =
      ggems::geometry::BuildRotationXYZ(0_deg, 0_deg, cardinal);
    for (std::int64_t const delta : {-1LL, 1LL}) {
      auto const adjacent = ggems::geometry::BuildRotationXYZ(
        0_deg, 0_deg, Angle{cardinal.value + delta});
      EXPECT_TRUE(adjacent.axis_x.x != exact.axis_x.x ||
                  adjacent.axis_x.y != exact.axis_x.y);
      EXPECT_FALSE(adjacent.IsIdentity());
    }
  }
}

TEST(GGEMSRotatedBox, SubPicoradianAngleUsesTheIdentityGeometryPath) {
  auto const angle = MakeRadians(1.0e-20L);
  EXPECT_EQ(angle.value, 0);
  GGEMSWorld world;
  world.SetSize(20_mm, 20_mm, 20_mm);
  GGEMSBox box;
  box.SetSize(2_mm, 4_mm, 6_mm)
    .SetRotation(0_rad, 0_rad, angle)
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  EXPECT_TRUE(box.GetRotation().IsIdentity());
  world.AddVolume(box);
  EXPECT_FALSE(world.BuildBoxPoseRecord().has_value());
}

TEST(GGEMSRotatedBox, RotationFitsWithSubPicometerWorldClearance) {
  GGEMSWorld world;
  world.SetSize(Length{20}, Length{20}, Length{20})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  GGEMSBox box;
  box.SetSize(Length{16}, Length{12}, Length{2})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"))
    .SetRotation(0_deg, 0_deg, 45_deg);
  // The XY bound is 14/sqrt(2) pm, strictly below the 10 pm World face.
  world.AddVolume(box);
  EXPECT_NO_THROW(world.Validate());
  GGEMSWorld shifted;
  shifted.SetSize(Length{20}, Length{20}, Length{20})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  box.SetPosition({.x = 1, .y = 0, .z = 0});
  shifted.AddVolume(box);
  EXPECT_THROW(shifted.Validate(), ggems::core::GGEMSRecoverable);
}

TEST(GGEMSRotatedBox, RejectsRotatedProtrusionAndExactContact) {
  GGEMSWorld world;
  world.SetSize(Length{20}, Length{20}, Length{20})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  GGEMSBox box;
  box.SetSize(Length{16}, Length{14}, Length{2})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"))
    .SetRotation(0_deg, 0_deg, 45_deg);
  world.AddVolume(box);
  EXPECT_THROW(world.Validate(), ggems::core::GGEMSRecoverable);
  GGEMSWorld contact;
  contact.SetSize(Length{20}, Length{20}, Length{20})
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  box.SetSize(Length{20}, Length{2}, Length{2})
    .SetRotation(0_deg, 0_deg, 90_deg);
  contact.AddVolume(box);
  EXPECT_THROW(contact.Validate(), ggems::core::GGEMSRecoverable);
}

TEST(GGEMSRotatedBox, WorldOwnsPoseAndPacksLocalFaces) {
  GGEMSWorld world;
  world.SetSize(20_mm, 20_mm, 20_mm)
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  GGEMSBox box;
  box.SetSize(2_mm, 4_mm, 6_mm)
    .SetPosition({.x = -100, .y = 200, .z = -300})
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  world.AddVolume(box);
  box.SetRotation(0_deg, 0_deg, 0_deg).SetPosition({});
  ASSERT_NO_THROW(world.Validate());
  auto const pose = world.BuildBoxPoseRecord();
  ASSERT_TRUE(pose.has_value());
  EXPECT_EQ(pose->center_x_pm, -100);
  EXPECT_EQ(pose->center_y_pm, 200);
  EXPECT_DOUBLE_EQ(pose->axis_x_y, 1.0);
  std::array<std::uint32_t, 1U> const material_ids{3U};
  auto const records = world.BuildBoxRecords(material_ids);
  EXPECT_EQ(records[0].lower_x_pm, -1'000'000'000LL);
  EXPECT_EQ(records[0].upper_y_pm, 2'000'000'000LL);
  EXPECT_EQ(records[0].volume_id, 1U);
  EXPECT_EQ(records[0].material_id, 3U);
  EXPECT_EQ(sizeof(ggems::geometry::GGEMSBoxPoseRecord), 96U);
  EXPECT_EQ(offsetof(ggems::geometry::GGEMSBoxPoseRecord, axis_x_x), 24U);
  GGEMSWorld identity;
  identity.SetSize(20_mm, 20_mm, 20_mm)
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"))
    .AddVolume(box);
  EXPECT_FALSE(identity.BuildBoxPoseRecord().has_value());
}
} // namespace
