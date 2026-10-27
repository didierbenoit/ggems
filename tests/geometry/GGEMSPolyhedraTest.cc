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
 * \brief Checks generic polygonal profile authoring and admission.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;

auto Shape() -> GGEMSPolyhedra {
  GGEMSPolyhedra shape;
  std::array const z{
    PositionCoordinate{-6'000'000'000LL},
    PositionCoordinate{0LL},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const inner{0_pm, 0_pm, 0_pm};
  std::array const outer{6_mm, 2_mm, 6_mm};
  shape.SetSideCount(6)
    .SetStartAngle(0_deg)
    .SetZProfile(z, inner, outer)
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

TEST(N1Polyhedra, RequiresDimensionsSideCountAndMaterial) {
  GGEMSPolyhedra shape;
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(shape.GetMaterial()), GGEMSRecoverable);
  std::array const z{
    PositionCoordinate{0},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const inner{0_pm, 0_pm};
  std::array const outer{4_mm, 6_mm};
  EXPECT_EQ(&shape.SetZProfile(z, inner, outer), &shape);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  EXPECT_EQ(&shape.SetSideCount(3), &shape);
  EXPECT_THROW(shape.Validate(), GGEMSRecoverable);
  shape.SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(shape.Validate());
  ASSERT_EQ(shape.GetZProfilePM().size(), 2U);
  EXPECT_EQ(shape.GetZProfilePM()[0].z_pm, 0);
  EXPECT_EQ(shape.GetZProfilePM()[1].outer_radius_pm, 6'000'000'000ULL);
}

TEST(N1Polyhedra, RejectsInvalidAndUnsupportedProfilesAtomically) {
  auto shape = Shape();
  std::array const z{
    PositionCoordinate{-6'000'000'000LL},
    PositionCoordinate{0},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const inner{0_pm, 0_pm, 0_pm};
  std::array const outer{6_mm, 2_mm, 6_mm};
  EXPECT_THROW(shape.SetSideCount(0), GGEMSRecoverable);
  EXPECT_THROW(shape.SetSideCount(2), GGEMSRecoverable);
  EXPECT_EQ(shape.GetSideCount(), 6U);
  EXPECT_THROW(shape.SetZProfile(std::span(z).first(1),
                                 std::span(inner).first(1),
                                 std::span(outer).first(1)),
               GGEMSRecoverable);
  EXPECT_THROW(shape.SetZProfile(z, std::span(inner).first(2), outer),
               GGEMSRecoverable);
  auto bad_z = z;
  bad_z[1] = bad_z[0];
  EXPECT_THROW(shape.SetZProfile(bad_z, inner, outer), GGEMSRecoverable);
  bad_z[1] = PositionCoordinate{-7'000'000'000LL};
  EXPECT_THROW(shape.SetZProfile(bad_z, inner, outer), GGEMSRecoverable);
  auto bad_outer = outer;
  bad_outer[1] = 0_pm;
  EXPECT_THROW(shape.SetZProfile(z, inner, bad_outer), GGEMSRecoverable);
  auto bad_inner = inner;
  bad_inner[1] = 2_mm;
  EXPECT_THROW(shape.SetZProfile(z, bad_inner, outer), GGEMSRecoverable);
  EXPECT_EQ(shape.GetZProfilePM()[1].outer_radius_pm, 2'000'000'000ULL);
  EXPECT_EQ(shape.GetZProfilePM()[1].z_pm, 0);
}

TEST(N1Polyhedra, AdmitsGenericCountsConcavityAndClosingBore) {
  for (std::uint32_t sides : {3U, 4U, 6U, 17U, 64U}) {
    auto shape = Shape();
    shape.SetSideCount(sides);
    auto world = World();
    world.AddVolume(shape);
    EXPECT_NO_THROW(world.Validate()) << sides;
  }
  auto shape = Shape();
  std::array const z{
    PositionCoordinate{-6'000'000'000LL},
    PositionCoordinate{0},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const inner{0_pm, 2_mm, 0_pm};
  std::array const outer{6_mm, 6_mm, 6_mm};
  shape.SetZProfile(z, inner, outer);
  auto world = World();
  world.AddVolume(shape);
  EXPECT_NO_THROW(world.Validate());
}

TEST(N1Polyhedra, OwnsProfilePoseAndMaterialAndRetainsOneVolumeLimit) {
  auto shape = Shape();
  shape.SetPosition(MakePositionPM(4'000'000'000LL, -2'000'000'000LL, 0))
    .SetRotation(0_deg, 30_deg, 0_deg);
  auto world = World();
  world.AddVolume(shape);
  shape.SetSideCount(3).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  auto copy = world;
  EXPECT_NO_THROW(copy.Validate());
  EXPECT_EQ(copy.BuildAnalyticVolumeRecord(7U).kind,
            GGEMSAnalyticKind::Polyhedra);
  ASSERT_NE(copy.GetVolumeMaterial(), nullptr);
  EXPECT_EQ(copy.GetVolumeMaterial()->GetName(), "Water");
  EXPECT_EQ(copy.BuildAnalyticAuxiliaryData(),
            world.BuildAnalyticAuxiliaryData());
  EXPECT_FALSE(copy.BuildAnalyticAuxiliaryData().empty());
  EXPECT_THROW(copy.AddVolume(shape), GGEMSRecoverable);
  GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_THROW(copy.AddVolume(box), GGEMSRecoverable);
}

TEST(N1Polyhedra, RejectsUnresolvedProfilesAndWorldClearance) {
  std::array const z{
    PositionCoordinate{-6'000'000'000LL},
    PositionCoordinate{6'000'000'000LL},
  };
  std::array const outer{6_mm, 6_mm};
  for (auto const &inner : {
         std::array{1_pm, 1_pm},
         std::array{Length{5'999'999'999ULL}, Length{5'999'999'999ULL}},
       }) {
    auto shape = Shape();
    shape.SetZProfile(z, inner, outer);
    auto world = World();
    EXPECT_THROW(
      {
        world.AddVolume(shape);
        world.Validate();
      },
      GGEMSRecoverable);
  }
  auto shape = Shape();
  shape.SetPosition(MakePositionPM(35'000'000'000LL, 0, 0));
  auto world = World();
  world.AddVolume(shape);
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
}
