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
 * \brief Verifies physical owner transitions through analytic and mesh volumes.
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
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <numbers>
#include <random>
#include <span>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMSOpenCLDeviceInventory.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSMesh.hh"
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

/*!
 * \brief Imports named slab shells with oppositely triangulated contact faces.
 *
 * \param[in] left_edges Local x coordinates in mm of the left slab faces.
 * \param[in] width_mm Width of every slab in mm along the x axis.
 * \param[in] y_step_mm Translation along y between successive slabs in mm.
 * \return World containing one homogeneous mesh with all of the slabs.
 */
auto SlabWorld(std::span<double const> left_edges, double width_mm,
               double y_step_mm = 0) -> geometry::GGEMSWorld {
  constexpr std::array<std::array<unsigned, 3>, 12> faces{{
    {1, 3, 2},
    {1, 4, 3},
    {5, 6, 7},
    {5, 7, 8},
    {1, 2, 6},
    {1, 6, 5},
    {2, 3, 7},
    {2, 7, 6},
    {3, 4, 8},
    {3, 8, 7},
    {4, 1, 5},
    {4, 5, 8},
  }};
  auto const path = std::filesystem::temp_directory_path() /
                    std::format("ggems_contact_{}.obj", std::random_device{}());
  std::ofstream output;
  output.exceptions(std::ios::failbit | std::ios::badbit);
  output.open(path);
  output.precision(std::numeric_limits<double>::max_digits10);
  for (std::size_t index = 0; index < left_edges.size(); ++index) {
    double const left = left_edges[index];
    double const right = left + width_mm;
    output << "o leaf\n";
    for (auto const &vertex : std::array<Vec, 8>{{
           {left, -5, -6},
           {right, -5, -6},
           {right, 5, -6},
           {left, 5, -6},
           {left, -5, 6},
           {right, -5, 6},
           {right, 5, 6},
           {left, 5, 6},
         }}) {
      output << "v " << vertex[0] << ' '
             << (vertex[1] + (static_cast<double>(index) * y_step_mm)) << ' '
             << vertex[2] << '\n';
    }
    auto const offset = 8U * index;
    for (auto const &face : faces) {
      output << "f " << offset + face[0] << ' ' << offset + face[1] << ' '
             << offset + face[2] << '\n';
    }
  }
  output.close();
  geometry::GGEMSMesh mesh{path};
  std::filesystem::remove(path);
  mesh.SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  auto world = World();
  world.AddVolume(mesh);
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

/*!
 * \brief Instruments the common mesh-enabled Transport kernel.
 *
 * \param[in] context Selected device and program cache.
 * \param[in] random Random engine supplying the production build definition.
 * \return Cached trace program for this device and build definition.
 */
auto MeshTrace(ocl::GGEMSOpenCLContext &context,
               random::GGEMSRandom const &random) -> cl::Program const & {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
    context, root / "transport", "particle_stream_transport",
    std::format(
      "-I{} {} -DGGEMS_HAS_MESH=1 -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
      root.generic_string(), random.GetKernelBuildDefinition()));
  return test::GetAnalyticTraceProgram(test::InstrumentStream(root),
                                       production.GetBuildOptions());
}

/*!
 * \brief Compares a device Transport trace with independent crossing data.
 *
 * \param[in] context Selected OpenCL device context.
 * \param[in] program Instrumented common Transport program for the context.
 * \param[in,out] random Configured engine used for the native source history.
 * \param[in] fixture Authored World, initial ray and expected transitions.
 * \param[in] budget_half_bands Distance and endpoint budget in half-bands.
 */
auto CheckFixture(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
                  random::GGEMSRandom &random, Fixture const &fixture,
                  double budget_half_bands = 8) -> void {
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
  std::vector<std::uint32_t> analytic_ids;
  std::vector<std::uint32_t> mesh_ids;
  for (std::size_t index = 0; index < ids.size(); ++index) {
    (world.GetMeshVolume(index) != nullptr ? mesh_ids : analytic_ids)
      .push_back(ids[index]);
  }
  auto const records = world.BuildAnalyticDeviceRecords(analytic_ids);
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer(mesh_ids);
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
    if (owner == k_exterior || owner == 0U) {
      return owner;
    }
    return ids[owner - 1U];
  };
  EXPECT_EQ(result.trace[test::k_initial_owner_index], fixture.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            material(fixture.initial_owner));
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  auto expected = fixture.crossings;
  auto const half = world.GetHalfExtentPM();
  Vec const half_mm{static_cast<double>(half.x) / k_mm,
                    static_cast<double>(half.y) / k_mm,
                    static_cast<double>(half.z) / k_mm};
  double exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (fixture.direction[axis] != 0) {
      exit =
        std::min(exit, (std::copysign(half_mm[axis], fixture.direction[axis]) -
                        fixture.start[axis]) /
                         fixture.direction[axis]);
    }
  }
  expected.push_back({exit, k_exterior});
  ASSERT_EQ(result.trace[0], expected.size());
  double const norm = std::hypot(fixture.direction[0], fixture.direction[1],
                                 fixture.direction[2]);
  double const budget = budget_half_bands * band;
  double previous{};
  double accumulated{};
  auto departure = fixture.initial_owner;
  for (std::size_t index = 0; index < expected.size(); ++index) {
    SCOPED_TRACE(index);
    auto const offset = 1U + (test::k_trace_stride * index);
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
                            (crossing.parameter_mm * fixture.direction[axis])) *
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

// Checks owner transitions across distinct occurrences and shared profiles.
TEST_F(GGEMSNavigationDeviceTest,
       MultipleOccurrencesPreservePhysicalIntervals) {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const instrumented = test::InstrumentStream(root);
  auto const cases = Cases();
  {
    auto &context = ggems::test::GetSelectedOpenCLContext();
    SCOPED_TRACE(context.GetDevice().GetName());
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231U);
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport",
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                  root.generic_string(), random.GetKernelBuildDefinition()));
    auto const &program =
      test::GetAnalyticTraceProgram(instrumented, production.GetBuildOptions());
    for (auto const &fixture : cases) {
      CheckFixture(context, program, random, fixture);
    }
  }
}

// Compare the committed stream to independent slab intervals, including two
// named solids in face contact, a separate anonymous solid, and a closed void.
TEST_F(GGEMSNavigationDeviceTest, MeshPreservesDisconnectedIntervalsAndVoids) {
  auto &context = test::GetSelectedOpenCLContext();
  SCOPED_TRACE(context.GetDevice().GetName());
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231U);
  auto const &trace = MeshTrace(context, random);
  auto const folder =
    std::filesystem::path{GGEMS_TEST_PROBE_ROOT}.parent_path() / "navigation" /
    "meshes";
  geometry::GGEMSMesh mesh{folder / "three_slabs.obj"};
  mesh.SetMaterial(materials::builtins::BuildBuiltInMaterial("Tungsten"));
  auto world = World();
  world.SetMaterial(materials::builtins::BuildBuiltInMaterial("Air"));
  world.AddVolume(mesh);
  ASSERT_EQ(world.GetVolumeCount(), 1U);
  EXPECT_THROW(world.AttachVolume(mesh, 1U), GGEMSRecoverable);
  EXPECT_THROW(world.AddVolume(Box({0, 0, 0}), 1U), GGEMSRecoverable);
  world.AddVolume(Box({30, 0, 0}));
  mesh.SetRotation(0_deg, 0_deg, 90_deg).SetPosition(Position({1, 5, 0}));
  auto posed = World();
  posed.AddVolume(mesh);
  mesh.SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  EXPECT_EQ(world.GetVolumeMaterial(0)->GetName(), "Tungsten");
  auto const check = [&](Fixture const &fixture) -> void {
    CheckFixture(context, trace, random, fixture);
  };
  check({.name =
           "composite mesh, gap, shared triangle diagonal and analytic sibling",
         .world = world,
         .crossings = {{52, 1}, {56, 0}, {64, 1}, {72, 0}, {86, 2}, {94, 0}}});
  check({.name = "inside first leaf then later same-owner reentry",
         .world = world,
         .start = {-6, 0, 0},
         .crossings = {{2, 0}, {10, 1}, {18, 0}, {32, 2}, {40, 0}},
         .initial_owner = 1U});
  check({.name = "nonunit ray born on the front face",
         .world = world,
         .start = {-8, 0, 0},
         .direction = {2, 0, 0},
         .crossings = {{2, 0}, {6, 1}, {10, 0}, {17, 2}, {21, 0}},
         .initial_owner = 1U});
  check({.name = "miss both mesh intervals and sibling",
         .world = world,
         .start = {-60, 8, 0},
         .crossings = {}});
  check({.name = "glide along mesh faces",
         .world = world,
         .start = {-60, 2, 0},
         .crossings = {{86, 2}, {94, 0}}});
  check({.name = "rigid rotation and translation",
         .world = posed,
         .start = {1, -60, 0},
         .direction = {0, 1, 0},
         .crossings = {{57, 1}, {61, 0}, {69, 1}, {77, 0}}});
  auto hierarchy = world;
  auto daughter = Box({0, 0, 0}, "Aluminum");
  daughter.SetSize(2_mm, 2_mm, 2_mm);
  hierarchy.AddVolume(daughter, 2U);
  // The ray crosses the mesh and the sibling mother, above its daughter.
  check({.name = "disconnected mesh beside a physical hierarchy",
         .world = hierarchy,
         .start = {-60, 1.5, 0},
         .crossings = {{52, 1}, {56, 0}, {64, 1}, {72, 0}, {86, 2}, {94, 0}}});
  auto hollow = World();
  geometry::GGEMSMesh shell{folder / "hollow_box.obj"};
  shell.SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  hollow.AddVolume(shell);
  check({.name = "anonymous BLAS enclosing a void",
         .world = hollow,
         .crossings = {{52, 1}, {56, 0}, {64, 1}, {68, 0}}});
}

// Compare one box with eight named, closed boxes that tile its exact volume.
// Opposite contact faces use different diagonals, as in the original fixture.
TEST_F(GGEMSNavigationDeviceTest,
       MeshContactFacetsPreserveResolvedHomogeneousUnion) {
  auto &context = test::GetSelectedOpenCLContext();
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231U);
  auto const &program = MeshTrace(context, random);
  auto const band_pm =
    SlabWorld(std::array{0.0}, 8.0).GetVolumeNavigationTolerancePM(0);
  for (double const factor : {1.0, 1.5}) {
    SCOPED_TRACE(factor);
    double const tile_width = factor * band_pm / k_mm;
    double const union_width = 8 * tile_width;
    auto const control = SlabWorld(std::array{0.0}, union_width);
    std::array<double, 8> left_edges{};
    for (std::size_t index = 0; index < left_edges.size(); ++index) {
      left_edges[index] = static_cast<double>(index) * tile_width;
    }
    auto const composite = SlabWorld(left_edges, tile_width);
    ASSERT_EQ(control.GetVolumeNavigationTolerancePM(0), band_pm);
    ASSERT_EQ(composite.GetVolumeNavigationTolerancePM(0), band_pm);
    std::vector<Crossing> const outer{{60, 1}, {60 + union_width, 0}};
    // A transition on the neighboring internal face is at least one T away.
    auto const check = [&](Fixture const &fixture) -> void {
      CheckFixture(context, program, random, fixture, 0.5);
    };
    check(
      {.name = "exterior-only control", .world = control, .crossings = outer});
    check({.name = "entry to eight contacting tiles",
           .world = composite,
           .crossings = outer});
    check({.name = "birth inside eight contacting tiles",
           .world = composite,
           .start = {union_width / 2, 0, 0},
           .crossings = {{union_width / 2, 0}},
           .initial_owner = 1});
  }
}

// Historical expectations for near-tangent chords and sub-band separations.
// Exact edge and vertex band coverage is outside the FP32 mesh contract.
TEST_F(GGEMSNavigationDeviceTest,
       DISABLED_MeshObliqueIntervalsRespectFiniteSurfaceBands) {
  auto &context = test::GetSelectedOpenCLContext();
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231U);
  auto const &program = MeshTrace(context, random);
  auto const check = [&](Fixture const &fixture) -> void {
    CheckFixture(context, program, random, fixture);
  };
  auto const single = SlabWorld(std::array{-24.0}, 8.0);
  double const band = single.GetVolumeNavigationTolerancePM(0) / k_mm;
  check({.name = "oblique shallow chord has no strict interior",
         .world = single,
         .start = {-24 - (0.5 * band), 5 - (100 * band), 0},
         .direction = {0.01, 1, 0},
         .crossings = {}});
  check({.name = "reverse traversal of the same shallow corner chord",
         .world = single,
         .start = {-24 + (0.75 * band), 5 + (25 * band), 0},
         .direction = {-0.01, -1, 0},
         .crossings = {}});
  // From the x crossing, its band covers [0, 4T] and the y face covers
  // [3.75T, 4.75T]. Their common interval lies beyond the chord midpoint.
  check({.name = "oblique face bands overlap away from the midpoint",
         .world = single,
         .start = {-24 - (0.25 * band), 5 - (5.75 * band), 0},
         .direction = {0.25, 1, 0},
         .crossings = {}});
  check({.name = "reverse traversal of the asymmetric corner chord",
         .world = single,
         .start = {-24 + (1.6875 * band), 5 + (2 * band), 0},
         .direction = {-0.25, -1, 0},
         .crossings = {}});
  // The upstream x face still bounds the entire chord when birth occurs on
  // that face or a quarter half-band inside it.
  for (auto const &start : {Vec{-24, 5 - (50 * band), 0},
                            Vec{-24 + (0.25 * band), 5 - (25 * band), 0}}) {
    SCOPED_TRACE(start[0]);
    check({.name = "surface-band birth with no forward strict interior",
           .world = single,
           .start = start,
           .direction = {0.01, 1, 0},
           .crossings = {}});
  }
  double const close_gap = 0.5 * band;
  check({.name = "oblique unresolved same-mesh gap has no Air step",
         .world = SlabWorld(std::array{-8.0, close_gap}, 8.0),
         .start = {-0.01, 0, 0},
         .direction = {0.01, 1, 0},
         .crossings = {{5, 0}},
         .initial_owner = 1});
}

TEST_F(GGEMSNavigationDeviceTest,
       MeshObliqueCrossingsRetainResolvedAirIntervals) {
  auto &context = test::GetSelectedOpenCLContext();
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231U);
  auto const &program = MeshTrace(context, random);
  auto const check = [&](Fixture const &fixture) -> void {
    CheckFixture(context, program, random, fixture);
  };
  double const band =
    SlabWorld(std::array{-8.0}, 8.0).GetVolumeNavigationTolerancePM(0) / k_mm;
  double const resolved_gap = 8 * band;
  check({.name = "oblique resolved same-mesh gap retains the Air interval",
         .world = SlabWorld(std::array{-8.0, resolved_gap}, 8.0),
         .start = {-0.01, 0, 0},
         .direction = {0.01, 1, 0},
         .crossings = {{1, 0}, {1 + (resolved_gap / 0.01), 1}, {5, 0}},
         .initial_owner = 1});

  // The x planes are only 0.5T apart, but the first slab ends at y = 5
  // and the next starts at y = 5 + 20T. Strict air separates their edges.
  double const close_gap = 0.5 * band;
  double const y_step = 10 + (20 * band);
  double const start_y = 4 - (2 * band);
  check({.name = "finite lateral separation preserves the oblique Air path",
         .world = SlabWorld(std::array{-8.0, close_gap}, 8.0, y_step),
         .start = {-0.01, start_y, 0},
         .direction = {0.01, 1, 0},
         .crossings = {{1, 0},
                       {1 + (close_gap / 0.01), 1},
                       {y_step + 5 - start_y, 0}},
         .initial_owner = 1});
}

// Independent FP64 intersections of the corrected source OBJ, in millimeters.
// Supply the exact file identified in the N4.2 qualification report.
TEST_F(GGEMSNavigationDeviceTest,
       CorrectedMlcPreservesPhysicalAirAndLeafIntervals) {
  auto const *path = std::getenv("GGEMS_TEST_MLC_OBJ");
  if (path == nullptr) {
    GTEST_SKIP() << "Set GGEMS_TEST_MLC_OBJ to the corrected MyMlc.obj.";
  }
  geometry::GGEMSMesh mesh{std::filesystem::path{path}};
  ASSERT_EQ(mesh.GetData().vertices_pm.size(), 14082U);
  ASSERT_EQ(mesh.GetData().triangles.size(), 27684U);
  ASSERT_EQ(mesh.GetData().components.size(), 120U);
  mesh.SetMaterial(materials::builtins::BuildBuiltInMaterial("Tungsten"));
  auto world = World();
  world.SetSize(400_mm, 400_mm, 400_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Air"))
    .AddVolume(mesh);
  auto &context = test::GetSelectedOpenCLContext();
  auto random = std::make_shared<random::GGEMSRandom>();
  random->SetEngine("philox").SetSeed(91231U);
  auto const &trace = MeshTrace(context, *random);
  auto const check = [&](Fixture const &fixture) -> void {
    CheckFixture(context, trace, *random, fixture);
  };
  std::vector<Crossing> const pair{{25.000026287475, 1},
                                   {184.55593182319, 0},
                                   {185.44406817681, 1},
                                   {344.99997371253, 0}};
  check({.name = "A30 then B30",
         .world = world,
         .start = {-185, 0.78, 12},
         .crossings = pair});
  check({.name = "B30 then A30",
         .world = world,
         .start = {185, 0.78, 12},
         .direction = {-1, 0, 0},
         .crossings = pair});
  check({.name = "oblique A30 then B30 across the physical air gap",
         .world = world,
         .start = {-185, 0.78, 8},
         .direction = {1, 0, 0.03125},
         .crossings = {{25.000009454099, 1},
                       {184.41498557607, 0},
                       {185.58794492877, 1},
                       {344.99993657047, 0}}});
  check({.name = "birth inside A30",
         .world = world,
         .start = {-52, 0.78, 12},
         .crossings = {{51.555931823186, 0},
                       {52.444068176814, 1},
                       {211.99997371253, 0}},
         .initial_owner = 1});
  check({.name = "birth in opposed-leaf gap",
         .world = world,
         .start = {0, 0.78, 12},
         .crossings = {{0.4440681768144, 1}, {159.99997371253, 0}}});
  check({.name = "axial B30 chord",
         .world = world,
         .start = {110, 0.78, -80},
         .direction = {0, 0, 1},
         .crossings = {{45.197051703643, 1}, {111.49732107834, 0}}});
  check({.name = "A08 then B08",
         .world = world,
         .start = {-185, 35.25, 15},
         .crossings = {{24.999816871446, 1},
                       {184.2834280635, 0},
                       {185.7165719365, 1},
                       {345.00018312855, 0}}});
  check({.name = "open aperture",
         .world = world,
         .start = {0, -90, 12},
         .direction = {0, 1, 0},
         .crossings = {}});

  // Each FP64 oracle row identifies one leaf, its axial ray (x, y), and
  // all intersected boundary z coordinates in increasing order, in mm.
  auto const table_path =
    std::filesystem::path{GGEMS_TEST_PROBE_ROOT}.parent_path() / "navigation" /
    "meshes" / "mlc_crossings_mm.tsv";
  std::ifstream table{table_path};
  ASSERT_TRUE(table.is_open()) << table_path;
  std::size_t rays{};
  std::size_t crossings{};
  for (std::string line; std::getline(table, line);) {
    std::istringstream row{line};
    Fixture ray{.name = {},
                .world = world,
                .start = {0, 0, -80},
                .direction = {0, 0, 1},
                .crossings = {}};
    ASSERT_TRUE(row >> ray.name >> ray.start[0] >> ray.start[1]) << line;
    for (double z; row >> z;) {
      auto const arrival = ray.crossings.size() % 2 == 0 ? 1U : 0U;
      ray.crossings.push_back({z - ray.start[2], arrival});
    }
    ASSERT_TRUE(row.eof()) << line;
    ASSERT_FALSE(ray.crossings.empty()) << ray.name;
    ASSERT_EQ(ray.crossings.back().arrival, 0U) << ray.name;
    check(ray);
    ++rays;
    crossings += ray.crossings.size();
  }
  EXPECT_EQ(rays, 120U);
  EXPECT_EQ(crossings, 660U);

  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetAnalytic()
    .SetPrimaryCount(1U)
    .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetPositionPicoMeter(-185'000'000'000LL, 780'000'000LL, 12'000'000'000LL)
    .SetDirection(1, 0, 0);
  auto observer = std::make_shared<observer::GGEMSTransportObserver>();
  observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);
  GGEMSRun run;
  run.SetRandom(random);
  run.AddSource(source);
  run.SetObserver(observer);
  run.SetWorld(world);
  run.Initialize();
  run.Run();
  auto const records = observer->GetRecords();
  ASSERT_EQ(records.size(), 2U);
  EXPECT_EQ(records[1].status, particles::ToKernelParticleStatus(
                                 particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_LE(
    std::abs(static_cast<double>(records[1].position_x_pm) - (200 * k_mm)),
    8 * world.GetNavigationTolerancePM());
}

// Preserves a multi-volume Run snapshot independently of observer capture.
TEST_F(GGEMSNavigationDeviceTest,
       MultipleOccurrenceRunOwnsSnapshotInBothObserverModes) {
  for (auto const [observed, with_mesh] :
       {std::pair{false, false}, std::pair{true, false},
        std::pair{true, true}}) {
    SCOPED_TRACE(with_mesh);
    SCOPED_TRACE(observed);
    auto world = Boxes(-20, 20);
    if (with_mesh) {
      auto const path =
        std::filesystem::path{GGEMS_TEST_PROBE_ROOT}.parent_path() /
        "navigation" / "meshes" / "three_slabs.obj";
      geometry::GGEMSMesh mesh{path};
      mesh.SetMaterial(materials::builtins::BuildBuiltInMaterial("Tungsten"));
      world.AddVolume(mesh);
    }
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
        8 * world.GetNavigationTolerancePM());
      EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
    }
  }
}
