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
 * \brief Validates the declared FP32 surface band through actual transport.
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
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSFP32TestOracle.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {

// =============================================================================
// =============================================================================

using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Point = std::array<float, 3>;
using Center = std::array<std::int64_t, 3>;
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};
constexpr std::int64_t k_mm{1'000'000'000};

// =============================================================================
// =============================================================================

/*! \brief Packed geometry plus an independently declared tolerant route. */
struct Scene {
  std::string name;
  geometry::GGEMSWorldRecord world{};
  geometry::GGEMSAnalyticVolumeRecord volume{};
  std::optional<geometry::GGEMSBoxPoseRecord> pose;
  sources::GGEMSSourceRecord source{};
  std::uint32_t initial_owner{0};
  std::size_t events{1};
  bool outside_world{false};
};

// =============================================================================
// =============================================================================

auto SphereScene(std::int64_t world_size, std::int64_t radius,
                 Center center = {}, bool same_material = false) -> Scene {
  auto const vacuum = materials::builtins::BuildBuiltInMaterial("Vacuum");
  geometry::GGEMSWorld world;
  auto const size = Length{static_cast<std::uint64_t>(world_size)};
  world.SetSize(size, size, size).SetMaterial(vacuum);
  geometry::GGEMSSphere sphere;
  sphere.SetRadius(Length{static_cast<std::uint64_t>(radius)})
    .SetPosition(geometry::MakePositionPM(center[0], center[1], center[2]))
    .SetMaterial(same_material
                   ? vacuum
                   : materials::builtins::BuildBuiltInMaterial("Water"));
  world.AddVolume(sphere);
  world.Validate();
  Scene result;
  result.world = world.BuildRecord();
  result.volume = world.BuildAnalyticVolumeRecord(same_material ? 0U : 7U);
  return result;
}

// =============================================================================
// =============================================================================

auto BoxScene(std::int64_t world_size, std::int64_t full_size,
              bool rotated = false, bool diagonal = false) -> Scene {
  geometry::GGEMSWorld world;
  auto const size = Length{static_cast<std::uint64_t>(world_size)};
  world.SetSize(size, size, size)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  geometry::GGEMSBox box;
  auto const width = Length{static_cast<std::uint64_t>(full_size)};
  box.SetSize(width, width, width)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  if (rotated) {
    box.SetRotation(0_deg, 0_deg, diagonal ? 45_deg : 90_deg);
  }
  world.AddVolume(box);
  world.Validate();
  Scene result;
  result.world = world.BuildRecord();
  result.volume = world.BuildAnalyticVolumeRecord(7U);
  result.pose = world.BuildBoxPoseRecord();
  return result;
}

// =============================================================================
// =============================================================================

auto AddRay(std::vector<Scene> &scenes, Scene scene, std::string name,
            Point position, Point direction, std::uint32_t owner,
            std::size_t events, bool outside_world = false) -> void {
  scene.name = std::move(name);
  scene.initial_owner = owner;
  scene.events = events;
  scene.outside_world = outside_world;
  scene.source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  scene.source.position_x_pm = std::llround(position[0]);
  scene.source.position_y_pm = std::llround(position[1]);
  scene.source.position_z_pm = std::llround(position[2]);
  scene.source.axis_z_x = direction[0];
  scene.source.axis_z_y = direction[1];
  scene.source.axis_z_z = direction[2];
  scene.source.energy_micro_eV = 123'456'789;
  scene.source.time_start_ps = 17;
  scene.source.time_stop_ps = 17;
  scenes.push_back(std::move(scene));
}

// =============================================================================
// =============================================================================

auto ReadFloat(std::int64_t bits) -> float {
  return std::bit_cast<float>(static_cast<std::uint32_t>(bits));
}

/*! \brief Compares publication and counters with the unchanged worker program.
 *
 * This control checks that witness capture did not change observable transport.
 * It does not infer the uninstrumented worker's hidden intermediate owners.
 */
auto CheckProductionControl(test::StreamResult const &captured,
                            test::StreamResult const &production) -> void {
  EXPECT_TRUE(production.unchanged_rng);
  auto const &actual = captured.counts;
  auto const &control = production.counts;
  EXPECT_EQ(actual.completed_history_count, control.completed_history_count);
  EXPECT_EQ(actual.escaped_world_count, control.escaped_world_count);
  EXPECT_EQ(actual.unresolved_geometry_count,
            control.unresolved_geometry_count);
  EXPECT_EQ(actual.outside_world_count, control.outside_world_count);
  EXPECT_EQ(actual.overflow_count, control.overflow_count);
  EXPECT_EQ(actual.created_secondary_count, control.created_secondary_count);
  EXPECT_EQ(actual.total_fake_step_count, control.total_fake_step_count);
  EXPECT_EQ(captured.observer_counts.record_count,
            production.observer_counts.record_count);
  EXPECT_EQ(captured.observer_counts.overflow_count,
            production.observer_counts.overflow_count);
  EXPECT_EQ(captured.observer_counts.captured_primary_count,
            production.observer_counts.captured_primary_count);
  ASSERT_LE(production.observer_counts.record_count, production.records.size());
  for (std::size_t index = 0; index < production.observer_counts.record_count;
       ++index) {
    SCOPED_TRACE(index);
    auto const &record = captured.records[index];
    auto const &reference = production.records[index];
    EXPECT_EQ(record.record_kind, reference.record_kind);
    EXPECT_EQ(record.status, reference.status);
    EXPECT_EQ(record.position_valid, reference.position_valid);
    EXPECT_EQ(record.position_x_pm, reference.position_x_pm);
    EXPECT_EQ(record.position_y_pm, reference.position_y_pm);
    EXPECT_EQ(record.position_z_pm, reference.position_z_pm);
    EXPECT_EQ(record.direction_x, reference.direction_x);
    EXPECT_EQ(record.direction_y, reference.direction_y);
    EXPECT_EQ(record.direction_z, reference.direction_z);
    EXPECT_EQ(record.time_ps, reference.time_ps);
    EXPECT_EQ(record.energy_micro_eV, reference.energy_micro_eV);
    EXPECT_EQ(record.deposited_energy_micro_eV,
              reference.deposited_energy_micro_eV);
    EXPECT_EQ(record.global_primary_id, reference.global_primary_id);
    EXPECT_EQ(record.global_particle_id, reference.global_particle_id);
    EXPECT_EQ(record.source_local_primary_id,
              reference.source_local_primary_id);
    EXPECT_EQ(record.particle_type, reference.particle_type);
    EXPECT_EQ(record.track_id, reference.track_id);
    EXPECT_EQ(record.parent_track_id, reference.parent_track_id);
    EXPECT_EQ(record.generation, reference.generation);
    EXPECT_EQ(record.source_index, reference.source_index);
    EXPECT_EQ(record.run_id, reference.run_id);
  }
}

// =============================================================================
// =============================================================================

auto CheckHistory(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
                  cl::Program const &production, random::GGEMSRandom &random,
                  Scene const &scene) -> void {
  SCOPED_TRACE(scene.name);
  auto const result =
    test::RunStream(context, program, random, scene.world, scene.volume,
                    scene.source, scene.pose ? &*scene.pose : nullptr);
  auto const control =
    test::RunStream(context, production, random, scene.world, scene.volume,
                    scene.source, scene.pose ? &*scene.pose : nullptr, false);
  CheckProductionControl(result, control);
  auto const &counts = result.counts;
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(counts.overflow_count, 0U);
  EXPECT_EQ(counts.created_secondary_count, 0U);
  EXPECT_EQ(counts.total_fake_step_count, 0U);
  EXPECT_EQ(counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  EXPECT_EQ(result.trace[0], scene.events);
  EXPECT_EQ(counts.outside_world_count, scene.outside_world ? 1U : 0U);
  EXPECT_EQ(counts.completed_history_count, scene.outside_world ? 0U : 1U);
  EXPECT_EQ(counts.escaped_world_count, scene.outside_world ? 0U : 1U);
  EXPECT_EQ(result.observer_counts.record_count, scene.outside_world ? 1U : 2U);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U],
            scene.outside_world ? 1U : 0U);
  if (scene.outside_world) {
    return;
  }
  std::uint32_t const solid_material =
    scene.volume.kind == geometry::GGEMSAnalyticKind::Sphere
      ? scene.volume.parameters.sphere.material_id
      : scene.volume.parameters.box.material_id;
  EXPECT_EQ(result.trace[test::k_initial_owner_index], scene.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            scene.initial_owner == 0U ? 0U : solid_material);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Terminal));
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, scene.source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, scene.source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_EQ(terminal.direction_x, scene.source.axis_z_x);
  EXPECT_EQ(terminal.direction_y, scene.source.axis_z_y);
  EXPECT_EQ(terminal.direction_z, scene.source.axis_z_z);
  auto const packed = sources::BuildSourceDeviceRecord(scene.source);
  Point position{
    packed.position_x_pm,
    packed.position_y_pm,
    packed.position_z_pm,
  };
  Point const direction{packed.axis_z_x, packed.axis_z_y, packed.axis_z_z};
  std::uint32_t owner = scene.initial_owner;
  double solid_parameter{0};
  auto const observed = static_cast<std::size_t>(result.trace[0]);
  for (std::size_t index = 0; index < std::min(scene.events, observed);
       ++index) {
    auto const offset = 1U + (test::k_trace_stride * index);
    bool const world_exit = index + 1U == scene.events;
    auto arrival = owner == 0U ? 1U : 0U;
    auto event = owner == 0U ? 2U : 3U;
    auto material = arrival == 0U ? 0U : solid_material;
    if (world_exit) {
      arrival = k_exterior;
      event = 1U;
      material = k_exterior;
    }
    EXPECT_EQ(result.trace[offset], owner);
    EXPECT_EQ(result.trace[offset + 1U], arrival);
    EXPECT_EQ(result.trace[offset + 6U], event);
    EXPECT_EQ(result.trace[offset + 7U], material);
    float const parameter = ReadFloat(result.trace[offset + 8U]);
    EXPECT_TRUE(std::isfinite(parameter));
    EXPECT_GE(parameter, 0.0F);
    if (owner == 1U) {
      solid_parameter += parameter;
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      float const endpoint = ReadFloat(result.trace[offset + 3U + axis]);
      // Each accepted parameter is the physical move. This operation budget
      // detects any additional relocation hidden in the committed endpoint.
      namespace fp = test::fp32;
      auto const expected = fp::Add(
        fp::FromFloat(position[axis]),
        fp::Multiply(fp::FromFloat(parameter), fp::FromFloat(direction[axis])));
      fp::ExpectWithin(endpoint, expected);
      position[axis] = endpoint;
    }
    owner = arrival;
  }
  if (scene.events == 1U) {
    EXPECT_EQ(solid_parameter, 0.0);
  } else {
    EXPECT_GT(solid_parameter, 0.0);
  }
  std::cout << std::format(
    "TOLERANCE_CASE,{},{},initial={},events={},completed={},unresolved={},"
    "solid_parameter_pm={}\n",
    context.GetDevice().GetName(), scene.name,
    result.trace[test::k_initial_owner_index], result.trace[0],
    counts.completed_history_count, counts.unresolved_geometry_count,
    solid_parameter);
}

// =============================================================================
// =============================================================================

auto RunScenes(std::vector<Scene> const &scenes) -> void {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const source = test::InstrumentStream(root);
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231);
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    auto const options =
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                  root.generic_string(), random.GetKernelBuildDefinition());
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport", options);
    cl::Program program{context.GetContextNative(), source};
    auto const device = context.GetDevice().GetDeviceNative();
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    for (auto const &scene : scenes) {
      CheckHistory(context, program, production.GetProgramNative(), random,
                   scene);
    }
  }
}
} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, SphereToleranceBandAndMedicalScales) {
  std::vector<Scene> scenes;
  std::array<std::int64_t, 3> const sizes{100 * k_mm, 1000 * k_mm, 3000 * k_mm};
  std::array<std::int64_t, 3> const radii{k_mm, 10 * k_mm, 50 * k_mm};
  for (std::size_t scale = 0; scale < sizes.size(); ++scale) {
    auto const base = SphereScene(sizes[scale], radii[scale]);
    auto const &sphere = base.volume.parameters.sphere;
    float const radius = sphere.radius_pm;
    float const band = sphere.tolerance_pm;
    float const inner = radius - band;
    float const outer = radius + band;
    EXPECT_GT(band, 128.0F);
    std::cout << std::format("TOLERANCE_POLICY,world_pm={},radius_pm={},"
                             "half_band_nm={},radius_ulp_pm={}\n",
                             sizes[scale], radius, band / 1000.0F,
                             std::nextafter(radius, INFINITY) - radius);
    auto const name = [scale](char const *label) -> std::string {
      return std::format("scale{} {}", scale, label);
    };
    for (float const offset : {-128.0F, -64.0F, 0.0F, 64.0F, 128.0F}) {
      AddRay(scenes, base, std::format("scale{} grazing {}pm", scale, offset),
             {-2 * radius, radius + offset, 0}, {1, 0, 0}, 0, 1);
    }
    std::array const grazing{
      std::nextafter(inner, -INFINITY),
      inner,
      std::nextafter(inner, INFINITY),
      outer,
    };
    for (std::size_t index = 0; index < grazing.size(); ++index) {
      AddRay(
        scenes, base, std::format("scale{} grazing band edge{}", scale, index),
        {-2 * radius, grazing[index], 0}, {1, 0, 0}, 0, index == 0U ? 3U : 1U);
    }
    std::array const births{
      std::nextafter(inner, -INFINITY), inner, std::nextafter(inner, INFINITY),
      std::nextafter(outer, -INFINITY), outer, std::nextafter(outer, INFINITY),
    };
    for (std::size_t index = 0; index < births.size(); ++index) {
      bool const strict_inside = index == 0U;
      bool const strict_outside = index + 1U == births.size();
      AddRay(scenes, base, std::format("scale{} outward birth{}", scale, index),
             {births[index], 0, 0}, {1, 0, 0}, strict_inside ? 1U : 0U,
             strict_inside ? 2U : 1U);
      AddRay(scenes, base, std::format("scale{} inward birth{}", scale, index),
             {births[index], 0, 0}, {-1, 0, 0}, strict_outside ? 0U : 1U,
             strict_outside ? 3U : 2U);
      AddRay(scenes, base, std::format("scale{} tangent birth{}", scale, index),
             {births[index], 0, 0}, {0, 1, 0}, strict_inside ? 1U : 0U,
             strict_inside ? 2U : 1U);
    }
    AddRay(scenes, base, name("center birth"), {0, 0, 0}, {1, 0, 0}, 1, 2);
    AddRay(scenes, base, name("ordinary secant"), {-2 * radius, radius / 2, 0},
           {1, 0, 0}, 0, 3);
    AddRay(scenes, base, name("clear miss"),
           {-2 * radius, radius + (4 * band), 0}, {1, 0, 0}, 0, 1);
  }
  RunScenes(scenes);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, SphereToleranceTranslatedOblique) {
  std::vector<Scene> scenes;
  for (int exponent : {28, 30, 33}) {
    auto const unit = static_cast<std::int64_t>(
      std::uint64_t{1} << static_cast<unsigned>(exponent));
    auto const center_value = static_cast<std::int64_t>(
      std::uint64_t{1} << static_cast<unsigned>(exponent + 6));
    Center const center{center_value, -center_value, 0};
    auto const base = SphereScene(3000 * k_mm, 5 * unit, center);
    auto const x = static_cast<float>(center[0] + (3 * unit));
    auto const y = static_cast<float>(center[1] + (4 * unit));
    AddRay(scenes, base, std::format("translated oblique{} outward", exponent),
           {x, y, 0}, {0.6F, 0.8F, 0}, 0, 1);
    AddRay(scenes, base, std::format("translated oblique{} inward", exponent),
           {x, y, 0}, {-0.6F, -0.8F, 0}, 1, 2);
    AddRay(scenes, base, std::format("translated oblique{} tangent", exponent),
           {x, y, 0}, {0, 0, 1}, 0, 1);
  }
  auto const same = SphereScene(100 * k_mm, k_mm, {}, true);
  AddRay(scenes, same, "same Material tangent", {-2e9F, 1e9F, 0}, {1, 0, 0}, 0,
         1);
  AddRay(scenes, same, "same Material secant", {-2e9F, 0.5e9F, 0}, {1, 0, 0}, 0,
         3);
  auto const automatic = SphereScene(100 * k_mm, k_mm);
  float const band = automatic.volume.parameters.sphere.tolerance_pm;
  AddRay(scenes, automatic, "automatic accepts resolved penetration",
         {-2e9F, 1e9F - (2 * band), 0}, {1, 0, 0}, 0, 3);
  RunScenes(scenes);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       NavigationToleranceWorldBoxAndRemoteFeatures) {
  std::vector<Scene> scenes;
  auto const box = BoxScene(3000 * k_mm, 20'000'000);
  AddRay(scenes, box, "remote 20um Box", {-1e12F, 0, 0}, {1, 0, 0}, 0, 3);
  auto const rotated = BoxScene(100 * k_mm, 20 * k_mm, true);
  AddRay(scenes, rotated, "rotated Box secant", {0, -20e9F, 0}, {0, 1, 0}, 0,
         3);
  float const half = rotated.volume.parameters.box.half_extent_x_pm;
  float const band = rotated.volume.parameters.box.tolerance_pm;
  AddRay(scenes, rotated, "rotated Box tangent face band",
         {-half, -2 * half, 0}, {0, 1, 0}, 0, 1);
  AddRay(scenes, rotated, "rotated Box penetrates beyond band",
         {-half + (4 * band), -2 * half, 0}, {0, 1, 0}, 0, 3);
  auto const diagonal = BoxScene(100 * k_mm, 20 * k_mm, true, true);
  auto const &pose = *diagonal.pose;
  Point const face{
    half * pose.axis_x_x,
    half * pose.axis_x_y,
    half * pose.axis_x_z,
  };
  AddRay(scenes, diagonal, "45-degree Box positive face tangent", face,
         {pose.axis_y_x, pose.axis_y_y, pose.axis_y_z}, 0, 1);
  AddRay(scenes, diagonal, "45-degree Box negative face tangent", face,
         {-pose.axis_y_x, -pose.axis_y_y, -pose.axis_y_z}, 0, 1);
  auto world = rotated;
  world.volume = {};
  world.pose.reset();
  float const world_half = world.world.half_extent_x_pm;
  float const world_band = world.world.tolerance_pm;
  for (float const offset : {-world_band, 0.0F, world_band}) {
    AddRay(scenes, world, std::format("World closed band {}pm", offset),
           {world_half + offset, 0, 0}, {1, 0, 0}, 0, 1);
  }
  AddRay(scenes, world, "World beyond band rejects source",
         {world_half + (2 * world_band), 0, 0}, {1, 0, 0}, 0, 0, true);
  auto const small = SphereScene(3000 * k_mm, 1'100'000);
  EXPECT_GT(small.volume.parameters.sphere.radius_pm,
            small.volume.parameters.sphere.tolerance_pm);
  AddRay(scenes, small, "remote Sphere near admission", {-1e12F, 0, 0},
         {1, 0, 0}, 0, 3);
  auto const far = SphereScene(3000 * k_mm, k_mm);
  float const tolerance = far.volume.parameters.sphere.tolerance_pm;
  for (float const transverse :
       {0.5e9F, 1e9F, 1e9F - (4 * tolerance), 1e9F + (4 * tolerance)}) {
    Point const position{
      -0.6e12F - (0.8F * transverse),
      -0.8e12F + (0.6F * transverse),
      0,
    };
    // The line is prescribed through its transverse offset. Four half-bands
    // separate the resolved hit/miss controls from upload and projection error.
    bool const hit = transverse < 1e9F - tolerance;
    AddRay(scenes, far, std::format("remote oblique transverse{}", transverse),
           position, {0.6F, 0.8F, 0}, 0, hit ? 3U : 1U);
  }
  RunScenes(scenes);
}
