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
 * \brief Verifies physical owner transitions through multiple analytic
 * occurrences.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <memory>
#include <numbers>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Vec = std::array<double, 3>;
constexpr double k_mm{1.0e9};
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};

struct Crossing {
  double parameter_mm;
  std::uint32_t arrival;
};

struct Fixture {
  std::string name;
  geometry::GGEMSWorld world;
  Vec start{-60, 0, 0};
  Vec direction{1, 0, 0};
  std::vector<Crossing> crossings;
  std::uint32_t initial_owner{0U};
};

auto Position(Vec const &value) -> geometry::Position3PM {
  return geometry::MakePositionPM(std::llround(value[0] * k_mm),
                                  std::llround(value[1] * k_mm),
                                  std::llround(value[2] * k_mm));
}

auto World() -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(200_mm, 200_mm, 200_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  return world;
}

auto Box(Vec const &center, char const *material = "Water")
  -> geometry::GGEMSBox {
  geometry::GGEMSBox box;
  box.SetSize(8_mm, 10_mm, 12_mm)
    .SetPosition(Position(center))
    .SetMaterial(materials::builtins::BuildBuiltInMaterial(material));
  return box;
}

auto Boxes(double left, double right, bool reverse = false,
           char const *second_material = "Aluminum") -> geometry::GGEMSWorld {
  auto world = World();
  auto const first = Box({left, 0, 0});
  auto const second = Box({right, 0, 0}, second_material);
  if (reverse) {
    world.AddVolume(second).AddVolume(first);
  } else {
    world.AddVolume(first).AddVolume(second);
  }
  return world;
}

// All expected distances are hand-derived slabs or circle chords in mm.
// Neither the production transforms nor intersection helpers generate goldens.
auto Cases() -> std::vector<Fixture> {
  std::vector<Fixture> cases;
  auto three = Boxes(-30, 0);
  three.AddVolume(Box({30, 0, 0}, "Vacuum"));
  cases.push_back(
    {.name = "three shared Boxes keep exact Material and Volume identities",
     .world = three,
     .crossings = {{26, 1}, {34, 0}, {56, 2}, {64, 0}, {86, 3}, {94, 0}}});
  cases.push_back({.name = "reversed registration retains physical ordering",
                   .world = Boxes(-20, 20, true),
                   .crossings = {{36, 2}, {44, 0}, {76, 1}, {84, 0}}});
  auto const touching = Boxes(-4, 4, false, "Water");
  cases.push_back({.name = "same Material contact keeps distinct VolumeIDs",
                   .world = touching,
                   .crossings = {{52, 1}, {60, 2}, {68, 0}}});
  cases.push_back({.name = "contact birth follows positive direction",
                   .world = touching,
                   .start = {0, 0, 0},
                   .crossings = {{8, 0}},
                   .initial_owner = 2});
  cases.push_back({.name = "contact birth follows negative direction",
                   .world = touching,
                   .start = {0, 0, 0},
                   .direction = {-1, 0, 0},
                   .crossings = {{8, 0}},
                   .initial_owner = 1});
  auto mixed = World();
  mixed.AddVolume(Box({-20, 0, 0}));
  geometry::GGEMSSphere sphere;
  sphere.SetRadius(6_mm)
    .SetPosition(Position({20, 0, 0}))
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Aluminum"));
  mixed.AddVolume(sphere);
  cases.push_back({.name = "mixed Box Sphere has a true curved chord",
                   .world = mixed,
                   .start = {-60, 3, 0},
                   .crossings = {{36, 1},
                                 {44, 0},
                                 {80 - std::sqrt(27.0), 2},
                                 {80 + std::sqrt(27.0), 0}}});
  auto posed = World();
  for (double center : {-20.0, 20.0}) {
    auto shape = Box({center, 3, -4});
    shape.SetRotation(0_deg, 0_deg, 30_deg);
    posed.AddVolume(shape);
  }
  double const chord = 8 / std::numbers::sqrt3;
  cases.push_back({.name = "posed shared Boxes with nonunit stored direction",
                   .world = posed,
                   .start = {-60, 3, -4},
                   .direction = {2, 0, 0},
                   .crossings = {{(40 - chord) / 2, 1},
                                 {(40 + chord) / 2, 0},
                                 {(80 - chord) / 2, 2},
                                 {(80 + chord) / 2, 0}}});
  auto polygons = World();
  geometry::GGEMSPolyhedra polygon;
  std::array const z{PositionCoordinate{-6'000'000'000LL},
                     PositionCoordinate{0},
                     PositionCoordinate{6'000'000'000LL}};
  std::array const inner{2_mm, 2_mm, 2_mm};
  std::array const outer{4_mm, 4_mm, 4_mm};
  polygon.SetSideCount(4)
    .SetStartAngle(45_deg)
    .SetZProfile(z, inner, outer)
    .SetPosition(Position({-20, 0, 0}))
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  polygons.AddVolume(polygon);
  polygon.SetPosition(Position({20, 0, 0}))
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Aluminum"));
  polygons.AddVolume(polygon);
  cases.push_back(
    {.name = "shared hollow Polyhedra payload preserves both walls",
     .world = polygons,
     .crossings = {{36, 1},
                   {38, 0},
                   {42, 1},
                   {44, 0},
                   {76, 2},
                   {78, 0},
                   {82, 2},
                   {84, 0}}});
  auto bore_scene = World();
  polygon.SetPosition(Position({0, 0, 0}))
    .SetRotation(0_deg, 0_deg, 0_deg)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  bore_scene.AddVolume(polygon);
  auto bore_box = Box({0, 0, 0}, "Aluminum");
  bore_box.SetSize(2_mm, 2_mm, 2_mm);
  bore_scene.AddVolume(bore_box);
  cases.push_back(
    {.name = "Box in polygon bore preserves later parent wall reentry",
     .world = bore_scene,
     .crossings = {{56, 1}, {58, 0}, {59, 2}, {61, 0}, {62, 1}, {64, 0}}});
  auto const separated = Boxes(-20, 20);
  double const half_band = separated.GetVolumeNavigationTolerancePM(0) / k_mm;
  double const start = -24 - (0.5 * half_band);
  cases.push_back(
    {.name = "surface band birth owns forward first occurrence",
     .world = separated,
     .start = {start, 0, 0},
     .crossings = {{-16 - start, 0}, {16 - start, 2}, {24 - start, 0}},
     .initial_owner = 1});
  auto rounding = World();
  auto small = Box({0, 0, 0});
  small.SetSize(200_um, 10_mm, 12_mm);
  rounding.AddVolume(small);
  small.SetPosition(geometry::MakePositionPM(200'000'000LL, 0, 0));
  rounding.AddVolume(small);
  // From this anchor, independently rounded exit and entry parameters differ
  // by 8 pm, but both physical surfaces lie at the same packed endpoint.
  cases.push_back(
    {.name = "contact resolves unequal rounded ray parameters",
     .world = rounding,
     .start = {0.033333334, 0, 0},
     .crossings = {{0.1 - 0.033333334, 2}, {0.3 - 0.033333334, 0}},
     .initial_owner = 1});
  auto rotated_contact = World();
  double const cosine = std::numbers::sqrt3 / 2;
  for (double center : {-4.0, 4.0}) {
    auto shape = Box({center * cosine, center * 0.5, 0});
    shape.SetRotation(0_deg, 0_deg, 30_deg);
    rotated_contact.AddVolume(shape);
  }
  cases.push_back({.name = "rotated contact has one physical owner transition",
                   .world = rotated_contact,
                   .start = {-40 * cosine, -20, 0},
                   .direction = {cosine, 0.5, 0},
                   .crossings = {{32, 1}, {40, 2}, {48, 0}}});
  double const gap = 32 * touching.GetVolumeNavigationTolerancePM(0) / k_mm;
  cases.push_back(
    {.name = "resolved narrow gap retains World material interval",
     .world = Boxes(-4, 4 + gap),
     .crossings = {{52, 1}, {60, 0}, {60 + gap, 2}, {68 + gap, 0}}});
  auto all_families = World();
  auto const water = materials::builtins::BuildBuiltInMaterial("Water");
  all_families.AddVolume(Box({-70, 0, 0}));
  sphere.SetRadius(4_mm).SetPosition(Position({-50, 0, 0})).SetMaterial(water);
  all_families.AddVolume(sphere);
  geometry::GGEMSTubs tubs;
  tubs.SetRadii(4_mm)
    .SetLength(8_mm)
    .SetPosition(Position({-30, 0, 0}))
    .SetMaterial(water);
  all_families.AddVolume(tubs);
  geometry::GGEMSEllipsoid ellipsoid;
  ellipsoid.SetSemiAxes(4_mm, 4_mm, 4_mm)
    .SetPosition(Position({-10, 0, 0}))
    .SetMaterial(water);
  all_families.AddVolume(ellipsoid);
  geometry::GGEMSTrd trd;
  trd.SetNegativeZFace(8_mm, 8_mm)
    .SetPositiveZFace(8_mm, 8_mm)
    .SetLength(8_mm)
    .SetPosition(Position({10, 0, 0}))
    .SetMaterial(water);
  all_families.AddVolume(trd);
  geometry::GGEMSTrap trap;
  trap.SetNegativeZFace(8_mm, 8_mm, 8_mm, 0_deg)
    .SetPositiveZFace(8_mm, 8_mm, 8_mm, 0_deg)
    .SetLength(8_mm)
    .SetPosition(Position({30, 0, 0}))
    .SetMaterial(water);
  all_families.AddVolume(trap);
  geometry::GGEMSCons cons;
  cons.SetNegativeZRadii(0_mm, 4_mm)
    .SetPositiveZRadii(0_mm, 4_mm)
    .SetLength(8_mm)
    .SetPosition(Position({50, 0, 0}))
    .SetMaterial(water);
  all_families.AddVolume(cons);
  std::array const solid_inner{0_mm, 0_mm, 0_mm};
  polygon.SetZProfile(z, solid_inner, outer).SetPosition(Position({70, 0, 0}));
  all_families.AddVolume(polygon);
  // All eight registered families have the same independent 8 mm central chord.
  std::vector<Crossing> family_crossings;
  for (std::uint32_t index = 0U; index < 8U; ++index) {
    double const center = -70.0 + (20.0 * index);
    family_crossings.push_back(
      {.parameter_mm = center + 86.0, .arrival = index + 1U});
    family_crossings.push_back({.parameter_mm = center + 94.0, .arrival = 0U});
  }
  cases.push_back({.name = "all eight families retain occurrence-specific IDs",
                   .world = all_families,
                   .start = {-90, 0, 0},
                   .crossings = family_crossings});
  return cases;
}

auto CheckFixture(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
                  random::GGEMSRandom &random, Fixture const &fixture) -> void {
  SCOPED_TRACE(fixture.name);
  auto const &world = fixture.world;
  ASSERT_NO_THROW(world.Validate());
  std::vector<materials::GGEMSMaterial> media{world.GetMaterial()};
  double band = world.GetNavigationTolerancePM();
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    media.push_back(*world.GetVolumeMaterial(index));
    band = std::max(
      band, static_cast<double>(world.GetVolumeNavigationTolerancePM(index)));
  }
  materials::GGEMSEMMaterialPackage const package{media};
  auto const ids = package.GetMaterialIds().subspan(1);
  auto const records = world.BuildAnalyticDeviceRecords(ids);
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = std::llround(fixture.start[0] * k_mm);
  source.position_y_pm = std::llround(fixture.start[1] * k_mm);
  source.position_z_pm = std::llround(fixture.start[2] * k_mm);
  source.axis_z_x = static_cast<float>(fixture.direction[0]);
  source.axis_z_y = static_cast<float>(fixture.direction[1]);
  source.axis_z_z = static_cast<float>(fixture.direction[2]);
  source.energy_micro_eV = 123456789U;
  source.time_start_ps = 17U;
  source.time_stop_ps = 17U;
  auto const result =
    test::RunStream(context, program, random, world.BuildRecord(),
                    std::span{records}, source, nullptr, true, auxiliary);
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  auto material = [&](std::uint32_t owner) -> std::uint32_t {
    return owner == k_exterior ? k_exterior
                               : (owner == 0 ? 0U : ids[owner - 1U]);
  };
  EXPECT_EQ(result.trace[test::k_initial_owner_index], fixture.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            material(fixture.initial_owner));
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  auto expected = fixture.crossings;
  double exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (fixture.direction[axis] != 0) {
      exit = std::min(exit, (std::copysign(100.0, fixture.direction[axis]) -
                             fixture.start[axis]) /
                              fixture.direction[axis]);
    }
  }
  expected.push_back({exit, k_exterior});
  ASSERT_EQ(result.trace[0], expected.size());
  double const norm = std::hypot(fixture.direction[0], fixture.direction[1],
                                 fixture.direction[2]);
  // Frozen positional/flight budget: eight automatic surface half-bands.
  double const budget = 8 * band;
  double previous{};
  double accumulated{};
  auto departure = fixture.initial_owner;
  for (std::size_t index = 0; index < expected.size(); ++index) {
    SCOPED_TRACE(index);
    auto const offset = 1U + test::k_trace_stride * index;
    auto const &crossing = expected[index];
    EXPECT_EQ(result.trace[offset], departure);
    EXPECT_EQ(result.trace[offset + 1U], crossing.arrival);
    EXPECT_EQ(result.trace[offset + 7U], material(crossing.arrival));
    EXPECT_EQ(result.trace[offset + 6U],
              crossing.arrival == k_exterior ? 1U : (departure == 0 ? 2U : 3U));
    double const parameter = std::bit_cast<float>(
      static_cast<std::uint32_t>(result.trace[offset + 8U]));
    ASSERT_TRUE(std::isfinite(parameter));
    EXPECT_GT(parameter, 0);
    accumulated += parameter;
    double const expected_parameter = crossing.parameter_mm * k_mm;
    EXPECT_LE(std::abs((parameter - (expected_parameter - previous)) * norm),
              budget);
    EXPECT_LE(std::abs((accumulated - expected_parameter) * norm), budget);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      double const endpoint = std::bit_cast<float>(
        static_cast<std::uint32_t>(result.trace[offset + 3U + axis]));
      double const ideal = (fixture.start[axis] +
                            crossing.parameter_mm * fixture.direction[axis]) *
                           k_mm;
      EXPECT_LE(std::abs(endpoint - ideal), budget);
    }
    previous = expected_parameter;
    departure = crossing.arrival;
  }
  ASSERT_EQ(result.observer_counts.record_count, 2U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_FLOAT_EQ(terminal.direction_x, source.axis_z_x);
  EXPECT_FLOAT_EQ(terminal.direction_y, source.axis_z_y);
  EXPECT_FLOAT_EQ(terminal.direction_z, source.axis_z_z);
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest,
       MultipleOccurrencesPreservePhysicalIntervals) {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const instrumented = test::InstrumentStream(root);
  auto const cases = Cases();
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231U);
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport",
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                  root.generic_string(), random.GetKernelBuildDefinition()));
    cl::Program program{context.GetContextNative(), instrumented};
    auto const device = context.GetDevice().GetDeviceNative();
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    for (auto const &fixture : cases) {
      CheckFixture(context, program, random, fixture);
    }
  }
}

TEST_F(GGEMSNavigationDeviceTest,
       MultipleOccurrenceRunOwnsSnapshotInBothObserverModes) {
  for (bool observed : {false, true}) {
    SCOPED_TRACE(observed);
    auto world = Boxes(-20, 20);
    auto source = std::make_shared<sources::GGEMSSource>();
    source->SetAnalytic()
      .SetPrimaryCount(8U)
      .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
      .SetPositionPicoMeter(-60'000'000'000LL, 0, 0)
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
    run.SetWorld(world);
    world.SetSize(400_mm, 400_mm, 400_mm);
    ASSERT_NO_THROW(run.Initialize());
    ASSERT_NO_THROW(run.Run());
    ASSERT_NO_THROW(run.Run());
    if (observed) {
      auto const records = observer->GetRecords();
      ASSERT_EQ(records.size(), 2U);
      EXPECT_EQ(records[1].status,
                particles::ToKernelParticleStatus(
                  particles::GGEMSParticleStatus::EscapedWorld));
      EXPECT_LE(
        std::abs(static_cast<double>(records[1].position_x_pm) - 100 * k_mm),
        8 * world.GetNavigationTolerancePM());
      EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
    }
  }
}
