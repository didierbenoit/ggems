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
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
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
        EXPECT_EQ(&shape.SetColor(MAGENTA_BlueViolet), &shape);

        auto copy = shape;
        EXPECT_EQ(copy.GetColor(), MAGENTA_BlueViolet);
        copy.SetColor(YELLOW_Gold);
        EXPECT_EQ(copy.GetColor(), YELLOW_Gold);
        EXPECT_EQ(shape.GetColor(), MAGENTA_BlueViolet);
      },
      volume);
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
  for (std::uint32_t parent : scene.occurrences[0].children) {
    auto const &mother = scene.occurrences[parent];
    EXPECT_EQ(mother.color, GRAY_Silver);
    EXPECT_EQ(mother.children,
              (std::vector<std::uint32_t>{parent + 1U, parent + 2U}));
    for (auto child : mother.children) {
      auto const &occurrence = scene.occurrences[child];
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
