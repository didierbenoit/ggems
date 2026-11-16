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
 * \brief Checks analytic display descriptors, counts, bounds, and physical
 * identity.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <type_traits>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using namespace ggems::render;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto World() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(1_m, 1_m, 1_m).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

auto Mesh(GGEMSGeometryScene const &scene, std::size_t occurrence)
  -> GGEMSGeometryMesh const & {
  return scene.meshes[scene.occurrences[occurrence].mesh_index];
}

auto HasInspection(GGEMSGeometryGroup const &group, std::string_view name,
                   std::string_view value) -> bool {
  return std::ranges::any_of(
    group.inspection, [&](auto const &property) -> bool {
      return property.name == name && property.value == value;
    });
}
} // namespace

TEST(GGEMSGeometryScene, AnalyticalModelColorsDefaultToWhiteAndCopyByValue) {
  std::array<GGEMSWorld::Volume, 8> volumes{
    GGEMSBox{},       GGEMSSphere{},    GGEMSTubs{}, GGEMSCons{},
    GGEMSPolyhedra{}, GGEMSEllipsoid{}, GGEMSTrd{},  GGEMSTrap{},
  };

  for (auto &volume : volumes) {
    std::visit(
      [](auto &shape) -> void {
        EXPECT_EQ(shape.GetColor(), WHITE_White);
        EXPECT_FALSE(shape.GetColorOverride());
        EXPECT_EQ(&shape.SetColor(MAGENTA_BlueViolet), &shape);
        EXPECT_EQ(shape.GetColorOverride(), MAGENTA_BlueViolet);

        auto copy = shape;
        EXPECT_EQ(copy.GetColor(), MAGENTA_BlueViolet);
        copy.SetColor(YELLOW_Gold);
        EXPECT_EQ(copy.GetColor(), YELLOW_Gold);
        EXPECT_EQ(shape.GetColor(), MAGENTA_BlueViolet);
      },
      volume);
  }
}

TEST(GGEMSGeometryScene,
     AnalyticColorsResolveMaterialUnlessExplicitlyOverridden) {
  auto material = BuildBuiltInMaterial("Water");
  auto const material_color = material.GetColor();
  ASSERT_NE(material_color, WHITE_White);
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm)
    .SetMaterial(material)
    .SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0));
  auto world = World();
  world.LinearRepeater(box, 2U, 4_mm, GGEMSRepeatAxis::X);
  box.SetColor(WHITE_White).SetPosition(MakePositionPM(0, 0, 0));
  ASSERT_TRUE(box.GetColorOverride());
  world.AddVolume(box);
  box.SetColor(RED_Ruby).SetPosition(MakePositionPM(20'000'000'000LL, 0, 0));
  world.LinearRepeater(box, 2U, 4_mm, GGEMSRepeatAxis::X);
  material.SetColor(GREEN_Pure);
  box.SetColor(BLUE_Pure).SetMaterial(material);
  auto const scene = BuildGeometryScene(world);
  world = World();
  ASSERT_EQ(scene.occurrences.size(), 6U);
  EXPECT_EQ(scene.meshes.size(), 2U);
  EXPECT_EQ(scene.occurrences[0].color, WHITE_White);
  std::array const expected{
    material_color, material_color, WHITE_White, RED_Ruby, RED_Ruby,
  };
  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(scene.occurrences[index + 1U].color, expected[index]);
    EXPECT_EQ(scene.occurrences[index + 1U].material_name, "Water");
    EXPECT_EQ(scene.occurrences[index + 1U].mesh_index,
              scene.occurrences[1].mesh_index);
  }
}

TEST(GGEMSGeometryScene, OwnsWorldAndSharedMeshesWithPhysicalHierarchy) {
  auto world = World();
  GGEMSBox mother;
  mother.SetSize(200_mm, 200_mm, 200_mm)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetColor(GRAY_Silver)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(mother);
  mother.SetColor(BLUE_Pure);

  GGEMSBox child;
  child.SetSize(10_mm, 20_mm, 30_mm)
    .SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetColor(MAGENTA_BlueViolet)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(child, 1U);
  child.SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0))
    .SetColor(YELLOW_Gold)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(child, 1U);
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  world = World();

  ASSERT_EQ(scene.occurrences.size(), 4U);
  EXPECT_EQ(scene.world_half_extent_m, (std::array{0.5, 0.5, 0.5}));
  EXPECT_EQ(scene.occurrences[0].volume_id, 0U);
  EXPECT_EQ(Mesh(scene, 0).shape.parameters[0],
            (std::array{0.5F, 0.5F, 0.5F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 0).vertex_count, 36U);
  EXPECT_TRUE(scene.profiles.empty());
  EXPECT_EQ(scene.meshes.size(), 3U);
  EXPECT_EQ(scene.occurrences[2].volume_id, 2U);
  EXPECT_EQ(scene.occurrences[3].volume_id, 3U);
  EXPECT_EQ(scene.occurrences[2].parent_volume_id, 1U);
  EXPECT_EQ(scene.occurrences[3].parent_volume_id, 1U);
  EXPECT_EQ(scene.occurrences[2].mesh_index, scene.occurrences[3].mesh_index);
  EXPECT_EQ(scene.occurrences[0].color, WHITE_White);
  EXPECT_EQ(scene.occurrences[1].color, GRAY_Silver);
  EXPECT_EQ(scene.occurrences[2].color, MAGENTA_BlueViolet);
  EXPECT_EQ(scene.occurrences[3].color, YELLOW_Gold);
  auto const &transform = scene.occurrences[2].local_to_world;
  EXPECT_FLOAT_EQ(transform[3], 0.1F);
  EXPECT_FLOAT_EQ(transform[7], 0.02F);
  EXPECT_FLOAT_EQ(transform[0], 0.0F);
  EXPECT_FLOAT_EQ(transform[1], -1.0F);
  EXPECT_FLOAT_EQ(transform[4], 1.0F);
  EXPECT_FLOAT_EQ(scene.occurrences[3].local_to_world[7], -0.02F);
  EXPECT_EQ(Mesh(scene, 2).shape.parameters[0],
            (std::array{0.005F, 0.01F, 0.015F, 0.0F}));

  auto const &root = scene.occurrences[0];
  EXPECT_EQ(root.shape_name, "GGEMSWorld");
  EXPECT_EQ(root.material_name, "Vacuum");
  EXPECT_EQ(root.material_density, BuildBuiltInMaterial("Vacuum").GetDensity());
  EXPECT_EQ(root.children, (std::vector<std::uint32_t>{1U}));
  EXPECT_EQ(scene.occurrences[1].children,
            (std::vector<std::uint32_t>{2U, 3U}));
  auto const &selected = scene.occurrences[2];
  EXPECT_TRUE(selected.children.empty());
  EXPECT_EQ(selected.shape_name, "Box");
  EXPECT_EQ(selected.material_name, "Aluminum");
  EXPECT_EQ(selected.material_density,
            BuildBuiltInMaterial("Aluminum").GetDensity());
  EXPECT_EQ(scene.occurrences[3].material_name, "Water");
  EXPECT_EQ(scene.occurrences[3].material_density,
            BuildBuiltInMaterial("Water").GetDensity());
  ASSERT_EQ(selected.dimensions.size(), 3U);
  EXPECT_EQ(selected.dimensions[0].name, "Size X");
  EXPECT_EQ(selected.dimensions[0].value, HumanReadable(10_mm));
  EXPECT_EQ(selected.dimensions[1].value, HumanReadable(20_mm));
  EXPECT_EQ(selected.dimensions[2].value, HumanReadable(30_mm));
  EXPECT_DOUBLE_EQ(selected.position_m[0], 0.1);
  EXPECT_DOUBLE_EQ(selected.position_m[1], 0.02);
  EXPECT_DOUBLE_EQ(selected.orientation[1], -1.0);
  EXPECT_DOUBLE_EQ(selected.orientation[3], 1.0);
  std::array const expected_min{0.09, 0.015, -0.015};
  std::array const expected_max{0.11, 0.025, 0.015};
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    EXPECT_NEAR(selected.minimum_m[axis], expected_min[axis], 1.0e-8);
    EXPECT_NEAR(selected.maximum_m[axis], expected_max[axis], 1.0e-8);
  }
}

TEST(GGEMSGeometryScene, PreservesAllFamiliesBoresApexAndPolygonApothems) {
  GGEMSBox box;
  box.SetSize(4_mm, 6_mm, 8_mm);
  GGEMSSphere sphere;
  sphere.SetRadius(3_mm);
  GGEMSTubs tubs;
  tubs.SetRadii(5_mm, 2_mm).SetLength(8_mm);
  GGEMSCons cons;
  cons.SetNegativeZRadii(2_mm, 6_mm)
    .SetPositiveZRadii(0_pm, 3_mm)
    .SetLength(8_mm);
  GGEMSPolyhedra polygon;
  std::array const z{
    PositionCoordinate{-4'000'000'000LL},
    PositionCoordinate{0},
    PositionCoordinate{5'000'000'000LL},
  };
  std::array const inner{2_mm, 1_mm, 0_pm};
  std::array const outer{6_mm, 8_mm, 5_mm};
  polygon.SetSideCount(6U).SetZProfile(z, inner, outer);
  GGEMSEllipsoid ellipsoid;
  ellipsoid.SetSemiAxes(3_mm, 4_mm, 5_mm);
  GGEMSTrd trd;
  trd.SetNegativeZFace(4_mm, 6_mm)
    .SetPositiveZFace(8_mm, 10_mm)
    .SetLength(12_mm);
  GGEMSTrap trap;
  trap.SetNegativeZFace(8_mm, 4_mm, 6_mm, 10_deg)
    .SetPositiveZFace(8_mm, 4_mm, 6_mm, 10_deg)
    .SetInclination(10_deg, 30_deg)
    .SetLength(12_mm);
  GGEMSCons cone;
  cone.SetNegativeZRadii(0_pm, 5_mm)
    .SetPositiveZRadii(0_pm, 0_pm)
    .SetLength(8_mm);
  std::vector<GGEMSWorld::Volume> shapes{
    box, sphere, tubs, cons, polygon, ellipsoid, trd, trap, cone,
  };
  auto world = World();
  for (std::size_t index = 0; index < shapes.size(); ++index) {
    std::visit(
      [&](auto shape) -> void {
        shape.SetColor(MAGENTA_BlueViolet)
          .SetMaterial(BuildBuiltInMaterial("Water"))
          .SetPosition(MakePositionPM(
            (static_cast<CoordinatePM>(index) - 4) * 50'000'000'000LL, 0, 0));
        world.AddVolume(shape);
      },
      shapes[index]);
  }
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.occurrences.size(), 10U);
  std::array const kinds{
    GGEMSGeometryKind::Box,       GGEMSGeometryKind::Sphere,
    GGEMSGeometryKind::Tubs,      GGEMSGeometryKind::Cons,
    GGEMSGeometryKind::Polyhedra, GGEMSGeometryKind::Ellipsoid,
    GGEMSGeometryKind::Trd,       GGEMSGeometryKind::Trap,
    GGEMSGeometryKind::Cons,
  };
  std::array<std::uint32_t, 9> const counts{
    36U, 11904U, 1536U, 1152U, 180U, 11904U, 36U, 36U, 384U,
  };
  for (std::size_t index = 0; index < kinds.size(); ++index) {
    SCOPED_TRACE(index);
    auto const &mesh = Mesh(scene, index + 1U);
    EXPECT_EQ(mesh.shape.header[0], static_cast<std::uint32_t>(kinds[index]));
    EXPECT_EQ(mesh.vertex_count, counts[index]);
    EXPECT_EQ(mesh.vertex_count % 3U, 0U);
    for (auto const &parameters : mesh.shape.parameters) {
      EXPECT_TRUE(std::ranges::all_of(
        parameters, [](float value) -> bool { return std::isfinite(value); }));
    }
  }
  EXPECT_EQ(Mesh(scene, 1).shape.parameters[0],
            (std::array{0.002F, 0.003F, 0.004F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 2).shape.parameters[0],
            (std::array{0.003F, 0.003F, 0.003F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 3).shape.parameters[0],
            (std::array{0.005F, 0.005F, 0.002F, 0.002F}));
  EXPECT_FLOAT_EQ(Mesh(scene, 3).shape.parameters[1][0], 0.004F);
  EXPECT_EQ(Mesh(scene, 4).shape.parameters[0],
            (std::array{0.006F, 0.003F, 0.002F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 5).shape.header,
            (std::array<std::uint32_t, 4>{4U, 6U, 0U, 3U}));
  ASSERT_EQ(scene.profiles.size(), 3U);
  EXPECT_FLOAT_EQ(scene.profiles[0].z_m, -0.004F);
  EXPECT_FLOAT_EQ(scene.profiles[0].inner_apothem_m, 0.002F);
  EXPECT_FLOAT_EQ(scene.profiles[0].outer_apothem_m, 0.006F);
  EXPECT_EQ(scene.profiles[0].first_triangle, 0U);
  EXPECT_EQ(scene.profiles[1].first_triangle, 4U);
  EXPECT_EQ(scene.profiles[2].first_triangle, 7U);
  EXPECT_FLOAT_EQ(scene.profiles[2].z_m, 0.005F);
  EXPECT_FLOAT_EQ(scene.profiles[2].inner_apothem_m, 0.0F);
  EXPECT_FLOAT_EQ(scene.profiles[2].outer_apothem_m, 0.005F);
  EXPECT_EQ(Mesh(scene, 6).shape.parameters[0],
            (std::array{0.003F, 0.004F, 0.005F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 7).shape.parameters[0],
            (std::array{0.003F, 0.002F, 0.002F, 0.006F}));
  EXPECT_EQ(Mesh(scene, 7).shape.parameters[1],
            (std::array{0.005F, 0.004F, 0.004F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 7).shape.parameters[2], (std::array<float, 4>{}));
  EXPECT_EQ(Mesh(scene, 8).shape.parameters[0],
            (std::array{0.004F, 0.002F, 0.003F, 0.006F}));
  EXPECT_NEAR(Mesh(scene, 8).shape.parameters[2][0], std::numbers::pi / 18.0,
              1.0e-8);
  EXPECT_NEAR(Mesh(scene, 8).shape.parameters[2][1], std::numbers::pi / 18.0,
              1.0e-8);
  EXPECT_NEAR(Mesh(scene, 8).shape.parameters[2][2], std::numbers::pi / 18.0,
              1.0e-8);
  EXPECT_NEAR(Mesh(scene, 8).shape.parameters[2][3], std::numbers::pi / 6.0,
              2.0e-8);
  EXPECT_EQ(Mesh(scene, 9).shape.parameters[0],
            (std::array{0.005F, 0.0F, 0.0F, 0.0F}));

  std::array<std::string_view, 9U> const names{
    "Box",       "Sphere", "Tubs", "Cons", "Polyhedra",
    "Ellipsoid", "Trd",    "Trap", "Cons",
  };
  std::array<std::size_t, 9U> const parameter_counts{
    3U, 1U, 3U, 5U, 11U, 3U, 5U, 11U, 5U,
  };
  for (std::size_t index = 0; index < names.size(); ++index) {
    SCOPED_TRACE(names[index]);
    auto const &occurrence = scene.occurrences[index + 1U];
    EXPECT_EQ(occurrence.shape_name, names[index]);
    EXPECT_EQ(occurrence.color, MAGENTA_BlueViolet);
    EXPECT_EQ(occurrence.dimensions.size(), parameter_counts[index]);
    EXPECT_TRUE(std::ranges::all_of(
      occurrence.dimensions, [](auto const &parameter) -> bool {
        return !parameter.name.empty() && !parameter.value.empty();
      }));
    for (std::size_t axis = 0; axis < 3U; ++axis) {
      EXPECT_TRUE(std::isfinite(occurrence.minimum_m[axis]));
      EXPECT_TRUE(std::isfinite(occurrence.maximum_m[axis]));
      EXPECT_LT(occurrence.minimum_m[axis], occurrence.maximum_m[axis]);
    }
  }
  EXPECT_EQ(scene.occurrences[3].dimensions[1].value, HumanReadable(2_mm));
  EXPECT_EQ(scene.occurrences[4].dimensions[0].value, HumanReadable(6_mm));
  EXPECT_EQ(scene.occurrences[5].dimensions[0].value, "6");
  EXPECT_EQ(scene.occurrences[5].dimensions[2].value,
            HumanReadable(PositionCoordinate{-4'000'000'000LL}));
  EXPECT_EQ(scene.occurrences[5].dimensions[4].name, "Plane 0 outer apothem");
  EXPECT_EQ(scene.occurrences[5].dimensions[4].value, HumanReadable(6_mm));
  EXPECT_EQ(scene.occurrences[6].dimensions[2].value, HumanReadable(5_mm));
  EXPECT_EQ(scene.occurrences[7].dimensions[3].value, HumanReadable(8_mm));
  EXPECT_EQ(scene.occurrences[8].dimensions[4].value, HumanReadable(10_deg));
  EXPECT_EQ(scene.occurrences[8].dimensions[10].value, HumanReadable(30_deg));
}

TEST(GGEMSGeometryScene, WorldOnlyHasRealMetadataAndIndependentBounds) {
  auto world = World();
  world.SetSize(2_m, 4_m, 6_m).SetMaterial(BuildBuiltInMaterial("Air"));
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.occurrences.size(), 1U);
  auto const &root = scene.occurrences.front();
  EXPECT_TRUE(root.children.empty());
  EXPECT_EQ(root.material_name, "Air");
  EXPECT_EQ(root.material_density, BuildBuiltInMaterial("Air").GetDensity());
  EXPECT_EQ(root.minimum_m, (std::array{-1.0, -2.0, -3.0}));
  EXPECT_EQ(root.maximum_m, (std::array{1.0, 2.0, 3.0}));
  EXPECT_EQ(root.position_m, (std::array{0.0, 0.0, 0.0}));
  EXPECT_EQ(root.orientation,
            (std::array{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}));
  ASSERT_EQ(root.dimensions.size(), 3U);
  EXPECT_EQ(root.dimensions[0].value, HumanReadable(2_m));
  EXPECT_EQ(root.dimensions[1].value, HumanReadable(4_m));
  EXPECT_EQ(root.dimensions[2].value, HumanReadable(6_m));
}

TEST(GGEMSGeometryScene,
     RepeatedSubtreesKeepOccurrenceIdentityAndComposedBounds) {
  GGEMSBox crystal;
  crystal.SetSize(2_mm, 4_mm, 6_mm)
    .SetColor(MAGENTA_BlueViolet)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  GGEMSBox module;
  module.SetSize(20_mm, 20_mm, 20_mm)
    .SetColor(GRAY_Silver)
    .SetMaterial(BuildBuiltInMaterial("Water"))
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0));
  module.LinearRepeater(crystal, 2U, 8_mm, GGEMSRepeatAxis::X);
  crystal.SetColor(YELLOW_Gold);

  auto world = World();
  world.CircularRepeater(module, 4U, GGEMSRepeatAxis::Z);
  module.SetColor(BLUE_Pure);

  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.occurrences.size(), 13U);
  EXPECT_EQ(scene.occurrences[0].children,
            (std::vector<std::uint32_t>{1U, 4U, 7U, 10U}));
  EXPECT_EQ(scene.occurrences[0].repeater_group_id, 0U);
  auto const outer_group = scene.occurrences[1].repeater_group_id;
  EXPECT_NE(outer_group, 0U);
  for (std::uint32_t parent : scene.occurrences[0].children) {
    auto const &mother = scene.occurrences[parent];
    EXPECT_EQ(mother.repeater_group_id, outer_group);
    auto const inner_group = scene.occurrences[parent + 1U].repeater_group_id;
    EXPECT_NE(inner_group, 0U);
    EXPECT_NE(inner_group, outer_group);
    for (auto const other_parent : scene.occurrences[0].children) {
      if (other_parent != parent) {
        EXPECT_NE(inner_group,
                  scene.occurrences[other_parent + 1U].repeater_group_id);
      }
    }
    EXPECT_EQ(mother.color, GRAY_Silver);
    EXPECT_EQ(mother.children,
              (std::vector<std::uint32_t>{parent + 1U, parent + 2U}));
    for (auto child : mother.children) {
      auto const &occurrence = scene.occurrences[child];
      EXPECT_EQ(occurrence.repeater_group_id, inner_group);
      EXPECT_EQ(occurrence.volume_id, child);
      EXPECT_EQ(occurrence.parent_volume_id, parent);
      EXPECT_EQ(occurrence.mesh_index, scene.occurrences[2].mesh_index);
      EXPECT_TRUE(occurrence.children.empty());
      EXPECT_EQ(occurrence.material_name, "Aluminum");
      EXPECT_EQ(occurrence.color, MAGENTA_BlueViolet);
      EXPECT_LT(occurrence.maximum_m[0] - occurrence.minimum_m[0], 0.005);
      EXPECT_LT(occurrence.maximum_m[1] - occurrence.minimum_m[1], 0.005);
    }
  }
  auto const &rotated = scene.occurrences[5];
  EXPECT_NEAR(rotated.position_m[0], 0.0, 1.0e-15);
  EXPECT_NEAR(rotated.position_m[1], 0.096, 1.0e-15);
  EXPECT_NEAR(rotated.minimum_m[0], -0.002, 1.0e-8);
  EXPECT_NEAR(rotated.maximum_m[0], 0.002, 1.0e-8);
  EXPECT_NEAR(rotated.minimum_m[1], 0.095, 1.0e-8);
  EXPECT_NEAR(rotated.maximum_m[1], 0.097, 1.0e-8);

  auto const captured = world;
  module.SetPosition(MakePositionPM(0, 0, 200'000'000'000LL));
  world.AttachVolume(module);
  auto const retained = BuildGeometryScene(captured);
  auto const extended = BuildGeometryScene(world);
  for (std::size_t index = 0U; index < scene.occurrences.size(); ++index) {
    EXPECT_EQ(retained.occurrences[index].repeater_group_id,
              scene.occurrences[index].repeater_group_id);
    EXPECT_EQ(extended.occurrences[index].repeater_group_id,
              scene.occurrences[index].repeater_group_id);
  }
  EXPECT_EQ(extended.occurrences[13].repeater_group_id, 0U);
  EXPECT_NE(extended.occurrences[14].repeater_group_id,
            scene.occurrences[2].repeater_group_id);
}

TEST(GGEMSGeometryScene,
     AuthoredRepeaterIdentitySurvivesSnapshotWithDistinctPhysicalGroups) {
  GGEMSBox crystal;
  crystal.SetSize(2_mm, 2_mm, 2_mm)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  GGEMSBox module;
  module.SetSize(20_mm, 20_mm, 20_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"))
    .SetRotation(0_deg, 0_deg, 90_deg);
  module.LinearRepeater(crystal, 3U, 6_mm, GGEMSRepeatAxis::X);

  auto world = World();
  world.LinearRepeater(module, 4U, 100_mm, GGEMSRepeatAxis::X);
  crystal.SetPosition(MakePositionPM(300'000'000'000LL, 0, 0));
  world.AddVolume(crystal);
  ggems::test::GGEMSVoxelizedTestFixture const fixture;
  auto voxelized = fixture.Import();
  voxelized.SetPosition(MakePositionPM(-300'000'000'000LL, 0, 0));
  world.AddVolume(voxelized);
  ASSERT_NO_THROW(world.Validate());

  auto const scene = BuildGeometryScene(world);
  world = World();
  ASSERT_EQ(scene.occurrences.size(), 19U);
  EXPECT_EQ(scene.occurrences[0].children,
            (std::vector<std::uint32_t>{1U, 5U, 9U, 13U, 17U, 18U}));
  auto const outer_recipe = scene.occurrences[1].repeater_recipe_id;
  auto const inner_recipe = scene.occurrences[2].repeater_recipe_id;
  auto const outer_group = scene.occurrences[1].repeater_group_id;
  EXPECT_NE(outer_recipe, 0U);
  EXPECT_NE(inner_recipe, 0U);
  EXPECT_NE(inner_recipe, outer_recipe);
  EXPECT_NE(outer_group, 0U);

  std::array const parents{1U, 5U, 9U, 13U};
  std::array const positions_x{-0.15, -0.05, 0.05, 0.15};
  std::array const positions_y{-0.006, 0.0, 0.006};
  std::vector<std::uint32_t> inner_groups;
  for (std::size_t copy = 0U; copy < parents.size(); ++copy) {
    auto const parent = parents[copy];
    auto const &mother = scene.occurrences[parent];
    EXPECT_EQ(mother.repeater_recipe_id, outer_recipe);
    EXPECT_EQ(mother.repeater_group_id, outer_group);
    EXPECT_EQ(mother.parent_volume_id, 0U);
    EXPECT_EQ(mother.children, (std::vector<std::uint32_t>{
                                 parent + 1U, parent + 2U, parent + 3U}));
    EXPECT_NEAR(mother.position_m[0], positions_x[copy], 1.0e-15);
    auto const local_group = scene.occurrences[parent + 1U].repeater_group_id;
    EXPECT_NE(local_group, 0U);
    EXPECT_NE(local_group, outer_group);
    EXPECT_EQ(std::ranges::find(inner_groups, local_group), inner_groups.end());
    inner_groups.push_back(local_group);

    for (std::size_t child = 0U; child < positions_y.size(); ++child) {
      auto const &occurrence = scene.occurrences[parent + 1U + child];
      EXPECT_EQ(occurrence.repeater_recipe_id, inner_recipe);
      EXPECT_EQ(occurrence.repeater_group_id, local_group);
      EXPECT_EQ(occurrence.parent_volume_id, parent);
      EXPECT_TRUE(occurrence.children.empty());
      EXPECT_NEAR(occurrence.position_m[0], positions_x[copy], 1.0e-15);
      EXPECT_NEAR(occurrence.position_m[1], positions_y[child], 1.0e-15);
      EXPECT_DOUBLE_EQ(occurrence.orientation[1], -1.0);
      EXPECT_DOUBLE_EQ(occurrence.orientation[3], 1.0);
      EXPECT_FLOAT_EQ(occurrence.local_to_world[3],
                      static_cast<float>(positions_x[copy]));
      EXPECT_FLOAT_EQ(occurrence.local_to_world[7],
                      static_cast<float>(positions_y[child]));
    }
  }
  for (std::uint32_t id = 0U; id < scene.occurrences.size(); ++id) {
    EXPECT_EQ(scene.occurrences[id].volume_id, id);
  }
  for (auto const ordinary : {0U, 17U, 18U}) {
    EXPECT_EQ(scene.occurrences[ordinary].repeater_group_id, 0U);
    EXPECT_EQ(scene.occurrences[ordinary].repeater_recipe_id, 0U);
  }
  ASSERT_TRUE(scene.occurrences[18].voxel_slices);
}

TEST(GGEMSGeometryScene, DescriptorAndProfileMatchScalarShaderABI) {
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSGeometryShape>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSGeometryShape>);
  EXPECT_EQ(sizeof(GGEMSGeometryShape), 64U);
  EXPECT_EQ(offsetof(GGEMSGeometryShape, header), 0U);
  EXPECT_EQ(offsetof(GGEMSGeometryShape, parameters), 16U);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSGeometryProfile>);
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSGeometryProfile>);
  EXPECT_EQ(sizeof(GGEMSGeometryProfile), 16U);
  EXPECT_EQ(offsetof(GGEMSGeometryProfile, z_m), 0U);
  EXPECT_EQ(offsetof(GGEMSGeometryProfile, inner_apothem_m), 4U);
  EXPECT_EQ(offsetof(GGEMSGeometryProfile, outer_apothem_m), 8U);
  EXPECT_EQ(offsetof(GGEMSGeometryProfile, first_triangle), 12U);
  std::array const kinds{
    GGEMSGeometryKind::Box,       GGEMSGeometryKind::Sphere,
    GGEMSGeometryKind::Tubs,      GGEMSGeometryKind::Cons,
    GGEMSGeometryKind::Polyhedra, GGEMSGeometryKind::Ellipsoid,
    GGEMSGeometryKind::Trd,       GGEMSGeometryKind::Trap,
  };
  for (std::size_t index = 0; index < kinds.size(); ++index) {
    EXPECT_EQ(static_cast<std::uint32_t>(kinds[index]), index);
  }
}

TEST(GGEMSGeometryScene, SharesProfilesAndCountsOpeningAndClosingBores) {
  GGEMSPolyhedra polygon;
  std::array const z{
    PositionCoordinate{-4'000'000'000LL},
    PositionCoordinate{-2'000'000'000LL},
    PositionCoordinate{0},
    PositionCoordinate{2'000'000'000LL},
    PositionCoordinate{4'000'000'000LL},
  };
  std::array const inner{0_pm, 2_mm, 0_pm, 0_pm, 1_mm};
  std::array const outer{4_mm, 5_mm, 4_mm, 3_mm, 4_mm};
  polygon.SetSideCount(7U)
    .SetStartAngle(17_deg)
    .SetZProfile(z, inner, outer)
    .SetMaterial(BuildBuiltInMaterial("Water"))
    .SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0));
  auto world = World();
  world.AddVolume(polygon);
  polygon.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(polygon);
  polygon.SetStartAngle(18_deg).SetPosition(
    MakePositionPM(50'000'000'000LL, 0, 0));
  world.AddVolume(polygon);
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.meshes.size(), 3U);
  ASSERT_EQ(scene.profiles.size(), 10U);
  EXPECT_EQ(scene.occurrences[1].mesh_index, scene.occurrences[2].mesh_index);
  EXPECT_NE(scene.occurrences[1].mesh_index, scene.occurrences[3].mesh_index);
  EXPECT_EQ(Mesh(scene, 1).vertex_count, 294U);
  EXPECT_EQ(Mesh(scene, 1).shape.header[2], 0U);
  EXPECT_EQ(Mesh(scene, 3).shape.header[2], 5U);
  EXPECT_EQ(Mesh(scene, 3).shape.header[3], 5U);
  EXPECT_NEAR(Mesh(scene, 1).shape.parameters[0][0],
              17.0 * std::numbers::pi / 180.0, 2.0e-8);
  std::array<std::uint32_t, 5> const prefix{0U, 3U, 6U, 8U, 11U};
  for (std::size_t index = 0; index < prefix.size(); ++index) {
    EXPECT_EQ(scene.profiles[index].first_triangle, prefix[index]);
    EXPECT_EQ(scene.profiles[index + 5U].first_triangle, prefix[index]);
  }
}

TEST(GGEMSGeometryScene, BoundsUseCanonicalAnalyticSupportWithoutTessellation) {
  GGEMSEllipsoid ellipsoid;
  ellipsoid.SetSemiAxes(3_mm, 4_mm, 5_mm)
    .SetRotation(0_deg, 0_deg, 45_deg)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  auto world = World();
  world.AddVolume(ellipsoid);
  GGEMSSphere sphere;
  sphere.SetRadius(3_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(sphere);
  GGEMSPolyhedra polygon;
  std::array const z{
    PositionCoordinate{-2'000'000'000LL},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const inner{1_mm, 0_pm};
  std::array const outer{4_mm, 5_mm};
  polygon.SetSideCount(3U)
    .SetZProfile(z, inner, outer)
    .SetRotation(0_deg, 90_deg, 0_deg)
    .SetPosition(MakePositionPM(-100'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(polygon);
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  auto const &rotated = scene.occurrences[1];
  auto const c = static_cast<double>(rotated.local_to_world[0]);
  auto const s = static_cast<double>(rotated.local_to_world[4]);
  auto const half_x =
    std::sqrt((0.003 * c * 0.003 * c) + (0.004 * s * 0.004 * s));
  auto const half_y =
    std::sqrt((0.003 * s * 0.003 * s) + (0.004 * c * 0.004 * c));
  EXPECT_NEAR(rotated.minimum_m[0], rotated.local_to_world[3] - half_x,
              1.0e-15);
  EXPECT_NEAR(rotated.maximum_m[1], half_y, 1.0e-15);
  EXPECT_DOUBLE_EQ(rotated.minimum_m[2], -0.005);
  EXPECT_DOUBLE_EQ(rotated.maximum_m[2], 0.005);
  EXPECT_EQ(scene.occurrences[2].minimum_m,
            (std::array{-0.003, -0.003, -0.003}));
  EXPECT_EQ(scene.occurrences[2].maximum_m, (std::array{0.003, 0.003, 0.003}));
  auto const &asymmetric = scene.occurrences[3];
  EXPECT_NEAR(asymmetric.minimum_m[0], asymmetric.local_to_world[3] - 0.002,
              1.0e-15);
  EXPECT_NEAR(asymmetric.maximum_m[0], asymmetric.local_to_world[3] + 0.006,
              1.0e-15);
  EXPECT_NEAR(asymmetric.minimum_m[1], -0.01, 1.0e-15);
  EXPECT_NEAR(asymmetric.maximum_m[1], 0.01, 1.0e-15);
}

TEST(GGEMSGeometryScene, VoxelSlicesKeepPhysicalIdsMetadataAndAnalyticSharing) {
  std::array<std::uint16_t, 30> labels{};
  labels.fill(10U);
  labels.back() = 20U;
  ggems::test::GGEMSVoxelizedTestFixture const fixture{
    {3U, 2U, 5U},
    "1 2 3",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto voxelized = fixture.Import();
  voxelized
    .SetPosition(
      MakePositionPM(100'000'000'000LL, 20'000'000'000LL, -30'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto world = World();
  GGEMSBox box;
  box.SetSize(10_mm, 10_mm, 10_mm)
    .SetPosition(MakePositionPM(-100'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(box);
  world.AddVolume(voxelized);
  voxelized.SetPosition(MakePositionPM(-200'000'000'000LL, 0, 0));
  world.AddVolume(voxelized);
  box.SetPosition(MakePositionPM(200'000'000'000LL, 0, 0));
  world.AddVolume(box);

  auto const scene = BuildGeometryScene(world);
  world = World();
  ASSERT_EQ(scene.occurrences.size(), 5U);
  EXPECT_EQ(scene.meshes.size(), 4U);
  EXPECT_TRUE(scene.profiles.empty());
  EXPECT_EQ(scene.occurrences[0].children,
            (std::vector<std::uint32_t>{1U, 2U, 3U, 4U}));
  EXPECT_EQ(scene.occurrences[1].mesh_index, scene.occurrences[4].mesh_index);
  for (std::uint32_t id = 0U; id < scene.occurrences.size(); ++id) {
    EXPECT_EQ(scene.occurrences[id].volume_id, id);
    EXPECT_EQ(scene.occurrences[id].parent_volume_id, 0U);
    EXPECT_EQ(scene.occurrences[id].voxel_slices.has_value(),
              id == 2U || id == 3U);
  }

  auto const &occurrence = scene.occurrences[2];
  ASSERT_TRUE(occurrence.voxel_slices);
  auto const &slices = *occurrence.voxel_slices;
  EXPECT_EQ(slices.dimensions, (std::array{3U, 2U, 5U}));
  EXPECT_EQ(slices.instance_counts, (std::array{6U, 15U, 10U}));
  EXPECT_EQ(slices.spacing_m, (std::array{0.001, 0.002, 0.003}));
  EXPECT_EQ(slices.size_m, (std::array{0.003, 0.004, 0.015}));
  EXPECT_EQ(slices.palette_size, 2U);
  EXPECT_EQ(occurrence.shape_name, "VoxelizedVolume");
  EXPECT_EQ(occurrence.color, WHITE_White);
  EXPECT_TRUE(occurrence.material_name.empty());
  EXPECT_TRUE(occurrence.children.empty());
  ASSERT_EQ(occurrence.dimensions.size(), 9U);
  EXPECT_EQ(occurrence.dimensions[0].value, "3");
  EXPECT_EQ(occurrence.dimensions[1].value, HumanReadable(1_mm));
  EXPECT_EQ(occurrence.dimensions[2].value, HumanReadable(3_mm));
  EXPECT_EQ(occurrence.dimensions[8].value, HumanReadable(15_mm));
  EXPECT_EQ(Mesh(scene, 2).shape.header, (std::array{8U, 3U, 2U, 5U}));
  EXPECT_EQ(Mesh(scene, 2).shape.parameters[0],
            (std::array{0.001F, 0.002F, 0.003F, 0.0F}));
  EXPECT_EQ(Mesh(scene, 2).vertex_count, 36U);

  std::array const expected_min{0.098, 0.0185, -0.0375};
  std::array const expected_max{0.102, 0.0215, -0.0225};
  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    EXPECT_NEAR(occurrence.minimum_m[axis], expected_min[axis], 1.0e-15);
    EXPECT_NEAR(occurrence.maximum_m[axis], expected_max[axis], 1.0e-15);
  }
  EXPECT_FLOAT_EQ(occurrence.local_to_world[3], 0.1F);
  EXPECT_FLOAT_EQ(occurrence.local_to_world[7], 0.02F);
  EXPECT_FLOAT_EQ(occurrence.local_to_world[11], -0.03F);
  EXPECT_DOUBLE_EQ(occurrence.orientation[1], -1.0);
  EXPECT_DOUBLE_EQ(occurrence.orientation[3], 1.0);

  GGEMSGeometryAppearance appearance{};
  EXPECT_EQ(ResolveGeometryDisplayMode(occurrence, appearance),
            GGEMSGeometryDisplayMode::Solid);
  for (auto mode : {
         GGEMSGeometryDisplayMode::None,
         GGEMSGeometryDisplayMode::Wireframe,
         GGEMSGeometryDisplayMode::Solid,
       }) {
    appearance.mode_override = mode;
    EXPECT_EQ(ResolveGeometryDisplayMode(occurrence, appearance), mode);
  }
}

TEST(GGEMSGeometryScene,
     CentralVoxelPlanesKeepMappingStylesAndPhysicalIdentity) {
  using Mode = GGEMSGeometryDisplayMode;
  std::array<std::uint16_t, 27> labels{};
  for (std::size_t index = 0U; index < labels.size(); ++index) {
    labels[index] = static_cast<std::uint16_t>(index % 5U);
  }
  ggems::test::GGEMSVoxelizedTestFixture const fixture{
    {3U, 3U, 3U},
    "1 2 3",
    labels,
    "0 0 Air\n1 1 Water Solid red_ruby\n"
    "2 2 Water Wireframe blue_pure\n3 3 Water None white_white\n"
    "4 4 Water Solid red_ruby\n",
  };
  auto volume = fixture.Import();
  ASSERT_EQ(volume.GetMaterials().size(), 2U);
  auto world = World();
  volume.SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0));
  world.AddVolume(volume);
  volume.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  world.AddVolume(volume);
  auto const scene = BuildGeometryScene(world);
  world = World();
  ASSERT_EQ(scene.occurrences.size(), 3U);
  // Independent X-fast RAW offsets for XY(z=1), XZ(y=1), and YZ(x=1).
  std::array const offsets{
    std::array{9U, 10U, 11U, 12U, 13U, 14U, 15U, 16U, 17U},
    std::array{3U, 4U, 5U, 12U, 13U, 14U, 21U, 22U, 23U},
    std::array{1U, 4U, 7U, 10U, 13U, 16U, 19U, 22U, 25U},
  };
  std::array const modes{
    Mode::Solid, Mode::Solid, Mode::Wireframe, Mode::None, Mode::Solid,
  };
  std::array const colors{
    BuildBuiltInMaterial("Air").GetColor(),
    RED_Ruby,
    BLUE_Pure,
    WHITE_White,
    RED_Ruby,
  };
  std::size_t next_offset{};
  for (std::size_t id = 1U; id < scene.occurrences.size(); ++id) {
    auto const &occurrence = scene.occurrences[id];
    EXPECT_EQ(occurrence.volume_id, id);
    EXPECT_EQ(occurrence.parent_volume_id, 0U);
    ASSERT_TRUE(occurrence.voxel_slices);
    auto const &slices = *occurrence.voxel_slices;
    EXPECT_EQ(slices.palette_size, 2U);
    EXPECT_EQ(slices.dimensions, (std::array{3U, 3U, 3U}));
    EXPECT_EQ(slices.instance_counts, (std::array{9U, 9U, 9U}));
    EXPECT_EQ(slices.spacing_m, (std::array{0.001, 0.002, 0.003}));
    EXPECT_EQ(slices.size_m, (std::array{0.003, 0.006, 0.009}));
    std::array<std::array<bool, 9>, 3> seen{};
    std::array<std::size_t, 3> batch_counts{};
    for (auto const &batch : slices.batches) {
      ASSERT_LT(batch.plane, offsets.size());
      EXPECT_EQ(batch.first_instance, next_offset);
      next_offset += batch.instance_count;
      ASSERT_LE(next_offset, scene.voxel_cells.size());
      ASSERT_GT(batch.instance_count, 0U);
      ASSERT_LT(batch.mapping_index, slices.mappings.size());
      ++batch_counts[batch.plane];
      for (std::uint32_t index = 0U; index < batch.instance_count; ++index) {
        auto const cell = scene.voxel_cells[batch.first_instance + index];
        ASSERT_LT(cell, 9U);
        EXPECT_FALSE(seen[batch.plane][cell]);
        seen[batch.plane][cell] = true;
        auto const label = labels[offsets[batch.plane][cell]];
        EXPECT_EQ(batch.mapping_index, label);
        EXPECT_EQ(batch.mode, modes[label]);
        EXPECT_EQ(batch.color, colors[label]);
      }
    }
    // Original mapping rows stay independent, even with equal color/material.
    EXPECT_EQ(batch_counts, (std::array<std::size_t, 3>{5U, 5U, 5U}));
    for (std::size_t plane = 0U; plane < offsets.size(); ++plane) {
      for (std::size_t cell = 0U; cell < offsets[plane].size(); ++cell) {
        EXPECT_TRUE(seen[plane][cell]);
      }
    }
  }
  EXPECT_EQ(next_offset, scene.voxel_cells.size());
  EXPECT_EQ(scene.voxel_cells.size(), 54U);
  auto const &rotated = scene.occurrences[2];
  EXPECT_EQ(rotated.position_m, (std::array{0.02, 0.0, 0.0}));
  EXPECT_DOUBLE_EQ(rotated.orientation[1], -1.0);
  EXPECT_DOUBLE_EQ(rotated.orientation[3], 1.0);
  EXPECT_NEAR(rotated.minimum_m[0], 0.017, 1.0e-15);
  EXPECT_NEAR(rotated.maximum_m[0], 0.023, 1.0e-15);
  EXPECT_NEAR(rotated.minimum_m[1], -0.0015, 1.0e-15);
  EXPECT_NEAR(rotated.maximum_m[1], 0.0015, 1.0e-15);
  EXPECT_NEAR(rotated.minimum_m[2], -0.0045, 1.0e-15);
  EXPECT_NEAR(rotated.maximum_m[2], 0.0045, 1.0e-15);
}

TEST(GGEMSGeometryScene, RunOwnedVoxelGeometryNeedsNoSimulationInitialization) {
  ggems::test::GGEMSVoxelizedTestFixture const fixture;
  auto world = World();
  world.AddVolume(fixture.Import());
  ggems::core::GGEMSRun run;
  run.SetWorld(world);
  ASSERT_NE(run.GetWorld(), nullptr);
  auto const scene = BuildGeometryScene(*run.GetWorld());
  ASSERT_EQ(scene.occurrences.size(), 2U);
  ASSERT_TRUE(scene.occurrences[1].voxel_slices);
  EXPECT_EQ(scene.occurrences[1].voxel_slices->instance_counts,
            (std::array{4U, 4U, 4U}));
  EXPECT_EQ(scene.occurrences[1].minimum_m,
            (std::array{-0.001, -0.002, -0.003}));
  EXPECT_EQ(scene.occurrences[1].maximum_m, (std::array{0.001, 0.002, 0.003}));
}

TEST(GGEMSGeometryScene, NamedGroupsCacheExactMembershipAndMixedLastCommands) {
  using Mode = GGEMSGeometryDisplayMode;
  auto air = BuildBuiltInMaterial("Air");
  GGEMSBox crystal{"Crystal"};
  crystal.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(air);
  GGEMSBox block{"Block"};
  block.SetSize(8_mm, 8_mm, 8_mm).SetMaterial(air);
  block.LinearRepeater(crystal, 2U, 4_mm, GGEMSRepeatAxis::X);
  GGEMSBox module{"Module"};
  module.SetSize(24_mm, 16_mm, 16_mm).SetMaterial(air);
  module.LinearRepeater(block, 2U, 12_mm, GGEMSRepeatAxis::X);
  GGEMSBox pet{"PET"};
  pet.SetSize(80_mm, 30_mm, 30_mm).SetMaterial(air);
  pet.LinearRepeater(module, 2U, 40_mm, GGEMSRepeatAxis::X);
  GGEMSWorld world{"PET / XCAT"};
  world.SetSize(1_m, 1_m, 1_m).SetMaterial(air);
  world.AddVolume(pet);
  GGEMSBox independent{"Block"};
  independent.SetSize(8_mm, 8_mm, 8_mm)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetMaterial(air);
  world.AddVolume(independent);
  auto const scene = BuildGeometryScene(world);
  world = World();

  ASSERT_EQ(scene.occurrences.size(), 17U);
  EXPECT_EQ(scene.occurrences[0].name, "PET / XCAT");
  ASSERT_EQ(scene.groups.size(), 5U);
  EXPECT_EQ(scene.groups[0].name, "PET");
  EXPECT_TRUE(scene.groups[0].subtree);
  EXPECT_EQ(scene.groups[0].members.size(), 15U);
  EXPECT_EQ(scene.groups[1].members.size(), 2U);
  EXPECT_EQ(scene.groups[2].members.size(), 4U);
  EXPECT_EQ(scene.groups[3].members.size(), 8U);
  EXPECT_TRUE(scene.groups[3].physical_leaves);
  EXPECT_FALSE(scene.groups[0].physical_leaves);
  EXPECT_FALSE(scene.groups[1].physical_leaves);
  EXPECT_EQ(scene.groups[4].name, "Block");
  EXPECT_EQ(scene.groups[4].members, (std::vector<std::uint32_t>{16U}));
  EXPECT_EQ(scene.groups[0].child_groups,
            (std::vector<std::uint32_t>{1U, 2U, 3U}));
  EXPECT_EQ(scene.occurrences[1].child_branches,
            (std::vector<std::vector<std::uint32_t>>{{2U, 9U}}));
  ASSERT_EQ(scene.root_entries.size(), 2U);
  EXPECT_EQ(scene.root_entries[0].group_index, 0U);
  EXPECT_EQ(scene.root_entries[1].group_index, 4U);

  GGEMSGeometryAppearanceState state;
  state.Reset(scene);
  EXPECT_FALSE(state.GetGroupMode(0U));
  EXPECT_EQ(state.GetGroupMode(1U), Mode::Wireframe);
  EXPECT_EQ(state.GetGroupMode(3U), Mode::Solid);
  state.SetGroupMode(scene, 0U, Mode::None);
  EXPECT_EQ(state.GetGroupMode(0U), Mode::None);
  state.SetOccurrenceMode(scene, 4U, Mode::Solid);
  EXPECT_FALSE(state.GetGroupMode(0U));
  EXPECT_FALSE(state.GetGroupMode(3U));
  for (auto const id : scene.groups[0].members) {
    EXPECT_EQ(state.GetOccurrenceMode(scene, id),
              id == 4U ? Mode::Solid : Mode::None);
  }
  state.SetGroupMode(scene, 2U, Mode::Wireframe);
  EXPECT_EQ(state.GetGroupMode(2U), Mode::Wireframe);
  EXPECT_FALSE(state.GetGroupMode(0U));
  EXPECT_EQ(state.GetOccurrenceMode(scene, 4U), Mode::Solid);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 5U), Mode::None);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 16U), Mode::Solid);
  state.SetGroupMode(scene, 0U, Mode::Solid);
  for (std::uint32_t group = 0U; group < 4U; ++group) {
    EXPECT_EQ(state.GetGroupMode(group), Mode::Solid);
  }
  state.SetOccurrenceMode(scene, 0U, Mode::None);
  EXPECT_EQ(state.GetGroupMode(0U), Mode::Solid);
  state.Reset(scene);
  EXPECT_FALSE(state.GetGroupMode(0U));
  EXPECT_EQ(state.GetOccurrenceMode(scene, 0U), Mode::Wireframe);
  EXPECT_FALSE(state.GetOccurrences()[4U].mode_override);
  state.Clear();
  EXPECT_TRUE(state.GetOccurrences().empty());
}

TEST(GGEMSGeometryScene, MappingEditsRetainAllThreePlanesAndIndependentModes) {
  using Mode = GGEMSGeometryDisplayMode;
  std::array<std::uint16_t, 8> const labels{0U, 1U, 2U, 3U, 0U, 1U, 2U, 3U};
  ggems::test::GGEMSVoxelizedTestFixture const fixture{
    {2U, 2U, 2U},
    "1 2 3",
    labels,
    "0 0 Air\n1 1 Water Solid blue_babyblue\n"
    "2 2 Water Wireframe red_ruby\n3 3 Air None white_white\n",
  };
  auto world = World();
  auto const imported = fixture.Import("XCAT");
  auto const physical_labels = std::vector(imported.GetMaterialLabels().begin(),
                                           imported.GetMaterialLabels().end());
  world.AddVolume(imported);
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.groups.size(), 1U);
  EXPECT_EQ(scene.groups[0].name, "XCAT");
  EXPECT_FALSE(scene.groups[0].physical_leaves);
  auto const &slices = *scene.occurrences[1U].voxel_slices;
  ASSERT_EQ(slices.mappings.size(), 4U);
  EXPECT_EQ(slices.mappings[0].material_name, "Air");
  EXPECT_EQ(slices.mappings[3].material_name, "Air");
  EXPECT_EQ(slices.mappings[3].first_label, 3U);
  EXPECT_EQ(slices.mappings[3].end_label, 4U);
  EXPECT_EQ(slices.palette_size, 2U);
  EXPECT_EQ(scene.voxel_cells.size(), 12U);
  auto const immutable_cells = scene.voxel_cells;
  GGEMSGeometryAppearanceState state;
  state.Reset(scene);
  auto const visible_count = [&] -> std::uint32_t {
    std::uint32_t result{};
    for (auto const &batch : slices.batches) {
      auto const mode = ResolveVoxelDisplayMode(
        state.GetOccurrenceMode(scene, 1U),
        state.GetVoxelMappingMode(1U, batch.mapping_index));
      if (mode != Mode::None) {
        result += batch.instance_count;
      }
    }
    return result;
  };
  EXPECT_EQ(visible_count(), 7U);
  state.SetVoxelMappingMode(1U, 0U, Mode::None);
  EXPECT_EQ(visible_count(), 6U);
  state.SetGroupMode(scene, 0U, Mode::Wireframe);
  EXPECT_EQ(state.GetVoxelMappingMode(1U, 1U), Mode::Solid);
  EXPECT_EQ(state.GetVoxelMappingMode(1U, 2U), Mode::Wireframe);
  EXPECT_EQ(visible_count(), 6U);
  state.SetGroupMode(scene, 0U, Mode::None);
  EXPECT_EQ(visible_count(), 0U);
  state.SetGroupMode(scene, 0U, Mode::Solid);
  EXPECT_EQ(visible_count(), 6U);
  state.SetVoxelMappingMode(1U, 3U, Mode::Solid);
  EXPECT_EQ(visible_count(), 11U);
  EXPECT_EQ(state.GetVoxelMappingMode(1U, 0U), Mode::None);
  EXPECT_EQ(slices.mappings[3].mode, Mode::None);
  EXPECT_EQ(slices.mappings[2].color, RED_Ruby);
  EXPECT_EQ(scene.voxel_cells, immutable_cells);
  EXPECT_TRUE(
    std::ranges::equal(imported.GetMaterialLabels(), physical_labels));
  state.Reset(scene);
  EXPECT_EQ(visible_count(), 7U);
  EXPECT_EQ(state.GetVoxelMappingMode(1U, 3U), Mode::None);
  state.Reset(BuildGeometryScene(World()));
  ASSERT_EQ(state.GetOccurrences().size(), 1U);
  state.Reset(scene);
  EXPECT_EQ(visible_count(), 7U);
}

TEST(GGEMSGeometryScene, NamedChildRemainsDiscoverableAcrossUnnamedRoots) {
  using Mode = GGEMSGeometryDisplayMode;
  auto const air = BuildBuiltInMaterial("Air");
  GGEMSBox child{"Shared child"};
  child.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(air);
  GGEMSBox unnamed;
  unnamed.SetSize(8_mm, 8_mm, 8_mm).SetMaterial(air);
  unnamed.AddVolume(child);
  auto world = World();
  unnamed.SetPosition(MakePositionPM(-50'000'000'000LL, 0, 0));
  world.AddVolume(unnamed);
  unnamed.SetPosition(MakePositionPM(0, 0, 0));
  GGEMSBox named{"PET"};
  named.SetSize(16_mm, 16_mm, 16_mm)
    .SetPosition(MakePositionPM(50'000'000'000LL, 0, 0))
    .SetMaterial(air);
  named.AddVolume(unnamed);
  world.AddVolume(named);
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.occurrences.size(), 6U);
  ASSERT_EQ(scene.groups.size(), 2U);
  EXPECT_EQ(scene.groups[0].members, (std::vector<std::uint32_t>{2U, 5U}));
  EXPECT_EQ(scene.groups[1].members, (std::vector<std::uint32_t>{3U, 4U, 5U}));
  EXPECT_EQ(scene.groups[1].child_groups, (std::vector<std::uint32_t>{0U}));
  ASSERT_EQ(scene.root_entries.size(), 3U);
  EXPECT_FALSE(scene.root_entries[0].group_index);
  EXPECT_EQ(scene.root_entries[0].volumes, (std::vector<std::uint32_t>{1U}));
  EXPECT_EQ(scene.root_entries[1].group_index, 0U);
  EXPECT_EQ(scene.root_entries[2].group_index, 1U);
  GGEMSGeometryAppearanceState state;
  state.Reset(scene);
  state.SetGroupMode(scene, 1U, Mode::None);
  EXPECT_FALSE(state.GetGroupMode(0U));
  state.SetGroupMode(scene, 0U, Mode::Wireframe);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 2U), Mode::Wireframe);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 5U), Mode::Wireframe);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 3U), Mode::None);
  EXPECT_EQ(state.GetOccurrenceMode(scene, 4U), Mode::None);
}

TEST(GGEMSGeometryScene, GroupInspectionUsesCapturedRecipesAndActualMembers) {
  auto const air = BuildBuiltInMaterial("Air");
  auto const lso = BuildBuiltInMaterial("LSO");
  GGEMSBox crystal{"Crystal"};
  crystal.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(lso);
  GGEMSBox module{"Module"};
  module.SetSize(20_mm, 30_mm, 10_mm)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetMaterial(air);
  module.LinearRepeater(crystal, {2U, 3U, 1U}, {4_mm, 4_mm, 0_mm});
  GGEMSTubs pet{"PET"};
  pet.SetRadii(200_mm, 70_mm).SetLength(50_mm).SetMaterial(air);
  pet.CircularRepeater(module, 4U, GGEMSRepeatAxis::Z);
  auto world = World();
  world.AddVolume(pet);
  GGEMSBox independent{"Crystal"};
  independent.SetSize(6_mm, 6_mm, 6_mm)
    .SetPosition(MakePositionPM(300'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(independent);
  auto const scene = BuildGeometryScene(world);
  world = World();
  ASSERT_EQ(scene.groups.size(), 4U);
  auto const &root = scene.groups[0];
  auto const &modules = scene.groups[1];
  auto const &crystals = scene.groups[2];
  EXPECT_TRUE(HasInspection(root, "Controlled occurrences", "29"));
  EXPECT_TRUE(HasInspection(root, "Authored roots (global)", "1"));
  EXPECT_TRUE(HasInspection(root, "Type", "Tubs (1 occurrences)"));
  EXPECT_TRUE(HasInspection(root, "Type", "Box (28 occurrences)"));
  EXPECT_TRUE(HasInspection(
    root, "Material",
    "Air; density " + HumanReadable(air.GetDensity(), 3) + "; 5 occurrences"));
  EXPECT_TRUE(HasInspection(
    root, "Material",
    "LSO; density " + HumanReadable(lso.GetDensity(), 3) + "; 24 occurrences"));
  EXPECT_TRUE(HasInspection(root, "Root placement", "Single attachment"));
  EXPECT_TRUE(HasInspection(modules, "Root placement", "Circular"));
  EXPECT_TRUE(HasInspection(modules, "Circular axis", "Z"));
  EXPECT_TRUE(HasInspection(modules, "Roots per parent", "4"));
  EXPECT_TRUE(HasInspection(modules, "Authored roots (global)", "4"));
  EXPECT_TRUE(HasInspection(crystals, "Root placement", "Linear"));
  EXPECT_TRUE(HasInspection(crystals, "Controlled occurrences", "24"));
  EXPECT_TRUE(HasInspection(crystals, "Roots per parent", "6"));
  EXPECT_TRUE(HasInspection(crystals, "Physical parents", "4"));
  EXPECT_TRUE(HasInspection(crystals, "Linear counts (X, Y, Z)", "(2, 3, 1)"));
  EXPECT_TRUE(HasInspection(crystals, "Linear pitch (X, Y, Z)",
                            "(" + HumanReadable(4_mm, 3) + ", " +
                              HumanReadable(4_mm, 3) + ", " +
                              HumanReadable(0_mm, 3) + ")"));
  auto const expected_dimension = HumanReadable(2_mm);
  EXPECT_TRUE(
    std::ranges::any_of(crystals.inspection, [&](auto const &property) -> bool {
      return property.name == "Dimensions (Box)" &&
             property.value.find(expected_dimension) != std::string::npos;
    }));
  EXPECT_EQ(scene.groups[3].name, crystals.name);
  EXPECT_TRUE(HasInspection(scene.groups[3], "Controlled occurrences", "1"));
  EXPECT_TRUE(
    HasInspection(scene.groups[3], "Root placement", "Not available"));
  EXPECT_FALSE(HasInspection(scene.groups[3], "Root placement", "Linear"));
}

TEST(GGEMSGeometryScene, ScalarZRepeaterInspectionUsesCanonicalAxisData) {
  GGEMSBox axial{"Axial"};
  axial.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  auto world = World();
  world.LinearRepeater(axial, 3U, 6_mm, GGEMSRepeatAxis::Z);
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.groups.size(), 1U);
  auto const &group = scene.groups.front();
  EXPECT_TRUE(group.physical_leaves);
  EXPECT_TRUE(HasInspection(group, "Root placement", "Linear"));
  EXPECT_TRUE(HasInspection(group, "Linear counts (X, Y, Z)", "(1, 1, 3)"));
  EXPECT_TRUE(HasInspection(group, "Linear pitch (X, Y, Z)",
                            "(" + HumanReadable(0_mm, 3) + ", " +
                              HumanReadable(0_mm, 3) + ", " +
                              HumanReadable(6_mm, 3) + ")"));
  EXPECT_FALSE(
    std::ranges::any_of(group.inspection, [](auto const &property) -> bool {
      return property.name == "Circular axis";
    }));
}
