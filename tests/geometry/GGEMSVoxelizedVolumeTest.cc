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
 * \brief Checks imported voxel labels, palette identity, and World attachment.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;
using ggems::test::GGEMSVoxelizedTestFixture;

/*! \brief Creates a small material-bearing World for attachment checks. */
auto MakeWorld() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(40_mm, 40_mm, 40_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}
} // namespace

TEST(GGEMSVoxelizedVolume, ImportsAnisotropicXFastLabelsAndCompactPalette) {
  GGEMSVoxelizedTestFixture const files;
  auto const volume = files.Import();
  EXPECT_EQ(volume.GetDimensions(), (std::array<std::size_t, 3>{2U, 2U, 2U}));
  EXPECT_EQ(volume.GetSpacing(), (std::array<Length, 3>{1_mm, 2_mm, 3_mm}));
  EXPECT_EQ(volume.GetSize(), (std::array<Length, 3>{2_mm, 4_mm, 6_mm}));
  EXPECT_TRUE(std::ranges::equal(volume.GetLabels(), files.k_labels));
  std::array<std::uint16_t, 8> const expected{0U, 1U, 0U, 0U, 0U, 1U, 1U, 0U};
  EXPECT_TRUE(std::ranges::equal(volume.GetMaterialLabels(), expected));
  ASSERT_EQ(volume.GetMaterials().size(), 2U);
  EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
    volume.GetMaterials()[0], BuildBuiltInMaterial("Water")));
  EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
    volume.GetMaterials()[1], BuildBuiltInMaterial("Air")));
  EXPECT_EQ(volume.GetPositionPM(), Position3PM{});
  EXPECT_TRUE(volume.GetRotation().IsIdentity());
}

TEST(GGEMSVoxelizedVolume, DistinguishesSingletonAndHalfOpenMappingEndpoints) {
  std::array<std::uint16_t, 4> const labels{7U, 8U, 9U, 65535U};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 1U},
    "1 1 1",
    labels,
    "7 9 Water\n9 9 Air\n65535 65536 Water\n",
  };
  auto const volume = files.Import();
  std::array<std::uint16_t, 4> const expected{0U, 0U, 1U, 0U};
  EXPECT_TRUE(std::ranges::equal(volume.GetLabels(), labels));
  EXPECT_TRUE(std::ranges::equal(volume.GetMaterialLabels(), expected));
  EXPECT_EQ(volume.GetMaterials().size(), 2U);
}

TEST(GGEMSVoxelizedVolume, RejectsUnmappedAnatomicalLabel) {
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 2U},
    "1 2 3",
    GGEMSVoxelizedTestFixture::k_labels,
    "10 10 Water\n20 20 Air\n",
  };
  EXPECT_THROW(static_cast<void>(files.Import()), GGEMSRecoverable);
}

TEST(GGEMSVoxelizedVolume, RejectsOverlappingMappingIntervals) {
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 2U},
    "1 2 3",
    GGEMSVoxelizedTestFixture::k_labels,
    "10 21 Water\n20 31 Air\n",
  };
  EXPECT_THROW(static_cast<void>(files.Import()), GGEMSRecoverable);
}

TEST(GGEMSVoxelizedVolume, RejectsIncorrectExternalRawByteCount) {
  GGEMSVoxelizedTestFixture const files;
  std::filesystem::resize_file(files.GetRawPath(), 14U);
  EXPECT_THROW(static_cast<void>(files.Import()), GGEMSRecoverable);
}

TEST(GGEMSVoxelizedVolume, CopiesKeepIndependentPoseAndOwnedImportedData) {
  auto volume = [] -> GGEMSVoxelizedVolume {
    GGEMSVoxelizedTestFixture const files;
    return files.Import();
  }();
  auto const original = volume;
  volume.SetPosition(MakePositionPM(3'000'000'000LL, -2'000'000'000LL, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  EXPECT_EQ(original.GetPositionPM(), Position3PM{});
  EXPECT_TRUE(original.GetRotation().IsIdentity());
  EXPECT_EQ(volume.GetRotation().axis_x.y, 1.0);
  EXPECT_EQ(volume.GetRotation().axis_y.x, -1.0);
  EXPECT_TRUE(std::ranges::equal(original.GetLabels(), volume.GetLabels()));
  EXPECT_TRUE(std::ranges::equal(original.GetMaterialLabels(),
                                 volume.GetMaterialLabels()));
  EXPECT_EQ(original.GetMaterials()[0].GetName(), "Water");
}

TEST(GGEMSVoxelizedVolume, PublishesOnePhysicalIDPerGridInMixedInsertionOrder) {
  GGEMSVoxelizedTestFixture const files;
  auto first = files.Import();
  first.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  auto second = files.Import();
  second.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  auto world = MakeWorld();
  world.AddVolume(first).AddVolume(box).AddVolume(second);
  ASSERT_EQ(world.GetVolumeCount(), 3U);
  ASSERT_NE(world.GetVoxelizedVolume(0U), nullptr);
  EXPECT_EQ(world.GetVoxelizedVolume(1U), nullptr);
  ASSERT_NE(world.GetVoxelizedVolume(2U), nullptr);
  EXPECT_EQ(world.GetVoxelizedVolume(0U)->GetLabels().size(), 8U);
  EXPECT_EQ(world.GetVoxelizedVolume(2U)->GetPositionPM(),
            second.GetPositionPM());
  first.SetPosition(MakePositionPM(0, 0, 0));
  EXPECT_EQ(world.GetVoxelizedVolume(0U)->GetPositionPM().x, -10'000'000'000LL);
  EXPECT_NO_THROW(world.Validate());
}

TEST(GGEMSVoxelizedVolume, RejectsNonWorldParentWithoutPublishingOccurrence) {
  GGEMSVoxelizedTestFixture const files;
  auto world = MakeWorld();
  GGEMSBox mother;
  mother.SetSize(20_mm, 20_mm, 20_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  world.AddVolume(mother);
  EXPECT_THROW(world.AttachVolume(files.Import(), 1U), GGEMSRecoverable);
  EXPECT_EQ(world.GetVolumeCount(), 1U);
  EXPECT_EQ(world.GetVoxelizedVolume(0U), nullptr);
}

TEST(GGEMSVoxelizedVolume, RejectsPosedOuterBoxProtrusionAtomically) {
  GGEMSVoxelizedTestFixture const files;
  auto world = MakeWorld();
  auto volume = files.Import();
  volume.SetPosition(MakePositionPM(17'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  world.AddVolume(volume);
  auto const position_before = world.GetVoxelizedVolume(0U)->GetPositionPM();
  volume.SetPosition(MakePositionPM(19'000'000'000LL, 0, 0));
  EXPECT_THROW(world.AddVolume(volume), GGEMSRecoverable);
  EXPECT_EQ(world.GetVolumeCount(), 1U);
  EXPECT_EQ(world.GetVoxelizedVolume(0U)->GetPositionPM(), position_before);
  EXPECT_NO_THROW(world.Validate());
}

TEST(GGEMSVoxelizedVolume, PacksMixedFamiliesWithoutRenumberingPhysicalIDs) {
  GGEMSVoxelizedTestFixture const files;
  auto first = files.Import();
  first.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  auto second = files.Import();
  second.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  auto world = MakeWorld();
  world.AddVolume(first).AddVolume(box).AddVolume(second);
  ASSERT_NO_THROW(world.Validate());
  std::array<std::uint32_t, 1> const analytic_ids{7U};
  std::array<std::uint32_t, 4> const palette_ids{11U, 7U, 11U, 7U};
  auto const analytic = world.BuildAnalyticDeviceRecords(analytic_ids);
  auto const voxels = world.BuildVoxelizedDeviceData(palette_ids);
  auto const occurrences = world.BuildOccurrenceRecords();
  ASSERT_EQ(analytic.size(), 1U);
  EXPECT_EQ(analytic[0].parameters.box.volume_id, 2U);
  EXPECT_EQ(analytic[0].parameters.box.material_id, 7U);
  ASSERT_EQ(voxels.records.size(), 2U);
  EXPECT_EQ(voxels.records[0].volume_id, 1U);
  EXPECT_EQ(voxels.records[1].volume_id, 3U);
  EXPECT_EQ(voxels.records[0].material_label_offset, 0U);
  EXPECT_EQ(voxels.records[1].material_label_offset, 8U);
  EXPECT_EQ(voxels.records[0].palette_offset, 0U);
  EXPECT_EQ(voxels.records[1].palette_offset, 2U);
  EXPECT_FLOAT_EQ(voxels.records[1].pose.center_x_pm, 10e9F);
  EXPECT_FLOAT_EQ(voxels.records[1].pose.axis_x_y, 1.0F);
  EXPECT_FLOAT_EQ(voxels.records[1].spacing_y_pm, 2e9F);
  EXPECT_FLOAT_EQ(voxels.records[1].half_extent_z_pm, 3e9F);
  EXPECT_EQ(voxels.records[1].dimension_z, 2U);
  EXPECT_GT(voxels.records[1].tolerance_pm, 0.0F);
  EXPECT_TRUE(std::ranges::equal(voxels.material_ids, palette_ids));
  ASSERT_EQ(voxels.material_labels.size(), 16U);
  EXPECT_TRUE(std::ranges::equal(std::span{voxels.material_labels}.first(8U),
                                 first.GetMaterialLabels()));
  EXPECT_TRUE(std::ranges::equal(std::span{voxels.material_labels}.subspan(8U),
                                 second.GetMaterialLabels()));
  ASSERT_EQ(occurrences.size(), 3U);
  EXPECT_EQ(occurrences[0].family, GGEMSOccurrenceFamily::Voxelized);
  EXPECT_EQ(occurrences[1].family, GGEMSOccurrenceFamily::Analytic);
  EXPECT_EQ(occurrences[2].family, GGEMSOccurrenceFamily::Voxelized);
  EXPECT_EQ(occurrences[0].record_index, 0U);
  EXPECT_EQ(occurrences[1].record_index, 0U);
  EXPECT_EQ(occurrences[2].record_index, 1U);
  EXPECT_EQ(world.BuildRecord().occurrence_count, 3U);
  EXPECT_EQ(world.BuildRecord().voxelized_count, 2U);
}

TEST(GGEMSVoxelizedVolume, ScalarDeviceRecordHasExplicitOffsetsAndStride) {
  using Record = GGEMSVoxelizedVolumeRecord;
  EXPECT_TRUE(std::is_standard_layout_v<Record>);
  EXPECT_TRUE(std::is_trivially_copyable_v<Record>);
  EXPECT_EQ(sizeof(Record), 104U);
  EXPECT_EQ(alignof(Record), 8U);
  std::array<std::size_t, 14> const offsets{
    offsetof(Record, pose),
    offsetof(Record, dimension_x),
    offsetof(Record, dimension_y),
    offsetof(Record, dimension_z),
    offsetof(Record, volume_id),
    offsetof(Record, spacing_x_pm),
    offsetof(Record, spacing_y_pm),
    offsetof(Record, spacing_z_pm),
    offsetof(Record, tolerance_pm),
    offsetof(Record, half_extent_x_pm),
    offsetof(Record, half_extent_y_pm),
    offsetof(Record, half_extent_z_pm),
    offsetof(Record, palette_offset),
    offsetof(Record, material_label_offset),
  };
  EXPECT_EQ(offsets, (std::array<std::size_t, 14>{
                       0U,
                       48U,
                       52U,
                       56U,
                       60U,
                       64U,
                       68U,
                       72U,
                       76U,
                       80U,
                       84U,
                       88U,
                       92U,
                       96U,
                     }));
}
