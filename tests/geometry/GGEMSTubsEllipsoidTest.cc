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
 * \brief Tests Tubs and Ellipsoid authoring, ownership, admission, and ABI.
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
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto Tubs() -> GGEMSTubs {
  GGEMSTubs shape;
  shape.SetRadii(4_mm, 2_mm)
    .SetLength(12_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  return shape;
}

auto Ellipsoid() -> GGEMSEllipsoid {
  GGEMSEllipsoid shape;
  shape.SetSemiAxes(4_mm, 3_mm, 6_mm)
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

TEST(N1Tubs, RequiresDimensionsAndOwnedMaterial) {
  GGEMSTubs shape;
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(shape.GetMaterial()), GGEMSRecoverable);
  EXPECT_THROW(shape.SetRadii(0_pm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetRadii(2_mm, 2_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetRadii(2_mm, 3_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetLength(0_pm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetLength(3_pm), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetRadii(4_mm).SetLength(12_mm), &shape);
  EXPECT_EQ(shape.GetInnerRadiusPM(), 0U);
  EXPECT_EQ(shape.GetOuterRadiusPM(), 4'000'000'000U);
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000U);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(shape.Validate());
  shape.SetRadii(4_mm, 2_mm);
  EXPECT_THROW(shape.SetRadii(1_mm, 2_mm), GGEMSRecoverable);
  EXPECT_EQ(shape.GetOuterRadiusPM(), 4'000'000'000U);
  EXPECT_EQ(shape.GetInnerRadiusPM(), 2'000'000'000U);
  EXPECT_THROW(shape.SetLength(1_pm), GGEMSRecoverable);
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000U);
}

TEST(N1Ellipsoid, RequiresPositiveSemiAxesAndOwnedMaterial) {
  GGEMSEllipsoid shape;
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(shape.GetMaterial()), GGEMSRecoverable);
  EXPECT_THROW(shape.SetSemiAxes(0_pm, 1_mm, 1_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetSemiAxes(1_mm, 0_pm, 1_mm), GGEMSRecoverable);
  EXPECT_THROW(shape.SetSemiAxes(1_mm, 1_mm, 0_pm), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetSemiAxes(4_mm, 3_mm, 6_mm), &shape);
  EXPECT_EQ(shape.GetSemiAxesPM(),
            (HalfExtent3PM{4'000'000'000U, 3'000'000'000U, 6'000'000'000U}));
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(shape.Validate());
  EXPECT_THROW(shape.SetSemiAxes(4_mm, 0_pm, 6_mm), GGEMSRecoverable);
  EXPECT_EQ(shape.GetSemiAxesPM().y, 3'000'000'000U);
  EXPECT_NO_THROW(shape.SetSemiAxes(1_pm, 3_pm, 5_pm));
}

TEST(N1Tubs, CopiesPoseDimensionsAndMaterialIntoWorld) {
  auto shape = Tubs();
  shape.SetPosition(MakePositionPM(-5'000'000'000LL, 7'000'000'000LL, 0))
    .SetRotation(0_deg, 90_deg, 0_deg);
  auto world = World();
  world.AddVolume(shape);
  shape.SetRadii(8_mm)
    .SetLength(2_mm)
    .SetRotation(0_deg, 0_deg, 0_deg)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  auto copy = world;
  EXPECT_NO_THROW(copy.Validate());
  auto const packed = copy.BuildAnalyticVolumeRecord(7U);
  EXPECT_EQ(packed.kind, GGEMSAnalyticKind::Tubs);
  EXPECT_FLOAT_EQ(packed.parameters.tubs.outer_radius_pm, 4.0e9F);
  EXPECT_FLOAT_EQ(packed.parameters.tubs.inner_radius_pm, 2.0e9F);
  EXPECT_FLOAT_EQ(packed.parameters.tubs.half_length_pm, 6.0e9F);
  EXPECT_EQ(packed.parameters.tubs.volume_id, 1U);
  EXPECT_EQ(packed.parameters.tubs.material_id, 7U);
  ASSERT_NE(copy.GetVolumeMaterial(), nullptr);
  EXPECT_EQ(copy.GetVolumeMaterial()->GetName(), "Water");
  EXPECT_TRUE(copy.BuildBoxPoseRecord().has_value());
  EXPECT_TRUE(copy.GetBoxes().empty());
  EXPECT_TRUE(copy.BuildBoxRecords({}).empty());
}

TEST(N1Ellipsoid, CopiesPoseDimensionsAndMaterialIntoWorld) {
  auto shape = Ellipsoid();
  shape.SetPosition(MakePositionPM(-5'000'000'000LL, 7'000'000'000LL, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto world = World();
  world.AddVolume(shape);
  shape.SetSemiAxes(8_mm, 8_mm, 8_mm)
    .SetRotation(0_deg, 0_deg, 0_deg)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  auto copy = world;
  EXPECT_NO_THROW(copy.Validate());
  auto const packed = copy.BuildAnalyticVolumeRecord(7U);
  EXPECT_EQ(packed.kind, GGEMSAnalyticKind::Ellipsoid);
  EXPECT_FLOAT_EQ(packed.parameters.ellipsoid.semi_axis_x_pm, 4.0e9F);
  EXPECT_FLOAT_EQ(packed.parameters.ellipsoid.semi_axis_y_pm, 3.0e9F);
  EXPECT_FLOAT_EQ(packed.parameters.ellipsoid.semi_axis_z_pm, 6.0e9F);
  EXPECT_EQ(packed.parameters.ellipsoid.volume_id, 1U);
  EXPECT_EQ(packed.parameters.ellipsoid.material_id, 7U);
  ASSERT_NE(copy.GetVolumeMaterial(), nullptr);
  EXPECT_EQ(copy.GetVolumeMaterial()->GetName(), "Water");
  EXPECT_TRUE(copy.BuildBoxPoseRecord().has_value());
  EXPECT_TRUE(copy.GetBoxes().empty());
}

TEST(N1TubsEllipsoid, PreservesTheOneVolumeLimitForEveryNewOverload) {
  auto world = World();
  world.AddVolume(Tubs());
  EXPECT_THROW(world.AddVolume(Ellipsoid()), GGEMSRecoverable);
  EXPECT_THROW(world.AddVolume(Tubs()), GGEMSRecoverable);
  auto second = World();
  second.AddVolume(Ellipsoid());
  EXPECT_THROW(second.AddVolume(Tubs()), GGEMSRecoverable);
  EXPECT_THROW(second.AddVolume(Ellipsoid()), GGEMSRecoverable);
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_THROW(second.AddVolume(box), GGEMSRecoverable);
  auto third = World();
  third.AddVolume(box);
  EXPECT_THROW(third.AddVolume(Tubs()), GGEMSRecoverable);
  EXPECT_THROW(third.AddVolume(Ellipsoid()), GGEMSRecoverable);
}

TEST(N1TubsEllipsoid, PoseValidationUsesTheSharedRotationContract) {
  auto tubs = Tubs();
  auto ellipsoid = Ellipsoid();
  tubs.SetRotation(90_deg, 90_deg, 90_deg);
  ellipsoid.SetRotation(90_deg, 90_deg, 90_deg);
  EXPECT_DOUBLE_EQ(tubs.GetRotation().axis_x.z, -1.0);
  EXPECT_DOUBLE_EQ(tubs.GetRotation().axis_y.y, 1.0);
  EXPECT_DOUBLE_EQ(tubs.GetRotation().axis_z.x, 1.0);
  EXPECT_DOUBLE_EQ(ellipsoid.GetRotation().axis_z.x, 1.0);
  auto const invalid = std::numeric_limits<long double>::quiet_NaN();
  EXPECT_THROW(tubs.SetRotation(MakeRadians(invalid), 0_rad, 0_rad),
               std::bad_expected_access<UnitConversionError>);
  EXPECT_THROW(ellipsoid.SetRotation(MakeRadians(invalid), 0_rad, 0_rad),
               std::bad_expected_access<UnitConversionError>);
  EXPECT_DOUBLE_EQ(tubs.GetRotation().axis_z.x, 1.0);
  EXPECT_DOUBLE_EQ(ellipsoid.GetRotation().axis_z.x, 1.0);
}

TEST(N1TubsEllipsoid, RejectsUnresolvedFeaturesAndWorldClearance) {
  auto tiny_bore = Tubs();
  tiny_bore.SetRadii(4_mm, 1_pm);
  auto thin_wall = Tubs();
  thin_wall.SetRadii(4_mm, Length{3'999'999'999ULL});
  for (auto const &shape : {tiny_bore, thin_wall}) {
    auto world = World();
    world.AddVolume(shape);
    EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  }
  auto flat = Ellipsoid();
  flat.SetSemiAxes(4_mm, 1_pm, 6_mm);
  auto world = World();
  world.AddVolume(flat);
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  for (auto const center : {std::int64_t{36'000'000'000LL},
                            std::numeric_limits<std::int64_t>::max(),
                            std::numeric_limits<std::int64_t>::min()}) {
    auto tubs = Tubs();
    auto ellipsoid = Ellipsoid();
    tubs.SetPosition(MakePositionPM(center, 0, 0));
    ellipsoid.SetPosition(MakePositionPM(center, 0, 0));
    auto first = World();
    first.AddVolume(tubs);
    EXPECT_THROW(first.Validate(), GGEMSRecoverable);
    auto second = World();
    second.AddVolume(ellipsoid);
    EXPECT_THROW(second.Validate(), GGEMSRecoverable);
  }
  auto rotated = Tubs();
  rotated.SetRotation(0_deg, 90_deg, 0_deg)
    .SetPosition(MakePositionPM(35'000'000'000LL, 0, 0));
  auto posed = World();
  posed.AddVolume(rotated);
  EXPECT_THROW(posed.Validate(), GGEMSRecoverable);
}

TEST(N1TubsEllipsoid, CentralUnitsConvertDimensionsBeforeAuthoring) {
  EXPECT_FALSE(MakeQuantity<Length>(-1.0, "mm").has_value());
  EXPECT_FALSE(MakeQuantity<Length>(1.0, "MM").has_value());
  auto const radius = MakeQuantity<Length>(1.25, "mm");
  ASSERT_TRUE(radius.has_value());
  EXPECT_EQ(GGEMSTubs{}.SetRadii(*radius).GetOuterRadiusPM(), 1'250'000'000U);
  EXPECT_EQ(
    GGEMSEllipsoid{}.SetSemiAxes(*radius, *radius, *radius).GetSemiAxesPM().x,
    1'250'000'000U);
}

TEST(N1TubsEllipsoid, SharedRecordsPreserveTheirPayloadOffsets) {
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSTubsRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSTubsRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSEllipsoidRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSEllipsoidRecord>);
  EXPECT_EQ(sizeof(GGEMSAnalyticVolumeRecord), 84U);
  EXPECT_EQ(offsetof(GGEMSAnalyticVolumeRecord, parameters), 4U);
  EXPECT_EQ(sizeof(GGEMSTubsRecord), 24U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, outer_radius_pm), 0U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, inner_radius_pm), 4U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, half_length_pm), 8U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, volume_id), 12U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, material_id), 16U);
  EXPECT_EQ(offsetof(GGEMSTubsRecord, tolerance_pm), 20U);
  EXPECT_EQ(sizeof(GGEMSEllipsoidRecord), 24U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, semi_axis_x_pm), 0U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, semi_axis_y_pm), 4U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, semi_axis_z_pm), 8U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, volume_id), 12U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, material_id), 16U);
  EXPECT_EQ(offsetof(GGEMSEllipsoidRecord, tolerance_pm), 20U);
}

TEST(N1TubsEllipsoid, AdmitsTheActualRotatedSurfaceEnvelope) {
  auto tubs = Tubs();
  tubs.SetRotation(0_deg, 0_deg, 45_deg);
  auto tubs_world = World();
  tubs_world.SetSize(9_mm, 9_mm, 14_mm).AddVolume(tubs);
  EXPECT_NO_THROW(tubs_world.Validate());
  // A 45-degree ellipsoid has XY support sqrt((4^2+3^2)/2) mm,
  // strictly below 3.6 mm; a rotated circumscribed Box does not fit.
  auto ellipsoid = Ellipsoid();
  ellipsoid.SetRotation(0_deg, 0_deg, 45_deg);
  auto ellipsoid_world = World();
  ellipsoid_world
    .SetSize(Length{7'200'000'000ULL}, Length{7'200'000'000ULL}, 14_mm)
    .AddVolume(ellipsoid);
  EXPECT_NO_THROW(ellipsoid_world.Validate());
  ellipsoid.SetPosition(MakePositionPM(100'000'000, 0, 0));
  auto outside = World();
  outside.SetSize(Length{7'200'000'000ULL}, Length{7'200'000'000ULL}, 14_mm)
    .AddVolume(ellipsoid);
  EXPECT_THROW(outside.Validate(), GGEMSRecoverable);
}
