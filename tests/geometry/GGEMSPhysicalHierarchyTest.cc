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
 * \brief Checks hierarchy authoring, occurrence links, and composed static
 * poses.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto World() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(200_mm, 200_mm, 200_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

auto Box(Length x = 8_mm, Length y = 10_mm, Length z = 12_mm) -> GGEMSBox {
  GGEMSBox box;
  box.SetSize(x, y, z).SetMaterial(BuildBuiltInMaterial("Water"));
  return box;
}
} // namespace

TEST(GGEMSPhysicalHierarchy, OwnsInsertionOrderedParentChildAndSiblingLinks) {
  auto world = World();
  world.AddVolume(Box(80_mm, 60_mm, 60_mm));
  GGEMSSphere sphere;
  sphere.SetRadius(10_mm).SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(sphere, 1U);
  auto second_child = Box();
  second_child.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0));
  world.AddVolume(second_child, 1U).AddVolume(Box(2_mm, 2_mm, 2_mm), 2U);
  auto const owned = world;
  sphere.SetRadius(1_mm).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  ASSERT_NO_THROW(owned.Validate());
  ASSERT_EQ(owned.GetVolumeCount(), 4U);
  auto const links = owned.BuildOccurrenceRecords();
  ASSERT_EQ(links.size(), 4U);
  std::array<std::uint32_t, 4> const parents{0U, 1U, 1U, 2U};
  std::array<std::uint32_t, 4> const children{2U, 4U, 0U, 0U};
  std::array<std::uint32_t, 4> const siblings{0U, 3U, 0U, 0U};
  for (std::size_t index = 0; index < links.size(); ++index) {
    EXPECT_EQ(links[index].parent_volume_id, parents[index]);
    EXPECT_EQ(links[index].first_child_id, children[index]);
    EXPECT_EQ(links[index].next_sibling_id, siblings[index]);
  }
  EXPECT_EQ(owned.BuildRecord().hierarchy_depth, 3U);
  EXPECT_EQ(owned.GetVolumeMaterial(1U)->GetName(), "Aluminum");
}

TEST(GGEMSPhysicalHierarchy, RejectsUnknownParentsAndUnsupportedDepth) {
  auto world = World();
  EXPECT_THROW(world.AddVolume(Box(), 1U), GGEMSRecoverable);
  EXPECT_EQ(world.GetVolumeCount(), 0U);
  world.AddVolume(Box(80_mm, 60_mm, 60_mm));
  EXPECT_THROW(world.AddVolume(Box(), 2U), GGEMSRecoverable);
  EXPECT_THROW(world.AddVolume(Box(), 100U), GGEMSRecoverable);
  EXPECT_EQ(world.GetVolumeCount(), 1U);
  world.AddVolume(Box(20_mm, 20_mm, 20_mm), 1U);
  world.AddVolume(Box(8_mm, 8_mm, 8_mm), 2U);
  EXPECT_THROW(world.AddVolume(Box(2_mm, 2_mm, 2_mm), 3U), GGEMSRecoverable);
  EXPECT_EQ(world.GetVolumeCount(), 3U);
  EXPECT_NO_THROW(world.Validate());
}

TEST(GGEMSPhysicalHierarchy, ComposesParentLocalTranslationAndRotation) {
  auto world = World();
  auto mother = Box(80_mm, 40_mm, 30_mm);
  mother
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 3'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto child = Box(12_mm, 8_mm, 10_mm);
  child
    .SetPosition(
      MakePositionPM(6'000'000'000LL, 3'000'000'000LL, -1'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  world.AddVolume(mother).AddVolume(child, 1U);
  ASSERT_NO_THROW(world.Validate());
  auto const pose = world.BuildBoxPoseRecord(1U);
  ASSERT_TRUE(pose.has_value());
  // Independent quarter-turn composition: (10,-5,3) + (-3,6,-1).
  EXPECT_FLOAT_EQ(pose->center_x_pm, 7.0e9F);
  EXPECT_FLOAT_EQ(pose->center_y_pm, 1.0e9F);
  EXPECT_FLOAT_EQ(pose->center_z_pm, 2.0e9F);
  EXPECT_FLOAT_EQ(pose->axis_x_x, -1.0F);
  EXPECT_FLOAT_EQ(pose->axis_y_y, -1.0F);
  EXPECT_FLOAT_EQ(pose->axis_z_z, 1.0F);

  auto sphere_world = World();
  GGEMSSphere sphere;
  sphere.SetRadius(30_mm)
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  auto offset_box = Box(8_mm, 4_mm, 6_mm);
  offset_box.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  sphere_world.AddVolume(sphere).AddVolume(offset_box, 1U);
  ASSERT_NO_THROW(sphere_world.Validate());
  auto const sphere_child_pose = sphere_world.BuildBoxPoseRecord(1U);
  ASSERT_TRUE(sphere_child_pose.has_value());
  EXPECT_NEAR(sphere_child_pose->center_x_pm, 0.0F,
              sphere_world.GetVolumeNavigationTolerancePM(1U));
  EXPECT_FLOAT_EQ(sphere_child_pose->center_y_pm, 10.0e9F);
  EXPECT_FLOAT_EQ(sphere_child_pose->center_z_pm, 0.0F);
  EXPECT_FLOAT_EQ(sphere_child_pose->axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(sphere_child_pose->axis_y_x, -1.0F);
}

TEST(GGEMSPhysicalHierarchy,
     AdmitsAncestorClippingButRejectsWorldSiblingOverlap) {
  auto clipped = World();
  clipped.AddVolume(Box(40_mm, 40_mm, 40_mm));
  GGEMSSphere daughter;
  daughter.SetRadius(10_mm)
    .SetPosition(MakePositionPM(18'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  clipped.AddVolume(daughter, 1U);
  auto grandchild = Box(20_mm, 4_mm, 6_mm);
  grandchild.SetPosition(MakePositionPM(6'000'000'000LL, 0, 0));
  clipped.AddVolume(grandchild, 2U);
  EXPECT_NO_THROW(clipped.Validate());
  EXPECT_EQ(clipped.GetVolumeCount(), 3U);

  auto overlap = World();
  overlap.AddVolume(Box()).AddVolume(Box());
  EXPECT_THROW(overlap.Validate(), GGEMSRecoverable);
}
