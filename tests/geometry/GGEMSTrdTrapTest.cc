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
 * \brief Tests planar Trd and Trap authoring, scene admission, and shared ABI.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto Trd() -> GGEMSTrd {
  GGEMSTrd shape;
  shape.SetNegativeZFace(4_mm, 8_mm)
    .SetPositiveZFace(8_mm, 4_mm)
    .SetLength(12_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  return shape;
}

auto Trap() -> GGEMSTrap {
  GGEMSTrap shape;
  shape
    .SetNegativeZFace(8_mm, Length{2'400'000'000ULL}, Length{5'600'000'000ULL},
                      MakeRadians(std::atan(.25)))
    .SetPositiveZFace(4_mm, Length{7'200'000'000ULL}, Length{8'800'000'000ULL},
                      MakeRadians(std::atan(.25)))
    .SetLength(12_mm)
    .SetInclination(MakeRadians(std::atan(5.0 / 12)),
                    MakeRadians(std::atan(.75)))
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

TEST(N1Trd, IndependentEndFacesUseFullDimensions) {
  GGEMSTrd shape;
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(shape.GetMaterial()), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetNegativeZFace(4_mm, 8_mm)
               .SetPositiveZFace(8_mm, 4_mm)
               .SetLength(12_mm),
            &shape);
  EXPECT_EQ(shape.GetNegativeZHalfSizePM(),
            (std::array<DistancePM, 2>{2'000'000'000ULL, 4'000'000'000ULL}));
  EXPECT_EQ(shape.GetPositiveZHalfSizePM(),
            (std::array<DistancePM, 2>{4'000'000'000ULL, 2'000'000'000ULL}));
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000ULL);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(shape.Validate());
  shape.SetNegativeZFace(6_mm, 10_mm);
  EXPECT_EQ(shape.GetPositiveZHalfSizePM(),
            (std::array<DistancePM, 2>{4'000'000'000ULL, 2'000'000'000ULL}));
  shape.SetPositiveZFace(12_mm, 14_mm);
  EXPECT_EQ(shape.GetNegativeZHalfSizePM(),
            (std::array<DistancePM, 2>{3'000'000'000ULL, 5'000'000'000ULL}));
}

TEST(N1Trap, FullFaceDimensionsSkewAndInclinationAreIndependentOfPose) {
  auto shape = Trap();
  EXPECT_NO_THROW(shape.Validate());
  EXPECT_EQ(shape.GetNegativeZHalfSizePM(),
            (std::array<DistancePM, 3>{4'000'000'000ULL, 1'200'000'000ULL,
                                       2'800'000'000ULL}));
  EXPECT_EQ(shape.GetPositiveZHalfSizePM(),
            (std::array<DistancePM, 3>{2'000'000'000ULL, 3'600'000'000ULL,
                                       4'400'000'000ULL}));
  EXPECT_EQ(shape.GetHalfLengthPM(), 6'000'000'000ULL);
  auto const negative_skew = shape.GetNegativeZSkew();
  auto const theta = shape.GetInclinationTheta();
  auto const phi = shape.GetInclinationPhi();
  shape.SetRotation(90_deg, 30_deg, -60_deg)
    .SetPosition(
      MakePositionPM(-7'000'000'000LL, 5'000'000'000LL, -3'000'000'000LL));
  EXPECT_EQ(shape.GetNegativeZSkew(), negative_skew);
  EXPECT_EQ(shape.GetInclinationTheta(), theta);
  EXPECT_EQ(shape.GetInclinationPhi(), phi);
  EXPECT_EQ(shape.GetPositionPM().x, -7'000'000'000LL);
  EXPECT_NO_THROW(shape.Validate());
  auto const negative = shape.GetNegativeZHalfSizePM();
  shape.SetPositiveZFace(6_mm, 6_mm, 6_mm);
  EXPECT_EQ(shape.GetNegativeZHalfSizePM(), negative);
  auto const positive = shape.GetPositiveZHalfSizePM();
  shape.SetNegativeZFace(10_mm, 4_mm, 6_mm);
  EXPECT_EQ(shape.GetPositiveZHalfSizePM(), positive);
}

TEST(N1TrdTrap, RejectsMissingDimensionsMaterialAndInvalidSettersAtomically) {
  GGEMSTrap incomplete;
  EXPECT_THROW(incomplete.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(incomplete.GetMaterial()), GGEMSRecoverable);
  incomplete.SetNegativeZFace(4_mm, 2_mm, 2_mm)
    .SetPositiveZFace(4_mm, 2_mm, 2_mm)
    .SetLength(6_mm);
  EXPECT_THROW(incomplete.Validate(), GGEMSRecoverable);
  auto trd = Trd();
  auto trap = Trap();
  for (auto const invalid : {0_pm, 1_pm, 3_pm}) {
    EXPECT_THROW(trd.SetNegativeZFace(invalid, 8_mm), GGEMSRecoverable);
    EXPECT_THROW(trd.SetPositiveZFace(8_mm, invalid), GGEMSRecoverable);
    EXPECT_THROW(trd.SetLength(invalid), GGEMSRecoverable);
    EXPECT_THROW(trap.SetNegativeZFace(invalid, 2_mm, 4_mm), GGEMSRecoverable);
    EXPECT_THROW(trap.SetNegativeZFace(8_mm, invalid, 4_mm), GGEMSRecoverable);
    EXPECT_THROW(trap.SetPositiveZFace(4_mm, 8_mm, invalid), GGEMSRecoverable);
    EXPECT_THROW(trap.SetLength(invalid), GGEMSRecoverable);
  }
  EXPECT_EQ(trd.GetNegativeZHalfSizePM()[0], 2'000'000'000ULL);
  EXPECT_EQ(trap.GetPositiveZHalfSizePM()[2], 4'400'000'000ULL);
  EXPECT_EQ(trd.GetHalfLengthPM(), 6'000'000'000ULL);
  EXPECT_EQ(trap.GetHalfLengthPM(), 6'000'000'000ULL);
  EXPECT_THROW(trap.SetInclination(90_deg, 0_deg), GGEMSRecoverable);
  EXPECT_THROW(trap.SetInclination(-90_deg, 0_deg), GGEMSRecoverable);
  EXPECT_THROW(trap.SetPositiveZFace(4_mm, 8_mm, 8_mm, 90_deg),
               GGEMSRecoverable);
  EXPECT_NO_THROW(trap.Validate());
  auto const invalid_angle = std::numeric_limits<long double>::quiet_NaN();
  EXPECT_THROW(trd.SetRotation(MakeRadians(invalid_angle), 0_rad, 0_rad),
               std::bad_expected_access<UnitConversionError>);
  EXPECT_THROW(trap.SetInclination(MakeRadians(invalid_angle), 0_rad),
               std::bad_expected_access<UnitConversionError>);
  EXPECT_NO_THROW(trd.Validate());
  EXPECT_NO_THROW(trap.Validate());
}

TEST(N1Trap, RejectsWarpedSidesWithoutRepairingAuthoredDimensions) {
  auto widths = Trap();
  widths.SetPositiveZFace(4_mm, 7_mm, 9_mm, MakeRadians(std::atan(.25)));
  EXPECT_THROW(widths.Validate(), GGEMSRecoverable);
  EXPECT_EQ(widths.GetPositiveZHalfSizePM()[1], 3'500'000'000ULL);
  EXPECT_THROW(World().AddVolume(widths), GGEMSRecoverable);
  auto skew = Trap();
  skew.SetPositiveZFace(4_mm, Length{7'200'000'000ULL},
                        Length{8'800'000'000ULL}, 0_deg);
  EXPECT_THROW(skew.Validate(), GGEMSRecoverable);
}

TEST(N1TrdTrap, OwnsValuesMaterialsAndCopiedWorldSnapshots) {
  auto check = [](auto shape, GGEMSAnalyticKind kind) -> void {
    shape
      .SetPosition(
        MakePositionPM(-7'000'000'000LL, 5'000'000'000LL, -3'000'000'000LL))
      .SetRotation(0_deg, -90_deg, 0_deg);
    auto world = World();
    world.AddVolume(shape);
    shape.SetLength(2_mm)
      .SetPosition(MakePositionPM(0, 0, 0))
      .SetRotation(0_deg, 0_deg, 0_deg)
      .SetMaterial(BuildBuiltInMaterial("Vacuum"));
    auto copied_world = world;
    world.SetSize(160_mm, 160_mm, 160_mm);
    ASSERT_NO_THROW(copied_world.Validate());
    auto const record = copied_world.BuildAnalyticVolumeRecord(7U);
    EXPECT_EQ(record.kind, kind);
    EXPECT_FLOAT_EQ(record.parameters.planar.half_length_pm, 6.0e9F);
    EXPECT_EQ(record.parameters.planar.volume_id, 1U);
    EXPECT_EQ(record.parameters.planar.material_id, 7U);
    EXPECT_EQ(record.parameters.planar.tolerance_pm,
              copied_world.GetVolumeNavigationTolerancePM());
    ASSERT_NE(copied_world.GetVolumeMaterial(), nullptr);
    EXPECT_EQ(copied_world.GetVolumeMaterial()->GetName(), "Water");
    auto const pose = copied_world.BuildBoxPoseRecord();
    ASSERT_TRUE(pose.has_value());
    EXPECT_FLOAT_EQ(pose->center_x_pm, -7.0e9F);
    EXPECT_FLOAT_EQ(pose->axis_z_x, -1.0F);
    EXPECT_FLOAT_EQ(pose->axis_x_z, 1.0F);
    EXPECT_TRUE(copied_world.GetBoxes().empty());
    EXPECT_TRUE(copied_world.BuildBoxRecords({}).empty());
  };
  check(Trd(), GGEMSAnalyticKind::Trd);
  check(Trap(), GGEMSAnalyticKind::Trap);
}

TEST(N1TrdTrap, EveryNewOverloadPreservesTheOneVolumeWorld) {
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSSphere sphere;
  sphere.SetRadius(1_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSTubs tubs;
  tubs.SetRadii(1_mm).SetLength(2_mm).SetMaterial(
    BuildBuiltInMaterial("Water"));
  GGEMSEllipsoid ellipsoid;
  ellipsoid.SetSemiAxes(1_mm, 2_mm, 3_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  auto first = World();
  first.AddVolume(Trd());
  EXPECT_THROW(first.AddVolume(Trd()), GGEMSRecoverable);
  EXPECT_THROW(first.AddVolume(Trap()), GGEMSRecoverable);
  EXPECT_THROW(first.AddVolume(box), GGEMSRecoverable);
  EXPECT_THROW(first.AddVolume(sphere), GGEMSRecoverable);
  EXPECT_THROW(first.AddVolume(tubs), GGEMSRecoverable);
  EXPECT_THROW(first.AddVolume(ellipsoid), GGEMSRecoverable);
  auto second = World();
  second.AddVolume(Trap());
  EXPECT_THROW(second.AddVolume(Trd()), GGEMSRecoverable);
  EXPECT_THROW(second.AddVolume(Trap()), GGEMSRecoverable);
  auto third = World();
  third.AddVolume(box);
  EXPECT_THROW(third.AddVolume(Trd()), GGEMSRecoverable);
  EXPECT_THROW(third.AddVolume(Trap()), GGEMSRecoverable);
}

TEST(N1TrdTrap, RejectsUnresolvedFeaturesAndRotatedWorldContact) {
  auto check = [](auto shape) -> void {
    shape.SetLength(2_pm);
    auto thin = World();
    thin.AddVolume(shape);
    EXPECT_THROW(thin.Validate(), GGEMSRecoverable);
    shape.SetLength(12_mm).SetPosition(MakePositionPM(35'000'000'000LL, 0, 0));
    // Trap has a larger +X endpoint than Trd; use a shared 90-degree pose
    // where both solids' Z caps alone exceed the World X extent.
    shape.SetRotation(0_deg, 90_deg, 0_deg);
    auto rotated = World();
    rotated.AddVolume(shape);
    EXPECT_THROW(rotated.Validate(), GGEMSRecoverable);
    shape.SetPosition(MakePositionPM(-7'000'000'000LL, 0, 0));
    auto inside = World();
    inside.AddVolume(shape);
    EXPECT_NO_THROW(inside.Validate());
  };
  check(Trd());
  check(Trap());
}

TEST(N1Trd, CentralUnitsConversionPrecedesFullDimensionAuthoring) {
  auto const size = MakeQuantity<Length>(1.25, "mm");
  ASSERT_TRUE(size.has_value());
  GGEMSTrd shape;
  shape.SetNegativeZFace(*size, 8_mm).SetPositiveZFace(8_mm, *size);
  EXPECT_EQ(shape.GetNegativeZHalfSizePM()[0], 625'000'000ULL);
  EXPECT_EQ(shape.GetPositiveZHalfSizePM()[1], 625'000'000ULL);
}

TEST(N1Trap, RejectsUnresolvedNarrowEdgeDespiteBroadFaceCenters) {
  GGEMSTrap shape;
  shape.SetNegativeZFace(20_mm, 20_mm, 2_pm)
    .SetPositiveZFace(20_mm, 20_mm, 2_pm)
    .SetLength(20_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  // Both face centers are several millimeters from every side, but the
  // positive-Y edge has only a 1-pm half-width and vanishes under FP32 packing.
  EXPECT_NO_THROW(shape.Validate());
  auto unresolved = World();
  ASSERT_NO_THROW(unresolved.AddVolume(shape));
  EXPECT_THROW(unresolved.Validate(), GGEMSRecoverable);

  shape.SetNegativeZFace(20_mm, 20_mm, 1_mm)
    .SetPositiveZFace(20_mm, 20_mm, 1_mm);
  auto resolved = World();
  resolved.AddVolume(shape);
  EXPECT_NO_THROW(resolved.Validate());
}

TEST(N1TrdTrap, HostPlanePayloadMatchesDeviceOffsetsWithoutParticleGrowth) {
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSPlaneRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSPlaneRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSPlanarRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSPlanarRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSAnalyticVolumeRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSAnalyticVolumeRecord>);
  EXPECT_EQ(sizeof(GGEMSPlaneRecord), 16U);
  EXPECT_EQ(alignof(GGEMSPlaneRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSPlaneRecord, normal_x), 0U);
  EXPECT_EQ(offsetof(GGEMSPlaneRecord, normal_y), 4U);
  EXPECT_EQ(offsetof(GGEMSPlaneRecord, normal_z), 8U);
  EXPECT_EQ(offsetof(GGEMSPlaneRecord, offset_pm), 12U);
  EXPECT_EQ(sizeof(GGEMSPlanarRecord), 80U);
  EXPECT_EQ(alignof(GGEMSPlanarRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSPlanarRecord, sides), 0U);
  EXPECT_EQ(offsetof(GGEMSPlanarRecord, half_length_pm), 64U);
  EXPECT_EQ(offsetof(GGEMSPlanarRecord, volume_id), 68U);
  EXPECT_EQ(offsetof(GGEMSPlanarRecord, material_id), 72U);
  EXPECT_EQ(offsetof(GGEMSPlanarRecord, tolerance_pm), 76U);
  EXPECT_EQ(sizeof(GGEMSAnalyticVolumeRecord), 84U);
  EXPECT_EQ(alignof(GGEMSAnalyticVolumeRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSAnalyticVolumeRecord, parameters), 4U);
  EXPECT_EQ(sizeof(ggems::core::particles::GGEMSParticleState), 80U);
}
