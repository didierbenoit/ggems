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
 * \brief Tests the World authoring lifecycle, lattice contract and scene
 * validation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::GGEMSEMMaterialPackage;
using ggems::core::materials::GGEMSMaterial;
using ggems::geometry::GGEMSBox;
using ggems::geometry::GGEMSBoxRecord;
using ggems::geometry::GGEMSWorld;
using ggems::geometry::MakePositionPM;

static_assert(std::is_default_constructible_v<GGEMSWorld>);
static_assert(std::is_copy_constructible_v<GGEMSWorld>);

/*! \brief Exact empty-matter World medium used by the geometry fixtures. */
auto Vacuum() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum");
}

auto Water() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Water");
}

/*! \brief Complete World from its full sizes and its medium. */
auto MakeWorld(Length size_x, Length size_y, Length size_z,
               ggems::core::materials::GGEMSMaterial material) -> GGEMSWorld {
  GGEMSWorld world{};
  world.SetSize(size_x, size_y, size_z).SetMaterial(std::move(material));
  return world;
}

/*! \brief The N1.1 asymmetric Water Box: x in [-20, 40], y in [-30, -10], z in
 * [-10, 20] mm, authored by its center. */
auto MakeBox() -> GGEMSBox {
  GGEMSBox box{};
  box.SetSize(60_mm, 20_mm, 30_mm)
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL))
    .SetMaterial(Water());
  return box;
}

/*! \brief Largest even full size admitted by the World. */
constexpr std::uint64_t k_max_size_pm{0x7FFF'FFFF'FFFF'FFFEULL};

/*! \brief Half extent of the largest admitted World. */
constexpr std::int64_t k_max_half_pm{0x3FFF'FFFF'FFFF'FFFFLL};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, StoresExactHalfExtentsAndCornersFromUserUnits) {
  GGEMSWorld const world = MakeWorld(2_m, 400_mm, 6_cm, Vacuum());

  auto const half = world.GetHalfExtentPM();
  EXPECT_EQ(half.x, 1'000'000'000'000ULL);
  EXPECT_EQ(half.y, 200'000'000'000ULL);
  EXPECT_EQ(half.z, 30'000'000'000ULL);

  EXPECT_EQ(world.GetLowerCornerPM(),
            MakePositionPM(-1'000'000'000'000LL, -200'000'000'000LL,
                           -30'000'000'000LL));
  EXPECT_EQ(
    world.GetUpperCornerPM(),
    MakePositionPM(1'000'000'000'000LL, 200'000'000'000LL, 30'000'000'000LL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RejectsZeroSizeOnEveryAxis) {
  EXPECT_THROW(GGEMSWorld{}.SetSize(0_pm, 2_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(2_pm, 0_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(2_pm, 2_pm, 0_pm), GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RejectsOddPicometerSizeOnEveryAxis) {
  EXPECT_THROW(GGEMSWorld{}.SetSize(1_pm, 2_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(2_pm, 3_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(2_pm, 2_pm, 1'000'000'001_pm),
               GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AcceptsSmallestWorld) {
  GGEMSWorld const world = MakeWorld(2_pm, 2_pm, 2_pm, Vacuum());

  EXPECT_EQ(world.GetHalfExtentPM(),
            (ggems::geometry::HalfExtent3PM{.x = 1ULL, .y = 1ULL, .z = 1ULL}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AcceptsLargestWorldAndRejectsLargerSizes) {
  Length const max_size{k_max_size_pm};
  Length const above_max_even{0x8000'0000'0000'0000ULL};
  Length const largest_even{std::numeric_limits<std::uint64_t>::max() - 1ULL};

  GGEMSWorld const world = MakeWorld(max_size, max_size, max_size, Vacuum());

  EXPECT_EQ(world.GetHalfExtentPM().x,
            static_cast<std::uint64_t>(k_max_half_pm));
  EXPECT_EQ(world.GetLowerCornerPM(),
            MakePositionPM(-k_max_half_pm, -k_max_half_pm, -k_max_half_pm));
  EXPECT_EQ(world.GetUpperCornerPM(),
            MakePositionPM(k_max_half_pm, k_max_half_pm, k_max_half_pm));

  // The displacement between opposite corners is the full size on every axis
  // and stays representable in signed picometers.
  auto const diagonal = world.GetUpperCornerPM() - world.GetLowerCornerPM();
  EXPECT_EQ(diagonal.x, static_cast<std::int64_t>(k_max_size_pm));
  EXPECT_EQ(diagonal.y, static_cast<std::int64_t>(k_max_size_pm));
  EXPECT_EQ(diagonal.z, static_cast<std::int64_t>(k_max_size_pm));

  EXPECT_THROW(GGEMSWorld{}.SetSize(above_max_even, max_size, max_size),
               GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(max_size, above_max_even, max_size),
               GGEMSRecoverable);
  EXPECT_THROW(GGEMSWorld{}.SetSize(max_size, max_size, largest_even),
               GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, ContainsClosedBoxPositionsForStaticInputValidation) {
  // Asymmetric World so that each axis is checked against its own half extent.
  GGEMSWorld const world = MakeWorld(20_pm, 40_pm, 60_pm, Vacuum());

  EXPECT_TRUE(world.Contains(MakePositionPM(0, 0, 0)));
  EXPECT_TRUE(world.Contains(MakePositionPM(10, -20, 30)));
  EXPECT_TRUE(world.Contains(MakePositionPM(-10, 20, -30)));
  EXPECT_TRUE(world.Contains(MakePositionPM(9, -19, 29)));
  EXPECT_FALSE(world.Contains(MakePositionPM(11, 0, 0)));
  EXPECT_FALSE(world.Contains(MakePositionPM(0, -21, 0)));
  EXPECT_FALSE(world.Contains(MakePositionPM(0, 0, 31)));
  EXPECT_FALSE(world.Contains(MakePositionPM(10, 20, 31)));

  constexpr std::int64_t k_min = std::numeric_limits<std::int64_t>::min();
  constexpr std::int64_t k_max = std::numeric_limits<std::int64_t>::max();
  EXPECT_FALSE(world.Contains(MakePositionPM(k_min, 0, 0)));
  EXPECT_FALSE(world.Contains(MakePositionPM(0, k_max, 0)));

  Length const max_size{k_max_size_pm};
  GGEMSWorld const largest = MakeWorld(max_size, max_size, max_size, Vacuum());
  EXPECT_TRUE(largest.Contains(
    MakePositionPM(-k_max_half_pm, k_max_half_pm, -k_max_half_pm)));
  EXPECT_FALSE(largest.Contains(MakePositionPM(0, -k_max_half_pm - 1, 0)));
  EXPECT_FALSE(largest.Contains(MakePositionPM(k_min, 0, 0)));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, OwnsExplicitMaterialIncludingExactVacuum) {
  GGEMSWorld const vacuum_world = MakeWorld(2_m, 2_m, 2_m, Vacuum());
  EXPECT_EQ(vacuum_world.GetMaterial().GetName(), "Vacuum");
  EXPECT_EQ(vacuum_world.GetMaterial().GetDensity().value, 0.0L);
  EXPECT_TRUE(vacuum_world.GetMaterial().GetElementalConstituents().empty());

  auto water = Water();
  auto const expected_density = water.GetDensity();
  auto const expected_constituents = water.GetElementalConstituents().size();

  GGEMSWorld const water_world = MakeWorld(1_m, 2_m, 3_m, std::move(water));
  EXPECT_EQ(water_world.GetMaterial().GetName(), "Water");
  EXPECT_EQ(water_world.GetMaterial().GetDensity(), expected_density);
  EXPECT_EQ(water_world.GetMaterial().GetElementalConstituents().size(),
            expected_constituents);
  EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
    water_world.GetMaterial(), Water()));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, PreservesExactFacesBeyondBinary64IntegerPrecision) {
  // Half extent 2^53 + 1 pm is not representable in binary64; authored as the
  // full size 2^54 + 2 pm. Exact one-picometer distinctions must survive.
  constexpr std::int64_t k_half_pm{(1LL << 53) + 1LL};
  Length const size{static_cast<std::uint64_t>(k_half_pm) * 2ULL};

  GGEMSWorld const world = MakeWorld(size, size, size, Vacuum());
  EXPECT_EQ(world.GetHalfExtentPM().x, static_cast<std::uint64_t>(k_half_pm));

  EXPECT_EQ(world.BuildRecord().half_extent_x_pm,
            static_cast<float>(k_half_pm));
  EXPECT_EQ(world.GetLowerCornerPM().x, -k_half_pm);
  EXPECT_EQ(world.GetUpperCornerPM().x, k_half_pm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, OwnedMaterialOutlivesTheCallerMaterial) {
  auto const build = [] -> GGEMSWorld {
    auto air = ggems::core::materials::builtins::BuildBuiltInMaterial("Air");
    return MakeWorld(1_m, 1_m, 1_m, air);
    // air is destroyed here; the World keeps its own copy.
  };

  GGEMSWorld const world = build();
  EXPECT_EQ(world.GetMaterial().GetName(), "Air");
  EXPECT_FALSE(world.GetMaterial().GetElementalConstituents().empty());
  EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
    world.GetMaterial(),
    ggems::core::materials::builtins::BuildBuiltInMaterial("Air")));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RejectedSizeKeepsThePreviousSizeAndCallerMaterialUsable) {
  auto const water = Water();

  GGEMSWorld world{};
  world.SetSize(2_pm, 2_pm, 2_pm);
  EXPECT_THROW(world.SetSize(0_pm, 2_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(world.SetSize(4_pm, 5_pm, 4_pm), GGEMSRecoverable);
  EXPECT_EQ(world.GetHalfExtentPM(),
            (ggems::geometry::HalfExtent3PM{.x = 1ULL, .y = 1ULL, .z = 1ULL}));

  // The caller-owned lvalue Material is untouched by the failed configuration.
  EXPECT_EQ(water.GetName(), "Water");
  world.SetMaterial(water);
  EXPECT_EQ(world.GetMaterial().GetName(), "Water");
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, ValidateRequiresASizeAndAnExplicitMaterial) {
  GGEMSWorld world{};
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(world.GetMaterial()), GGEMSRecoverable);

  world.SetSize(2_m, 2_m, 2_m);
  EXPECT_THROW(world.Validate(), GGEMSRecoverable);

  world.SetMaterial(Vacuum());
  EXPECT_NO_THROW(world.Validate());
  EXPECT_EQ(world.GetMaterial().GetName(), "Vacuum");
  EXPECT_TRUE(world.GetBoxes().empty());
  EXPECT_TRUE(world.BuildBoxRecords({}).empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, SetterOrderDoesNotChangeTheCompiledScene) {
  GGEMSWorld first{};
  first.SetSize(200_mm, 200_mm, 200_mm)
    .SetMaterial(Vacuum())
    .AddVolume(MakeBox());

  GGEMSWorld second{};
  second.AddVolume(MakeBox()).SetMaterial(Vacuum()).SetSize(200_mm, 200_mm,
                                                            200_mm);

  EXPECT_NO_THROW(first.Validate());
  EXPECT_NO_THROW(second.Validate());
  EXPECT_EQ(first.GetHalfExtentPM(), second.GetHalfExtentPM());
  EXPECT_EQ(first.GetMaterial().GetName(), second.GetMaterial().GetName());
  ASSERT_EQ(first.GetBoxes().size(), 1U);
  ASSERT_EQ(second.GetBoxes().size(), 1U);

  std::array<std::uint32_t, 1U> const material_ids{1U};
  GGEMSBoxRecord const first_record = first.BuildBoxRecords(material_ids)[0U];
  GGEMSBoxRecord const second_record = second.BuildBoxRecords(material_ids)[0U];
  EXPECT_EQ(first_record.half_extent_x_pm,
            static_cast<float>(second_record.half_extent_x_pm));
  EXPECT_EQ(first_record.half_extent_y_pm,
            static_cast<float>(second_record.half_extent_y_pm));
  EXPECT_EQ(first_record.half_extent_z_pm,
            static_cast<float>(second_record.half_extent_z_pm));
  EXPECT_EQ(first_record.half_extent_x_pm,
            static_cast<float>(30'000'000'000ULL));
  auto const first_pose = first.BuildBoxPoseRecord();
  auto const second_pose = second.BuildBoxPoseRecord();
  ASSERT_TRUE(first_pose.has_value());
  ASSERT_TRUE(second_pose.has_value());
  EXPECT_EQ(first_pose->center_x_pm,
            static_cast<float>(second_pose->center_x_pm));
  EXPECT_EQ(first_pose->center_y_pm,
            static_cast<float>(second_pose->center_y_pm));
  EXPECT_EQ(first_pose->center_z_pm,
            static_cast<float>(second_pose->center_z_pm));
  EXPECT_EQ(first_record.volume_id, 1U);
  EXPECT_EQ(first_record.material_id, 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AddVolumeRejectsIncompleteBoxesAndOwnsMultipleOccurrences) {
  GGEMSWorld world = MakeWorld(200_mm, 200_mm, 200_mm, Vacuum());

  // Incomplete Boxes are rejected at registration and leave the World empty.
  GGEMSBox no_size{};
  no_size.SetMaterial(Water());
  EXPECT_THROW(world.AddVolume(no_size), GGEMSRecoverable);
  GGEMSBox no_material{};
  no_material.SetSize(40_mm, 20_mm, 30_mm);
  EXPECT_THROW(world.AddVolume(no_material), GGEMSRecoverable);
  EXPECT_TRUE(world.GetBoxes().empty());

  world.AddVolume(MakeBox());
  auto second = MakeBox();
  second.SetPosition(MakePositionPM(-60'000'000'000LL, 40'000'000'000LL, 0LL));
  EXPECT_NO_THROW(world.AddVolume(second));
  EXPECT_EQ(world.GetVolumeCount(), 2U);
  EXPECT_EQ(world.GetBoxes().size(), 1U);
  EXPECT_EQ(world.GetBoxes(1).size(), 1U);
  EXPECT_NO_THROW(world.Validate());
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, ValidateRejectsVolumesThatEscapeTheWorld) {
  // Box whose +X face coincides with the World +X face.
  {
    GGEMSWorld world = MakeWorld(200_mm, 200_mm, 200_mm, Vacuum());
    GGEMSBox box{};
    box.SetSize(40_mm, 20_mm, 30_mm)
      .SetPosition(MakePositionPM(80'000'000'000LL, 0LL, 0LL))
      .SetMaterial(Water());
    world.AddVolume(box);
    EXPECT_THROW(world.Validate(), GGEMSRecoverable);
  }
  // The World size set after the volume still decides containment.
  {
    GGEMSWorld world{};
    world.SetMaterial(Vacuum()).AddVolume(MakeBox());
    world.SetSize(60_mm, 60_mm, 60_mm);
    EXPECT_THROW(world.Validate(), GGEMSRecoverable);
    world.SetSize(200_mm, 200_mm, 200_mm);
    EXPECT_NO_THROW(world.Validate());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, CompilesExactMaterialIdentitiesWithTheMaterialsAuthority) {
  // The scene compiler interns World then Box media; the Box receives the
  // dense identity 1 for a distinct Material and 0 for the World's own one.
  auto const compile = [](GGEMSMaterial box_material) -> GGEMSBoxRecord {
    GGEMSWorld world = MakeWorld(200_mm, 200_mm, 200_mm, Vacuum());
    GGEMSBox box = MakeBox();
    box.SetMaterial(std::move(box_material));
    world.AddVolume(box);
    world.Validate();
    std::vector<GGEMSMaterial> const scene_materials{
      world.GetMaterial(), world.GetBoxes()[0U].GetMaterial()};
    GGEMSEMMaterialPackage const package{scene_materials};
    return world.BuildBoxRecords(package.GetMaterialIds().subspan(1U))[0U];
  };

  GGEMSBoxRecord const water = compile(Water());
  EXPECT_EQ(water.volume_id, 1U);
  EXPECT_EQ(water.material_id, 1U);

  GGEMSBoxRecord const vacuum = compile(Vacuum());
  EXPECT_EQ(vacuum.volume_id, 1U);
  EXPECT_EQ(vacuum.material_id, 0U);

  EXPECT_EQ(water.half_extent_x_pm,
            static_cast<float>(vacuum.half_extent_x_pm));
  EXPECT_EQ(water.half_extent_z_pm,
            static_cast<float>(vacuum.half_extent_z_pm));
  EXPECT_EQ(water.half_extent_x_pm, static_cast<float>(30'000'000'000ULL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, CopiesOwnTheirMaterialAndVolumes) {
  GGEMSWorld original = MakeWorld(200_mm, 200_mm, 200_mm, Vacuum());
  GGEMSWorld const copy_before = original;

  GGEMSBox box = MakeBox();
  original.AddVolume(box).SetMaterial(Water());
  // Later edits of the caller's Box do not reach the registered copy.
  box.SetMaterial(Vacuum()).SetSize(2_pm, 2_pm, 2_pm);
  GGEMSWorld const copy_after = original;
  original.SetMaterial(Vacuum());

  EXPECT_TRUE(copy_before.GetBoxes().empty());
  EXPECT_EQ(copy_before.GetMaterial().GetName(), "Vacuum");

  ASSERT_EQ(copy_after.GetBoxes().size(), 1U);
  EXPECT_EQ(copy_after.GetMaterial().GetName(), "Water");
  EXPECT_EQ(copy_after.GetBoxes()[0U].GetMaterial().GetName(), "Water");
  EXPECT_EQ(copy_after.GetBoxes()[0U].GetHalfExtentPM(),
            MakeBox().GetHalfExtentPM());

  EXPECT_EQ(original.GetMaterial().GetName(), "Vacuum");
  EXPECT_EQ(original.GetBoxes().size(), 1U);
  EXPECT_EQ(original.GetBoxes()[0U].GetMaterial().GetName(), "Water");
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AutomaticToleranceAdaptsToMedicalWorldAndSolidScales) {
  std::array const sizes{100_mm, 1_m, 3_m};
  std::array const expected_world_pm{32768.0F, 262144.0F, 1048576.0F};
  for (std::size_t index = 0U; index < sizes.size(); ++index) {
    SCOPED_TRACE(index);
    auto world = MakeWorld(sizes[index], sizes[index], sizes[index], Vacuum());
    ggems::geometry::GGEMSSphere sphere;
    sphere.SetRadius(1_mm).SetMaterial(Water());
    world.AddVolume(sphere);
    ASSERT_NO_THROW(world.Validate());
    auto const record = world.BuildRecord();
    EXPECT_EQ(record.tolerance_pm, expected_world_pm[index]);
    EXPECT_EQ(world.GetNavigationTolerancePM(), record.tolerance_pm);
    EXPECT_TRUE(std::isnormal(record.tolerance_pm));
    EXPECT_GT(record.half_extent_x_pm + record.tolerance_pm,
              record.half_extent_x_pm);
    EXPECT_LT(record.half_extent_x_pm - record.tolerance_pm,
              record.half_extent_x_pm);
    auto const solid = world.BuildAnalyticVolumeRecord(7U).parameters.sphere;
    EXPECT_EQ(solid.tolerance_pm, expected_world_pm[index]);
    EXPECT_EQ(solid.volume_id, 1U);
    EXPECT_EQ(solid.material_id, 7U);
    EXPECT_EQ(world.GetVolumeNavigationTolerancePM(), solid.tolerance_pm);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld,
     RetainsResolvedSmallFeaturesAndRejectsRemoteParameterCollapse) {
  auto world = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  GGEMSBox box;
  box.SetSize(20_um, 20_um, 20_um).SetMaterial(Water());
  world.AddVolume(box);
  ASSERT_NO_THROW(world.Validate());
  auto const solid = world.BuildAnalyticVolumeRecord(5U).parameters.box;
  EXPECT_EQ(solid.tolerance_pm, 1048576.0F);
  EXPECT_GT(solid.half_extent_x_pm, 8.0F * solid.tolerance_pm);
  EXPECT_EQ(solid.volume_id, 1U);
  EXPECT_EQ(solid.material_id, 5U);

  box.SetSize(1_um, 20_um, 20_um);
  auto unresolved = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  unresolved.AddVolume(box);
  EXPECT_THROW(unresolved.Validate(), GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, LocalSurfaceEnvelopeCanExceedTheWorldBaseline) {
  auto centered = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  GGEMSBox box;
  box.SetSize(20_um, 20_um, 20_um).SetMaterial(Water());
  centered.AddVolume(box);
  ASSERT_NO_THROW(centered.Validate());
  box.SetPosition(MakePositionPM(1'300'000'000'000LL, 0, 0));
  auto shifted = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  shifted.AddVolume(box);
  ASSERT_NO_THROW(shifted.Validate());
  EXPECT_EQ(centered.GetVolumeNavigationTolerancePM(), 1048576.0F);
  EXPECT_EQ(shifted.GetVolumeNavigationTolerancePM(), 2097152.0F);
  EXPECT_GT(shifted.GetVolumeNavigationTolerancePM(),
            shifted.GetNavigationTolerancePM());
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RotatedBoxIncludesTheRemoteWorldQueryResolution) {
  auto world = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  GGEMSBox box;
  box.SetSize(20_um, 20_um, 20_um)
    .SetRotation(13_deg, -27_deg, 41_deg)
    .SetMaterial(Water());
  world.AddVolume(box);
  ASSERT_NO_THROW(world.Validate());
  EXPECT_EQ(world.GetVolumeNavigationTolerancePM(),
            world.GetNavigationTolerancePM());
  EXPECT_EQ(world.BuildAnalyticVolumeRecord(3U).parameters.box.tolerance_pm,
            1048576.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RejectsUnresolvedPhysicalFeatures) {
  auto world = MakeWorld(3_m, 3_m, 3_m, Vacuum());
  ggems::geometry::GGEMSSphere sphere;
  sphere.SetRadius(1_um).SetMaterial(Water());
  world.AddVolume(sphere);

  EXPECT_THROW(world.Validate(), GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RequiresDistinctWorldAndSolidToleranceBandsAfterPacking) {
  for (auto const gap : {1_um, 10_um}) {
    SCOPED_TRACE(gap.value);
    auto world = MakeWorld(3_m, 3_m, 3_m, Vacuum());
    ggems::geometry::GGEMSSphere sphere;
    sphere.SetRadius(1_mm)
      .SetPosition(MakePositionPM(1'500'000'000'000LL - 1'000'000'000LL -
                                    static_cast<std::int64_t>(gap.value),
                                  0, 0))
      .SetMaterial(Water());
    world.AddVolume(sphere);
    if (gap == 1_um) {
      EXPECT_THROW(world.Validate(), GGEMSRecoverable);
    } else {
      EXPECT_NO_THROW(world.Validate());
      EXPECT_GT(world.GetVolumeNavigationTolerancePM(),
                world.GetNavigationTolerancePM());
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, ScalarWorldRecordCarriesTheToleranceWithoutParticleState) {
  using ggems::geometry::GGEMSWorldRecord;
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSWorldRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSWorldRecord>);
  EXPECT_EQ(sizeof(GGEMSWorldRecord), 24U);
  EXPECT_EQ(alignof(GGEMSWorldRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSWorldRecord, tolerance_pm), 12U);
  EXPECT_EQ(offsetof(GGEMSWorldRecord, occurrence_count), 16U);
  EXPECT_EQ(offsetof(GGEMSWorldRecord, max_boundary_events), 20U);
  GGEMSWorld const empty;
  EXPECT_EQ(empty.BuildRecord().tolerance_pm, 0.0F);
  EXPECT_EQ(empty.GetVolumeNavigationTolerancePM(), 0.0F);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RotatedBoxClearanceProjectsTheLocalSurfaceBand) {
  for (auto const gap_pm : {3'500'000LL, 5'000'000LL}) {
    SCOPED_TRACE(gap_pm);
    auto world = MakeWorld(3_m, 3_m, 3_m, Vacuum());
    GGEMSBox box;
    box.SetSize(2_mm, 2_mm, 2_mm)
      .SetRotation(0_deg, 0_deg, 45_deg)
      .SetMaterial(Water());
    // The local Cartesian band expands X by sqrt(2) times its width.
    auto const extent_pm = static_cast<std::int64_t>(
      std::ceil(std::numbers::sqrt2 * 1'000'000'000.0));
    box.SetPosition(
      MakePositionPM(1'500'000'000'000LL - extent_pm - gap_pm, 0, 0));
    world.AddVolume(box);
    if (gap_pm == 3'500'000LL) {
      EXPECT_THROW(world.Validate(), GGEMSRecoverable);
    } else {
      EXPECT_NO_THROW(world.Validate());
    }
  }
}
