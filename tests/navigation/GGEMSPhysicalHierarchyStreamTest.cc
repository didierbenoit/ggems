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
 * \brief Verifies nested physical ownership through the common OpenCL worker.
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
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
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
  std::uint32_t event;
  std::uint32_t face_mask{0U};
};

struct Scene {
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

auto Box(Length x, Length y, Length z, Vec center = {0, 0, 0},
         char const *material = "Water") -> geometry::GGEMSBox {
  geometry::GGEMSBox box;
  box.SetSize(x, y, z)
    .SetPosition(Position(center))
    .SetMaterial(materials::builtins::BuildBuiltInMaterial(material));
  return box;
}

auto Sphere(Length radius, Vec center = {0, 0, 0}) -> geometry::GGEMSSphere {
  geometry::GGEMSSphere sphere;
  sphere.SetRadius(radius)
    .SetPosition(Position(center))
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Aluminum"));
  return sphere;
}

auto NestedWorld() -> geometry::GGEMSWorld {
  auto world = World();
  world.AddVolume(Box(60_mm, 40_mm, 40_mm));
  world.AddVolume(Sphere(10_mm), 1U);
  return world;
}

// Goldens use independent central chords, slab intervals, and quarter turns.
// Events are explicit: descendant entry2, ancestor/sibling departure3, World1.
auto Scenes() -> std::vector<Scene> {
  std::vector<Scene> scenes;
  auto const nested = NestedWorld();
  scenes.push_back(
    {.name = "H1 World Box Sphere Box World",
     .world = nested,
     .crossings = {{30, 1, 2}, {50, 2, 2}, {70, 1, 3}, {90, 0, 3}}});
  scenes.push_back({.name = "H1 birth in mother material",
                    .world = nested,
                    .start = {-20, 0, 0},
                    .crossings = {{10, 2, 2}, {30, 1, 3}, {50, 0, 3}},
                    .initial_owner = 1U});
  scenes.push_back({.name = "H1 birth in nested daughter",
                    .world = nested,
                    .start = {0, 0, 0},
                    .crossings = {{10, 1, 3}, {30, 0, 3}},
                    .initial_owner = 2U});
  scenes.push_back({.name = "H1 daughter boundary birth outward",
                    .world = nested,
                    .start = {10, 0, 0},
                    .crossings = {{20, 0, 3}},
                    .initial_owner = 1U});
  scenes.push_back({.name = "H1 daughter boundary birth inward",
                    .world = nested,
                    .start = {10, 0, 0},
                    .direction = {-1, 0, 0},
                    .crossings = {{20, 1, 3}, {40, 0, 3}},
                    .initial_owner = 2U});

  auto depth_three = World();
  depth_three.AddVolume(Box(80_mm, 60_mm, 60_mm));
  depth_three.AddVolume(Sphere(20_mm), 1U);
  depth_three.AddVolume(Box(12_mm, 6_mm, 8_mm, {4, 0, 0}, "Air"), 2U);
  scenes.push_back({.name = "H2 three physical levels forward",
                    .world = depth_three,
                    .crossings = {{20, 1, 2},
                                  {40, 2, 2},
                                  {58, 3, 2},
                                  {70, 2, 3},
                                  {80, 1, 3},
                                  {100, 0, 3}}});
  scenes.push_back({.name = "H2 three physical levels reverse",
                    .world = depth_three,
                    .start = {60, 0, 0},
                    .direction = {-1, 0, 0},
                    .crossings = {{20, 1, 2},
                                  {40, 2, 2},
                                  {50, 3, 2},
                                  {62, 2, 3},
                                  {80, 1, 3},
                                  {100, 0, 3}}});
  scenes.push_back({.name = "H2 birth at deepest owner",
                    .world = depth_three,
                    .start = {4, 0, 0},
                    .crossings = {{6, 2, 3}, {16, 1, 3}, {36, 0, 3}},
                    .initial_owner = 3U});

  for (bool reversed : {false, true}) {
    auto siblings = World();
    siblings.AddVolume(Box(80_mm, 60_mm, 60_mm));
    auto const sphere = Sphere(5_mm, {-15, 0, 0});
    auto const box = Box(8_mm, 10_mm, 12_mm, {15, 0, 0}, "Air");
    if (reversed) {
      siblings.AddVolume(box, 1U).AddVolume(sphere, 1U);
    } else {
      siblings.AddVolume(sphere, 1U).AddVolume(box, 1U);
    }
    std::uint32_t const left = reversed ? 3U : 2U;
    std::uint32_t const right = reversed ? 2U : 3U;
    scenes.push_back({.name = reversed ? "H3 reversed daughter registration"
                                       : "H3 ordered daughters",
                      .world = siblings,
                      .crossings = {{20, 1, 2},
                                    {40, left, 2},
                                    {50, 1, 3},
                                    {71, right, 2},
                                    {79, 1, 3},
                                    {100, 0, 3}}});
  }

  auto posed = World();
  auto mother = Box(80_mm, 40_mm, 30_mm, {10, -5, 3});
  mother.SetRotation(0_deg, 0_deg, 90_deg);
  auto daughter = Box(12_mm, 8_mm, 10_mm, {6, 3, -1}, "Aluminum");
  daughter.SetRotation(0_deg, 0_deg, 90_deg);
  posed.AddVolume(mother).AddVolume(daughter, 1U);
  scenes.push_back(
    {.name = "H4 composed local poses forward",
     .world = posed,
     .start = {7, -55, 2},
     .direction = {0, 1, 0},
     .crossings = {{10, 1, 2}, {52, 2, 2}, {60, 1, 3}, {90, 0, 3}}});
  scenes.push_back(
    {.name = "H4 composed local poses reverse",
     .world = posed,
     .start = {7, 45, 2},
     .direction = {0, -1, 0},
     .crossings = {{10, 1, 2}, {40, 2, 2}, {48, 1, 3}, {90, 0, 3}}});

  auto sphere_frame = World();
  auto sphere_parent = Sphere(30_mm);
  sphere_parent.SetRotation(0_deg, 0_deg, 90_deg);
  sphere_frame.AddVolume(sphere_parent);
  sphere_frame.AddVolume(Box(8_mm, 4_mm, 6_mm, {10, 0, 0}), 1U);
  scenes.push_back(
    {.name = "H4 rotated Sphere carries its daughter frame",
     .world = sphere_frame,
     .start = {0, -60, 0},
     .direction = {0, 1, 0},
     .crossings = {{30, 1, 2}, {66, 2, 2}, {74, 1, 3}, {90, 0, 3}}});

  auto touching = World();
  touching.AddVolume(Box(60_mm, 40_mm, 40_mm));
  touching.AddVolume(Box(30_mm, 16_mm, 16_mm, {-15, 0, 0}, "Aluminum"), 1U);
  touching.AddVolume(Box(10_mm, 16_mm, 16_mm, {5, 0, 0}, "Aluminum"), 1U);
  scenes.push_back(
    {.name = "H5 coincident ancestor entry and sibling contact",
     .world = touching,
     .crossings = {{30, 2, 2}, {60, 3, 3}, {70, 1, 3}, {90, 0, 3}}});
  scenes.push_back(
    {.name = "H5 coincident ancestor exit in reverse",
     .world = touching,
     .start = {60, 0, 0},
     .direction = {-1, 0, 0},
     .crossings = {{30, 1, 2}, {50, 3, 2}, {60, 2, 3}, {90, 0, 3}}});
  scenes.push_back({.name = "H5 birth at sibling contact",
                    .world = touching,
                    .start = {0, 0, 0},
                    .crossings = {{10, 1, 3}, {30, 0, 3}},
                    .initial_owner = 3U});

  auto clipped = World();
  clipped.AddVolume(Box(40_mm, 40_mm, 40_mm));
  clipped.AddVolume(Sphere(10_mm, {18, 0, 0}), 1U);
  clipped.AddVolume(Box(20_mm, 4_mm, 6_mm, {6, 0, 0}, "Air"), 2U);
  scenes.push_back(
    {.name = "H6 ancestor truncates two protruding descendants",
     .world = clipped,
     .crossings = {{40, 1, 2}, {68, 2, 2}, {74, 3, 2}, {80, 0, 3, 2U}}});
  scenes.push_back(
    {.name = "H6 reverse entry selects deepest clipped support",
     .world = clipped,
     .start = {60, 0, 0},
     .direction = {-1, 0, 0},
     .crossings = {{40, 3, 2}, {46, 2, 3}, {52, 1, 3}, {80, 0, 3}}});
  scenes.push_back(
    {.name = "H6 birth in nominal child outside ancestor is World",
     .world = clipped,
     .start = {24, 0, 0},
     .crossings = {}});

  auto hollow = World();
  geometry::GGEMSPolyhedra polygon;
  std::array const z{units::PositionCoordinate{-15'000'000'000LL},
                     units::PositionCoordinate{15'000'000'000LL}};
  std::array const inner{6_mm, 6_mm};
  std::array const outer{12_mm, 12_mm};
  polygon.SetSideCount(4U)
    .SetStartAngle(45_deg)
    .SetZProfile(z, inner, outer)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  hollow.AddVolume(polygon);
  hollow.AddVolume(Box(20_mm, 4_mm, 6_mm, {0, 0, 0}, "Aluminum"), 1U);
  scenes.push_back(
    {.name = "H6 nonconvex ancestor permits later convex child reentry",
     .world = hollow,
     .crossings = {{48, 1, 2},
                   {50, 2, 2},
                   {54, 0, 3},
                   {66, 2, 2},
                   {70, 1, 3},
                   {72, 0, 3}}});
  return scenes;
}

auto CheckScene(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
                random::GGEMSRandom &random, Scene const &scene) -> void {
  SCOPED_TRACE(scene.name);
  auto const &world = scene.world;
  ASSERT_NO_THROW(world.Validate());
  std::vector<materials::GGEMSMaterial> media{world.GetMaterial()};
  double band = world.GetNavigationTolerancePM();
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    media.push_back(*world.GetVolumeMaterial(index));
    band = std::max(
      band, static_cast<double>(world.GetVolumeNavigationTolerancePM(index)));
  }
  materials::GGEMSEMMaterialPackage const package{media};
  auto const ids = package.GetMaterialIds();
  auto const volumes = world.BuildAnalyticDeviceRecords(ids.subspan(1U));
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = std::llround(scene.start[0] * k_mm);
  source.position_y_pm = std::llround(scene.start[1] * k_mm);
  source.position_z_pm = std::llround(scene.start[2] * k_mm);
  source.axis_z_x = static_cast<float>(scene.direction[0]);
  source.axis_z_y = static_cast<float>(scene.direction[1]);
  source.axis_z_z = static_cast<float>(scene.direction[2]);
  source.energy_micro_eV = 123456789U;
  source.time_start_ps = 17U;
  source.time_stop_ps = 17U;
  auto const result =
    test::RunStream(context, program, random, world.BuildRecord(),
                    std::span{volumes}, source, nullptr, true, auxiliary);
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  EXPECT_EQ(result.counts.total_fake_step_count, 0U);
  EXPECT_EQ(result.counts.created_secondary_count, 0U);
  auto material = [&](std::uint32_t owner) {
    return owner == k_exterior ? k_exterior : ids[owner];
  };
  EXPECT_EQ(result.trace[test::k_initial_owner_index], scene.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            material(scene.initial_owner));
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  auto expected = scene.crossings;
  double exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (scene.direction[axis] != 0) {
      exit = std::min(exit, (std::copysign(100.0, scene.direction[axis]) -
                             scene.start[axis]) /
                              scene.direction[axis]);
    }
  }
  expected.push_back(
    {.parameter_mm = exit, .arrival = k_exterior, .event = 1U});
  ASSERT_EQ(result.trace[0], expected.size());
  double const norm =
    std::hypot(scene.direction[0], scene.direction[1], scene.direction[2]);
  // Predeclared budget matches the existing N2.1 physical-flight oracle.
  double const budget = 8 * band;
  double previous{};
  double accumulated{};
  auto departure = scene.initial_owner;
  for (std::size_t index = 0; index < expected.size(); ++index) {
    SCOPED_TRACE(index);
    auto const offset = 1U + (test::k_trace_stride * index);
    auto const &crossing = expected[index];
    EXPECT_EQ(result.trace[offset], departure);
    EXPECT_EQ(result.trace[offset + 1U], crossing.arrival);
    EXPECT_EQ(result.trace[offset + 6U], crossing.event);
    if (crossing.face_mask != 0U) {
      EXPECT_EQ(result.trace[offset + 2U], crossing.face_mask);
    }
    EXPECT_EQ(result.trace[offset + 7U], material(crossing.arrival));
    double const parameter = std::bit_cast<float>(
      static_cast<std::uint32_t>(result.trace[offset + 8U]));
    ASSERT_TRUE(std::isfinite(parameter));
    EXPECT_GT(parameter, 0);
    accumulated += parameter;
    double const ideal_parameter = crossing.parameter_mm * k_mm;
    EXPECT_LE(std::abs((parameter - (ideal_parameter - previous)) * norm),
              budget);
    EXPECT_LE(std::abs((accumulated - ideal_parameter) * norm), budget);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      double const endpoint = std::bit_cast<float>(
        static_cast<std::uint32_t>(result.trace[offset + 3U + axis]));
      double const ideal =
        (scene.start[axis] + (crossing.parameter_mm * scene.direction[axis])) *
        k_mm;
      EXPECT_LE(std::abs(endpoint - ideal), budget);
    }
    previous = ideal_parameter;
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
       PhysicalHierarchyPreservesH1ThroughH6Intervals) {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const instrumented = test::InstrumentStream(root);
  auto const scenes = Scenes();
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
    for (auto const &scene : scenes) {
      CheckScene(context, program, random, scene);
    }
  }
}

TEST_F(GGEMSNavigationDeviceTest,
       PhysicalHierarchyRunOwnsSnapshotInBothObserverModes) {
  for (bool observed : {false, true}) {
    SCOPED_TRACE(observed);
    auto world = NestedWorld();
    auto const band = world.GetNavigationTolerancePM();
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
        std::abs(static_cast<double>(records[1].position_x_pm) - (100 * k_mm)),
        8 * band);
      EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
    }
  }
}
