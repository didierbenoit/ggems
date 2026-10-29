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
 * \brief Checks root-first object attachment, copied geometry, and weak
 * registration lifetime through the private C++ binding helper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "detail/GGEMSGeometryAttachments.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::GGEMSRecoverable;
using ggems::core::materials::builtins::BuildBuiltInMaterial;
using ggems::python::detail::GGEMSGeometryAttachments;

auto MakeWorld() -> std::shared_ptr<GGEMSWorld> {
  auto world = std::make_shared<GGEMSWorld>();
  world->SetSize(200_mm, 200_mm, 200_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

template <typename Shape>
auto MakeShape(Length half) -> std::shared_ptr<Shape> {
  auto shape = std::make_shared<Shape>();
  Length const full{2U * half.value};
  if constexpr (std::is_same_v<Shape, GGEMSBox>) {
    shape->SetSize(full, full, full);
  } else if constexpr (std::is_same_v<Shape, GGEMSSphere>) {
    shape->SetRadius(half);
  } else if constexpr (std::is_same_v<Shape, GGEMSTubs>) {
    shape->SetRadii(half).SetLength(full);
  } else if constexpr (std::is_same_v<Shape, GGEMSEllipsoid>) {
    shape->SetSemiAxes(half, half, half);
  } else if constexpr (std::is_same_v<Shape, GGEMSTrd>) {
    shape->SetNegativeZFace(full, full)
      .SetPositiveZFace(full, full)
      .SetLength(full);
  } else if constexpr (std::is_same_v<Shape, GGEMSTrap>) {
    shape->SetNegativeZFace(full, full, full, 0_deg)
      .SetPositiveZFace(full, full, full, 0_deg)
      .SetLength(full);
  } else if constexpr (std::is_same_v<Shape, GGEMSCons>) {
    shape->SetNegativeZRadii(0_pm, half)
      .SetPositiveZRadii(0_pm, half)
      .SetLength(full);
  } else if constexpr (std::is_same_v<Shape, GGEMSPolyhedra>) {
    auto const extent = static_cast<std::int64_t>(half.value);
    std::array const z{
      PositionCoordinate{-extent},
      PositionCoordinate{0},
      PositionCoordinate{extent},
    };
    std::array const inner{0_pm, 0_pm, 0_pm};
    std::array const outer{half, half, half};
    shape->SetSideCount(6U).SetStartAngle(15_deg).SetZProfile(z, inner, outer);
  }
  shape->SetMaterial(BuildBuiltInMaterial("Water"));
  return shape;
}

auto ExpectSameCompilation(GGEMSWorld const &actual, GGEMSWorld const &expected)
  -> void {
  ASSERT_NO_THROW(actual.Validate());
  ASSERT_NO_THROW(expected.Validate());
  ASSERT_EQ(actual.GetVolumeCount(), expected.GetVolumeCount());
  auto const actual_world = actual.BuildRecord();
  auto const expected_world = expected.BuildRecord();
  EXPECT_EQ(std::memcmp(&actual_world, &expected_world, sizeof(actual_world)),
            0);
  EXPECT_EQ(actual.BuildAnalyticAuxiliaryBuffer(),
            expected.BuildAnalyticAuxiliaryBuffer());

  std::vector<std::uint32_t> material_ids(actual.GetVolumeCount());
  for (std::size_t index = 0U; index < material_ids.size(); ++index) {
    material_ids[index] = static_cast<std::uint32_t>(index) + 5U;
    EXPECT_EQ(actual.GetVolumeMaterial(index)->GetName(),
              expected.GetVolumeMaterial(index)->GetName());
  }
  auto const actual_records = actual.BuildAnalyticDeviceRecords(material_ids);
  auto const expected_records =
    expected.BuildAnalyticDeviceRecords(material_ids);
  ASSERT_EQ(actual_records.size(), expected_records.size());
  // Compare initialized active payloads, excluding unused union storage.
  std::array const payload_sizes{
    sizeof(GGEMSBoxRecord),       sizeof(GGEMSBoxRecord),
    sizeof(GGEMSTubsRecord),      sizeof(GGEMSSphereRecord),
    sizeof(GGEMSEllipsoidRecord), sizeof(GGEMSPlanarRecord),
    sizeof(GGEMSPlanarRecord),    sizeof(GGEMSConsRecord),
    sizeof(GGEMSPolyhedraRecord),
  };
  for (std::size_t index = 0U; index < actual_records.size(); ++index) {
    SCOPED_TRACE(index);
    auto const &record = actual_records[index];
    auto const &reference = expected_records[index];
    ASSERT_EQ(record.kind, reference.kind);
    EXPECT_EQ(std::memcmp(&record.parameters, &reference.parameters,
                          payload_sizes[static_cast<std::size_t>(record.kind)]),
              0);
  }
}

template <typename Mother, typename Child> auto CheckFamilyPair() -> void {
  GGEMSGeometryAttachments attachments;
  auto world = MakeWorld();
  auto mother = MakeShape<Mother>(20_mm);
  auto child = MakeShape<Child>(4_mm);
  mother
    ->SetPosition(
      MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 3'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 30_deg);
  child->SetPosition(MakePositionPM(2'000'000'000LL, 1'000'000'000LL, 0))
    .SetRotation(15_deg, 0_deg, -20_deg)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  auto expected = *world;
  expected.AddVolume(*mother).AddVolume(*child, 1U);
  attachments.AddVolume(world, mother);
  attachments.AddVolume(mother, child);
  ExpectSameCompilation(*world, expected);
}
} // namespace

TEST(GGEMSGeometryAttachments, AllFamiliesPreserveIndexedCompilation) {
  CheckFamilyPair<GGEMSBox, GGEMSSphere>();
  CheckFamilyPair<GGEMSSphere, GGEMSTubs>();
  CheckFamilyPair<GGEMSTubs, GGEMSEllipsoid>();
  CheckFamilyPair<GGEMSEllipsoid, GGEMSTrd>();
  CheckFamilyPair<GGEMSTrd, GGEMSTrap>();
  CheckFamilyPair<GGEMSTrap, GGEMSCons>();
  CheckFamilyPair<GGEMSCons, GGEMSPolyhedra>();
  CheckFamilyPair<GGEMSPolyhedra, GGEMSBox>();
}

TEST(GGEMSGeometryAttachments, ThreeLevelsRetainCopyAtAddAndWorldIndependence) {
  GGEMSGeometryAttachments attachments;
  auto world = MakeWorld();
  auto mother = MakeShape<GGEMSBox>(20_mm);
  auto daughter = MakeShape<GGEMSSphere>(8_mm);
  auto grandchild = MakeShape<GGEMSBox>(2_mm);
  attachments.AddVolume(world, mother);
  GGEMSWorld const before_daughters = *world;
  attachments.AddVolume(mother, daughter);
  attachments.AddVolume(daughter, grandchild);
  GGEMSWorld const copied_world = *world;

  mother->SetSize(2_mm, 2_mm, 2_mm)
    .SetPosition(MakePositionPM(50'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 45_deg)
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  daughter->SetRadius(1_mm).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  grandchild->SetSize(2_mm, 2_mm, 2_mm);
  ExpectSameCompilation(*world, copied_world);
  EXPECT_EQ(before_daughters.GetVolumeCount(), 1U);
  EXPECT_EQ(copied_world.BuildRecord().hierarchy_depth, 3U);
  auto const links = copied_world.BuildOccurrenceRecords();
  ASSERT_EQ(links.size(), 3U);
  EXPECT_EQ(links[0].parent_volume_id, 0U);
  EXPECT_EQ(links[1].parent_volume_id, 1U);
  EXPECT_EQ(links[2].parent_volume_id, 2U);
  EXPECT_EQ(copied_world.GetBoxes().front().GetHalfExtentPM().x,
            20'000'000'000ULL);
  EXPECT_FLOAT_EQ(
    copied_world.BuildAnalyticVolumeRecord(6U, 1U).parameters.sphere.radius_pm,
    8.0e9F);

  auto sibling = MakeShape<GGEMSBox>(2_mm);
  sibling->SetPosition(MakePositionPM(50'000'000'000LL, 0, 0));
  attachments.AddVolume(world, sibling);
  world->SetSize(300_mm, 300_mm, 300_mm);
  ASSERT_NO_THROW(world->Validate());
  EXPECT_EQ(world->GetVolumeCount(), 4U);
  EXPECT_EQ(copied_world.GetVolumeCount(), 3U);
  EXPECT_EQ(copied_world.GetHalfExtentPM().x, 100'000'000'000ULL);
}

TEST(GGEMSGeometryAttachments, RejectsInvalidAttachmentsWithoutConsumingChild) {
  GGEMSGeometryAttachments attachments;
  auto world = MakeWorld();
  auto other_world = MakeWorld();
  auto mother = MakeShape<GGEMSBox>(20_mm);
  auto child = MakeShape<GGEMSBox>(8_mm);
  auto grandchild = MakeShape<GGEMSBox>(2_mm);
  auto leaf = MakeShape<GGEMSBox>(1_mm);
  std::shared_ptr<GGEMSWorld> const null_world;
  std::shared_ptr<GGEMSBox> const null_shape;

  EXPECT_THROW(attachments.AddVolume(null_world, mother), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(world, null_shape), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(null_shape, child), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(mother, child), GGEMSRecoverable);
  EXPECT_EQ(world->GetVolumeCount(), 0U);
  attachments.AddVolume(world, mother);
  EXPECT_THROW(attachments.AddVolume(mother, null_shape), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(mother, mother), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(world, mother), GGEMSRecoverable);
  attachments.AddVolume(mother, child);
  EXPECT_THROW(attachments.AddVolume(mother, child), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(other_world, child), GGEMSRecoverable);
  EXPECT_THROW(attachments.AddVolume(child, mother), GGEMSRecoverable);
  EXPECT_EQ(other_world->GetVolumeCount(), 0U);
  EXPECT_EQ(world->GetVolumeCount(), 2U);
  attachments.AddVolume(child, grandchild);
  EXPECT_THROW(attachments.AddVolume(grandchild, leaf), GGEMSRecoverable);
  EXPECT_EQ(world->GetVolumeCount(), 3U);
  leaf->SetPosition(MakePositionPM(50'000'000'000LL, 0, 0));
  EXPECT_NO_THROW(attachments.AddVolume(world, leaf));

  auto incomplete = std::make_shared<GGEMSBox>();
  EXPECT_THROW(attachments.AddVolume(mother, incomplete), GGEMSRecoverable);
  EXPECT_EQ(world->GetVolumeCount(), 4U);
  incomplete->SetSize(2_mm, 2_mm, 2_mm)
    .SetPosition(MakePositionPM(60'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  EXPECT_NO_THROW(attachments.AddVolume(world, incomplete));
  EXPECT_EQ(world->GetVolumeCount(), 5U);
  EXPECT_NO_THROW(world->Validate());
}

TEST(GGEMSGeometryAttachments, WeakRegistrationsPermitExpirationAndRootReuse) {
  GGEMSGeometryAttachments attachments;
  auto world = MakeWorld();
  auto mother = MakeShape<GGEMSBox>(20_mm);
  auto child = MakeShape<GGEMSSphere>(4_mm);
  std::weak_ptr const weak_world = world;
  std::weak_ptr const weak_mother = mother;
  std::weak_ptr const weak_child = child;
  attachments.AddVolume(world, mother);
  attachments.AddVolume(mother, child);
  child.reset();
  mother.reset();
  EXPECT_TRUE(weak_child.expired());
  EXPECT_TRUE(weak_mother.expired());
  EXPECT_EQ(world->GetVolumeCount(), 2U);
  EXPECT_NO_THROW(world->Validate());

  auto surviving_mother = MakeShape<GGEMSBox>(4_mm);
  surviving_mother->SetPosition(MakePositionPM(50'000'000'000LL, 0, 0));
  attachments.AddVolume(world, surviving_mother);
  world.reset();
  EXPECT_TRUE(weak_world.expired());
  auto late_child = MakeShape<GGEMSBox>(1_mm);
  EXPECT_THROW(attachments.AddVolume(surviving_mother, late_child),
               GGEMSRecoverable);
  auto new_world = MakeWorld();
  EXPECT_NO_THROW(attachments.AddVolume(new_world, surviving_mother));
  EXPECT_NO_THROW(attachments.AddVolume(surviving_mother, late_child));
  EXPECT_EQ(new_world->GetVolumeCount(), 2U);
  EXPECT_NO_THROW(new_world->Validate());
  std::weak_ptr const weak_new_world = new_world;
  std::weak_ptr const weak_survivor = surviving_mother;
  std::weak_ptr const weak_late_child = late_child;
  new_world.reset();
  surviving_mother.reset();
  late_child.reset();
  EXPECT_TRUE(weak_new_world.expired());
  EXPECT_TRUE(weak_survivor.expired());
  EXPECT_TRUE(weak_late_child.expired());
}
