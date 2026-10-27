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
 * \brief Qualifies conical boundary events through actual OpenCL transport.
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
#include <limits>
#include <numbers>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
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

// Independent fixtures use millimeters and absolute ray parameters. Default
// surfaces are rho = 5+z/6, rho_inner = 0, and caps at z = +/-6 mm.
struct Case {
  std::string name;
  Vec start;
  Vec direction;
  std::vector<double> crossings;
  std::uint32_t initial_owner{0U};
  std::array<double, 2> inner{0, 0};
  std::array<double, 2> outer{4, 6};
  Vec center{};
  bool rotated{false};
  double conditioning{1};
};

auto Rotate(Vec const &value, bool rotated) -> Vec {
  if (!rotated) {
    return value;
  }
  // Independent Ry(30 degrees), with no production pose helper.
  double const cosine = std::numbers::sqrt3 / 2;
  return {
    (cosine * value[0]) + (0.5 * value[2]),
    value[1],
    (-0.5 * value[0]) + (cosine * value[2]),
  };
}

auto Scene(Case const &fixture) -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(80_mm, 80_mm, 80_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  geometry::GGEMSCons shape;
  shape
    .SetNegativeZRadii(
      Length{static_cast<std::uint64_t>(fixture.inner[0] * k_mm)},
      Length{static_cast<std::uint64_t>(fixture.outer[0] * k_mm)})
    .SetPositiveZRadii(
      Length{static_cast<std::uint64_t>(fixture.inner[1] * k_mm)},
      Length{static_cast<std::uint64_t>(fixture.outer[1] * k_mm)})
    .SetLength(12_mm)
    .SetPosition(
      geometry::MakePositionPM(std::llround(fixture.center[0] * k_mm),
                               std::llround(fixture.center[1] * k_mm),
                               std::llround(fixture.center[2] * k_mm)))
    .SetRotation(0_deg, fixture.rotated ? 30_deg : 0_deg, 0_deg)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  world.AddVolume(shape);
  world.Validate();
  return world;
}

auto CheckCase(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
               random::GGEMSRandom &random, Case const &fixture,
               bool capture = true, std::uint32_t material = 7U) -> void {
  SCOPED_TRACE(fixture.name);
  auto const world = Scene(fixture);
  auto const volume = world.BuildAnalyticVolumeRecord(material);
  auto const pose = world.BuildBoxPoseRecord();
  ASSERT_TRUE(pose.has_value());
  double const band = world.GetVolumeNavigationTolerancePM();
  auto const rotated_start = Rotate(fixture.start, fixture.rotated);
  auto const direction = Rotate(fixture.direction, fixture.rotated);
  Vec start{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    start[axis] = (fixture.center[axis] + rotated_start[axis]) * k_mm;
  }
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = std::llround(start[0]);
  source.position_y_pm = std::llround(start[1]);
  source.position_z_pm = std::llround(start[2]);
  source.axis_z_x = static_cast<float>(direction[0]);
  source.axis_z_y = static_cast<float>(direction[1]);
  source.axis_z_z = static_cast<float>(direction[2]);
  source.energy_micro_eV = 123456789U;
  source.time_start_ps = 17U;
  source.time_stop_ps = 17U;
  auto const result =
    test::RunStream(context, program, random, world.BuildRecord(), volume,
                    source, &*pose, capture);
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  ASSERT_EQ(result.observer_counts.record_count, 2U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_x),
            std::bit_cast<std::uint32_t>(source.axis_z_x));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_y),
            std::bit_cast<std::uint32_t>(source.axis_z_y));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_z),
            std::bit_cast<std::uint32_t>(source.axis_z_z));
  double world_exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (direction[axis] != 0) {
      world_exit = std::min(
        world_exit, (std::copysign(40 * k_mm, direction[axis]) - start[axis]) /
                      direction[axis]);
    }
  }
  if (!capture) {
    return;
  }
  EXPECT_EQ(result.trace[test::k_initial_owner_index], fixture.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            fixture.initial_owner == 0U ? 0U : material);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  ASSERT_EQ(result.trace[0], fixture.crossings.size() + 1U);
  auto coordinate = [&](std::size_t index) -> double {
    return std::bit_cast<float>(
      static_cast<std::uint32_t>(result.trace[index]));
  };
  double const norm =
    std::sqrt((direction[0] * direction[0]) + (direction[1] * direction[1]) +
              (direction[2] * direction[2]));
  double previous{0};
  double accumulated{0};
  double max_error{0};
  auto owner = fixture.initial_owner;
  // Frozen before execution: eight automatic half-bands, with explicit
  // inverse-incidence conditioning only for declared shallow/tangent cases.
  double const budget = 8 * band * fixture.conditioning;
  for (std::size_t index = 0; index <= fixture.crossings.size(); ++index) {
    auto const offset = 1U + (test::k_trace_stride * index);
    bool const escape = index == fixture.crossings.size();
    auto const arrival = escape ? k_exterior : 1U - owner;
    double const parameter =
      escape ? world_exit : fixture.crossings[index] * k_mm;
    EXPECT_EQ(result.trace[offset], owner);
    EXPECT_EQ(result.trace[offset + 1U], arrival);
    EXPECT_EQ(result.trace[offset + 6U],
              escape ? 1U : (arrival == 1U ? 2U : 3U));
    EXPECT_EQ(result.trace[offset + 7U],
              escape ? k_exterior : (arrival == 1U ? material : 0U));
    auto const measured = coordinate(offset + 8U);
    ASSERT_TRUE(std::isfinite(measured));
    EXPECT_GE(measured, 0);
    accumulated += measured;
    double const error = std::abs((measured - (parameter - previous)) * norm);
    EXPECT_LE(error, budget);
    max_error = std::max(max_error, error / band);
    EXPECT_LE(std::abs((accumulated - parameter) * norm),
              static_cast<double>(index + 1U) * budget);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      EXPECT_LE(std::abs(coordinate(offset + 3U + axis) -
                         (start[axis] + (parameter * direction[axis]))),
                budget);
    }
    previous = parameter;
    owner = arrival;
  }
  std::cout << std::format("N1 Cons '{}': {} events, T={} pm, "
                           "maximum segment-error/T={}, budget/T={}\n",
                           fixture.name, result.trace[0], band, max_error,
                           8 * fixture.conditioning);
}

auto RunCases(std::vector<Case> const &cases) -> void {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const source = test::InstrumentStream(root) + R"cl(
__kernel void cons_layout(__global ulong *layout) {
  layout[0] = sizeof(GGEMSConsRecord);
  layout[1] = __alignof(GGEMSConsRecord);
  layout[2] = __builtin_offsetof(GGEMSConsRecord, outer_radius_pm);
  layout[3] = __builtin_offsetof(GGEMSConsRecord, inner_radius_pm);
  layout[4] = __builtin_offsetof(GGEMSConsRecord, outer_slope);
  layout[5] = __builtin_offsetof(GGEMSConsRecord, inner_slope);
  layout[6] = __builtin_offsetof(GGEMSConsRecord, half_length_pm);
  layout[7] = __builtin_offsetof(GGEMSConsRecord, outer_normal_scale);
  layout[8] = __builtin_offsetof(GGEMSConsRecord, inner_normal_scale);
  layout[9] = __builtin_offsetof(GGEMSConsRecord, volume_id);
  layout[10] = __builtin_offsetof(GGEMSConsRecord, material_id);
  layout[11] = __builtin_offsetof(GGEMSConsRecord, tolerance_pm);
  layout[12] = sizeof(GGEMSAnalyticVolumeRecord);
  layout[13] = sizeof(GGEMSParticleState);
}
)cl";
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231U);
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
    std::array<std::uint64_t, 14> layout{};
    auto buffer = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    ocl::GGEMSOpenCLKernel kernel{context, cl::Kernel(program, "cons_layout"),
                                  "cons_layout"};
    kernel.SetArgSVMPointer(0U, buffer.GetData());
    kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(buffer, std::span{layout});
    EXPECT_EQ(layout, (std::array<std::uint64_t, 14>{40, 4, 0, 4, 8, 12, 16, 20,
                                                     24, 28, 32, 36, 84, 80}));
    for (auto const &fixture : cases) {
      CheckCase(context, program, random, fixture);
    }
    CheckCase(context, production.GetProgramNative(), random, cases.front(),
              false);
    CheckCase(context, program, random, cases.front(), true, 0U);
    std::cout << "N1 Cons device=" << context.GetDevice().GetName()
              << "; driver=" << device.getInfo<CL_DRIVER_VERSION>()
              << "; options=" << production.GetBuildOptions() << '\n';
  }
}
auto IndependentConsCrossings(Case const &fixture) -> std::vector<double> {
  auto const &p = fixture.start;
  auto const &d = fixture.direction;
  double world_exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (d[axis] != 0) {
      world_exit = std::min(world_exit,
                            (std::copysign(40.0, d[axis]) - p[axis]) / d[axis]);
    }
  }
  std::vector<double> candidates{0, world_exit};
  auto add = [&](double parameter) -> void {
    double const z = p[2] + (parameter * d[2]);
    if (parameter > 0 && parameter < world_exit && z >= -6 && z <= 6) {
      candidates.push_back(parameter);
    }
  };
  if (d[2] != 0) {
    candidates.push_back((-6 - p[2]) / d[2]);
    candidates.push_back((6 - p[2]) / d[2]);
  }
  for (auto const &radii : {fixture.outer, fixture.inner}) {
    if (radii[0] == 0 && radii[1] == 0) {
      continue;
    }
    double const slope = (radii[1] - radii[0]) / 12;
    double const radius = radii[0] + (slope * (p[2] + 6));
    double const radial_component = slope * d[2];
    double const a =
      (d[0] * d[0]) + (d[1] * d[1]) - (radial_component * radial_component);
    double const b =
      2 * ((p[0] * d[0]) + (p[1] * d[1]) - (radius * radial_component));
    double const c = (p[0] * p[0]) + (p[1] * p[1]) - (radius * radius);
    if (a == 0) {
      if (b != 0) {
        add(-c / b);
      }
      continue;
    }
    double const discriminant = (b * b) - (4 * a * c);
    if (discriminant >= 0) {
      add((-b - std::sqrt(discriminant)) / (2 * a));
      add((-b + std::sqrt(discriminant)) / (2 * a));
    }
  }
  std::ranges::sort(candidates);
  std::vector<double> crossings;
  bool previous = false;
  for (std::size_t index = 0; index + 1 < candidates.size(); ++index) {
    double const lower = candidates[index];
    double const upper = candidates[index + 1];
    if (upper - lower < 1e-10 || lower < 0 || upper > world_exit) {
      continue;
    }
    double const parameter = .5 * (lower + upper);
    double const z = p[2] + (parameter * d[2]);
    double const radius =
      std::hypot(p[0] + (parameter * d[0]), p[1] + (parameter * d[1]));
    double const outer =
      fixture.outer[0] + ((z + 6) * (fixture.outer[1] - fixture.outer[0]) / 12);
    double const inner =
      fixture.inner[0] + ((z + 6) * (fixture.inner[1] - fixture.inner[0]) / 12);
    bool const current =
      z > -6 && z < 6 && radius < outer && (inner == 0 || radius > inner);
    if (current != previous) {
      crossings.push_back(lower);
      previous = current;
    }
  }
  return crossings;
}

} // namespace

TEST_F(GGEMSNavigationDeviceTest, N1ConsIndependentFrustumAndBoreChords) {
  RunCases({
    {"transverse frustum", {-8, 0, 0}, {1, 0, 0}, {3, 13}},
    {"negative end asymmetry", {-8, 0, -3}, {1, 0, 0}, {3.5, 12.5}},
    {"positive end asymmetry", {-8, 0, 3}, {1, 0, 0}, {2.5, 13.5}},
    {"axial nonunit caps", {0, 0, -8}, {0, 0, .5}, {4, 28}},
    {"tapered outer axial crossing", {5, 0, -8}, {0, 0, 1}, {8, 14}},
    {"hollow two walls", {-8, 0, 0}, {1, 0, 0}, {3, 6.5, 9.5, 13}, 0, {1, 2}},
    {"hollow bore birth", {0, 0, 0}, {.5, 0, 0}, {3, 10}, 0, {1, 2}},
    {"hollow material birth", {-3, 0, 0}, {1, 0, 0}, {1.5, 4.5, 8}, 1, {1, 2}},
    {"hollow axial bore miss", {0, 0, -8}, {0, 0, 1}, {}, 0, {1, 2}},
    {
      "tapered inner axial crossing",
      {1.5, 0, -8},
      {0, 0, 1},
      {2, 8},
      0,
      {1, 2},
    },
    {"hollow both caps", {3, 0, -8}, {0, 0, 1}, {2, 14}, 0, {1, 2}},
    {"solid interior birth", {0, 0, 0}, {1, 0, 0}, {5}, 1},
    {"nonunit transverse", {-8, 0, 0}, {.75, 0, 0}, {4, 52.0 / 3}},
    {"oblique side and cap", {-8, 0, -8}, {1, 0, 1}, {26.0 / 7, 14}},
    {
      "generator-parallel linear equation",
      {-5, 0, -6},
      {1.0 / 6, 0, 1},
      {3, 12},
    },
    {
      "cylindrical limiting case",
      {-8, 0, 0},
      {1, 0, 0},
      {4, 12},
      0,
      {0, 0},
      {4, 4},
    },
    {"tip axial chord", {0, 0, -8}, {0, 0, 1}, {2, 14}, 0, {0, 0}, {0, 6}},
    {"tip transverse chord", {-8, 0, 0}, {1, 0, 0}, {5, 11}, 0, {0, 0}, {0, 6}},
    {
      .name = "positive tip axial chord",
      .start = {0, 0, -8},
      .direction = {0, 0, 1},
      .crossings = {2, 14},
      .outer = {6, 0},
    },
    {
      .name = "positive tip reverse axial chord",
      .start = {0, 0, 8},
      .direction = {0, 0, -1},
      .crossings = {2, 14},
      .outer = {6, 0},
    },
    {
      .name = "positive tip transverse chord",
      .start = {-8, 0, 0},
      .direction = {1, 0, 0},
      .crossings = {5, 11},
      .outer = {6, 0},
    },
    {
      .name = "positive tip reverse transverse chord",
      .start = {8, 0, 0},
      .direction = {-1, 0, 0},
      .crossings = {5, 11},
      .outer = {6, 0},
    },
    {"closing bore axial crossing", {.5, 0, -8}, {0, 0, 1}, {2, 5}, 0, {0, 2}},
    {
      "translated rotated hollow",
      {-8, 0, 0},
      {1, 0, 0},
      {3, 6.5, 9.5, 13},
      0,
      {1, 2},
      {4, 6},
      {-7, 5, -3},
      true,
    },
    {"separated miss", {-8, 7, 0}, {1, 0, 0}, {}},
    {"nearly axial bore miss", {0, 0, -8}, {1e-12, 0, 1}, {}, 0, {1, 2}},
  });
}

TEST_F(GGEMSNavigationDeviceTest, N1ConsDirectionalBandsTangentsAndCaps) {
  Case const base{"band reference", {-8, 0, 0}, {1, 0, 0}, {}};
  double const band = Scene(base).GetVolumeNavigationTolerancePM() / k_mm;
  double const side_band = band * std::sqrt(37.0 / 36);
  double const near = 5 - (64 * side_band);
  double const chord = std::sqrt(25 - (near * near));
  double const shallow_inner = 1.5 - (band / 2);
  double const outer_chord = std::sqrt(25 - (shallow_inner * shallow_inner));
  RunCases({
    {"conical transverse tangent", {-8, 5, 0}, {1, 0, 0}, {}},
    {"sub-band outer chord", {-8, 5 - (side_band / 2), 0}, {1, 0, 0}, {}},
    {
      "resolved near tangent",
      {-8, near, 0},
      {1, 0, 0},
      {8 - chord, 8 + chord},
      0,
      {0, 0},
      {4, 6},
      {},
      false,
      5 / chord,
    },
    {"side inward birth", {5, 0, 0}, {-1, 0, 0}, {10}, 1},
    {"side outward birth", {5, 0, 0}, {1, 0, 0}, {}},
    {
      "outer half-band inward birth",
      {5 + (side_band / 2), 0, 0},
      {-1, 0, 0},
      {10 + (side_band / 2)},
      1,
    },
    {
      "inner half-band outward birth",
      {5 - (side_band / 2), 0, 0},
      {1, 0, 0},
      {},
    },
    {"inner surface toward bore", {1.5, 0, 0}, {-1, 0, 0}, {3, 6.5}, 0, {1, 2}},
    {"inner surface toward material", {1.5, 0, 0}, {1, 0, 0}, {3.5}, 1, {1, 2}},
    {
      "inner tangent contact",
      {-8, 1.5, 0},
      {1, 0, 0},
      {8 - std::sqrt(22.75), 8 + std::sqrt(22.75)},
      0,
      {1, 2},
    },
    {
      "sub-band bore contact",
      {-8, shallow_inner, 0},
      {1, 0, 0},
      {8 - outer_chord, 8 + outer_chord},
      0,
      {1, 2},
    },
    {"positive cap inward", {3, 0, 6}, {0, 0, -1}, {12}, 1, {1, 2}},
    {"positive cap outward", {3, 0, 6}, {0, 0, 1}, {}, 0, {1, 2}},
    {
      "cap half-band inward",
      {3, 0, 6 + (band / 2)},
      {0, 0, -1},
      {12 + (band / 2)},
      1,
      {1, 2},
    },
    {"cap half-band outward", {3, 0, 6 - (band / 2)}, {0, 0, 1}, {}, 0, {1, 2}},
    {"generator contact", {4, 0, -6}, {1.0 / 6, 0, 1}, {}},
    {"tip generator contact", {0, 0, -6}, {.5, 0, 1}, {}, 0, {0, 0}, {0, 6}},
  });
}

TEST_F(GGEMSNavigationDeviceTest, N1ConsIndependentRandomQuadraticOracle) {
  std::vector<Case> cases;
  cases.reserve(250);
  std::uint32_t state{511283U};
  auto uniform = [&] -> double {
    state = (state * 1664525U) + 1013904223U;
    return static_cast<double>(state >> 8U) / 16777216.0;
  };
  for (std::size_t index = 0; index < 250; ++index) {
    Case fixture{
      .name = std::format("independent random quadratic ray {}", index),
      .start = {(16 * uniform()) - 8, (16 * uniform()) - 8, -8},
      .direction = {uniform() - .5, uniform() - .5, .7 + (.6 * uniform())},
      .crossings = {},
      .inner = index % 2 == 0
                 ? std::array<double, 2>{0, 0}
                 : std::array<double, 2>{.3 + uniform(), .3 + uniform()},
      .outer = {2 + (4 * uniform()), 2 + (4 * uniform())},
    };
    if (index % 7 == 0) {
      fixture.inner = {0, 0};
      fixture.outer = {0, 6};
    }
    fixture.crossings = IndependentConsCrossings(fixture);
    cases.push_back(std::move(fixture));
  }
  RunCases(cases);
}
