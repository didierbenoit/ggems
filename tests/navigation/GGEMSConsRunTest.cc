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
 * \brief Tests copied Cons snapshots through both production observer modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSNavigationTestDevices.hh"

using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;

TEST_F(GGEMSNavigationDeviceTest, N1ConsRunOwnsSnapshotWithObserverModes) {
  for (bool observed : {false, true}) {
    SCOPED_TRACE(observed);
    auto source = std::make_shared<sources::GGEMSSource>();
    source->SetAnalytic()
      .SetPrimaryCount(8U)
      .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
      .SetPositionPicoMeter(-15'000'000'000LL, 5'000'000'000LL,
                            -3'000'000'000LL)
      .SetDirection(1, 0, 0);
    auto random = std::make_shared<random::GGEMSRandom>();
    random->SetSeed(3U);
    auto observer = std::make_shared<observer::GGEMSTransportObserver>();
    observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);
    GGEMSRun run;
    run.SetRandom(random);
    run.AddSource(source);
    if (observed) {
      run.SetObserver(observer);
    }
    geometry::GGEMSCons shape;
    shape.SetNegativeZRadii(1_mm, 4_mm)
      .SetPositiveZRadii(2_mm, 6_mm)
      .SetLength(12_mm)
      .SetPosition(geometry::MakePositionPM(-7'000'000'000LL, 5'000'000'000LL,
                                            -3'000'000'000LL))
      .SetRotation(0_deg, 30_deg, 0_deg)
      .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
    geometry::GGEMSWorld world;
    world.SetSize(80_mm, 80_mm, 80_mm)
      .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
    world.AddVolume(shape);
    run.SetWorld(world);
    world.SetSize(160_mm, 160_mm, 160_mm);
    ASSERT_NO_THROW(run.Initialize());
    ASSERT_NO_THROW(run.Run());
    ASSERT_NO_THROW(run.Run());
    if (observed) {
      auto const records = observer->GetRecords();
      ASSERT_EQ(records.size(), 2U);
      EXPECT_EQ(records[1].status,
                particles::ToKernelParticleStatus(
                  particles::GGEMSParticleStatus::EscapedWorld));
      EXPECT_LE(std::abs(static_cast<double>(records[1].position_x_pm) - 40e9),
                8 * world.GetNavigationTolerancePM());
      EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
    }
  }
}
