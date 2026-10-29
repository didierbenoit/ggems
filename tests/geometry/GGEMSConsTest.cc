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
 * \brief Checks Cons authoring and shared host record contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto Cons() -> GGEMSCons {
  GGEMSCons shape;
  shape.SetNegativeZRadii(1_mm, 4_mm)
    .SetPositiveZRadii(2_mm, 6_mm)
    .SetLength(12_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  return shape;
}

auto World() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(80_mm, 80_mm, 80_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}
} // namespace

TEST(N1Cons, RequiresCompleteDimensionsAndOwnedMaterial) {
  GGEMSCons shape;
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(shape.GetMaterial()), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetNegativeZRadii(0_pm, 4_mm), &shape);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetPositiveZRadii(0_pm, 6_mm).SetLength(12_mm);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetMaterial(BuildBuiltInMaterial("Water")), &shape);
  EXPECT_NO_THROW(shape.Validate());
  EXPECT_EQ(shape.GetNegativeZInnerRadiusPM(), 0U);
  EXPECT_EQ(shape.GetPositiveZInnerRadiusPM(), 0U);
  EXPECT_EQ(shape.GetNegativeZOuterRadiusPM(), 4'000'000'000U);
  EXPECT_EQ(shape.GetPositiveZOuterRadiusPM(), 6'000'000'000U);
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000U);
}

TEST(N1Cons, RejectsInvalidRadiiAndLengthsWithoutMutatingAcceptedValues) {
  auto shape = Cons();
  EXPECT_THROW(shape.SetNegativeZRadii(4_mm, 4_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetNegativeZRadii(5_mm, 4_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetPositiveZRadii(1_mm, 0_pm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetPositiveZRadii(6_mm, 6_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetLength(0_pm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetLength(3_pm), GGEMSRecoverable);
  EXPECT_EQ(shape.GetNegativeZInnerRadiusPM(), 1'000'000'000U);
  EXPECT_EQ(shape.GetPositiveZOuterRadiusPM(), 6'000'000'000U);
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000U);
  EXPECT_FALSE(MakeQuantity<Length>(-1.0, "mm").has_value());
  EXPECT_FALSE(MakeQuantity<Length>(1.0, "MM").has_value());
  EXPECT_FALSE(
    MakeQuantity<Length>(std::numeric_limits<long double>::infinity(), "mm")
      .has_value());
}

TEST(N1Cons, AdmitsSolidTipCylinderAndClosingBoreWithoutRadiusReplacement) {
  auto shape = Cons();
  shape.SetNegativeZRadii(0_pm, 0_pm).SetPositiveZRadii(0_pm, 6_mm);
  auto cone = World();
  cone.AddVolume(shape);
  EXPECT_NO_THROW(cone.Validate());
  shape.SetNegativeZRadii(0_pm, 4_mm).SetPositiveZRadii(2_mm, 4_mm);
  EXPECT_EQ(shape.GetNegativeZInnerRadiusPM(), 0U);
  auto closing_bore = World();
  closing_bore.AddVolume(shape);
  EXPECT_NO_THROW(closing_bore.Validate());
  shape.SetNegativeZRadii(0_pm, 4_mm).SetPositiveZRadii(0_pm, 4_mm);
  auto cylinder = World();
  cylinder.AddVolume(shape);
  EXPECT_NO_THROW(cylinder.Validate());
  shape.SetNegativeZRadii(0_pm, 0_pm).SetPositiveZRadii(0_pm, 0_pm);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetNegativeZRadii(0_pm, 0_pm).SetPositiveZRadii(2_mm, 6_mm);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetNegativeZRadii(2_mm, 6_mm).SetPositiveZRadii(0_pm, 0_pm);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
}

TEST(N1Cons, CopiesPoseAndMaterial) {
  auto shape = Cons();
  shape.SetPosition(MakePositionPM(-5'000'000'000LL, 7'000'000'000LL, 0))
    .SetRotation(0_deg, 90_deg, 0_deg);
  auto world = World();
  world.AddVolume(shape);
  shape.SetNegativeZRadii(0_pm, 8_mm)
    .SetRotation(0_deg, 0_deg, 0_deg)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  auto copy = world;
  EXPECT_NO_THROW(copy.Validate());
  EXPECT_EQ(copy.BuildAnalyticVolumeRecord(7U).kind, GGEMSAnalyticKind::Cons);
  ASSERT_NE(copy.GetVolumeMaterial(), nullptr);
  EXPECT_EQ(copy.GetVolumeMaterial()->GetName(), "Water");
  EXPECT_TRUE(copy.BuildBoxPoseRecord().has_value());
}

TEST(N1Cons, RejectsUnresolvedFeaturesButAdmitsWorldContact) {
  auto tiny_bore = Cons();
  tiny_bore.SetNegativeZRadii(1_pm, 4_mm).SetPositiveZRadii(1_pm, 6_mm);
  auto thin_wall = Cons();
  thin_wall.SetNegativeZRadii(Length{3'999'999'999ULL}, 4_mm);
  auto short_shape = Cons();
  short_shape.SetLength(2_pm);
  for (auto const &shape : {tiny_bore, thin_wall, short_shape}) {
    auto world = World();
    world.AddVolume(shape);
    EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  }
  auto shape = Cons();
  shape.SetPosition(MakePositionPM(34'000'000'000LL, 0, 0));
  auto world = World();
  world.AddVolume(shape);
  EXPECT_NO_THROW(world.Validate());
}

TEST(N1Cons, PreservesTheSharedRotationContract) {
  auto shape = Cons();
  shape.SetRotation(90_deg, 90_deg, 90_deg);
  EXPECT_DOUBLE_EQ(shape.GetRotation().axis_x.z, -1.0);
  EXPECT_DOUBLE_EQ(shape.GetRotation().axis_y.y, 1.0);
  EXPECT_DOUBLE_EQ(shape.GetRotation().axis_z.x, 1.0);
  EXPECT_THROW(
    shape.SetRotation(
      MakeRadians(std::numeric_limits<long double>::quiet_NaN()), 0_rad, 0_rad),
    std::bad_expected_access<UnitConversionError>);
  EXPECT_DOUBLE_EQ(shape.GetRotation().axis_z.x, 1.0);
}

TEST(N1Cons, CompactSharedRecordPreservesExistingSceneStride) {
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSConsRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSConsRecord>);
  EXPECT_EQ(sizeof(GGEMSConsRecord), 40U);
  EXPECT_EQ(alignof(GGEMSConsRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, outer_radius_pm), 0U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, inner_radius_pm), 4U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, outer_slope), 8U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, inner_slope), 12U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, half_length_pm), 16U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, outer_normal_scale), 20U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, inner_normal_scale), 24U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, volume_id), 28U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, material_id), 32U);
  EXPECT_EQ(offsetof(GGEMSConsRecord, tolerance_pm), 36U);
  EXPECT_EQ(sizeof(GGEMSAnalyticVolumeRecord), 84U);
  EXPECT_EQ(offsetof(GGEMSAnalyticVolumeRecord, parameters), 4U);
}

TEST(N1Cons, RejectsConesWhoseNormalBandsConsumeTheEntireInterior) {
  auto world = World();
  auto const band =
    static_cast<std::uint64_t>(world.GetNavigationTolerancePM());
  // Endpoint radius and axial half length each exceed T, but the contracted
  // tip and cap cannot enclose any material for this squat cone.
  auto const half_length = band + (band / 8U);
  GGEMSCons shape;
  shape.SetNegativeZRadii(0_pm, 0_pm)
    .SetPositiveZRadii(0_pm, Length{2U * half_length})
    .SetLength(Length{2U * half_length})
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(shape);
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
}
