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
 * \brief Checks detached recipes, repeaters, and copied geometry attachments.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <numbers>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
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
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
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
  EXPECT_EQ(actual_world.half_extent_x_pm, expected_world.half_extent_x_pm);
  EXPECT_EQ(actual_world.half_extent_y_pm, expected_world.half_extent_y_pm);
  EXPECT_EQ(actual_world.half_extent_z_pm, expected_world.half_extent_z_pm);
  EXPECT_EQ(actual_world.tolerance_pm, expected_world.tolerance_pm);
  EXPECT_EQ(actual_world.occurrence_count, expected_world.occurrence_count);
  EXPECT_EQ(actual_world.max_boundary_events,
            expected_world.max_boundary_events);
  EXPECT_EQ(actual_world.hierarchy_depth, expected_world.hierarchy_depth);
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

// Checks shape dispatch and occurrence indexing for all supported solid
// families.
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

// Preserves nested snapshots when the caller later mutates an attached solid.
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

// Keeps the child reusable after a rejected hierarchy attachment.
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

// Allows a surviving solid to be reused after its former parent expires.
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

// Capturing each child freezes its value while a detached mother's pose remains
// editable; both authoring orders produce the same independent World snapshot.
TEST(GGEMSGeometryAttachments, DetachedSubtreePreservesCaptureAndPoseOrder) {
  auto mother = *MakeShape<GGEMSBox>(20_mm);
  auto daughter = *MakeShape<GGEMSSphere>(8_mm);
  auto leaf = *MakeShape<GGEMSBox>(2_mm);
  daughter.SetPosition(MakePositionPM(6'000'000'000LL, 3'000'000'000LL, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  leaf.SetPosition(MakePositionPM(2'000'000'000LL, 1'000'000'000LL, 0))
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  auto const captured_daughter = daughter;
  auto const captured_leaf = leaf;
  daughter.AddVolume(leaf);
  leaf.SetSize(20_mm, 20_mm, 20_mm)
    .SetPosition(MakePositionPM(90'000'000'000LL, 0, 0));

  auto posed_first = mother;
  posed_first
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 3'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto const captured_mother = posed_first;
  posed_first.AddVolume(daughter);
  mother.AddVolume(daughter)
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 3'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  daughter.SetRadius(1_mm).SetPosition(MakePositionPM(80'000'000'000LL, 0, 0));

  auto before = *MakeWorld();
  auto after = *MakeWorld();
  before.AddVolume(posed_first);
  after.AddVolume(mother);
  auto expected = *MakeWorld();
  expected.AddVolume(captured_mother)
    .AddVolume(captured_daughter, 1U)
    .AddVolume(captured_leaf, 2U);
  mother.SetSize(2_mm, 2_mm, 2_mm)
    .SetPosition(MakePositionPM(70'000'000'000LL, 0, 0));
  ExpectSameCompilation(before, expected);
  ExpectSameCompilation(after, expected);
  EXPECT_EQ(after.BuildRecord().hierarchy_depth, 3U);
}

// Detached source handles do not silently become live handles to captured
// descendants; an explicitly registered mother still supports top-down adds.
TEST(GGEMSGeometryAttachments, DetachedBindingKeepsCapturedSourcesIndependent) {
  GGEMSGeometryAttachments attachments;
  auto world = MakeWorld();
  auto mother = MakeShape<GGEMSBox>(20_mm);
  auto child = MakeShape<GGEMSBox>(4_mm);
  auto leaf = MakeShape<GGEMSBox>(1_mm);
  attachments.AddVolume(mother, child);
  attachments.AddVolume(world, mother);
  auto const captured = *world;
  attachments.AddVolume(child, leaf);
  ExpectSameCompilation(*world, captured);

  auto sibling = MakeShape<GGEMSBox>(1_mm);
  sibling->SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  mother->SetPosition(MakePositionPM(80'000'000'000LL, 0, 0));
  attachments.AddVolume(mother, sibling);
  ASSERT_NO_THROW(world->Validate());
  ASSERT_EQ(world->GetVolumeCount(), 3U);
  EXPECT_EQ(world->BuildOccurrenceRecords()[2].parent_volume_id, 1U);
  EXPECT_FLOAT_EQ(world->BuildBoxPoseRecord(2U)->center_x_pm, 10.0e9F);

  auto before_rejection = *world;
  auto rejected_child = MakeShape<GGEMSBox>(1_mm);
  EXPECT_THROW(attachments.AddVolume(mother, rejected_child), GGEMSRecoverable);
  ExpectSameCompilation(*world, before_rejection);
  rejected_child->SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  attachments.AddVolume(mother, rejected_child);
  ASSERT_EQ(world->GetVolumeCount(), 4U);
  EXPECT_EQ(world->BuildOccurrenceRecords()[3].parent_volume_id, 1U);

  before_rejection = *world;
  auto rejected_root = MakeShape<GGEMSBox>(1_mm);
  EXPECT_THROW(attachments.AddVolume(world, rejected_root), GGEMSRecoverable);
  ExpectSameCompilation(*world, before_rejection);
  rejected_root->SetPosition(MakePositionPM(60'000'000'000LL, 0, 0));
  attachments.AddVolume(world, rejected_root);
  ASSERT_EQ(world->GetVolumeCount(), 5U);
  EXPECT_EQ(world->BuildOccurrenceRecords()[4].parent_volume_id, 0U);

  auto unconfigured_world = std::make_shared<GGEMSWorld>();
  auto unregistered_leaf = MakeShape<GGEMSBox>(1_mm);
  EXPECT_THROW(attachments.AddVolume(unconfigured_world, unregistered_leaf),
               GGEMSRecoverable);
  EXPECT_EQ(unconfigured_world->GetVolumeCount(), 0U);
  unconfigured_world->SetSize(200_mm, 200_mm, 200_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  attachments.AddVolume(unconfigured_world, unregistered_leaf);
  EXPECT_EQ(unconfigured_world->GetVolumeCount(), 1U);
}

// Odd and even arrays are centered on the reference pose. Identical materials
// retain distinct physical identities, including at face contact.
TEST(GGEMSGeometryAttachments, LinearRepeatersPreserveGapsContactAndCountOne) {
  std::array const pitches{0_mm, 2_mm, 4_mm, 4_mm};
  for (auto const count : {1U, 2U, 3U, 4U}) {
    SCOPED_TRACE(count);
    auto const pitch = pitches[count - 1U];
    auto mother = *MakeShape<GGEMSBox>(20_mm);
    auto child = *MakeShape<GGEMSBox>(1_mm);
    child.SetPosition(MakePositionPM(-4'000'000'000LL, 0, 0))
      .SetRotation(0_deg, 0_deg, 90_deg);
    auto expected = *MakeWorld();
    expected.AddVolume(mother);
    for (std::uint32_t index = 0U; index < count; ++index) {
      auto copy = child;
      auto const offset = (2 * static_cast<std::int64_t>(index)) - count + 1;
      copy.SetPosition(
        MakePositionPM(-4'000'000'000LL +
                         (offset * static_cast<std::int64_t>(pitch.value) / 2),
                       0, 0));
      expected.AddVolume(copy, 1U);
    }
    mother.LinearRepeater(child, count, pitch, GGEMSRepeatAxis::X);
    auto world = *MakeWorld();
    world.AddVolume(mother);
    ExpectSameCompilation(world, expected);
    std::vector<std::uint32_t> const material_ids(world.GetVolumeCount(), 7U);
    auto const records = world.BuildAnalyticDeviceRecords(material_ids);
    for (std::size_t index = 0U; index < records.size(); ++index) {
      EXPECT_EQ(records[index].parameters.box.volume_id, index + 1U);
      EXPECT_EQ(records[index].parameters.box.material_id, 7U);
    }
  }
  auto mother = *MakeShape<GGEMSBox>(20_mm);
  mother.CircularRepeater(*MakeShape<GGEMSBox>(1_mm), 1U, GGEMSRepeatAxis::Z);
  auto world = *MakeWorld();
  world.AddVolume(mother);
  EXPECT_EQ(world.GetVolumeCount(), 2U);
}

// Cartesian roots are direct siblings in X-fast order, with the same captured
// local geometry whether the mother is posed before or after authoring.
TEST(GGEMSGeometryAttachments, CartesianRepeatersPreserveCenteredPosesAndIds) {
  for (std::array<std::uint32_t, 3> const counts : {
         std::array{1U, 1U, 1U},
         std::array{3U, 1U, 1U},
         std::array{2U, 3U, 1U},
         std::array{2U, 3U, 2U},
       }) {
    SCOPED_TRACE(counts[0]);
    SCOPED_TRACE(counts[1]);
    SCOPED_TRACE(counts[2]);
    auto child = *MakeShape<GGEMSBox>(1_mm);
    child.SetSize(2_mm, 4_mm, 6_mm)
      .SetPosition(
        MakePositionPM(3'000'000'000LL, 4'000'000'000LL, 5'000'000'000LL))
      .SetRotation(0_deg, 0_deg, 90_deg);
    auto reference = child;
    auto before = *MakeShape<GGEMSBox>(40_mm);
    auto after = before;
    before
      .SetPosition(
        MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 3'000'000'000LL))
      .SetRotation(0_deg, 0_deg, 90_deg);
    auto expected_mother = before;
    before.LinearRepeater(child, counts, {8_mm, 10_mm, 12_mm});
    after.LinearRepeater(child, counts, {8_mm, 10_mm, 12_mm})
      .SetPosition(expected_mother.GetPositionPM())
      .SetRotation(0_deg, 0_deg, 90_deg);
    child.SetSize(1_mm, 1_mm, 1_mm)
      .SetPosition(MakePositionPM(80'000'000'000LL, 0, 0));
    auto world_before = *MakeWorld();
    auto world_after = *MakeWorld();
    world_before.AddVolume(before);
    world_after.AddVolume(after);
    auto expected = *MakeWorld();
    expected.AddVolume(expected_mother);
    for (std::uint32_t z = 0U; z < counts[2]; ++z) {
      for (std::uint32_t y = 0U; y < counts[1]; ++y) {
        for (std::uint32_t x = 0U; x < counts[0]; ++x) {
          reference.SetPosition(MakePositionPM(
            3'000'000'000LL +
              (((2 * static_cast<std::int64_t>(x)) - counts[0] + 1) *
               4'000'000'000LL),
            4'000'000'000LL +
              (((2 * static_cast<std::int64_t>(y)) - counts[1] + 1) *
               5'000'000'000LL),
            5'000'000'000LL +
              (((2 * static_cast<std::int64_t>(z)) - counts[2] + 1) *
               6'000'000'000LL)));
          expected.AddVolume(reference, 1U);
        }
      }
    }
    ExpectSameCompilation(world_before, expected);
    ExpectSameCompilation(world_after, expected);
    auto const total = counts[0] * counts[1] * counts[2];
    ASSERT_EQ(world_after.GetVolumeCount(), total + 1U);
    EXPECT_EQ(world_after.BuildRecord().hierarchy_depth, 2U);
    auto const links = world_after.BuildOccurrenceRecords();
    for (std::uint32_t index = 1U; index <= total; ++index) {
      EXPECT_EQ(links[index].parent_volume_id, 1U);
      EXPECT_EQ(links[index].first_child_id, 0U);
      EXPECT_EQ(world_after.BuildAnalyticVolumeRecord(7U, index)
                  .parameters.box.volume_id,
                index + 1U);
    }
  }
}

// The user-facing 10-by-10-by-10 example is bounded host admission only;
// it performs no particle transport or device initialization.
TEST(GGEMSGeometryAttachments, ThousandCenteredBoxesHaveTwelveMillimeterPitch) {
  auto world = *MakeWorld();
  auto child = *MakeShape<GGEMSBox>(5_mm);
  world.LinearRepeater(child, {10U, 10U, 10U}, {12_mm, 12_mm, 12_mm});
  ASSERT_EQ(world.GetVolumeCount(), 1000U);
  EXPECT_EQ(world.BuildRecord().hierarchy_depth, 1U);
  auto const links = world.BuildOccurrenceRecords();
  for (std::uint32_t index = 0U; index < 1000U; ++index) {
    auto const center = world.GetBoxes(index).front().GetPositionPM();
    EXPECT_EQ(center.x, -54'000'000'000LL + ((index % 10U) * 12'000'000'000LL));
    EXPECT_EQ(center.y,
              -54'000'000'000LL + (((index / 10U) % 10U) * 12'000'000'000LL));
    EXPECT_EQ(center.z,
              -54'000'000'000LL + ((index / 100U) * 12'000'000'000LL));
    EXPECT_EQ(links[index].parent_volume_id, 0U);
    EXPECT_EQ(links[index].first_child_id, 0U);
    auto const record = world.BuildAnalyticVolumeRecord(7U, index);
    EXPECT_EQ(record.parameters.box.volume_id, index + 1U);
    EXPECT_EQ(record.parameters.box.material_id, 7U);
  }
  auto const &first = world.GetBoxes(0U).front();
  auto const &last = world.GetBoxes(999U).front();
  auto const diameter =
    static_cast<std::int64_t>(2U * first.GetHalfExtentPM().x);
  EXPECT_EQ(last.GetPositionPM().x - first.GetPositionPM().x + diameter,
            (118_mm).value);
  EXPECT_EQ(world.GetBoxes(1U).front().GetPositionPM().x -
              first.GetPositionPM().x - diameter,
            (2_mm).value);
}

// Invalid recipes leave no retained child; invalid expanded scenes leave the
// previously attached World unchanged and can be corrected before retrying.
TEST(GGEMSGeometryAttachments, InvalidRecipesAndBatchesRejectAtomically) {
  auto mother = *MakeShape<GGEMSBox>(20_mm);
  auto child = *MakeShape<GGEMSBox>(1_mm);
  GGEMSWorld unconfigured;
  EXPECT_THROW(unconfigured.AttachVolume(child), GGEMSRecoverable);
  EXPECT_EQ(unconfigured.GetVolumeCount(), 0U);
  EXPECT_NO_THROW(unconfigured.AddVolume(child));
  EXPECT_EQ(unconfigured.GetVolumeCount(), 1U);
  EXPECT_THROW(mother.LinearRepeater(child, 0U, 4_mm, GGEMSRepeatAxis::X),
               GGEMSRecoverable);
  EXPECT_THROW(mother.LinearRepeater(child, 2U, 0_mm, GGEMSRepeatAxis::X),
               GGEMSRecoverable);
  EXPECT_THROW(
    mother.LinearRepeater(child, 2U, 4_mm, static_cast<GGEMSRepeatAxis>(3U)),
    GGEMSRecoverable);
  EXPECT_THROW(mother.CircularRepeater(child, 0U, GGEMSRepeatAxis::Z),
               GGEMSRecoverable);
  EXPECT_THROW(mother.CircularRepeater(child, 2U, GGEMSRepeatAxis::Z),
               GGEMSRecoverable);
  EXPECT_THROW(mother.LinearRepeater(child,
                                     std::numeric_limits<std::uint32_t>::max(),
                                     4_mm, GGEMSRepeatAxis::X),
               GGEMSRecoverable);
  EXPECT_THROW(mother.LinearRepeater(child, {2U, 0U, 1U}, {4_mm, 4_mm, 0_mm}),
               GGEMSRecoverable);
  EXPECT_THROW(mother.LinearRepeater(child, {2U, 1U, 1U}, {0_mm, 0_mm, 0_mm}),
               GGEMSRecoverable);
  EXPECT_THROW(
    mother.LinearRepeater(child, {65536U, 65536U, 1U}, {4_mm, 4_mm, 0_mm}),
    GGEMSRecoverable);
  EXPECT_THROW(mother.AddVolume(mother), GGEMSRecoverable);
  auto empty = *MakeWorld();
  empty.AddVolume(mother);
  EXPECT_EQ(empty.GetVolumeCount(), 1U);

  auto world = *MakeWorld();
  auto existing = *MakeShape<GGEMSBox>(1_mm);
  existing.SetPosition(MakePositionPM(60'000'000'000LL, 0, 0));
  world.AddVolume(existing);
  auto const before = world;
  auto const excessive_descriptors =
    static_cast<std::uint32_t>((std::numeric_limits<std::uint32_t>::max() /
                                sizeof(GGEMSAnalyticOccurrenceRecord)) +
                               1U);
  EXPECT_THROW(
    world.CircularRepeater(existing, excessive_descriptors, GGEMSRepeatAxis::Z),
    GGEMSRecoverable);
  ExpectSameCompilation(world, before);
  EXPECT_THROW(world.LinearRepeater(existing, {excessive_descriptors, 1U, 1U},
                                    {4_mm, 0_mm, 0_mm}),
               GGEMSRecoverable);
  ExpectSameCompilation(world, before);
  auto overlapping = *MakeShape<GGEMSBox>(10_mm);
  overlapping.LinearRepeater(child, {2U, 2U, 1U}, {1_mm, 4_mm, 0_mm});
  EXPECT_THROW(world.AddVolume(overlapping), GGEMSRecoverable);
  ExpectSameCompilation(world, before);

  auto protruding = *MakeShape<GGEMSBox>(5_mm);
  child.SetPosition(MakePositionPM(4'000'000'000LL, 0, 0));
  protruding.LinearRepeater(child, 2U, 4_mm, GGEMSRepeatAxis::X);
  EXPECT_THROW(world.AddVolume(protruding), GGEMSRecoverable);
  ExpectSameCompilation(world, before);

  auto mixed = *MakeShape<GGEMSBox>(10_mm);
  mixed.AddVolume(child).LinearRepeater(child, 3U, 4_mm, GGEMSRepeatAxis::X);
  EXPECT_THROW(world.AddVolume(mixed), GGEMSRecoverable);
  ExpectSameCompilation(world, before);
  EXPECT_THROW(world.LinearRepeater(existing, 3U, 4_mm, GGEMSRepeatAxis::X),
               GGEMSRecoverable);
  ExpectSameCompilation(world, before);

  child.SetPosition(MakePositionPM(0, 0, 0));
  mother.AddVolume(child);
  EXPECT_THROW(unconfigured.AddVolume(mother), GGEMSRecoverable);
  EXPECT_EQ(unconfigured.GetVolumeCount(), 1U);
  mother.SetPosition(MakePositionPM(90'000'000'000LL, 0, 0));
  EXPECT_THROW(world.AddVolume(mother), GGEMSRecoverable);
  ExpectSameCompilation(world, before);
  mother.SetPosition(MakePositionPM(0, 0, 0));
  EXPECT_NO_THROW(world.AddVolume(mother));
  EXPECT_EQ(world.GetVolumeCount(), 3U);
}

// Independent axis-rotation formulas check position and a nonidentity reference
// orientation for two nontrivial counts, without a duplicate full-turn copy.
TEST(GGEMSGeometryAttachments,
     CircularRepeatersRotateReferencePoseOnEveryAxis) {
  for (auto const count : {7U, 11U}) {
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      SCOPED_TRACE(count);
      SCOPED_TRACE(axis);
      auto mother = *MakeShape<GGEMSBox>(40_mm);
      auto child = *MakeShape<GGEMSBox>(1_mm);
      child.SetSize(2_mm, 4_mm, 6_mm).SetRotation(0_deg, 0_deg, 90_deg);
      std::array<double, 3> position{};
      position[axis] = 1.0e9;
      position[(axis + 1U) % 3U] = 20.0e9;
      child.SetPosition(MakePositionPM(static_cast<std::int64_t>(position[0]),
                                       static_cast<std::int64_t>(position[1]),
                                       static_cast<std::int64_t>(position[2])));
      mother.CircularRepeater(child, count, static_cast<GGEMSRepeatAxis>(axis));
      auto world = *MakeWorld();
      ASSERT_NO_THROW(world.AddVolume(mother));
      ASSERT_EQ(world.GetVolumeCount(), count + 1U);
      for (std::uint32_t index = 0U; index < count; ++index) {
        SCOPED_TRACE(index);
        auto const angle = 2.0 * std::numbers::pi * index / count;
        auto const cosine = std::cos(angle);
        auto const sine = std::sin(angle);
        auto const rotate =
          [&](std::array<double, 3> value) -> std::array<double, 3> {
          auto result = value;
          auto const first = (axis + 1U) % 3U;
          auto const second = (axis + 2U) % 3U;
          result[first] = (cosine * value[first]) - (sine * value[second]);
          result[second] = (sine * value[first]) + (cosine * value[second]);
          return result;
        };
        auto const center = rotate(position);
        auto const axis_x = rotate({0.0, 1.0, 0.0});
        auto const axis_y = rotate({-1.0, 0.0, 0.0});
        auto const axis_z = rotate({0.0, 0.0, 1.0});
        auto const pose = world.BuildBoxPoseRecord(index + 1U);
        ASSERT_TRUE(pose.has_value());
        // Four FP32 relative roundoff units at the largest center component;
        // sixteen for products of the two independently authored rotations.
        constexpr double coordinate_budget =
          4.0 * std::numeric_limits<float>::epsilon() * 20.0e9;
        constexpr double rotation_budget =
          16.0 * std::numeric_limits<float>::epsilon();
        EXPECT_NEAR(pose->center_x_pm, center[0], coordinate_budget);
        EXPECT_NEAR(pose->center_y_pm, center[1], coordinate_budget);
        EXPECT_NEAR(pose->center_z_pm, center[2], coordinate_budget);
        EXPECT_NEAR(pose->axis_x_x, axis_x[0], rotation_budget);
        EXPECT_NEAR(pose->axis_x_y, axis_x[1], rotation_budget);
        EXPECT_NEAR(pose->axis_x_z, axis_x[2], rotation_budget);
        EXPECT_NEAR(pose->axis_y_x, axis_y[0], rotation_budget);
        EXPECT_NEAR(pose->axis_y_y, axis_y[1], rotation_budget);
        EXPECT_NEAR(pose->axis_y_z, axis_y[2], rotation_budget);
        EXPECT_NEAR(pose->axis_z_x, axis_z[0], rotation_budget);
        EXPECT_NEAR(pose->axis_z_y, axis_z[1], rotation_budget);
        EXPECT_NEAR(pose->axis_z_z, axis_z[2], rotation_budget);
      }
    }
  }
}

// Repetition clones a complete subtree with unique physical IDs, retaining the
// existing maximum depth and leaving a failed depth extension unrecorded.
TEST(GGEMSGeometryAttachments, RepeatedSubtreesPreserveParentageAndDepth) {
  auto scanner = *MakeShape<GGEMSBox>(50_mm);
  auto module = *MakeShape<GGEMSBox>(10_mm);
  auto crystal = *MakeShape<GGEMSBox>(2_mm);
  crystal.SetPosition(MakePositionPM(2'000'000'000LL, 0, 0));
  module.AddVolume(crystal);
  scanner.LinearRepeater(module, {2U, 1U, 1U}, {40_mm, 0_mm, 0_mm});
  auto world = *MakeWorld();
  world.AddVolume(scanner);
  ASSERT_EQ(world.GetVolumeCount(), 5U);
  auto const links = world.BuildOccurrenceRecords();
  std::array<std::uint32_t, 5> const parents{0U, 1U, 2U, 1U, 4U};
  for (std::size_t index = 0U; index < links.size(); ++index) {
    EXPECT_EQ(links[index].parent_volume_id, parents[index]);
    EXPECT_EQ(
      world.BuildAnalyticVolumeRecord(9U, index).parameters.box.volume_id,
      index + 1U);
  }
  EXPECT_FLOAT_EQ(world.BuildBoxPoseRecord(2U)->center_x_pm, -18.0e9F);
  EXPECT_FLOAT_EQ(world.BuildBoxPoseRecord(4U)->center_x_pm, 22.0e9F);
  EXPECT_EQ(world.BuildRecord().hierarchy_depth, 3U);
  auto outer = *MakeShape<GGEMSBox>(60_mm);
  EXPECT_THROW(outer.AddVolume(scanner), GGEMSRecoverable);
  auto unchanged = *MakeWorld();
  unchanged.AddVolume(outer);
  EXPECT_EQ(unchanged.GetVolumeCount(), 1U);

  auto repeated_module = *MakeShape<GGEMSBox>(5_mm);
  auto repeated_crystal = *MakeShape<GGEMSBox>(1_mm);
  repeated_module
    .LinearRepeater(repeated_crystal, {2U, 1U, 1U}, {4_mm, 0_mm, 0_mm})
    .SetPosition(MakePositionPM(30'000'000'000LL, 0, 0));
  auto ring_before = *MakeShape<GGEMSBox>(50_mm);
  auto ring_after = ring_before;
  ring_before.SetPosition(MakePositionPM(1'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg)
    .CircularRepeater(repeated_module, 7U, GGEMSRepeatAxis::Z);
  ring_after.CircularRepeater(repeated_module, 7U, GGEMSRepeatAxis::Z)
    .SetPosition(MakePositionPM(1'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto before_pose = *MakeWorld();
  auto after_pose = *MakeWorld();
  before_pose.AddVolume(ring_before);
  after_pose.AddVolume(ring_after);
  ExpectSameCompilation(after_pose, before_pose);
  ASSERT_EQ(after_pose.GetVolumeCount(), 22U);
  auto const ring_links = after_pose.BuildOccurrenceRecords();
  for (std::uint32_t index = 0U; index < 7U; ++index) {
    auto const module_id = 2U + (3U * index);
    EXPECT_EQ(ring_links[module_id - 1U].parent_volume_id, 1U);
    EXPECT_EQ(ring_links[module_id].parent_volume_id, module_id);
    EXPECT_EQ(ring_links[module_id + 1U].parent_volume_id, module_id);
  }
}

// The same two operations dispatch every admitted analytical family without
// introducing a shape-specific authoring path.
TEST(GGEMSGeometryAttachments, BothRepeatersAcceptAllEightAnalyticalFamilies) {
  auto const check = []<typename Shape> -> void {
    for (bool circular : {false, true}) {
      SCOPED_TRACE(circular);
      auto mother = *MakeShape<GGEMSBox>(40_mm);
      auto child = *MakeShape<Shape>(1_mm);
      child.SetPosition(MakePositionPM(20'000'000'000LL, 0, 0));
      if (circular) {
        mother.CircularRepeater(child, 7U, GGEMSRepeatAxis::Z);
      } else {
        mother.LinearRepeater(child, {2U, 1U, 1U}, {4_mm, 0_mm, 0_mm});
      }
      auto world = *MakeWorld();
      world.AddVolume(mother);
      EXPECT_EQ(world.GetVolumeCount(), circular ? 8U : 3U);
      auto const kind = world.BuildAnalyticVolumeRecord(0U, 1U).kind;
      for (std::size_t index = 2U; index < world.GetVolumeCount(); ++index) {
        EXPECT_EQ(world.BuildAnalyticVolumeRecord(0U, index).kind, kind);
      }
    }
  };
  check.template operator()<GGEMSBox>();
  check.template operator()<GGEMSSphere>();
  check.template operator()<GGEMSTubs>();
  check.template operator()<GGEMSCons>();
  check.template operator()<GGEMSPolyhedra>();
  check.template operator()<GGEMSEllipsoid>();
  check.template operator()<GGEMSTrd>();
  check.template operator()<GGEMSTrap>();
}
