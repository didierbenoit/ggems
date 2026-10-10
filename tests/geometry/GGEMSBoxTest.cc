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
 * \brief Validates analytic Box authoring, containment and device packing.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <limits>

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
using ggems::geometry::GGEMSBox;
using ggems::geometry::GGEMSWorld;
using ggems::geometry::MakePositionPM;
using namespace ggems::units;

auto Vacuum() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum");
}

auto Water() -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial("Water");
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBox, AcceptsAnyPositiveIntegerSizesAndOffsetPlacement) {
  EXPECT_THROW(GGEMSBox(0_pm, 2_pm, 2_pm, MakePositionPM(0, 0, 0), Water()),
               GGEMSRecoverable);

  // Odd and one-picometer sizes are admitted: faces stay on the lattice.
  GGEMSBox const thin{1_pm, 23_pm, 3_pm, MakePositionPM(-11, -13, -17),
                      Water()};
  EXPECT_EQ(thin.GetLowerCornerPM(), MakePositionPM(-11, -13, -17));
  EXPECT_EQ(thin.GetUpperCornerPM(), MakePositionPM(-10, 10, -14));

  GGEMSBox const box{
    60_mm, 20_mm, 30_mm,
    MakePositionPM(-20'000'000'000, -30'000'000'000, -10'000'000'000), Water()};
  EXPECT_EQ(box.GetUpperCornerPM(),
            MakePositionPM(40'000'000'000, -10'000'000'000, 20'000'000'000));
  EXPECT_EQ(box.GetMaterial().GetName(), "Water");
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, StrictContainmentRejectsCoincidentWorldFaces) {
  GGEMSWorld const world{200_mm, 200_mm, 200_mm, Vacuum()};

  EXPECT_TRUE(GGEMSBox(40_mm, 20_mm, 30_mm, MakePositionPM(0, 0, 0), Water())
                .IsStrictlyInside(world));
  // Upper X face one picometer inside the World face is still strictly inside.
  EXPECT_TRUE(GGEMSBox(40_mm, 20_mm, 30_mm,
                       MakePositionPM(60'000'000'000 - 1, 0, 0), Water())
                .IsStrictlyInside(world));
  // Coincident face.
  EXPECT_FALSE(
    GGEMSBox(40_mm, 20_mm, 30_mm, MakePositionPM(60'000'000'000, 0, 0), Water())
      .IsStrictlyInside(world));
  // Crossing face.
  EXPECT_FALSE(GGEMSBox(40_mm, 20_mm, 30_mm,
                        MakePositionPM(0, 0, -110'000'000'000), Water())
                 .IsStrictlyInside(world));
  // Same size as the World.
  EXPECT_FALSE(GGEMSBox(200_mm, 200_mm, 200_mm,
                        MakePositionPM(-100'000'000'000, -100'000'000'000,
                                       -100'000'000'000),
                        Water())
                 .IsStrictlyInside(world));
}

// =============================================================================
// =============================================================================

TEST(GGEMSBox, BuildsTheDeviceRecordWithSnapshotIdentities) {
  GGEMSBox const box{
    60_mm, 20_mm, 30_mm,
    MakePositionPM(-20'000'000'000, -30'000'000'000, -10'000'000'000), Water()};
  ggems::geometry::GGEMSBoxRecord const record = box.BuildRecord(1U, 1U);

  EXPECT_EQ(record.lower_x_pm, -20'000'000'000);
  EXPECT_EQ(record.lower_y_pm, -30'000'000'000);
  EXPECT_EQ(record.lower_z_pm, -10'000'000'000);
  EXPECT_EQ(record.upper_x_pm, 40'000'000'000);
  EXPECT_EQ(record.upper_y_pm, -10'000'000'000);
  EXPECT_EQ(record.upper_z_pm, 20'000'000'000);
  EXPECT_EQ(record.volume_id, 1U);
  EXPECT_EQ(record.material_id, 1U);
}
