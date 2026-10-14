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
 * \brief Tests Sphere value ownership, radius conversion, containment, and
 * shared records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto Sphere(Length radius = 3_pm) -> GGEMSSphere {
  GGEMSSphere sphere;
  sphere.SetRadius(radius).SetMaterial(BuildBuiltInMaterial("Water"));
  return sphere;
}

auto World() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(20_pm, 40_pm, 60_pm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}
} // namespace

TEST(GGEMSSphere, RequiresPositiveRadiusAndExplicitMaterial) {
  GGEMSSphere sphere;
  EXPECT_EQ(sphere.GetPositionPM(), MakePositionPM(0, 0, 0));
  EXPECT_EQ(sphere.GetRadiusPM(), 0U);
  EXPECT_THROW(sphere.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(sphere.GetMaterial()), GGEMSRecoverable);
  EXPECT_THROW(sphere.SetRadius(0_pm), GGEMSRecoverable);
  sphere.SetRadius(1_pm);
  EXPECT_THROW(sphere.Validate(), GGEMSRecoverable);
  sphere.SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(sphere.Validate());
  EXPECT_THROW(sphere.SetRadius(0_pm), GGEMSRecoverable);
  EXPECT_EQ(sphere.GetRadiusPM(), 1U);
  sphere.SetRadius(3_pm).SetPosition(MakePositionPM(-1, 2, -3));
  EXPECT_EQ(sphere.GetRadiusPM(), 3U);
  EXPECT_EQ(sphere.GetPositionPM(), MakePositionPM(-1, 2, -3));
}

TEST(GGEMSSphere, CopiesTheAuthoredValueAndKeepsTheBoxViewCompatible) {
  auto world = World();
  {
    auto sphere = Sphere();
    sphere.SetPosition(MakePositionPM(-2, 3, -4));
    world.AddVolume(sphere);
    sphere.SetRadius(5_pm).SetPosition(MakePositionPM(0, 0, 0));
  }
  auto copy = world;
  EXPECT_NO_THROW(copy.Validate());
  EXPECT_TRUE(copy.GetBoxes().empty());
  EXPECT_TRUE(copy.BuildBoxRecords({}).empty());
  EXPECT_FALSE(copy.BuildBoxPoseRecord().has_value());
  auto const record = copy.BuildAnalyticVolumeRecord(7U);
  EXPECT_EQ(record.kind, GGEMSAnalyticKind::Sphere);
  EXPECT_EQ(record.parameters.sphere.radius_pm, 3U);
  EXPECT_EQ(record.parameters.sphere.center_x_pm, -2);
  EXPECT_EQ(record.parameters.sphere.center_y_pm, 3);
  EXPECT_EQ(record.parameters.sphere.center_z_pm, -4);
  EXPECT_EQ(record.parameters.sphere.volume_id, 1U);
  EXPECT_EQ(record.parameters.sphere.material_id, 7U);
  ASSERT_NE(copy.GetVolumeMaterial(), nullptr);
  EXPECT_EQ(copy.GetVolumeMaterial()->GetName(), "Water");
  EXPECT_EQ(copy.GetMaterial().GetName(), "Vacuum");
}

TEST(GGEMSSphere, RejectsEverySecondPhysicalVolumeAndPreservesTheFirst) {
  GGEMSBox box;
  box.SetSize(2_pm, 2_pm, 2_pm).SetMaterial(BuildBuiltInMaterial("Water"));
  auto sphere_world = World();
  sphere_world.AddVolume(Sphere());
  EXPECT_THROW(sphere_world.AddVolume(Sphere()), GGEMSRecoverable);
  EXPECT_THROW(sphere_world.AddVolume(box), GGEMSRecoverable);
  EXPECT_EQ(sphere_world.BuildAnalyticVolumeRecord(1U).kind,
            GGEMSAnalyticKind::Sphere);
  auto box_world = World();
  box_world.AddVolume(box);
  EXPECT_THROW(box_world.AddVolume(Sphere()), GGEMSRecoverable);
  EXPECT_EQ(box_world.GetBoxes().size(), 1U);
  EXPECT_EQ(box_world.BuildAnalyticVolumeRecord(1U).kind,
            GGEMSAnalyticKind::Box);
  auto empty = World();
  EXPECT_THROW(empty.AddVolume(GGEMSSphere{}), GGEMSRecoverable);
  EXPECT_EQ(empty.BuildAnalyticVolumeRecord(0U).kind, GGEMSAnalyticKind::None);
}

TEST(GGEMSSphere, RequiresStrictContainmentOnAllSixWorldFaces) {
  std::array<std::int64_t, 3> const half{10, 20, 30};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    for (auto const sign : {-1, 1}) {
      for (auto const gap : {0, 1}) {
        std::array<std::int64_t, 3> center{};
        center[axis] = sign * (half[axis] - 3 - gap);
        auto sphere = Sphere();
        sphere.SetPosition(MakePositionPM(center[0], center[1], center[2]));
        auto world = World();
        world.AddVolume(sphere);
        if (gap == 0) {
          EXPECT_THROW(world.Validate(), GGEMSRecoverable);
        } else {
          EXPECT_NO_THROW(world.Validate());
        }
      }
    }
  }
}

TEST(GGEMSSphere, ContainmentDoesNotOverflowAtIntegerLimits) {
  for (auto center : {std::numeric_limits<std::int64_t>::min(),
                      std::numeric_limits<std::int64_t>::max()}) {
    auto sphere = Sphere();
    sphere.SetPosition(MakePositionPM(center, 0, 0));
    auto world = World();
    world.AddVolume(sphere);
    EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  }
  auto world = World();
  world.AddVolume(Sphere(Length{std::numeric_limits<std::uint64_t>::max()}));
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  constexpr std::uint64_t half = 0x3FFF'FFFF'FFFF'FFFFULL;
  GGEMSWorld large;
  large.SetSize(Length{2 * half}, Length{2 * half}, Length{2 * half})
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  large.AddVolume(Sphere(Length{half - 1}));
  EXPECT_NO_THROW(large.Validate());
}

TEST(GGEMSSphere, ScalarHostLayoutMatchesTheDocumentedDeviceABI) {
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSAnalyticVolumeRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSAnalyticVolumeRecord>);
  EXPECT_EQ(sizeof(GGEMSSphereRecord), 40U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, center_x_pm), 0U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, center_y_pm), 8U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, center_z_pm), 16U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, radius_pm), 24U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, volume_id), 32U);
  EXPECT_EQ(offsetof(GGEMSSphereRecord, material_id), 36U);
  EXPECT_EQ(sizeof(GGEMSAnalyticVolumeRecord), 64U);
  EXPECT_EQ(offsetof(GGEMSAnalyticVolumeRecord, parameters), 8U);
}

TEST(GGEMSSphere, UsesTheCentralLengthConversionWithoutASeparateRadiusPolicy) {
  EXPECT_FALSE(MakeQuantity<Length>(-1.0, "pm").has_value());
  auto const zero = MakeQuantity<Length>(0.49, "pm");
  ASSERT_TRUE(zero.has_value());
  EXPECT_THROW(GGEMSSphere{}.SetRadius(*zero), GGEMSRecoverable);
  auto const odd = MakeQuantity<Length>(3.0, "pm");
  ASSERT_TRUE(odd.has_value());
  EXPECT_EQ(Sphere(*odd).GetRadiusPM(), 3U);
  auto const medical = MakeQuantity<Length>(1.25, "mm");
  ASSERT_TRUE(medical.has_value());
  EXPECT_EQ(Sphere(*medical).GetRadiusPM(), 1'250'000'000U);
}
