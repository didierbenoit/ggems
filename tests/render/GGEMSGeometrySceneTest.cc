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
 * \brief Checks physical placement and analytical wireframe presentation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <span>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

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

auto MeshVertices(GGEMSGeometryScene const &scene, std::size_t occurrence)
  -> std::span<GGEMSGeometryVertex const> {
  auto const &mesh = scene.meshes[scene.occurrences[occurrence].mesh_index];
  return std::span{scene.vertices}.subspan(mesh.first_vertex,
                                           mesh.vertex_count);
}

auto HasPoint(std::span<GGEMSGeometryVertex const> vertices, double x, double y,
              double z) -> bool {
  return std::ranges::any_of(vertices, [=](auto const &point) -> bool {
    return std::abs(point.x - x) < 1.0e-8 && std::abs(point.y - y) < 1.0e-8 &&
           std::abs(point.z - z) < 1.0e-8;
  });
}
} // namespace

TEST(GGEMSGeometryScene, OwnsWorldAndSharedMeshesWithPhysicalHierarchy) {
  auto world = World();
  GGEMSBox mother;
  mother.SetSize(200_mm, 200_mm, 200_mm)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(mother);
  GGEMSBox child;
  child.SetSize(10_mm, 20_mm, 30_mm)
    .SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(child, 1U);
  child.SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(child, 1U);
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  world = World();

  ASSERT_EQ(scene.occurrences.size(), 4U);
  EXPECT_EQ(scene.world_half_extent_m, (std::array{0.5, 0.5, 0.5}));
  EXPECT_EQ(scene.occurrences[0].volume_id, 0U);
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 0), -0.5, -0.5, -0.5));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 0), 0.5, 0.5, 0.5));
  EXPECT_EQ(scene.occurrences[2].volume_id, 2U);
  EXPECT_EQ(scene.occurrences[3].volume_id, 3U);
  EXPECT_EQ(scene.occurrences[2].parent_volume_id, 1U);
  EXPECT_EQ(scene.occurrences[3].parent_volume_id, 1U);
  EXPECT_EQ(scene.occurrences[2].mesh_index, scene.occurrences[3].mesh_index);
  auto const &transform = scene.occurrences[2].local_to_world;
  EXPECT_FLOAT_EQ(transform[3], 0.1F);
  EXPECT_FLOAT_EQ(transform[7], 0.02F);
  EXPECT_FLOAT_EQ(transform[0], 0.0F);
  EXPECT_FLOAT_EQ(transform[1], -1.0F);
  EXPECT_FLOAT_EQ(transform[4], 1.0F);
  EXPECT_FLOAT_EQ(scene.occurrences[3].local_to_world[7], -0.02F);
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 2), 0.005, 0.01, 0.015));
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
        shape.SetMaterial(BuildBuiltInMaterial("Water"))
          .SetPosition(MakePositionPM(
            (static_cast<CoordinatePM>(index) - 4) * 50'000'000'000LL, 0, 0));
        world.AddVolume(shape);
      },
      shapes[index]);
  }
  ASSERT_NO_THROW(world.Validate());
  auto const scene = BuildGeometryScene(world);
  ASSERT_EQ(scene.occurrences.size(), 10U);
  for (std::size_t occurrence = 1; occurrence < scene.occurrences.size();
       ++occurrence) {
    SCOPED_TRACE(occurrence);
    auto const vertices = MeshVertices(scene, occurrence);
    ASSERT_FALSE(vertices.empty());
    ASSERT_EQ(vertices.size() % 2U, 0U);
    for (auto const &vertex : vertices) {
      EXPECT_TRUE(std::isfinite(vertex.x));
      EXPECT_TRUE(std::isfinite(vertex.y));
      EXPECT_TRUE(std::isfinite(vertex.z));
    }
  }
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 3), 0.002, 0.0, -0.004));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 3), 0.005, 0.0, 0.004));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 4), 0.002, 0.0, -0.004));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 4), 0.0, 0.0, 0.004));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 5),
                       0.006 / std::cos(std::numbers::pi / 6.0), 0.0, -0.004));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 6), 0.0, 0.004, 0.0));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 7), 0.004, 0.005, 0.006));
  auto const tilt = std::tan(std::numbers::pi / 18.0);
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 8),
                       (0.006 * tilt * std::cos(std::numbers::pi / 6.0)) +
                         (0.004 * tilt) + 0.003,
                       (0.006 * tilt * 0.5) + 0.004, 0.006));
  EXPECT_TRUE(HasPoint(MeshVertices(scene, 9), 0.0, 0.0, 0.004));
}
