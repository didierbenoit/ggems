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
 * \brief Checks independent analytic-solid chords in the real worker.
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
#include <memory>
#include <numbers>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSource.hh"
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

enum class Shape : std::uint8_t { Tubs, Ellipsoid, Trd, Trap };

// Inputs and independently hand-derived absolute ray parameters are in mm.
// Poses 1/2 are Ry/Rz(90 degrees), 3 is Rz(45), 4 is Ry(30),
// 5 is Ry(-90), and 6 is Rz(-60)*Ry(30)*Rx(90).
// Neither oracle calls production transform or intersection functions.
struct Case {
  char const *name;
  Vec start;
  Vec direction;
  std::vector<double> crossings;
  std::uint32_t initial_owner{0U};
  double bore{0};
  Vec axes{4, 3, 6};
  Vec center{};
  unsigned int pose{0U};
  double conditioning{1};
};

auto Rotate(Vec const &value, unsigned int pose) -> Vec {
  if (pose == 1U) {
    return {value[2], value[1], -value[0]};
  }
  if (pose == 2U) {
    return {-value[1], value[0], value[2]};
  }
  if (pose == 3U) {
    double const half_sqrt_two = std::sqrt(0.5);
    return {half_sqrt_two * (value[0] - value[1]),
            half_sqrt_two * (value[0] + value[1]), value[2]};
  }
  if (pose == 4U) {
    double const half_sqrt_three = std::sqrt(3.0) / 2;
    return {half_sqrt_three * value[0] + 0.5 * value[2], value[1],
            -0.5 * value[0] + half_sqrt_three * value[2]};
  }
  if (pose == 5U) {
    return {-value[2], value[1], value[0]};
  }
  if (pose == 6U) {
    double const cosine = std::numbers::sqrt3 / 2;
    double const x = (cosine * value[0]) + (0.5 * value[1]);
    double const y = -value[2];
    return {
      (0.5 * x) + (cosine * y),
      (-cosine * x) + (0.5 * y),
      (-0.5 * value[0]) + (cosine * value[1]),
    };
  }
  return value;
}

auto Scene(Case const &fixture, Shape kind) -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(80_mm, 80_mm, 80_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  auto const position =
    geometry::MakePositionPM(std::llround(fixture.center[0] * k_mm),
                             std::llround(fixture.center[1] * k_mm),
                             std::llround(fixture.center[2] * k_mm));
  auto angle_y = fixture.pose == 1U ? 90_deg : 0_deg;
  auto angle_z = fixture.pose == 2U ? 90_deg : 0_deg;
  auto angle_x = 0_deg;
  if (fixture.pose == 3U) {
    angle_z = 45_deg;
  }
  if (fixture.pose == 4U) {
    angle_y = 30_deg;
  }
  if (fixture.pose == 5U) {
    angle_y = -90_deg;
  }
  if (fixture.pose == 6U) {
    angle_x = 90_deg;
    angle_y = 30_deg;
    angle_z = -60_deg;
  }
  auto configure = [&](auto &shape) -> void {
    shape.SetPosition(position)
      .SetRotation(angle_x, angle_y, angle_z)
      .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
    world.AddVolume(shape);
  };
  if (kind == Shape::Ellipsoid) {
    geometry::GGEMSEllipsoid shape;
    shape.SetSemiAxes(
      Length{static_cast<std::uint64_t>(fixture.axes[0] * k_mm)},
      Length{static_cast<std::uint64_t>(fixture.axes[1] * k_mm)},
      Length{static_cast<std::uint64_t>(fixture.axes[2] * k_mm)});
    configure(shape);
  } else if (kind == Shape::Tubs) {
    geometry::GGEMSTubs shape;
    shape
      .SetRadii(4_mm, Length{static_cast<std::uint64_t>(fixture.bore * k_mm)})
      .SetLength(12_mm);
    configure(shape);
  } else if (kind == Shape::Trd) {
    geometry::GGEMSTrd shape;
    shape.SetNegativeZFace(4_mm, 8_mm)
      .SetPositiveZFace(8_mm, 4_mm)
      .SetLength(12_mm);
    configure(shape);
  } else {
    // Independent defining inequalities, in mm:
    // x - .45*y - .3875*z <= 3; -x + .05*y + (37/240)*z <= 3;
    // y - z/12 <= 3; -y + 5*z/12 <= 3; abs(z) <= 6.
    geometry::GGEMSTrap shape;
    shape
      .SetNegativeZFace(8_mm, Length{2'400'000'000ULL},
                        Length{5'600'000'000ULL}, MakeRadians(std::atan(.25)))
      .SetPositiveZFace(4_mm, Length{7'200'000'000ULL},
                        Length{8'800'000'000ULL}, MakeRadians(std::atan(.25)))
      .SetLength(12_mm)
      .SetInclination(MakeRadians(std::atan(5.0 / 12)),
                      MakeRadians(std::atan(.75)));
    configure(shape);
  }
  world.Validate();
  return world;
}

auto CheckCase(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
               random::GGEMSRandom &random, Case const &fixture, Shape kind,
               bool capture = true, std::uint32_t material = 7U) -> void {
  SCOPED_TRACE(fixture.name);
  auto const world = Scene(fixture, kind);
  auto const volume = world.BuildAnalyticVolumeRecord(material);
  auto const pose = world.BuildBoxPoseRecord();
  ASSERT_TRUE(pose.has_value());
  auto const world_record = world.BuildRecord();
  double const band = world.GetVolumeNavigationTolerancePM();
  auto const rotated_start = Rotate(fixture.start, fixture.pose);
  auto const direction = Rotate(fixture.direction, fixture.pose);
  Vec start{};
  sources::GGEMSSourceRecord source{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    start[axis] = (fixture.center[axis] + rotated_start[axis]) * k_mm;
  }
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
  auto const result = test::RunStream(context, program, random, world_record,
                                      volume, source, &*pose, capture);
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  ASSERT_EQ(result.observer_counts.record_count, 2U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Terminal));
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_EQ(terminal.particle_type, source.emitted_particle_type);
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
  std::array<std::int64_t, 3> const terminal_position{
    terminal.position_x_pm, terminal.position_y_pm, terminal.position_z_pm};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    EXPECT_LE(std::abs(static_cast<double>(terminal_position[axis]) -
                       (start[axis] + world_exit * direction[axis])),
              8 * band);
  }
  if (!capture) {
    return;
  }
  auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
    auto result_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
    ocl::WriteSVMFromHost(result_buffer, value);
    return result_buffer;
  };
  auto query_world = buffer(world_record);
  auto query_volume = buffer(volume);
  auto query_pose = buffer(*pose);
  auto const packed = sources::BuildSourceDeviceRecord(source);
  std::array<float, 6> const ray{packed.position_x_pm, packed.position_y_pm,
                                 packed.position_z_pm, source.axis_z_x,
                                 source.axis_z_y,      source.axis_z_z};
  auto query_ray = context.CreateSVMBuffer(units::Bytes{sizeof(ray)});
  ocl::WriteSVMFromHost(query_ray, std::span{ray});
  std::array<std::uint32_t, 7> answer{};
  auto query_answer = context.CreateSVMBuffer(units::Bytes{sizeof(answer)});
  ocl::GGEMSOpenCLKernel query{context, cl::Kernel(program, "n1_query"),
                               "n1_query"};
  query.SetArgSVMPointer(0U, query_world.GetData());
  query.SetArgSVMPointer(1U, query_volume.GetData());
  query.SetArgSVMPointer(2U, query_pose.GetData());
  query.SetArgSVMPointer(3U, query_ray.GetData());
  query.SetArgSVMPointer(4U, query_answer.GetData());
  query.Run({1U}, {1U});
  ocl::ReadSVMToHost(query_answer, std::span{answer});
  EXPECT_EQ(answer[0], fixture.initial_owner);
  EXPECT_EQ(answer[1], fixture.initial_owner == 0U ? 0U : material);
  EXPECT_EQ(answer[2], 0U);
  EXPECT_EQ(answer[3], 0U);
  EXPECT_EQ(answer[4], fixture.crossings.empty()
                         ? 1U
                         : (fixture.initial_owner == 0U ? 2U : 3U));
  EXPECT_EQ(answer[5], fixture.crossings.empty() ? k_exterior
                                                 : 1U - fixture.initial_owner);
  double const first_parameter =
    fixture.crossings.empty() ? world_exit : fixture.crossings.front() * k_mm;
  EXPECT_LE(std::abs(static_cast<double>(std::bit_cast<float>(answer[6])) -
                     first_parameter),
            8 * band * fixture.conditioning);
  EXPECT_EQ(result.trace[test::k_initial_owner_index], fixture.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            fixture.initial_owner == 0U ? 0U : material);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  ASSERT_EQ(result.trace[0], fixture.crossings.size() + 1U);
  auto coordinate = [&](std::size_t index) -> double {
    return std::bit_cast<float>(
      static_cast<std::uint32_t>(result.trace[index]));
  };
  double norm2{0};
  for (auto const component : direction) {
    norm2 += component * component;
  }
  double const norm = std::sqrt(norm2);
  double previous{0};
  double accumulated{0};
  double max_position_bands{0};
  double max_parameter_bands{0};
  auto owner = fixture.initial_owner;
  for (std::size_t index = 0; index <= fixture.crossings.size(); ++index) {
    auto const offset = 1U + test::k_trace_stride * index;
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
    // Frozen admission budget: ordinary endpoint arithmetic stays within eight
    // automatic half-bands. Shallow chords explicitly scale by inverse normal
    // incidence; no measured discrepancy determines this budget.
    double const budget = 8 * band * fixture.conditioning;
    EXPECT_LE(std::abs((measured - (parameter - previous)) * norm), budget);
    EXPECT_LE(std::abs((accumulated - parameter) * norm),
              static_cast<double>(index + 1U) * budget);
    max_parameter_bands =
      std::max(max_parameter_bands,
               std::abs((measured - (parameter - previous)) * norm) / band);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      double const error =
        std::abs(coordinate(offset + 3U + axis) -
                 (start[axis] + parameter * direction[axis]));
      EXPECT_LE(error, budget);
      max_position_bands = std::max(max_position_bands, error / band);
    }
    previous = parameter;
    owner = arrival;
  }
  std::cout << std::format(
    "N1 '{}': {} events, T={} pm, maximum position/T={}, "
    "segment-error/T={}, budget/T={}\n",
    fixture.name, result.trace[0], band, max_position_bands,
    max_parameter_bands, 8 * fixture.conditioning);
}

auto RunCases(std::vector<Case> const &cases, Shape kind) -> void {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const source = test::InstrumentStream(root) + R"cl(
__kernel void n1_layout(__global ulong *layout) {
  layout[0] = sizeof(GGEMSAnalyticVolumeRecord);
  layout[1] = __builtin_offsetof(GGEMSAnalyticVolumeRecord, parameters);
  layout[2] = sizeof(GGEMSTubsRecord);
  layout[3] = __builtin_offsetof(GGEMSTubsRecord, outer_radius_pm);
  layout[4] = __builtin_offsetof(GGEMSTubsRecord, inner_radius_pm);
  layout[5] = __builtin_offsetof(GGEMSTubsRecord, half_length_pm);
  layout[6] = __builtin_offsetof(GGEMSTubsRecord, volume_id);
  layout[7] = __builtin_offsetof(GGEMSTubsRecord, material_id);
  layout[8] = __builtin_offsetof(GGEMSTubsRecord, tolerance_pm);
  layout[9] = sizeof(GGEMSEllipsoidRecord);
  layout[10] = __builtin_offsetof(GGEMSEllipsoidRecord, semi_axis_x_pm);
  layout[11] = __builtin_offsetof(GGEMSEllipsoidRecord, semi_axis_y_pm);
  layout[12] = __builtin_offsetof(GGEMSEllipsoidRecord, semi_axis_z_pm);
  layout[13] = __builtin_offsetof(GGEMSEllipsoidRecord, volume_id);
  layout[14] = __builtin_offsetof(GGEMSEllipsoidRecord, material_id);
  layout[15] = __builtin_offsetof(GGEMSEllipsoidRecord, tolerance_pm);
  layout[16] = sizeof(GGEMSParticleState);
  layout[17] = sizeof(GGEMSBoxPoseRecord);
  layout[18] = sizeof(GGEMSPlaneRecord);
  layout[19] = __builtin_offsetof(GGEMSPlaneRecord, normal_x);
  layout[20] = __builtin_offsetof(GGEMSPlaneRecord, normal_y);
  layout[21] = __builtin_offsetof(GGEMSPlaneRecord, normal_z);
  layout[22] = __builtin_offsetof(GGEMSPlaneRecord, offset_pm);
  layout[23] = sizeof(GGEMSPlanarRecord);
  layout[24] = __builtin_offsetof(GGEMSPlanarRecord, sides);
  layout[25] = __builtin_offsetof(GGEMSPlanarRecord, half_length_pm);
  layout[26] = __builtin_offsetof(GGEMSPlanarRecord, volume_id);
  layout[27] = __builtin_offsetof(GGEMSPlanarRecord, material_id);
  layout[28] = __builtin_offsetof(GGEMSPlanarRecord, tolerance_pm);
  layout[29] = __alignof(GGEMSPlanarRecord);
  layout[30] = __alignof(GGEMSAnalyticVolumeRecord);
}
__kernel void n1_query(__global GGEMSWorldRecord const *world,
                       __global GGEMSAnalyticVolumeRecord const *volume,
                       __global GGEMSBoxPoseRecord const *pose,
                       __global float const *ray, __global uint *answer) {
  float3 const position = vload3(0, ray);
  float3 const direction = vload3(1, ray);
  ulong owner = 0;
  uint const located = GGEMS_AnalyticLocate(world, volume, pose, position,
                                            direction, &owner);
  GGEMSBoundaryWitness witness;
  uint const status = GGEMS_AnalyticFindBoundary(world, volume, pose, position,
    direction, (uint)(owner >> 32U), GGEMS_NAVIGATION_EVENT_NONE, &witness);
  answer[0] = (uint)(owner >> 32U);
  answer[1] = (uint)owner;
  answer[2] = located;
  answer[3] = status;
  answer[4] = witness.event;
  answer[5] = witness.arrival_volume_id;
  answer[6] = as_uint(witness.parameter);
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
    std::cout << "N1 device: " << context.GetDevice().GetName()
              << "; driver=" << device.getInfo<CL_DRIVER_VERSION>()
              << "; runtime=" << device.getInfo<CL_DEVICE_VERSION>() << '\n';
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    std::array<std::uint64_t, 31> layout{};
    auto buffer = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    ocl::GGEMSOpenCLKernel kernel{context, cl::Kernel(program, "n1_layout"),
                                  "n1_layout"};
    kernel.SetArgSVMPointer(0U, buffer.GetData());
    kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(buffer, std::span{layout});
    EXPECT_EQ(layout,
              (std::array<std::uint64_t, 31>{
                84, 4,  24, 0, 4, 8, 12, 16, 20, 24, 0,  4,  8,  12, 16, 20,
                80, 48, 16, 0, 4, 8, 12, 80, 0,  64, 68, 72, 76, 4,  4}));
    for (auto const &fixture : cases) {
      CheckCase(context, program, random, fixture, kind);
    }
    // The untouched kernel entry point also executes, with its original args.
    CheckCase(context, production.GetProgramNative(), random, cases.front(),
              kind, false);
    // Physical volume identity remains distinct when both media have ID zero.
    CheckCase(context, program, random, cases.front(), kind, true, 0U);
    std::cout << "N1 options: " << production.GetBuildOptions() << '\n';
  }
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest, N1TubsFilledHollowAndPoseChords) {
  double const radial = std::sqrt(12.0);
  RunCases(
    {
      {"filled transverse", {-8, 0, 0}, {1, 0, 0}, {4, 12}},
      {"hollow two walls", {-8, 0, 0}, {1, 0, 0}, {4, 6, 10, 12}, 0, 2},
      {"hollow bore birth", {0, 0, 0}, {0.5, 0, 0}, {4, 8}, 0, 2},
      {"hollow first wall birth", {-3, 0, 0}, {1, 0, 0}, {1, 5, 7}, 1, 2},
      {"hollow inner tangent",
       {-8, 2, 0},
       {1, 0, 0},
       {8 - radial, 8 + radial},
       0,
       2},
      {"filled axial", {0, 0, -8}, {0, 0, 0.5}, {4, 28}},
      {"filled axis birth has no inner wall", {0, 0, 0}, {1, 0, 0}, {4}, 1},
      {"hollow axial bore", {0, 0, -8}, {0, 0, 1}, {}, 0, 2},
      {"hollow axial wall", {3, 0, -8}, {0, 0, 1}, {2, 14}, 0, 2},
      {"hollow nearly axial wall", {3, 0, -8}, {1.0e-12, 0, 1}, {2, 14}, 0, 2},
      {"hollow nearly axial bore", {0, 0, -8}, {1.0e-12, 0, 1}, {}, 0, 2},
      {"oblique cap chord", {0, 0, -8}, {0.25, 0, 1}, {2, 14}},
      {"nonunit transverse", {-8, 0, 0}, {0.75, 0, 0}, {4 / 0.75, 12 / 0.75}},
      {"thirty degree hollow pose",
       {-8, 0, 0},
       {1, 0, 0},
       {4, 6, 10, 12},
       0,
       2,
       {4, 3, 6},
       {-7, 5, -3},
       4},
      {"negative translated rotated axis",
       {0, 0, -8},
       {0, 0, 1},
       {2, 14},
       0,
       0,
       {4, 3, 6},
       {-7, 5, -3},
       1},
      {"rotated hollow transverse",
       {-8, 0, 0},
       {1, 0, 0},
       {4, 6, 10, 12},
       0,
       2,
       {4, 3, 6},
       {-7, 5, -3},
       1},
    },
    Shape::Tubs);
}

TEST_F(GGEMSNavigationDeviceTest, N1TubsDirectionalSurfacesAndTolerance) {
  Case base{"band probe", {-8, 0, 0}, {1, 0, 0}, {}};
  double const band =
    Scene(base, Shape::Tubs).GetVolumeNavigationTolerancePM() / k_mm;
  double const near = 4 - 64 * band;
  double const half_chord = std::sqrt(16 - near * near);
  double const shallow_bore = 2 - band / 2;
  double const shallow_outer = std::sqrt(16 - shallow_bore * shallow_bore);
  double const resolved_bore = 2 - 64 * band;
  double const resolved_inner = std::sqrt(4 - resolved_bore * resolved_bore);
  double const resolved_outer = std::sqrt(16 - resolved_bore * resolved_bore);
  RunCases(
    {
      {"outer tangent", {-8, 4, 0}, {1, 0, 0}, {}},
      {"outer shallow sub-band chord", {-8, 4 - band / 2, 0}, {1, 0, 0}, {}},
      {"outer resolved near tangent",
       {-8, near, 0},
       {1, 0, 0},
       {8 - half_chord, 8 + half_chord},
       0,
       0,
       {4, 3, 6},
       {},
       0,
       4 / half_chord},
      {"outer surface inward", {4, 0, 0}, {-1, 0, 0}, {8}, 1},
      {"outer surface outward", {4, 0, 0}, {1, 0, 0}, {}},
      {"closed outer band inward", {4 + band, 0, 0}, {-1, 0, 0}, {8 + band}, 1},
      {"closed contracted band outward", {4 - band, 0, 0}, {1, 0, 0}, {}},
      {"inner surface into bore", {2, 0, 0}, {-1, 0, 0}, {4, 6}, 0, 2},
      {"inner surface into material", {2, 0, 0}, {1, 0, 0}, {2}, 1, 2},
      {"inner tangent birth", {2, 0, 0}, {0, 1, 0}, {std::sqrt(12.0)}, 1, 2},
      {"closed inner band toward material",
       {2 - band, 0, 0},
       {1, 0, 0},
       {2 + band},
       1,
       2},
      {"closed inner band toward bore",
       {2 + band, 0, 0},
       {-1, 0, 0},
       {4 + band, 6 + band},
       0,
       2},
      {"shallow bore chord suppressed",
       {-8, shallow_bore, 0},
       {1, 0, 0},
       {8 - shallow_outer, 8 + shallow_outer},
       0,
       2},
      {"resolved bore near tangent",
       {-8, resolved_bore, 0},
       {1, 0, 0},
       {8 - resolved_outer, 8 - resolved_inner, 8 + resolved_inner,
        8 + resolved_outer},
       0,
       2,
       {4, 3, 6},
       {},
       0,
       2 / resolved_inner},
      {"cap inward", {3, 0, 6}, {0, 0, -1}, {12}, 1, 2},
      {"cap outward", {3, 0, 6}, {0, 0, 1}, {}, 0, 2},
      {"closed cap band inward",
       {3, 0, 6 + band},
       {0, 0, -1},
       {12 + band},
       1,
       2},
      {"closed contracted cap outward", {3, 0, 6 - band}, {0, 0, 1}, {}, 0, 2},
      {"bore cap axis", {0, 0, 6}, {0, 0, -1}, {}, 0, 2},
      {"inner rim inward", {2, 0, 6}, {1, 0, -1}, {2}, 1, 2},
      {"inner rim through bore", {2, 0, 6}, {-1, 0, -1}, {4, 6}, 0, 2},
      {"rim tangential axial contact", {4, 0, -8}, {0, 0, 1}, {}},
      {"rim inward", {4, 0, 6}, {-1, 0, -1}, {8}, 1},
    },
    Shape::Tubs);
}

TEST_F(GGEMSNavigationDeviceTest, N1EllipsoidIndependentChordsAndPoses) {
  double const chord = 2 * std::sqrt(3.0);
  double const oblique = 1 / std::sqrt(1.0 / 16 + 1.0 / 36);
  RunCases(
    {
      {"unequal axes central", {-8, 0, 0}, {1, 0, 0}, {4, 12}},
      {"unequal axes off axis",
       {-8, 1.5, 0},
       {1, 0, 0},
       {8 - chord, 8 + chord}},
      {"z axis nonunit", {0, 0, -10}, {0, 0, 0.5}, {8, 32}},
      {"center oblique", {0, 0, 0}, {1, 0, 1}, {oblique}, 1},
      {"oblique central chord",
       {-8, 0, -8},
       {1, 0, 1},
       {8 - oblique, 8 + oblique}},
      {"interior off axis", {0, 1.5, 0}, {0.75, 0, 0}, {chord / 0.75}, 1},
      {"sphere equivalent",
       {-8, 2, 0},
       {1, 0, 0},
       {8 - chord, 8 + chord},
       0,
       0,
       {4, 4, 4}},
      {"negative translated rotated",
       {-8, 1.5, 0},
       {1, 0, 0},
       {8 - chord, 8 + chord},
       0,
       0,
       {4, 3, 6},
       {-7, 5, -3},
       2},
      {"separated miss", {-8, 4, 0}, {1, 0, 0}, {}},
      {"forty five degree unequal pose",
       {-8, 1.5, 0},
       {1, 0, 0},
       {8 - chord, 8 + chord},
       0,
       0,
       {4, 3, 6},
       {-7, 5, -3},
       3},
    },
    Shape::Ellipsoid);
}

TEST_F(GGEMSNavigationDeviceTest, N1EllipsoidDirectionalSurfacesAndTolerance) {
  Case base{"band probe", {-8, 0, 0}, {1, 0, 0}, {}};
  double const band =
    Scene(base, Shape::Ellipsoid).GetVolumeNavigationTolerancePM() / k_mm;
  double const near = 3 - 64 * band;
  double const half_chord = 4 * std::sqrt(1 - near * near / 9);
  RunCases(
    {
      {"ordinary tangent", {-8, 3, 0}, {1, 0, 0}, {}},
      {"surface tangent birth", {0, 3, 0}, {1, 0, 0}, {}},
      {"shallow sub-band chord", {-8, 3 - band / 2, 0}, {1, 0, 0}, {}},
      {"resolved near tangent",
       {-8, near, 0},
       {1, 0, 0},
       {8 - half_chord, 8 + half_chord},
       0,
       0,
       {4, 3, 6},
       {},
       0,
       6 / half_chord},
      {"surface inward", {4, 0, 0}, {-1, 0, 0}, {8}, 1},
      {"surface outward", {4, 0, 0}, {1, 0, 0}, {}},
      {"closed X band inward", {4 + band, 0, 0}, {-1, 0, 0}, {8 + band}, 1},
      {"closed X contracted band outward", {4 - band, 0, 0}, {1, 0, 0}, {}},
      {"closed Y band inward", {0, 3 + band, 0}, {0, -1, 0}, {6 + band}, 1},
      {"closed Y contracted band outward", {0, 3 - band, 0}, {0, 1, 0}, {}},
      {"closed Z band inward", {0, 0, 6 + band}, {0, 0, -1}, {12 + band}, 1},
      {"closed Z contracted band outward", {0, 0, 6 - band}, {0, 0, 1}, {}},
      {"outside band inward birth",
       {4 + band / 2, 0, 0},
       {-1, 0, 0},
       {8 + band / 2},
       1},
      {"inside band outward birth", {4 - band / 2, 0, 0}, {1, 0, 0}, {}},
    },
    Shape::Ellipsoid);
}

TEST_F(GGEMSNavigationDeviceTest, N1TrdIndependentInclinedFacesAndPoses) {
  // |x| <= 3+z/6, |y| <= 3-z/6, |z| <= 6, in mm.
  RunCases(
    {
      {
        .name = "Trd central X chord",
        .start = {-8, 0, 0},
        .direction = {1, 0, 0},
        .crossings = {5, 11},
      },
      {
        .name = "Trd negative Z independent X",
        .start = {-8, 0, -3},
        .direction = {1, 0, 0},
        .crossings = {5.5, 10.5},
      },
      {
        .name = "Trd negative Z independent Y",
        .start = {0, -8, -3},
        .direction = {0, 1, 0},
        .crossings = {4.5, 11.5},
      },
      {
        .name = "Trd positive Z independent X",
        .start = {-8, 0, 3},
        .direction = {1, 0, 0},
        .crossings = {4.5, 11.5},
      },
      {
        .name = "Trd positive Z independent Y",
        .start = {0, -8, 3},
        .direction = {0, 1, 0},
        .crossings = {5.5, 10.5},
      },
      {
        .name = "Trd both end caps",
        .start = {0, 0, -8},
        .direction = {0, 0, 1},
        .crossings = {2, 14},
      },
      {
        .name = "Trd nonunit direction",
        .start = {-8, 0, 0},
        .direction = {.75, 0, 0},
        .crossings = {5 / .75, 11 / .75},
      },
      {
        .name = "Trd interior birth",
        .start = {0, 0, 0},
        .direction = {0, 1, 0},
        .crossings = {3},
        .initial_owner = 1,
      },
      {
        .name = "Trd two inclined faces",
        .start = {5, 0, -6},
        .direction = {-1, 0, 1},
        .crossings = {18.0 / 7, 42.0 / 5},
      },
      {
        .name = "Trd edge entry and exit",
        .start = {-6, -6, 0},
        .direction = {1, 1, 0},
        .crossings = {3, 9},
      },
      {
        .name = "Trd cap corner entry",
        .start = {-4, -8, -12},
        .direction = {1.0 / 3, 2.0 / 3, 1},
        .crossings = {6, 15.6},
      },
      {
        .name = "Trd positive right hand quarter turn",
        .start = {-8, 0, -3},
        .direction = {1, 0, 0},
        .crossings = {5.5, 10.5},
        .center = {-7, 5, -3},
        .pose = 1,
      },
      {
        .name = "Trd negative right hand quarter turn",
        .start = {-8, 0, -3},
        .direction = {1, 0, 0},
        .crossings = {5.5, 10.5},
        .center = {-7, 5, -3},
        .pose = 5,
      },
      {
        .name = "Trd nontrivial XYZ pose",
        .start = {-8, 0, 3},
        .direction = {.75, 0, 0},
        .crossings = {4.5 / .75, 11.5 / .75},
        .center = {-7, 5, -3},
        .pose = 6,
      },
      {
        .name = "Trd separated parallel miss",
        .start = {-8, 5, 0},
        .direction = {1, 0, 0},
        .crossings = {},
      },
    },
    Shape::Trd);
}

TEST_F(GGEMSNavigationDeviceTest, N1TrdDirectionalSurfacesAndTolerance) {
  Case const base{
    .name = "Trd band probe",
    .start = {},
    .direction = {1, 0, 0},
    .crossings = {},
  };
  double const band =
    Scene(base, Shape::Trd).GetVolumeNavigationTolerancePM() / k_mm;
  double const side_band = band * std::sqrt(37.0 / 36);
  RunCases(
    {
      {
        .name = "Trd side inward",
        .start = {3, 0, 0},
        .direction = {-1, 0, 0},
        .crossings = {6},
        .initial_owner = 1,
      },
      {
        .name = "Trd side outward",
        .start = {3, 0, 0},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trd side parallel",
        .start = {3, 0, 0},
        .direction = {1.0 / 6, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trd side sub-band parallel",
        .start = {3 - (side_band / 2), 0, 0},
        .direction = {1.0 / 6, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trd side resolved parallel interior",
        .start = {3 - (16 * side_band), 0, 0},
        .direction = {1.0 / 6, 0, 1},
        .crossings = {6},
        .initial_owner = 1,
      },
      {
        .name = "Trd side near grazing inward",
        .start = {3, 0, 0},
        .direction = {(1.0 / 6) - .001, 0, 1},
        .crossings = {6},
        .initial_owner = 1,
        .conditioning = 1000,
      },
      {
        .name = "Trd side near grazing outward",
        .start = {3, 0, 0},
        .direction = {(1.0 / 6) + .001, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trd cap inward",
        .start = {0, 0, 6},
        .direction = {0, 0, -1},
        .crossings = {12},
        .initial_owner = 1,
      },
      {
        .name = "Trd cap outward",
        .start = {0, 0, 6},
        .direction = {0, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trd cap parallel",
        .start = {0, 0, 6},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trd closed cap outer band",
        .start = {0, 0, 6 + band},
        .direction = {0, 0, -1},
        .crossings = {12 + band},
        .initial_owner = 1,
      },
      {
        .name = "Trd closed cap contracted band",
        .start = {0, 0, 6 - band},
        .direction = {0, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trd outside cap band",
        .start = {0, 0, 6 + (4 * band)},
        .direction = {0, 0, -1},
        .crossings = {4 * band, 12 + (4 * band)},
      },
      {
        .name = "Trd cap shallow chord suppressed",
        .start = {-8, 0, 6 - (band / 2)},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trd lower corner inward",
        .start = {-2, -4, -6},
        .direction = {1, 1, 1},
        .crossings = {24.0 / 5},
        .initial_owner = 1,
      },
      {
        .name = "Trd lower corner outward",
        .start = {-2, -4, -6},
        .direction = {-1, -1, -1},
        .crossings = {},
      },
      {
        .name = "Trd corner touch only",
        .start = {0, 0, -12},
        .direction = {1.0 / 3, 2.0 / 3, 1},
        .crossings = {},
      },
    },
    Shape::Trd);
}

TEST_F(GGEMSNavigationDeviceTest, N1TrapIndependentSkewInclinationAndPoses) {
  RunCases(
    {
      {
        .name = "Trap central X chord",
        .start = {-8, 0, 0},
        .direction = {1, 0, 0},
        .crossings = {5, 11},
      },
      {
        .name = "Trap unequal skewed X sides",
        .start = {-8, 1, 0},
        .direction = {1, 0, 0},
        .crossings = {5.05, 11.45},
      },
      {
        .name = "Trap opposite skewed X sides",
        .start = {-8, -1, 0},
        .direction = {1, 0, 0},
        .crossings = {4.95, 10.55},
      },
      {
        .name = "Trap negative face dimensions",
        .start = {-8, -.75, -3},
        .direction = {1, 0, 0},
        .crossings = {4.5, 9.5},
      },
      {
        .name = "Trap positive face dimensions",
        .start = {-8, .75, 3},
        .direction = {1, 0, 0},
        .crossings = {5.5, 12.5},
      },
      {
        .name = "Trap Y chord",
        .start = {0, -8, 0},
        .direction = {0, 1, 0},
        .crossings = {5, 11},
      },
      {
        .name = "Trap parallel end caps",
        .start = {0, 0, -8},
        .direction = {0, 0, 1},
        .crossings = {2, 14},
      },
      {
        .name = "Trap inclined center line",
        .start = {-8.0 / 3, -2, -8},
        .direction = {1.0 / 3, .25, 1},
        .crossings = {2, 14},
      },
      {
        .name = "Trap interior nonunit",
        .start = {0, 1, 0},
        .direction = {.75, 0, 0},
        .crossings = {3.45 / .75},
        .initial_owner = 1,
      },
      {
        .name = "Trap lower cap corner entry",
        .start = {-6.2, -7.5, -8},
        .direction = {1, 1, 1},
        .crossings = {2, 118.0 / 11},
      },
      {
        .name = "Trap positive right hand quarter turn",
        .start = {-8, 1, 0},
        .direction = {1, 0, 0},
        .crossings = {5.05, 11.45},
        .center = {-7, 5, -3},
        .pose = 1,
      },
      {
        .name = "Trap negative right hand quarter turn",
        .start = {-8, 1, 0},
        .direction = {1, 0, 0},
        .crossings = {5.05, 11.45},
        .center = {-7, 5, -3},
        .pose = 5,
      },
      {
        .name = "Trap nontrivial XYZ pose",
        .start = {-8, 1, 0},
        .direction = {.75, 0, 0},
        .crossings = {5.05 / .75, 11.45 / .75},
        .center = {-7, 5, -3},
        .pose = 6,
      },
      {
        .name = "Trap separated parallel miss",
        .start = {-8, 5, 0},
        .direction = {1, 0, 0},
        .crossings = {},
      },
    },
    Shape::Trap);
}

TEST_F(GGEMSNavigationDeviceTest, N1TrapDirectionalSurfacesAndTolerance) {
  Case const base{
    .name = "Trap band probe",
    .start = {},
    .direction = {1, 0, 0},
    .crossings = {},
  };
  double const band =
    Scene(base, Shape::Trap).GetVolumeNavigationTolerancePM() / k_mm;
  RunCases(
    {
      {
        .name = "Trap positive side inward",
        .start = {3, 0, 0},
        .direction = {-1, 0, 0},
        .crossings = {6},
        .initial_owner = 1,
      },
      {
        .name = "Trap positive side outward",
        .start = {3, 0, 0},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trap positive side parallel",
        .start = {3, 0, 0},
        .direction = {.3875, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trap negative side inward",
        .start = {-3, 0, 0},
        .direction = {1, 0, 0},
        .crossings = {6},
        .initial_owner = 1,
      },
      {
        .name = "Trap negative side outward",
        .start = {-3, 0, 0},
        .direction = {-1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trap negative side parallel",
        .start = {-3, 0, 0},
        .direction = {37.0 / 240, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trap near grazing inward",
        .start = {3, 0, 0},
        .direction = {.3875 - .001, 0, 1},
        .crossings = {6},
        .initial_owner = 1,
        .conditioning = 1000,
      },
      {
        .name = "Trap near grazing outward",
        .start = {3, 0, 0},
        .direction = {.3875 + .001, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trap cap inward",
        .start = {0, 0, 6},
        .direction = {0, 0, -1},
        .crossings = {12},
        .initial_owner = 1,
      },
      {
        .name = "Trap cap outward",
        .start = {0, 0, 6},
        .direction = {0, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trap cap parallel",
        .start = {0, 0, 6},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trap closed cap outer band",
        .start = {0, 0, 6 + band},
        .direction = {0, 0, -1},
        .crossings = {12 + band},
        .initial_owner = 1,
      },
      {
        .name = "Trap closed cap contracted band",
        .start = {0, 0, 6 - band},
        .direction = {0, 0, 1},
        .crossings = {},
      },
      {
        .name = "Trap outside cap band",
        .start = {0, 0, 6 + (4 * band)},
        .direction = {0, 0, -1},
        .crossings = {4 * band, 12 + (4 * band)},
      },
      {
        .name = "Trap cap shallow chord suppressed",
        .start = {-8, 0, 6 - (band / 2)},
        .direction = {1, 0, 0},
        .crossings = {},
      },
      {
        .name = "Trap lower corner inward",
        .start = {-4.2, -5.5, -6},
        .direction = {1, 1, 1},
        .crossings = {96.0 / 11},
        .initial_owner = 1,
      },
      {
        .name = "Trap upper corner inward",
        .start = {6.9, 3.5, 6},
        .direction = {-1, -1, -1},
        .crossings = {48.0 / 7},
        .initial_owner = 1,
      },
      {
        .name = "Trap lower corner outward",
        .start = {-4.2, -5.5, -6},
        .direction = {-1, -1, -1},
        .crossings = {},
      },
    },
    Shape::Trap);
}

TEST_F(GGEMSNavigationDeviceTest, N1TubsEllipsoidTrdTrapRunOwnsPosedSnapshot) {
  for (auto const kind :
       {Shape::Tubs, Shape::Ellipsoid, Shape::Trd, Shape::Trap}) {
    SCOPED_TRACE(static_cast<unsigned int>(kind));
    auto source = std::make_shared<sources::GGEMSSource>();
    source->SetAnalytic()
      .SetPrimaryCount(1U)
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
    run.SetObserver(observer);
    float world_band_pm{0};
    {
      Case const fixture{"retained posed snapshot",
                         {-8, 0, 0},
                         {1, 0, 0},
                         {},
                         0,
                         2,
                         {4, 3, 6},
                         {-7, 5, -3},
                         1};
      auto world = Scene(fixture, kind);
      world_band_pm = world.GetNavigationTolerancePM();
      run.SetWorld(world);
      world.SetSize(160_mm, 160_mm, 160_mm);
    }
    ASSERT_NO_THROW(run.Initialize());
    ASSERT_NO_THROW(run.Run());
    auto const records = observer->GetRecords();
    ASSERT_EQ(records.size(), 2U);
    EXPECT_EQ(records[1].status,
              particles::ToKernelParticleStatus(
                particles::GGEMSParticleStatus::EscapedWorld));
    // The Observer converts an FP32 terminal position to integer pm. This
    // conversion does not make the accepted World endpoint exactly integral.
    EXPECT_LE(std::abs(static_cast<double>(records[1].position_x_pm) - 40.0e9),
              static_cast<double>(world_band_pm));
    EXPECT_EQ(records[1].position_y_pm, 5'000'000'000LL);
    EXPECT_EQ(records[1].position_z_pm, -3'000'000'000LL);
    EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
    // Public observation proves snapshot lifetime and successful host upload;
    // direct queries and worker traces above prove intermediate topology.
  }
}
