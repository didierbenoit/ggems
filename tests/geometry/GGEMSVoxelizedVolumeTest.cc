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
#include <fstream>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMSScopedLoggerEncoding.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
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
  EXPECT_TRUE(volume.GetName().empty());
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
  for (auto const label : files.k_labels) {
    auto const &style = volume.GetDisplayStyle(label);
    EXPECT_EQ(style.mode, ggems::render::GGEMSGeometryDisplayMode::Solid);
    EXPECT_EQ(style.color,
              BuildBuiltInMaterial(label == 20U ? "Air" : "Water").GetColor());
  }
}

TEST(GGEMSVoxelizedVolume,
     NameFirstAndTwoPathConstructorsPreserveImportedPayload) {
  GGEMSVoxelizedTestFixture const files;
  auto const directory = files.GetRawPath().parent_path();
  auto const image = directory / "grid.mhd";
  auto const mapping = directory / "materials.txt";
  GGEMSVoxelizedVolume const unnamed{image, mapping};
  std::string name{"XCAT"};
  GGEMSVoxelizedVolume const named{name, image, mapping};
  name = "Edited source string";
  EXPECT_TRUE(unnamed.GetName().empty());
  EXPECT_EQ(named.GetName(), "XCAT");
  EXPECT_EQ(named.GetDimensions(), unnamed.GetDimensions());
  EXPECT_EQ(named.GetSpacing(), unnamed.GetSpacing());
  EXPECT_EQ(named.GetSize(), unnamed.GetSize());
  EXPECT_TRUE(std::ranges::equal(named.GetLabels(), unnamed.GetLabels()));
  EXPECT_TRUE(
    std::ranges::equal(named.GetMaterialLabels(), unnamed.GetMaterialLabels()));
  ASSERT_EQ(named.GetMaterials().size(), unnamed.GetMaterials().size());
  for (std::size_t index = 0U; index < named.GetMaterials().size(); ++index) {
    EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
      named.GetMaterials()[index], unnamed.GetMaterials()[index]));
  }
  for (auto const label : named.GetLabels()) {
    EXPECT_EQ(named.GetDisplayStyle(label).mode,
              unnamed.GetDisplayStyle(label).mode);
    EXPECT_EQ(named.GetDisplayStyle(label).color,
              unnamed.GetDisplayStyle(label).color);
  }
  // A name that looks like a real path remains a label, never an import hint.
  GGEMSVoxelizedVolume const path_named{image.string(), image, mapping};
  EXPECT_EQ(path_named.GetName(), image.string());
  EXPECT_THROW(
    (GGEMSVoxelizedVolume{"XCAT", directory / "missing.mhd", mapping}),
    GGEMSRecoverable);
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

TEST(GGEMSVoxelizedVolume, MappingStylesPreserveRangesAndPhysicalPalette) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  auto const water_color = BuildBuiltInMaterial("Water").GetColor();
  std::array<std::uint16_t, 4> const labels{7U, 8U, 9U, 65535U};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 1U},
    "1 1 1",
    labels,
    "# Deliberately unsorted mapping intervals.\n\n"
    "65535 65536 Water Wireframe blue_pure\n"
    "  9 9 Air None white_white  \n"
    "7 9 Water Solid red_ruby\n",
  };
  auto const volume = files.Import();
  for (auto const label : {7U, 8U}) {
    auto const &style =
      volume.GetDisplayStyle(static_cast<std::uint16_t>(label));
    EXPECT_EQ(style.mode, Mode::Solid);
    EXPECT_EQ(style.color, ggems::render::RED_Ruby);
  }
  EXPECT_EQ(volume.GetDisplayStyle(9U).mode, Mode::None);
  EXPECT_EQ(volume.GetDisplayStyle(9U).color, ggems::render::WHITE_White);
  EXPECT_EQ(volume.GetDisplayStyle(65535U).mode, Mode::Wireframe);
  EXPECT_EQ(volume.GetDisplayStyle(65535U).color, ggems::render::BLUE_Pure);
  auto const mappings = volume.GetDisplayMappings();
  ASSERT_EQ(mappings.size(), 3U);
  EXPECT_EQ(mappings[0].first_label, 7U);
  EXPECT_EQ(mappings[0].end_label, 9U);
  EXPECT_EQ(mappings[0].material_name, "Water");
  EXPECT_EQ(mappings[1].first_label, 9U);
  EXPECT_EQ(mappings[1].end_label, 10U);
  EXPECT_EQ(mappings[1].material_name, "Air");
  EXPECT_EQ(mappings[2].first_label, 65535U);
  EXPECT_EQ(mappings[2].end_label, 65536U);
  EXPECT_EQ(mappings[2].material_name, "Water");
  EXPECT_EQ(volume.GetDisplayMappingIndex(7U), 0U);
  EXPECT_EQ(volume.GetDisplayMappingIndex(8U), 0U);
  EXPECT_EQ(volume.GetDisplayMappingIndex(9U), 1U);
  EXPECT_EQ(volume.GetDisplayMappingIndex(65535U), 2U);
  EXPECT_TRUE(std::ranges::equal(volume.GetLabels(), labels));
  ASSERT_EQ(volume.GetMaterials().size(), 2U);
  EXPECT_EQ(volume.GetMaterials()[0].GetColor(), water_color);
  auto &manager = ggems::core::materials::GGEMSMaterialManager::GetInstance();
  EXPECT_EQ(manager.Require(manager.GetOrAdd("Water")).GetColor(), water_color);

  GGEMSVoxelizedTestFixture const physical_only{
    {2U, 2U, 1U},
    "1 1 1",
    labels,
    "65535 65536 Water\n9 9 Air\n7 9 Water\n",
  };
  auto const reference = physical_only.Import();
  EXPECT_TRUE(std::ranges::equal(volume.GetMaterialLabels(),
                                 reference.GetMaterialLabels()));
  ASSERT_EQ(reference.GetMaterials().size(), volume.GetMaterials().size());
  for (std::size_t index = 0U; index < volume.GetMaterials().size(); ++index) {
    EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
      volume.GetMaterials()[index], reference.GetMaterials()[index]));
  }
}

TEST(GGEMSVoxelizedVolume,
     NamedMaterialDefaultsSurviveScientificDeduplication) {
  using ggems::core::materials::GGEMSMaterial;
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  auto const density = BuildBuiltInMaterial("Carbon").GetDensity();
  GGEMSMaterial red{
    "GGEMSVoxelRedCarbon",
    density,
    {{.atomic_number = 6U, .mass_fraction = 1.0L}},
  };
  GGEMSMaterial blue{
    "GGEMSVoxelBlueCarbon",
    density,
    {{.atomic_number = 6U, .mass_fraction = 1.0L}},
  };
  red.SetColor(ggems::render::RED_Ruby);
  blue.SetColor(ggems::render::BLUE_Pure);
  ASSERT_TRUE(ggems::core::materials::HasSameScientificIdentity(red, blue));
  auto &manager = ggems::core::materials::GGEMSMaterialManager::GetInstance();
  manager.AddCustomMaterial(red);
  manager.AddCustomMaterial(blue);
  std::array<std::uint16_t, 4> const labels{10U, 20U, 10U, 20U};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 1U},
    "1 1 1",
    labels,
    "10 10 GGEMSVoxelRedCarbon\n20 20 GGEMSVoxelBlueCarbon\n",
  };
  auto const volume = files.Import();
  ASSERT_EQ(volume.GetMaterials().size(), 1U);
  auto const mappings = volume.GetDisplayMappings();
  ASSERT_EQ(mappings.size(), 2U);
  EXPECT_EQ(mappings[0].material_name, "GGEMSVoxelRedCarbon");
  EXPECT_EQ(mappings[1].material_name, "GGEMSVoxelBlueCarbon");
  EXPECT_NE(volume.GetDisplayMappingIndex(10U),
            volume.GetDisplayMappingIndex(20U));
  for (auto const material_label : volume.GetMaterialLabels()) {
    EXPECT_EQ(material_label, 0U);
  }
  EXPECT_EQ(volume.GetDisplayStyle(10U).mode, Mode::Solid);
  EXPECT_EQ(volume.GetDisplayStyle(20U).mode, Mode::Solid);
  EXPECT_EQ(volume.GetDisplayStyle(10U).color, red.GetColor());
  EXPECT_EQ(volume.GetDisplayStyle(20U).color, blue.GetColor());
}

TEST(GGEMSVoxelizedVolume, VerboseReportsMappingsAndPoseWithoutChangingData) {
  std::array<std::uint16_t, 4> const labels{7U, 8U, 9U, 65535U};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 1U},
    "1 2 3",
    labels,
    "65535 65536 Water Wireframe blue_pure\n"
    "9 9 Air None white_white\n"
    "7 9 Water Solid red_ruby\n",
  };
  auto volume = files.Import("XCAT##%s");
  volume.SetPosition(MakePositionPM(3'000'000'000LL, -2'000'000'000LL, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto const original = volume;
  auto const report_path = files.GetRawPath().parent_path() / "verbose.log";
  ggems::test::ScopedLoggerEncoding const encoding{
    ggems::core::Encoding::Ascii};
  auto &logger = ggems::core::GGEMSLogger::GetInstance();
  logger.ClearSinks();
  logger.SetDetailLevel(0);
  logger.AddSink(std::make_unique<ggems::core::FileSink>(report_path.string()));
  logger.AddSink(std::make_unique<ggems::core::StdoutSink>());
  EXPECT_NO_THROW(EXPECT_EQ(&volume.Verbose(), &volume));
  logger.ClearSinks();
  logger.SetDetailLevel(1);

  std::ifstream input{report_path};
  std::string const report{std::istreambuf_iterator<char>{input},
                           std::istreambuf_iterator<char>{}};
  for (auto const *field : {
         "Voxelized volume: XCAT##%s",
         "Dimensions (X, Y, Z): 2 x 2 x 1",
         "Spacing (X, Y, Z): 1.000 mm, 2.000 mm, 3.000 mm",
         "Full size (X, Y, Z): 2.000 mm, 4.000 mm, 3.000 mm",
         "Voxel count: 4",
         "World center (X, Y, Z): 3.000 mm, -2.000 mm, 0.000 pm",
         "X: (0, 1, -0)",
         "Y: (-1, 0, 0)",
         "Z: (0, 0, 1)",
         "[7, 9): Water; mode=Solid; color=red_ruby",
         "[9, 10): Air; mode=None; color=white_white",
         "[65535, 65536): Water; mode=Wireframe; color=blue_pure",
         "Local Material palette:",
         "[0] Water; density=1.000000 g/cm3",
         "[1] Air; density=",
       }) {
    EXPECT_TRUE(report.contains(field)) << field;
  }
  EXPECT_EQ(
    report.find("Voxelized volume:", report.find("Voxelized volume:") + 1U),
    std::string::npos);
  EXPECT_EQ(volume.GetPositionPM(), original.GetPositionPM());
  EXPECT_EQ(volume.GetRotation().axis_x.y, 1.0);
  EXPECT_EQ(volume.GetRotation().axis_y.x, -1.0);
  EXPECT_EQ(volume.GetLabels().data(), original.GetLabels().data());
  EXPECT_EQ(volume.GetMaterialLabels().data(),
            original.GetMaterialLabels().data());
  EXPECT_EQ(volume.GetMaterials().data(), original.GetMaterials().data());
  EXPECT_EQ(volume.GetDisplayMappings().data(),
            original.GetDisplayMappings().data());
  EXPECT_TRUE(std::ranges::equal(volume.GetLabels(), labels));
  std::array<std::uint16_t, 4> const expected{0U, 0U, 1U, 0U};
  EXPECT_TRUE(std::ranges::equal(volume.GetMaterialLabels(), expected));
}

TEST(GGEMSVoxelizedVolume, RejectsInvalidPresentationArityModeAndColor) {
  std::array<std::uint16_t, 1> const labels{0U};
  for (auto const *mapping : {
         "0 Air\n",
         "0 0 Air None\n",
         "0 0 Air Solid white_white extra\n",
         "0 0 Air N white_white\n",
         "0 0 Air solid white_white\n",
         "0 0 Air Solid missing_color\n",
         "0 0 Air None missing_color\n",
         "0 0 Air Solid WHITE_White\n",
       }) {
    SCOPED_TRACE(mapping);
    GGEMSVoxelizedTestFixture const files{
      {1U, 1U, 1U}, "1 1 1", labels, mapping};
    EXPECT_THROW(static_cast<void>(files.Import()), GGEMSRecoverable);
  }
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
    return files.Import("XCAT");
  }();
  auto const original = volume;
  EXPECT_EQ(original.GetName(), "XCAT");
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
  EXPECT_EQ(volume.GetName(), "XCAT");
  for (auto const label : original.GetLabels()) {
    EXPECT_EQ(original.GetDisplayStyle(label).mode,
              volume.GetDisplayStyle(label).mode);
    EXPECT_EQ(original.GetDisplayStyle(label).color,
              volume.GetDisplayStyle(label).color);
  }
}

TEST(GGEMSVoxelizedVolume, PublishesOnePhysicalIDPerGridInMixedInsertionOrder) {
  GGEMSVoxelizedTestFixture const files;
  auto first = files.Import("XCAT");
  first.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  auto second = files.Import("XCAT");
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
  EXPECT_EQ(world.GetVoxelizedVolume(0U)->GetName(), "XCAT");
  EXPECT_EQ(world.GetVoxelizedVolume(2U)->GetName(), "XCAT");
  EXPECT_EQ(world.GetAuthoredGroupId(0U), 1U);
  EXPECT_EQ(world.GetAuthoredGroupId(1U), 2U);
  EXPECT_EQ(world.GetAuthoredGroupId(2U), 3U);
  EXPECT_EQ(world.GetAuthoredRecipe(0U), nullptr);
  EXPECT_EQ(world.GetAuthoredRecipe(2U), nullptr);
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

// A straight ray crosses planes, not every cell in the grid's Cartesian
// product.
TEST(GGEMSVoxelizedVolume, BoundaryEventBoundAddsLinearGridContributions) {
  std::vector<std::uint16_t> const labels(std::size_t{3U} * 4U * 5U, 10U);
  GGEMSVoxelizedTestFixture const files{
    {3U, 4U, 5U},
    "1 1 1",
    labels,
    "10 10 Water\n",
  };
  auto world = MakeWorld();
  auto grid = files.Import();
  grid.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  world.AddVolume(grid);
  EXPECT_EQ(world.BuildRecord().max_boundary_events, 1U + (3U + 4U + 5U - 1U));
  EXPECT_LT(world.BuildRecord().max_boundary_events, labels.size());
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  world.AddVolume(box);
  EXPECT_EQ(world.BuildRecord().max_boundary_events, 14U);
  grid.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  world.AddVolume(grid);
  EXPECT_EQ(world.BuildRecord().max_boundary_events, 25U);
}
