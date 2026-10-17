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
 * \brief Tests the center-placed analytic Box authoring and its World
 * compilation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using ggems::core::GGEMSRecoverable;
using ggems::geometry::CoordinatePM;
using ggems::geometry::GGEMSBox;
using ggems::geometry::GGEMSBoxRecord;
using ggems::geometry::GGEMSWorld;
using ggems::geometry::HalfExtent3PM;
using ggems::geometry::MakePositionPM;
using namespace ggems::units;

static_assert(std::is_default_constructible_v<GGEMSBox>);
static_assert(std::is_copy_constructible_v<GGEMSBox>);

auto Vacuum() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum");
}

auto Water() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Water");
}

/*! \brief Cubic Vacuum World of the given full size. */
auto MakeWorld(Length size) -> GGEMSWorld {
  GGEMSWorld world{};
  world.SetSize(size, size, size).SetMaterial(Vacuum());
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

/*! \brief Water Box of the given full sizes centered at a global point. */
auto PlaceBox(Length size_x, Length size_y, Length size_z,
              ggems::geometry::Position3PM center) -> GGEMSBox {
  GGEMSBox box{};
  box.SetSize(size_x, size_y, size_z).SetPosition(center).SetMaterial(Water());
  return box;
}

/*! \brief Whether the Box is admitted as the single volume of the World. */
auto Fits(GGEMSWorld world, GGEMSBox const &box) -> bool {
  world.AddVolume(box);
  try {
    world.Validate();
  } catch (GGEMSRecoverable const &) {
    return false;
  }
  return true;
}

/*! \brief Compiles the Box as the single volume of the World, Material 1. */
auto Compile(GGEMSWorld world, GGEMSBox const &box) -> GGEMSBoxRecord {
  world.AddVolume(box);
  world.Validate();
  std::array<std::uint32_t, 1U> const material_ids{1U};
  return world.BuildBoxRecords(material_ids)[0U];
}

/*! \brief Half extent of the largest admitted World. */
constexpr std::int64_t k_max_half_pm{0x3FFF'FFFF'FFFF'FFFFLL};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBox, StoresCenterAndHalfExtentsFromUserUnits) {
  GGEMSBox const box = MakeBox();
  EXPECT_EQ(
    box.GetPositionPM(),
    MakePositionPM(10'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL));
  EXPECT_EQ(box.GetHalfExtentPM(), (HalfExtent3PM{.x = 30'000'000'000ULL,
                                                  .y = 10'000'000'000ULL,
                                                  .z = 15'000'000'000ULL}));
  EXPECT_EQ(box.GetMaterial().GetName(), "Water");

  // A 13 cm side is 130 000 000 000 pm with the exact half extent
  // 65 000 000 000 pm; the center defaults to the origin.
  GGEMSBox thirteen{};
  thirteen.SetSize(13_cm, 13_cm, 13_cm);
  EXPECT_EQ(thirteen.GetPositionPM(), MakePositionPM(0, 0, 0));
  EXPECT_EQ(thirteen.GetHalfExtentPM(),
            (HalfExtent3PM{.x = 65'000'000'000ULL,
                           .y = 65'000'000'000ULL,
                           .z = 65'000'000'000ULL}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, AdmitsTheSmallestEvenSideAndRejectsZeroOrOddSides) {
  GGEMSBox box{};
  box.SetSize(2_pm, 2_pm, 2_pm);
  EXPECT_EQ(box.GetHalfExtentPM(),
            (HalfExtent3PM{.x = 1ULL, .y = 1ULL, .z = 1ULL}));

  EXPECT_THROW(box.SetSize(0_pm, 2_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(box.SetSize(1_pm, 2_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(box.SetSize(2_pm, 3_pm, 2_pm), GGEMSRecoverable);
  EXPECT_THROW(box.SetSize(2_pm, 2_pm, 1'000'000'001_pm), GGEMSRecoverable);

  // A rejected size keeps the previous size.
  EXPECT_EQ(box.GetHalfExtentPM(),
            (HalfExtent3PM{.x = 1ULL, .y = 1ULL, .z = 1ULL}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, CompilesPositiveLocalHalfExtentsForEveryPlacement) {
  GGEMSBoxRecord const record = Compile(MakeWorld(200_mm), MakeBox());
  EXPECT_EQ(record.half_extent_x_pm, 30'000'000'000ULL);
  EXPECT_EQ(record.half_extent_y_pm, 10'000'000'000ULL);
  EXPECT_EQ(record.half_extent_z_pm, 15'000'000'000ULL);
  EXPECT_EQ(record.volume_id, 1U);
  EXPECT_EQ(record.material_id, 1U);

  // 13 cm cube at the origin of a 2 m World.
  GGEMSBoxRecord const thirteen = Compile(
    MakeWorld(2_m), PlaceBox(13_cm, 13_cm, 13_cm, MakePositionPM(0, 0, 0)));
  EXPECT_EQ(thirteen.half_extent_x_pm, 65'000'000'000ULL);
  EXPECT_EQ(thirteen.half_extent_z_pm, 65'000'000'000ULL);

  // Smallest Box centered in the smallest World that can contain it: one
  // picometer of clearance from every World face.
  GGEMSBoxRecord const tiny = Compile(
    MakeWorld(4_pm), PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(0, 0, 0)));
  EXPECT_EQ(tiny.half_extent_x_pm, 1ULL);
  EXPECT_EQ(tiny.half_extent_z_pm, 1ULL);
  EXPECT_FALSE(
    Fits(MakeWorld(2_pm), PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(0, 0, 0))));

  // Smallest Box at an odd center: faces one picometer on each side.
  GGEMSBoxRecord const smallest =
    Compile(MakeWorld(200_mm),
            PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(-11, -13, -17)));
  EXPECT_EQ(smallest.half_extent_x_pm, 1ULL);
  EXPECT_EQ(smallest.half_extent_y_pm, 1ULL);
  EXPECT_EQ(smallest.half_extent_z_pm, 1ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, StrictContainmentRejectsCoincidentWorldFaces) {
  GGEMSWorld const world = MakeWorld(200_mm);
  auto const place = [](CoordinatePM center_x, CoordinatePM center_y,
                        CoordinatePM center_z) -> GGEMSBox {
    return PlaceBox(40_mm, 20_mm, 30_mm,
                    MakePositionPM(center_x, center_y, center_z));
  };

  EXPECT_TRUE(Fits(world, place(0, 0, 0)));
  // Upper X face one picometer inside the World face is still strictly inside.
  EXPECT_TRUE(Fits(world, place(80'000'000'000LL - 1, 0, 0)));
  // Coincident +X face.
  EXPECT_FALSE(Fits(world, place(80'000'000'000LL, 0, 0)));
  // Coincident -Y face.
  EXPECT_FALSE(Fits(world, place(0, -90'000'000'000LL, 0)));
  // Crossing -Z face.
  EXPECT_FALSE(Fits(world, place(0, 0, -95'000'000'000LL)));
  // Same size as the World, and larger than the World.
  EXPECT_FALSE(
    Fits(world, PlaceBox(200_mm, 200_mm, 200_mm, MakePositionPM(0, 0, 0))));
  EXPECT_FALSE(
    Fits(world, PlaceBox(40_mm, 202_mm, 30_mm, MakePositionPM(0, 0, 0))));
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, PacksCanonicalHalfExtentsAtTheAdmittedBounds) {
  GGEMSWorld const largest =
    MakeWorld(Length{ggems::geometry::k_max_world_size_pm});
  constexpr std::int64_t k_min = std::numeric_limits<std::int64_t>::min();
  constexpr std::int64_t k_max = std::numeric_limits<std::int64_t>::max();

  // Smallest Box with one picometer of clearance from two World faces.
  GGEMSBoxRecord const edge = Compile(
    largest,
    PlaceBox(2_pm, 2_pm, 2_pm,
             MakePositionPM(k_max_half_pm - 2, -(k_max_half_pm - 2), 0)));
  EXPECT_EQ(edge.half_extent_x_pm, 1ULL);
  EXPECT_EQ(edge.half_extent_y_pm, 1ULL);
  EXPECT_EQ(edge.half_extent_z_pm, 1ULL);

  // Extreme signed centers are rejected without forming any face sum.
  EXPECT_FALSE(
    Fits(largest, PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(k_max, 0, 0))));
  EXPECT_FALSE(
    Fits(largest, PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(0, k_min, 0))));
  EXPECT_FALSE(
    Fits(largest,
         PlaceBox(2_pm, 2_pm, 2_pm, MakePositionPM(0, 0, k_max_half_pm - 1))));

  // A Box as wide as, or wider than, the largest World is rejected.
  EXPECT_FALSE(
    Fits(largest, PlaceBox(Length{ggems::geometry::k_max_world_size_pm}, 2_pm,
                           2_pm, MakePositionPM(0, 0, 0))));
  EXPECT_FALSE(
    Fits(largest,
         PlaceBox(2_pm, Length{std::numeric_limits<std::uint64_t>::max() - 1},
                  2_pm, MakePositionPM(0, 0, 0))));
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, SettersApplyInAnyOrderAndValidateRequiresSizeAndMaterial) {
  GGEMSBox box{};
  EXPECT_THROW(box.Validate(), GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(box.GetMaterial()), GGEMSRecoverable);

  box.SetMaterial(Vacuum());
  EXPECT_THROW(box.Validate(), GGEMSRecoverable);

  box
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL))
    .SetSize(60_mm, 20_mm, 30_mm);
  EXPECT_NO_THROW(box.Validate());

  GGEMSBox const reference = MakeBox();
  EXPECT_EQ(box.GetPositionPM(), reference.GetPositionPM());
  EXPECT_EQ(box.GetHalfExtentPM(), reference.GetHalfExtentPM());
  EXPECT_EQ(box.GetMaterial().GetName(), "Vacuum");
  EXPECT_EQ(reference.GetMaterial().GetName(), "Water");

  GGEMSBox sized{};
  sized.SetSize(2_pm, 2_pm, 2_pm);
  EXPECT_THROW(sized.Validate(), GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, ScalarHostLayoutMatchesTheDocumentedDeviceABI) {
  EXPECT_TRUE(std::is_trivially_copyable_v<GGEMSBoxRecord>);
  EXPECT_TRUE(std::is_standard_layout_v<GGEMSBoxRecord>);
  EXPECT_EQ(sizeof(GGEMSBoxRecord), 32U);
  EXPECT_EQ(alignof(GGEMSBoxRecord), 8U);
  EXPECT_EQ(offsetof(GGEMSBoxRecord, half_extent_x_pm), 0U);
  EXPECT_EQ(offsetof(GGEMSBoxRecord, half_extent_y_pm), 8U);
  EXPECT_EQ(offsetof(GGEMSBoxRecord, half_extent_z_pm), 16U);
  EXPECT_EQ(offsetof(GGEMSBoxRecord, volume_id), 24U);
  EXPECT_EQ(offsetof(GGEMSBoxRecord, material_id), 28U);
}
