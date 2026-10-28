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
 * \brief Checks occurrence identity, admission, and shared polygon profile
 * packing.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"
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

auto Box(std::int64_t x) -> GGEMSBox {
  GGEMSBox box;
  box.SetSize(8_mm, 10_mm, 12_mm)
    .SetPosition(MakePositionPM(x, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  return box;
}

auto Polygon() -> GGEMSPolyhedra {
  GGEMSPolyhedra polygon;
  std::array const z{PositionCoordinate{-6'000'000'000LL},
                     PositionCoordinate{0},
                     PositionCoordinate{6'000'000'000LL}};
  std::array const inner{1_mm, 2_mm, 1_mm};
  std::array const outer{6_mm, 4_mm, 6_mm};
  polygon.SetSideCount(6)
    .SetStartAngle(0_deg)
    .SetZProfile(z, inner, outer)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  return polygon;
}
} // namespace

TEST(GGEMSAnalyticOccurrences,
     RejectsOverlappingInteriorsAndAdmitsFaceContact) {
  auto overlap = World();
  overlap.AddVolume(Box(0)).AddVolume(Box(7'000'000'000LL));
  EXPECT_THROW(overlap.Validate(), GGEMSRecoverable);
  auto touching = World();
  touching.AddVolume(Box(0)).AddVolume(Box(8'000'000'000LL));
  EXPECT_NO_THROW(touching.Validate());
  auto separated = World();
  separated.AddVolume(Box(0)).AddVolume(Box(9'000'000'000LL));
  EXPECT_NO_THROW(separated.Validate());
}

TEST(GGEMSAnalyticOccurrences, CopiesOccurrencePoseAndMaterialIndependently) {
  auto world = World();
  auto shape = Box(-20'000'000'000LL);
  world.AddVolume(shape);
  shape.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 30_deg)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(shape);
  shape.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  auto const copy = world;
  ASSERT_NO_THROW(copy.Validate());
  ASSERT_EQ(copy.GetVolumeCount(), 2U);
  EXPECT_EQ(copy.GetVolumeMaterial(0)->GetName(), "Water");
  EXPECT_EQ(copy.GetVolumeMaterial(1)->GetName(), "Aluminum");
  std::array<std::uint32_t, 2> const ids{7U, 11U};
  auto const records = copy.BuildAnalyticDeviceRecords(ids);
  ASSERT_EQ(records.size(), 2U);
  EXPECT_EQ(records[0].parameters.box.volume_id, 1U);
  EXPECT_EQ(records[1].parameters.box.volume_id, 2U);
  EXPECT_EQ(records[0].parameters.box.material_id, 7U);
  EXPECT_EQ(records[1].parameters.box.material_id, 11U);
  EXPECT_FLOAT_EQ(records[0].parameters.box.half_extent_x_pm, 4e9F);
  EXPECT_FLOAT_EQ(records[1].parameters.box.half_extent_x_pm, 4e9F);
  EXPECT_FLOAT_EQ(copy.BuildBoxPoseRecord(0)->center_x_pm, -20e9F);
  EXPECT_FLOAT_EQ(copy.BuildBoxPoseRecord(1)->center_x_pm, 20e9F);
  EXPECT_EQ(copy.BuildRecord().occurrence_count, 2U);
  EXPECT_GE(copy.BuildRecord().max_boundary_events, 5U);
}

TEST(GGEMSAnalyticOccurrences,
     SharedPolygonProfileRetainsIndependentPosesAndMaterials) {
  auto world = World();
  auto polygon = Polygon();
  polygon.SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0));
  world.AddVolume(polygon);
  polygon.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 30_deg)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(polygon);
  ASSERT_NO_THROW(world.Validate());
  std::array<std::uint32_t, 2> const ids{7U, 11U};
  auto const records = world.BuildAnalyticDeviceRecords(ids);
  auto const occurrences = world.BuildOccurrenceRecords();
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  ASSERT_EQ(records.size(), 2U);
  ASSERT_EQ(occurrences.size(), 2U);
  EXPECT_EQ(occurrences[0].record_index, 0U);
  EXPECT_EQ(occurrences[1].record_index, 1U);
  EXPECT_NE(occurrences[0].auxiliary_offset, occurrences[1].auxiliary_offset);
  auto const first_profile =
    occurrences[0].auxiliary_offset +
    records[0].parameters.polyhedra.profile_offset_bytes;
  auto const second_profile =
    occurrences[1].auxiliary_offset +
    records[1].parameters.polyhedra.profile_offset_bytes;
  EXPECT_EQ(first_profile, second_profile);
  EXPECT_LE(first_profile + 6U * sizeof(GGEMSPolyhedraSideRecord) +
              2U * sizeof(GGEMSPolyhedraSectionRecord),
            auxiliary.size());
  for (std::size_t index = 0; index < occurrences.size(); ++index) {
    GGEMSAnalyticOccurrenceRecord uploaded;
    std::memcpy(&uploaded, auxiliary.data() + index * sizeof(uploaded),
                sizeof(uploaded));
    EXPECT_EQ(uploaded.record_index, occurrences[index].record_index);
    EXPECT_EQ(uploaded.auxiliary_offset, occurrences[index].auxiliary_offset);
    ASSERT_LE(occurrences[index].auxiliary_offset + sizeof(GGEMSBoxPoseRecord),
              auxiliary.size());
    GGEMSBoxPoseRecord pose;
    std::memcpy(&pose, auxiliary.data() + occurrences[index].auxiliary_offset,
                sizeof(pose));
    EXPECT_FLOAT_EQ(pose.center_x_pm, index == 0 ? -20e9F : 20e9F);
    EXPECT_EQ(records[index].parameters.polyhedra.volume_id, index + 1U);
    EXPECT_EQ(records[index].parameters.polyhedra.material_id, ids[index]);
  }
}

TEST(GGEMSAnalyticOccurrences, SharedRecordsRetainCompactScalarLayout) {
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSAnalyticOccurrenceRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSAnalyticOccurrenceRecord>);
  EXPECT_EQ(sizeof(GGEMSAnalyticOccurrenceRecord), 8U);
  EXPECT_EQ(alignof(GGEMSAnalyticOccurrenceRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSAnalyticOccurrenceRecord, record_index), 0U);
  EXPECT_EQ(offsetof(GGEMSAnalyticOccurrenceRecord, auxiliary_offset), 4U);
  EXPECT_EQ(sizeof(GGEMSPolyhedraRecord), 24U);
  EXPECT_EQ(offsetof(GGEMSPolyhedraRecord, profile_offset_bytes), 20U);
  EXPECT_EQ(sizeof(GGEMSAnalyticVolumeRecord), 84U);
}
