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
 * \brief Tests variable Polyhedra uploads through both production observer
 * modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
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

TEST_F(GGEMSNavigationDeviceTest, N1PolyhedraRunUploadsVariableAuxiliaryData) {
  for (bool observed : {false, true}) {
    for (std::int64_t sections : {1, 4}) {
      SCOPED_TRACE(observed);
      SCOPED_TRACE(sections);
      auto source = std::make_shared<sources::GGEMSSource>();
      source->SetAnalytic()
        .SetPrimaryCount(8U)
        .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
        .SetPositionPicoMeter(-15'000'000'000LL, 5'000'000'000LL,
                              -7'500'000'000LL)
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

      std::vector<PositionCoordinate> z_planes;
      std::vector<Length> inner;
      std::vector<Length> outer;
      for (std::int64_t index = 0; index <= sections; ++index) {
        z_planes.emplace_back(-6'000'000'000LL +
                              (index * (12'000'000'000LL / sections)));
        inner.push_back(1_mm);
        outer.push_back(index % 2 == 0 ? 4_mm : 6_mm);
      }
      geometry::GGEMSPolyhedra shape;
      shape.SetSideCount(6U)
        .SetStartAngle(-30_deg)
        .SetZProfile(z_planes, inner, outer)
        .SetPosition(geometry::MakePositionPM(-7'000'000'000LL, 5'000'000'000LL,
                                              -3'000'000'000LL))
        .SetRotation(0_deg, 90_deg, 0_deg)
        .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
      geometry::GGEMSWorld world;
      world.SetSize(80_mm, 80_mm, 80_mm)
        .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
      world.AddVolume(shape);
      auto const tolerance = world.GetNavigationTolerancePM();
      ASSERT_GT(world.BuildAnalyticAuxiliaryData().size(),
                sizeof(geometry::GGEMSBoxPoseRecord));
      run.SetWorld(world);
      // The +X ray is local +Z at x=4.5 mm: four sections require two
      // disjoint material intervals. Only the copied profile may reach Run.
      shape.SetSideCount(3U);
      z_planes.clear();
      inner.clear();
      outer.clear();
      world.SetSize(160_mm, 160_mm, 160_mm);
      ASSERT_NO_THROW(run.Initialize());
      for (int repetition = 0; repetition < 2; ++repetition) {
        ASSERT_NO_THROW(run.Run());
        if (observed) {
          auto const &records = observer->GetRecords();
          ASSERT_EQ(records.size(), 2U);
          EXPECT_EQ(observer->GetCapturedPrimaryCount(), 1U);
          EXPECT_EQ(observer->GetOverflowCount(), 0U);
          EXPECT_EQ(records[1].status,
                    particles::ToKernelParticleStatus(
                      particles::GGEMSParticleStatus::EscapedWorld));
          EXPECT_LE(
            std::abs(static_cast<double>(records[1].position_x_pm) - 40e9),
            8 * tolerance);
          EXPECT_LE(
            std::abs(static_cast<double>(records[1].position_y_pm) - 5e9),
            8 * tolerance);
          EXPECT_LE(
            std::abs(static_cast<double>(records[1].position_z_pm) + 7.5e9),
            8 * tolerance);
          EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
          EXPECT_EQ(records[0].energy_micro_eV, records[1].energy_micro_eV);
          EXPECT_EQ(records[0].time_ps, records[1].time_ps);
          EXPECT_EQ(records[0].direction_x, records[1].direction_x);
          EXPECT_EQ(records[0].direction_y, records[1].direction_y);
          EXPECT_EQ(records[0].direction_z, records[1].direction_z);
        }
      }
    }
  }
}
