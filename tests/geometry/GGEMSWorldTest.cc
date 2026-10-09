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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using namespace ggems::units;
using ggems::geometry::GGEMSWorld;
using ggems::geometry::MakePositionPM;

static_assert(!std::is_default_constructible_v<GGEMSWorld>);
static_assert(std::is_copy_constructible_v<GGEMSWorld>);

/*! \brief Exact empty-matter World medium used by the geometry fixtures. */
auto Vacuum() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum");
}

/*! \brief Largest even full size admitted by the World. */
constexpr std::uint64_t k_max_size_pm{0x7FFF'FFFF'FFFF'FFFEULL};

/*! \brief Half extent of the largest admitted World. */
constexpr std::int64_t k_max_half_pm{0x3FFF'FFFF'FFFF'FFFFLL};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, StoresExactHalfExtentsAndCornersFromUserUnits) {
  GGEMSWorld const world{2_m, 400_mm, 6_cm, Vacuum()};

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
  EXPECT_THROW((GGEMSWorld{0_pm, 2_pm, 2_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{2_pm, 0_pm, 2_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{2_pm, 2_pm, 0_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, RejectsOddPicometerSizeOnEveryAxis) {
  EXPECT_THROW((GGEMSWorld{1_pm, 2_pm, 2_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{2_pm, 3_pm, 2_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{2_pm, 2_pm, 1'000'000'001_pm, Vacuum()}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AcceptsSmallestWorld) {
  GGEMSWorld const world{2_pm, 2_pm, 2_pm, Vacuum()};

  EXPECT_EQ(world.GetHalfExtentPM(),
            (ggems::geometry::HalfExtent3PM{.x = 1ULL, .y = 1ULL, .z = 1ULL}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, AcceptsLargestWorldAndRejectsLargerSizes) {
  Length const max_size{k_max_size_pm};
  Length const above_max_even{0x8000'0000'0000'0000ULL};
  Length const largest_even{std::numeric_limits<std::uint64_t>::max() - 1ULL};

  GGEMSWorld const world{max_size, max_size, max_size, Vacuum()};

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

  EXPECT_THROW((GGEMSWorld{above_max_even, max_size, max_size, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{max_size, above_max_even, max_size, Vacuum()}),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW((GGEMSWorld{max_size, max_size, largest_even, Vacuum()}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, ContainsClosedBoxPositionsForStaticInputValidation) {
  // Asymmetric World so that each axis is checked against its own half extent.
  GGEMSWorld const world{20_pm, 40_pm, 60_pm, Vacuum()};

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
  GGEMSWorld const largest{max_size, max_size, max_size, Vacuum()};
  EXPECT_TRUE(largest.Contains(
    MakePositionPM(-k_max_half_pm, k_max_half_pm, -k_max_half_pm)));
  EXPECT_FALSE(largest.Contains(MakePositionPM(0, -k_max_half_pm - 1, 0)));
  EXPECT_FALSE(largest.Contains(MakePositionPM(k_min, 0, 0)));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, OwnsExplicitMaterialIncludingExactVacuum) {
  GGEMSWorld const vacuum_world{2_m, 2_m, 2_m, Vacuum()};
  EXPECT_EQ(vacuum_world.GetMaterial().GetName(), "Vacuum");
  EXPECT_EQ(vacuum_world.GetMaterial().GetDensity().value, 0.0L);
  EXPECT_TRUE(vacuum_world.GetMaterial().GetElementalConstituents().empty());

  auto water = ggems::core::materials::builtins::BuildBuiltInMaterial("Water");
  auto const expected_density = water.GetDensity();
  auto const expected_constituents = water.GetElementalConstituents().size();

  GGEMSWorld const water_world{1_m, 2_m, 3_m, std::move(water)};
  EXPECT_EQ(water_world.GetMaterial().GetName(), "Water");
  EXPECT_EQ(water_world.GetMaterial().GetDensity(), expected_density);
  EXPECT_EQ(water_world.GetMaterial().GetElementalConstituents().size(),
            expected_constituents);
  EXPECT_TRUE(ggems::core::materials::HasSameScientificIdentity(
    water_world.GetMaterial(),
    ggems::core::materials::builtins::BuildBuiltInMaterial("Water")));
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, PreservesExactFacesBeyondBinary64IntegerPrecision) {
  // Half extent 2^53 + 1 pm is not representable in binary64; authored as the
  // full size 2^54 + 2 pm. Exact one-picometer distinctions must survive.
  constexpr std::int64_t k_half_pm{(1LL << 53) + 1LL};
  Length const size{static_cast<std::uint64_t>(k_half_pm) * 2ULL};

  GGEMSWorld const world{size, size, size, Vacuum()};
  EXPECT_EQ(world.GetHalfExtentPM().x, static_cast<std::uint64_t>(k_half_pm));

  EXPECT_EQ(world.BuildRecord().half_extent_x_pm, k_half_pm);
  EXPECT_EQ(world.GetLowerCornerPM().x, -k_half_pm);
  EXPECT_EQ(world.GetUpperCornerPM().x, k_half_pm);
}

// =============================================================================
// =============================================================================

TEST(GGEMSWorld, OwnedMaterialOutlivesTheCallerMaterial) {
  auto const build = []() -> GGEMSWorld {
    auto air = ggems::core::materials::builtins::BuildBuiltInMaterial("Air");
    return GGEMSWorld{1_m, 1_m, 1_m, air};
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

TEST(GGEMSWorld, RejectedSizeLeavesNoWorldAndCallerMaterialUsable) {
  auto const water =
    ggems::core::materials::builtins::BuildBuiltInMaterial("Water");

  EXPECT_THROW((GGEMSWorld{0_pm, 2_pm, 2_pm, water}),
               ggems::core::GGEMSRecoverable);

  // The caller-owned lvalue Material is untouched by the failed construction.
  EXPECT_EQ(water.GetName(), "Water");
  GGEMSWorld const world{2_pm, 2_pm, 2_pm, water};
  EXPECT_EQ(world.GetMaterial().GetName(), "Water");
}
