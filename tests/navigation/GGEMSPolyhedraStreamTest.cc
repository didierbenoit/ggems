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
 * \brief Qualifies generic polygonal profiles through actual OpenCL transport.
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

#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"
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

// Expected crossings below are independent analytic apothem-plane chords.
struct Case {
  std::string name;
  Vec start;
  Vec direction;
  std::vector<double> crossings;
  std::uint32_t initial_owner{0U};
  std::uint32_t sides{4U};
  double angle_degrees{45};
  std::vector<double> z{-6, 6};
  std::vector<double> inner{0, 0};
  std::vector<double> outer{5, 5};
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
  geometry::GGEMSPolyhedra shape;
  std::vector<PositionCoordinate> z;
  std::vector<Length> inner;
  std::vector<Length> outer;
  z.reserve(fixture.z.size());
  inner.reserve(fixture.inner.size());
  outer.reserve(fixture.outer.size());
  for (auto value : fixture.z) {
    z.push_back(PositionCoordinate{std::llround(value * k_mm)});
  }
  for (auto value : fixture.inner) {
    inner.push_back(Length{static_cast<std::uint64_t>(value * k_mm)});
  }
  for (auto value : fixture.outer) {
    outer.push_back(Length{static_cast<std::uint64_t>(value * k_mm)});
  }
  shape.SetSideCount(fixture.sides)
    .SetStartAngle(MakeDegrees(fixture.angle_degrees))
    .SetZProfile(z, inner, outer)
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
  auto const result = test::RunStream(
    context, program, random, world.BuildRecord(), volume, source, &*pose,
    capture, world.BuildAnalyticAuxiliaryData());
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
  std::cout << std::format("N1 Polyhedra '{}': {} events, T={} pm, "
                           "maximum segment-error/T={}, budget/T={}\n",
                           fixture.name, result.trace[0], band, max_error,
                           8 * fixture.conditioning);
}

auto RunCases(std::vector<Case> const &cases) -> void {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const source = test::InstrumentStream(root) + R"cl(
__kernel void polyhedra_layout(__global ulong *layout) {
  layout[0] = sizeof(GGEMSPolyhedraRecord);
  layout[1] = __alignof(GGEMSPolyhedraRecord);
  layout[2] = __builtin_offsetof(GGEMSPolyhedraRecord, side_count);
  layout[3] = __builtin_offsetof(GGEMSPolyhedraRecord, section_count);
  layout[4] = __builtin_offsetof(GGEMSPolyhedraRecord, volume_id);
  layout[5] = __builtin_offsetof(GGEMSPolyhedraRecord, material_id);
  layout[6] = __builtin_offsetof(GGEMSPolyhedraRecord, tolerance_pm);
  layout[7] = sizeof(GGEMSPolyhedraSideRecord);
  layout[8] = __alignof(GGEMSPolyhedraSideRecord);
  layout[9] = __builtin_offsetof(GGEMSPolyhedraSideRecord, normal_x);
  layout[10] = __builtin_offsetof(GGEMSPolyhedraSideRecord, normal_y);
  layout[11] = sizeof(GGEMSPolyhedraSectionRecord);
  layout[12] = __alignof(GGEMSPolyhedraSectionRecord);
  layout[13] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, z_lower_pm);
  layout[14] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, z_upper_pm);
  layout[15] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, outer_radius_pm);
  layout[16] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, outer_slope);
  layout[17] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, outer_normal_length);
  layout[18] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, inner_radius_pm);
  layout[19] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, inner_slope);
  layout[20] = __builtin_offsetof(GGEMSPolyhedraSectionRecord, inner_normal_length);
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
    std::array<std::uint64_t, 21> layout{};
    auto layout_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    ocl::GGEMSOpenCLKernel layout_kernel{
      context, cl::Kernel(program, "polyhedra_layout"), "polyhedra_layout"};
    layout_kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
    layout_kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(layout_buffer, std::span{layout});
    EXPECT_EQ(
      layout,
      (std::array<std::uint64_t, 21>{
        sizeof(geometry::GGEMSPolyhedraRecord),
        alignof(geometry::GGEMSPolyhedraRecord),
        offsetof(geometry::GGEMSPolyhedraRecord, side_count),
        offsetof(geometry::GGEMSPolyhedraRecord, section_count),
        offsetof(geometry::GGEMSPolyhedraRecord, volume_id),
        offsetof(geometry::GGEMSPolyhedraRecord, material_id),
        offsetof(geometry::GGEMSPolyhedraRecord, tolerance_pm),
        sizeof(geometry::GGEMSPolyhedraSideRecord),
        alignof(geometry::GGEMSPolyhedraSideRecord),
        offsetof(geometry::GGEMSPolyhedraSideRecord, normal_x),
        offsetof(geometry::GGEMSPolyhedraSideRecord, normal_y),
        sizeof(geometry::GGEMSPolyhedraSectionRecord),
        alignof(geometry::GGEMSPolyhedraSectionRecord),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, z_lower_pm),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, z_upper_pm),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, outer_radius_pm),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, outer_slope),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, outer_normal_length),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, inner_radius_pm),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, inner_slope),
        offsetof(geometry::GGEMSPolyhedraSectionRecord, inner_normal_length)}));
    for (auto const &fixture : cases) {
      CheckCase(context, program, random, fixture);
    }
    CheckCase(context, production.GetProgramNative(), random, cases.front(),
              false);
    CheckCase(context, program, random, cases.front(), true, 0U);
    std::cout << "N1 Polyhedra device=" << context.GetDevice().GetName()
              << "; driver=" << device.getInfo<CL_DRIVER_VERSION>()
              << "; options=" << production.GetBuildOptions() << '\n';
  }
}
auto IndependentCrossings(Case const &fixture) -> std::vector<double> {
  auto const &p = fixture.start;
  auto const &d = fixture.direction;
  double const pi = std::numbers::pi;
  std::vector<std::array<double, 2>> normals;
  normals.reserve(fixture.sides);
  for (std::uint32_t side = 0; side < fixture.sides; ++side) {
    double const angle =
      (fixture.angle_degrees * pi / 180) +
      (((2 * static_cast<double>(side)) + 1) * pi / fixture.sides);
    normals.push_back({std::cos(angle), std::sin(angle)});
  }
  auto inside = [&](double parameter) -> bool {
    double const z = p[2] + (parameter * d[2]);
    if (z <= fixture.z.front() || z >= fixture.z.back()) {
      return false;
    }
    std::size_t section = 0;
    while (z > fixture.z[section + 1]) {
      ++section;
    }
    double const fraction =
      (z - fixture.z[section]) / (fixture.z[section + 1] - fixture.z[section]);
    double const outer =
      fixture.outer[section] +
      (fraction * (fixture.outer[section + 1] - fixture.outer[section]));
    double const inner =
      fixture.inner[section] +
      (fraction * (fixture.inner[section + 1] - fixture.inner[section]));
    double maximum = -std::numeric_limits<double>::infinity();
    for (auto const &normal : normals) {
      maximum = std::max(maximum, (normal[0] * (p[0] + (parameter * d[0]))) +
                                    (normal[1] * (p[1] + (parameter * d[1]))));
    }
    return maximum < outer && (inner == 0 || maximum > inner);
  };
  std::vector<double> candidates{0};
  double world_exit = std::numeric_limits<double>::infinity();
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (d[axis] != 0) {
      world_exit = std::min(world_exit,
                            (std::copysign(40.0, d[axis]) - p[axis]) / d[axis]);
    }
  }
  auto add = [&](double parameter) -> void {
    if (parameter > 0 && parameter < world_exit) {
      candidates.push_back(parameter);
    }
  };
  if (d[2] != 0) {
    for (auto z : fixture.z) {
      add((z - p[2]) / d[2]);
    }
  }
  for (std::size_t section = 0; section + 1 < fixture.z.size(); ++section) {
    for (auto const *profile : {&fixture.outer, &fixture.inner}) {
      double const slope = ((*profile)[section + 1] - (*profile)[section]) /
                           (fixture.z[section + 1] - fixture.z[section]);
      for (auto const &normal : normals) {
        double const component =
          (normal[0] * d[0]) + (normal[1] * d[1]) - (slope * d[2]);
        if (component == 0) {
          continue;
        }
        double const parameter =
          ((*profile)[section] + (slope * (p[2] - fixture.z[section])) -
           (normal[0] * p[0]) - (normal[1] * p[1])) /
          component;
        double const z = p[2] + (parameter * d[2]);
        if (z >= fixture.z[section] && z <= fixture.z[section + 1]) {
          add(parameter);
        }
      }
    }
  }
  candidates.push_back(world_exit);
  std::ranges::sort(candidates);
  std::vector<double> crossings;
  bool previous = false; // Random births lie below the first cap.
  for (std::size_t index = 0; index + 1 < candidates.size(); ++index) {
    double const lower = candidates[index];
    double const upper = candidates[index + 1];
    if (upper - lower < 1e-10) {
      continue;
    }
    bool const current = inside((lower + upper) * .5);
    if (current != previous) {
      crossings.push_back(lower);
      previous = current;
    }
  }
  return crossings;
}

} // namespace

TEST_F(GGEMSNavigationDeviceTest,
       N1PolyhedraApothemOrientationAndHollowChords) {
  double const vertex = 10 / std::numbers::sqrt3;
  RunCases({
    {"square faces", {-8, 0, 0}, {1, 0, 0}, {3, 13}},
    {"triangular prism", {-8, 0, 0}, {1, 0, 0}, {3, 18}, 0, 3, 0},
    {
      "hexagon vertices",
      {-8, 0, 0},
      {1, 0, 0},
      {8 - vertex, 8 + vertex},
      0,
      6,
      0,
    },
    {"hexagon faces", {0, -8, 0}, {0, 1, 0}, {3, 13}, 0, 6, 0},
    {"rotated hexagon faces", {-8, 0, 0}, {1, 0, 0}, {3, 13}, 0, 6, 30},
    {
      "seventeen sided faces",
      {-8, 0, 0},
      {1, 0, 0},
      {8 - (5 / std::cos(std::acos(-1.0) / 17)), 13},
      0,
      17,
      -180.0 / 17,
    },
    {
      "hollow hexagon two walls",
      {0, -8, 0},
      {0, 1, 0},
      {3, 6, 10, 13},
      0,
      6,
      0,
      {-6, 6},
      {2, 2},
    },
    {
      "hollow square bore birth",
      {0, 0, 0},
      {.5, 0, 0},
      {4, 10},
      0,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
    {
      "hollow square material birth",
      {-3, 0, 0},
      {1, 0, 0},
      {1, 5, 8},
      1,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
    {"solid interior birth", {0, 0, 0}, {1, 0, 0}, {5}, 1},
    {"nonunit axial caps", {0, 0, -8}, {0, 0, .5}, {4, 28}},
    {"diagonal corner crossing", {-8, -8, 0}, {.5, .5, 0}, {6, 26}},
    {"separated miss", {-8, 7, 0}, {1, 0, 0}, {}},
    {"parallel side miss", {6, 0, -8}, {0, 0, 1}, {}},
    {"almost parallel side entry", {-8, 0, 0}, {1, 1e-10, 0}, {3, 13}},
    {
      "hollow axial bore miss",
      {0, 0, -8},
      {0, 0, 1},
      {},
      0,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest,
       N1PolyhedraContinuousProfilesAndNonconvexIntervals) {
  RunCases({
    {
      "outer taper transverse",
      {-8, 0, 0},
      {1, 0, 0},
      {3, 13},
      0,
      4,
      45,
      {-6, 6},
      {0, 0},
      {4, 6},
    },
    {
      "outer taper axial",
      {5, 0, -8},
      {0, 0, 1},
      {8, 14},
      0,
      4,
      45,
      {-6, 6},
      {0, 0},
      {4, 6},
    },
    {
      "inner taper axial",
      {2, 0, -8},
      {0, 0, 1},
      {2, 8},
      0,
      4,
      45,
      {-6, 6},
      {1, 3},
      {6, 6},
    },
    {
      "concave axial two intervals",
      {4, 0, -8},
      {0, 0, 1},
      {2, 5, 11, 14},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "concave interior birth",
      {4, 0, -4},
      {0, 0, 1},
      {1, 7, 10},
      1,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "concave three intervals",
      {4, 0, -10},
      {0, 0, 1},
      {2, 4, 8, 12, 16, 18},
      0,
      4,
      45,
      {-8, -4, 0, 4, 8},
      {0, 0, 0, 0, 0},
      {6, 2, 6, 2, 6},
    },
    {
      "same material seams invisible",
      {0, 0, -8},
      {0, 0, 1},
      {2, 14},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "transverse on internal profile",
      {-8, 0, 0},
      {1, 0, 0},
      {6, 10},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "hollow transverse on internal profile",
      {-8, 0, 0},
      {1, 0, 0},
      {3, 7, 9, 13},
      0,
      4,
      45,
      {-6, 0, 6},
      {2, 1, 2},
      {6, 5, 6},
    },
    {
      "hollow seam bore birth",
      {0, 0, 0},
      {1, 0, 0},
      {1, 5},
      0,
      4,
      45,
      {-6, 0, 6},
      {2, 1, 2},
      {6, 5, 6},
    },
    {
      "concave reverse axial two intervals",
      {4, 0, 8},
      {0, 0, -1},
      {2, 5, 11, 14},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "varying bore two intervals",
      {2, 0, -8},
      {0, 0, 1},
      {2, 5, 11, 14},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 4, 0},
      {6, 6, 6},
    },
    {
      "zero bore regions",
      {0, 0, -10},
      {0, 0, 1},
      {2, 6, 14, 18},
      0,
      4,
      45,
      {-8, -4, 0, 4, 8},
      {0, 0, 3, 0, 0},
      {6, 6, 6, 6, 6},
    },
    {
      "off-center local profile",
      {0, 0, -2},
      {0, 0, 1},
      {2, 14},
      0,
      4,
      45,
      {0, 6, 12},
      {0, 0, 0},
      {6, 2, 6},
    },
    {
      "translated rotated concavity",
      {4, 0, -8},
      {0, 0, 1},
      {2, 5, 11, 14},
      0,
      4,
      45,
      {-6, 0, 6},
      {0, 0, 0},
      {6, 2, 6},
      {-7, 5, -3},
      true,
    },
    {
      "translated rotated hollow",
      {-8, 0, 0},
      {1, 0, 0},
      {3, 6, 10, 13},
      0,
      4,
      45,
      {-6, 6},
      {2, 2},
      {5, 5},
      {3, -4, 2},
      true,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest, N1PolyhedraDirectionalBandsEdgesAndVertices) {
  Case const base{"band reference", {-8, 0, 0}, {1, 0, 0}, {}};
  double const band = Scene(base).GetVolumeNavigationTolerancePM() / k_mm;
  RunCases({
    {"side inward birth", {5, 0, 0}, {-1, 0, 0}, {10}, 1},
    {"side outward birth", {5, 0, 0}, {1, 0, 0}, {}},
    {"side parallel contact", {-8, 5, 0}, {1, 0, 0}, {}},
    {"corner inward birth", {5, 5, 0}, {-1, -1, 0}, {10}, 1},
    {"corner outward birth", {5, 5, 0}, {1, 1, 0}, {}},
    {"corner mixed direction touch", {5, 5, 0}, {-1, 1, 0}, {}},
    {"cap inward birth", {0, 0, 6}, {0, 0, -1}, {12}, 1},
    {"cap outward birth", {0, 0, 6}, {0, 0, 1}, {}},
    {
      "outer half-band inward",
      {5 + (band / 2), 0, 0},
      {-1, 0, 0},
      {10 + (band / 2)},
      1,
    },
    {"inner half-band outward", {5 - (band / 2), 0, 0}, {1, 0, 0}, {}},
    {"sub-band face graze", {-8, 5 - (band / 2), 0}, {1, 0, 0}, {}},
    {
      "resolved shallow face chord",
      {-8, 5 - (8 * band), 0},
      {1, 0, 0},
      {3, 13},
    },
    {
      "inner face toward bore",
      {2, 0, 0},
      {-1, 0, 0},
      {4, 7},
      0,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
    {
      "inner face toward material",
      {2, 0, 0},
      {1, 0, 0},
      {3},
      1,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
    {
      "inner face tangency",
      {-8, 2, 0},
      {1, 0, 0},
      {3, 13},
      0,
      4,
      45,
      {-6, 6},
      {2, 2},
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest, N1PolyhedraIndependentRandomPlaneOracle) {
  std::vector<Case> cases;
  cases.reserve(250);
  std::uint32_t state{719391U};
  auto uniform = [&] -> double {
    state = (state * 1664525U) + 1013904223U;
    return static_cast<double>(state >> 8U) / 16777216.0;
  };
  std::array const sides{3U, 4U, 6U, 11U, 17U};
  for (std::size_t index = 0; index < 250; ++index) {
    Case fixture{
      .name = std::format("independent random plane ray {}", index),
      .start = {(16 * uniform()) - 8, (16 * uniform()) - 8, -12},
      .direction = {uniform() - .5, uniform() - .5, .7 + (.6 * uniform())},
      .crossings = {},
      .sides = sides[index % sides.size()],
      .angle_degrees = 360 * uniform(),
      .z = {-8, -4, 0, 4, 8},
      .inner = index % 2 == 0 ? std::vector<double>{0, 0, 0, 0, 0}
                              : std::vector<double>{1, 0, 1.5, 0, 1},
      .outer = {6, 2, 5, 3, 6},
    };
    fixture.crossings = IndependentCrossings(fixture);
    cases.push_back(std::move(fixture));
  }
  RunCases(cases);
}

TEST_F(GGEMSNavigationDeviceTest, N1PolyhedraAdjacentProfileSurfaceBands) {
  Case const reference{
    .name = "adjacent surface band reference",
    .start = {},
    .direction = {1, 0, 0},
    .crossings = {},
    .z = {-1, 0, .1},
    .inner = {0, 0, 0},
    .outer = {10, 10, .1},
  };
  double const band = Scene(reference).GetVolumeNavigationTolerancePM() / k_mm;
  // The nearest point on the steep adjacent face lies above the join. A
  // transverse ray below that join can still be wholly inside its true band.
  RunCases({
    {
      .name = "adjacent outer face sub-band birth",
      .start = {10 - (4 * band), 0, -.1 * band},
      .direction = {1, 0, 0},
      .crossings = {},
      .z = {-1, 0, .1},
      .inner = {0, 0, 0},
      .outer = {10, 10, .1},
    },
    {
      .name = "mirrored adjacent outer face sub-band birth",
      .start = {10 - (4 * band), 0, .1 * band},
      .direction = {1, 0, 0},
      .crossings = {},
      .z = {-.1, 0, 1},
      .inner = {0, 0, 0},
      .outer = {.1, 10, 10},
    },
    {
      .name = "resolved birth below adjacent outer face band",
      .start = {10 - (4 * band), 0, -2 * band},
      .direction = {1, 0, 0},
      .crossings = {4 * band},
      .initial_owner = 1,
      .z = {-1, 0, .1},
      .inner = {0, 0, 0},
      .outer = {10, 10, .1},
    },
    {
      .name = "resolved radial depth below adjacent outer face",
      .start = {10 - (200 * band), 0, -.1 * band},
      .direction = {1, 0, 0},
      .crossings = {200 * band},
      .initial_owner = 1,
      .z = {-1, 0, .1},
      .inner = {0, 0, 0},
      .outer = {10, 10, .1},
    },
    {
      .name = "adjacent inner face sub-band birth and reentry",
      .start = {.1 + (4 * band), 0, -.1 * band},
      .direction = {-1, 0, 0},
      .crossings = {.2 + (4 * band), 12.1 + (4 * band)},
      .z = {-1, 0, .1},
      .inner = {.1, .1, 10},
      .outer = {12, 12, 12},
    },
    {
      .name = "mirrored adjacent inner face sub-band birth and reentry",
      .start = {.1 + (4 * band), 0, .1 * band},
      .direction = {-1, 0, 0},
      .crossings = {.2 + (4 * band), 12.1 + (4 * band)},
      .z = {-.1, 0, 1},
      .inner = {10, .1, .1},
      .outer = {12, 12, 12},
    },
    {
      .name = "resolved birth below adjacent inner face band",
      .start = {.1 + (4 * band), 0, -2 * band},
      .direction = {-1, 0, 0},
      .crossings = {4 * band, .2 + (4 * band), 12.1 + (4 * band)},
      .initial_owner = 1,
      .z = {-1, 0, .1},
      .inner = {.1, .1, 10},
      .outer = {12, 12, 12},
    },
  });
}
