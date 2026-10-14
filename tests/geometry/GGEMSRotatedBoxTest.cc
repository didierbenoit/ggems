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
  EXPECT_NEAR(thirty.axis_x.x, std::sqrt(3.0) / 2.0, 2.0e-16);
  EXPECT_NEAR(thirty.axis_x.y, 0.5, 2.0e-16);
  auto const negative =
    ggems::geometry::BuildRotationXYZ(0_deg, 0_deg, -45.0_deg);
  auto const radians = ggems::geometry::BuildRotationXYZ(
    0_rad, 0_rad, MakeRadians(-std::numbers::pi_v<long double> / 4.0L));
  EXPECT_NEAR(negative.axis_x.x, std::sqrt(0.5), 2.0e-16);
  EXPECT_NEAR(negative.axis_x.y, -std::sqrt(0.5), 2.0e-16);
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
                 ggems::core::GGEMSRecoverable);
    EXPECT_DOUBLE_EQ(box.GetRotation().axis_x.x, previous);
  }
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
