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
 * \brief Qualifies rotated Box crossings against independently chosen planes.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <numbers>
#include <string>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSNavigationTestDevices.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Position = std::array<std::int64_t, 3>;
using Vector = std::array<long double, 3>;

/*! \brief A hand-selected physical plane, not a host slab navigator. */
struct ExpectedPlane {
  Vector normal;
  long double offset;
  std::uint32_t faces;
  std::uint32_t event;
};

/*! \brief One authored scene and the analytically known crossing sequence. */
struct Case {
  char const *name;
  std::int64_t world_half{100};
  Position half{20, 10, 5};
  Position center{};
  std::array<units::Angle, 3> rotation{0.0_deg, 0.0_deg, 90.0_deg};
  Position start{-50, 0, 0};
  std::array<float, 3> direction{1.0F, 0.0F, 0.0F};
  std::uint32_t initial_owner{0};
  std::vector<ExpectedPlane> planes;
  std::uint32_t expected_status{0};
  bool qualify_world_parameter{true};
};

/*! \brief Builds authored poses, with expected planes derived independently. */
auto Cases() -> std::vector<Case> {
  Vector const x{1.0L, 0.0L, 0.0L};
  auto const inverse_root_two = 1.0L / std::numbers::sqrt2_v<long double>;
  auto const cosine_thirty = std::numbers::sqrt3_v<long double> / 2.0L;
  std::vector<Case> cases;
  Case axis{.name = "90-degree axis swap", .planes = {}};
  axis.planes = {{.normal = x, .offset = -10, .faces = 8, .event = 2},
                 {.normal = x, .offset = 10, .faces = 4, .event = 3},
                 {.normal = x, .offset = 100, .faces = 2, .event = 1}};
  cases.push_back(axis);
  for (auto const sign : {-1.0F, 1.0F}) {
    for (auto const initial_y : {0, 1}) {
      Case half = axis;
      half.name = "final signed half-pm rounding";
      half.start = {
        -11, static_cast<std::int64_t>(-sign * static_cast<float>(initial_y)),
        0};
      half.direction[1] = 0.5F * sign;
      cases.push_back(half);
    }
  }
  Case pathological = axis;
  pathological.name = "near-parallel unresolved surface";
  pathological.rotation[2] = units::MakeRadians(1.0e-20L);
  pathological.start = {-50, 10, 0};
  pathological.expected_status = 2;
  pathological.planes.clear();
  cases.push_back(pathological);
  Case inside = axis;
  inside.name = "inside birth";
  inside.start = {0, 0, 0};
  inside.initial_owner = 1;
  inside.planes.erase(inside.planes.begin());
  cases.push_back(inside);
  inside.name = "inward face birth";
  inside.start = {-10, 0, 0};
  cases.push_back(inside);
  Case outward = axis;
  outward.name = "outward face birth";
  outward.start = {-10, 0, 0};
  outward.direction = {-1.0F, 0.0F, 0.0F};
  outward.planes = {{.normal = x, .offset = -100, .faces = 1, .event = 1}};
  cases.push_back(outward);
  for (auto const height : {20, 21}) {
    Case miss = axis;
    miss.name = height == 20 ? "grazing face" : "no hit";
    miss.start[1] = height;
    miss.planes = {{.normal = x, .offset = 100, .faces = 2, .event = 1}};
    cases.push_back(miss);
    if (height == 20) {
      miss.name = "grazing edge";
      miss.start[2] = 5;
      cases.push_back(miss);
    }
  }
  Case corner = axis;
  corner.name = "corner entry";
  corner.start = {-30, -40, 0};
  corner.direction = {0.5F, 0.5F, 0.0F};
  corner.planes.front().faces = 9;
  cases.push_back(corner);
  Case shifted = axis;
  shifted.name = "negative shifted center";
  shifted.center = {-13, 17, -9};
  shifted.start = {-63, 17, -9};
  shifted.planes.back().offset = 113;
  cases.push_back(shifted);
  Case negative = axis;
  negative.name = "negative quarter turn";
  negative.rotation[2] = -90.0_deg;
  negative.planes[0].faces = 4;
  negative.planes[1].faces = 8;
  cases.push_back(negative);
  Case diagonal = axis;
  diagonal.name = "45-degree diagonal non-unit stored direction";
  diagonal.rotation[2] = 45.0_deg;
  diagonal.start = {-40, -40, 0};
  diagonal.direction = {0.5F, 0.5F, 0.0F};
  Vector const diagonal_normal{inverse_root_two, inverse_root_two, 0};
  diagonal.planes = {
    {.normal = diagonal_normal, .offset = -20, .faces = 1, .event = 2},
    {.normal = diagonal_normal, .offset = 20, .faces = 2, .event = 3},
    {.normal = x, .offset = 100, .faces = 10, .event = 1}};
  cases.push_back(diagonal);
  Case tiny = axis;
  tiny.name = "positive entry with zero rounded displacement";
  tiny.half = {2, 5, 5};
  tiny.rotation[2] = 45.0_deg;
  tiny.start = {-3, 0, 0};
  tiny.planes = {
    {.normal = diagonal_normal, .offset = -2, .faces = 1, .event = 2},
    {.normal = diagonal_normal, .offset = 2, .faces = 2, .event = 3},
    {.normal = x, .offset = 100, .faces = 2, .event = 1}};
  cases.push_back(tiny);
  tiny.name = "entry and exit at the same canonical point";
  tiny.half = {1, 1, 1};
  tiny.start = {-1, 1, 0};
  tiny.planes = {
    {.normal = {-inverse_root_two, inverse_root_two, 0},
     .offset = 1,
     .faces = 8,
     .event = 2},
    {.normal = diagonal_normal, .offset = 1, .faces = 2, .event = 3},
    {.normal = x, .offset = 100, .faces = 2, .event = 1}};
  cases.push_back(tiny);
  Case spatial = axis;
  spatial.name = "three-angle oblique ray";
  spatial.half = {10, 30, 50};
  spatial.rotation = {30.0_deg, 45.0_deg, 30.0_deg};
  spatial.start = {-60, -40, -20};
  spatial.direction = {0.75F, 0.5F, 0.25F};
  Vector const spatial_normal{cosine_thirty * inverse_root_two,
                              0.5L * inverse_root_two, -inverse_root_two};
  spatial.planes = {
    {.normal = spatial_normal, .offset = -10, .faces = 1, .event = 2},
    {.normal = spatial_normal, .offset = 10, .faces = 2, .event = 3},
    {.normal = x, .offset = 100, .faces = 2, .event = 1}};
  cases.push_back(spatial);
  for (auto const scale :
       {1LL, 100'000'000LL, 1'000'000'000LL, 100'000'000'000LL}) {
    Case medical = axis;
    medical.name = "meter Box";
    if (scale == 1) {
      medical.name = "30-degree oblique face";
    } else if (scale == 100'000'000LL) {
      medical.name = "millimeter Box";
    } else if (scale == 1'000'000'000LL) {
      medical.name = "centimeter Box";
    }
    medical.rotation[2] = 30.0_deg;
    medical.half = {10 * scale, 30 * scale, 50 * scale};
    medical.world_half = 100 * scale;
    medical.start = {-60 * scale, 3 * scale, 0};
    Vector const normal{cosine_thirty, 0.5L, 0};
    medical.planes = {
      {.normal = normal, .offset = -10.0L * scale, .faces = 1, .event = 2},
      {.normal = normal, .offset = 10.0L * scale, .faces = 2, .event = 3},
      {.normal = x, .offset = 100.0L * scale, .faces = 2, .event = 1}};
    cases.push_back(medical);
    if (scale == 1'000'000'000LL) {
      medical.name = "small offset at large common translation";
      medical.qualify_world_parameter = false;
      medical.world_half = 1LL << 61;
      medical.center = {(1LL << 60) + 123, -(1LL << 59) + 17, -111};
      for (std::size_t axis_index = 0; axis_index < 3; ++axis_index) {
        medical.start[axis_index] += medical.center[axis_index];
      }
      medical.planes.back().offset = medical.world_half - medical.center[0];
      cases.push_back(medical);
    }
  }
  return cases;
}

/*! \brief Records the separately measured position, parameter and length
 * errors. */
struct Errors {
  long double position{0};
  long double segment_parameter{0};
  long double accumulated_parameter{0};
  long double accumulated_length{0};
  long double world_position{0};
  long double world_parameter{0};
  long double world_length{0};
};

/*! \brief Intersects a prescribed global plane with a ray, without any search.
 */
auto Parameter(ExpectedPlane const &plane, Position const &position,
               Position const &center, std::array<float, 3> const &direction)
  -> long double {
  long double numerator = plane.offset;
  long double denominator = 0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    numerator -= plane.normal[axis] *
                 static_cast<long double>(position[axis] - center[axis]);
    denominator += plane.normal[axis] * direction[axis];
  }
  return numerator / denominator;
}

/*! \brief Executes production queries/transactions and checks the prescribed
 * sequence. */
auto CheckCase(ocl::GGEMSOpenCLContext &context,
               ocl::GGEMSOpenCLProgram const &program, Case const &test,
               Errors &errors) -> void {
  SCOPED_TRACE(test.name);
  auto const material = materials::builtins::BuildBuiltInMaterial("Vacuum");
  geometry::GGEMSBox box;
  box
    .SetSize(units::Length{static_cast<std::uint64_t>(2 * test.half[0])},
             units::Length{static_cast<std::uint64_t>(2 * test.half[1])},
             units::Length{static_cast<std::uint64_t>(2 * test.half[2])})
    .SetPosition(
      geometry::MakePositionPM(test.center[0], test.center[1], test.center[2]))
    .SetRotation(test.rotation[0], test.rotation[1], test.rotation[2])
    .SetMaterial(material);
  geometry::GGEMSWorld world;
  auto const size =
    units::Length{static_cast<std::uint64_t>(2 * test.world_half)};
  world.SetSize(size, size, size).SetMaterial(material).AddVolume(box);
  world.Validate();
  auto const pose = *world.BuildBoxPoseRecord();
  auto const volume = world.BuildAnalyticVolumeRecord(7U);
  auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
    auto result = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
    ocl::WriteSVMFromHost(result, value);
    return result;
  };
  auto world_buffer = buffer(world.BuildRecord());
  auto box_buffer = buffer(volume);
  auto pose_buffer = buffer(pose);
  particles::GGEMSParticleState particle;
  particle.position_x_pm = test.start[0];
  particle.position_y_pm = test.start[1];
  particle.position_z_pm = test.start[2];
  particle.direction_x = test.direction[0];
  particle.direction_y = test.direction[1];
  particle.direction_z = test.direction[2];
  particle.energy_micro_eV = 123;
  particle.time_ps = 17;
  particle.status =
    particles::ToKernelParticleStatus(particles::GGEMSParticleStatus::Alive);
  auto particle_buffer = buffer(particle);
  std::array<std::int64_t, 31> trace{};
  std::array<double, 6> parameters{};
  std::array<std::uint64_t, 15> layout{};
  auto trace_buffer = buffer(trace);
  auto parameter_buffer = buffer(parameters);
  auto layout_buffer = buffer(layout);
  ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel("rotated_box_navigation_probe"),
    "rotated_box_navigation_probe"};
  std::array<void *, 7> pointers{
    world_buffer.GetData(), box_buffer.GetData(),
    pose_buffer.GetData(),  particle_buffer.GetData(),
    trace_buffer.GetData(), parameter_buffer.GetData(),
    layout_buffer.GetData()};
  for (cl_uint index = 0; index < pointers.size(); ++index) {
    kernel.SetArgSVMPointer(index, pointers[index]);
  }
  kernel.Run({1U}, {1U});
  ocl::ReadSVMToHost(trace_buffer, std::span{trace});
  ocl::ReadSVMToHost(parameter_buffer, std::span{parameters});
  ocl::ReadSVMToHost(layout_buffer, std::span{layout});
  std::array<std::uint64_t, 15> const expected_layout{
    sizeof(geometry::GGEMSBoxPoseRecord),
    offsetof(geometry::GGEMSBoxPoseRecord, center_x_pm),
    offsetof(geometry::GGEMSBoxPoseRecord, center_y_pm),
    offsetof(geometry::GGEMSBoxPoseRecord, center_z_pm),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_x_x),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_x_y),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_x_z),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_y_x),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_y_y),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_y_z),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_z_x),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_z_y),
    offsetof(geometry::GGEMSBoxPoseRecord, axis_z_z),
    sizeof(particles::GGEMSParticleState),
    sizeof(geometry::GGEMSBoxRecord)};
  EXPECT_EQ(layout, expected_layout);
  ASSERT_EQ(trace[1], test.expected_status);
  if (test.expected_status != 0) {
    EXPECT_EQ(trace[2], 0);
    return;
  }
  ASSERT_EQ(trace[2], test.planes.size());
  EXPECT_EQ(trace[0], test.initial_owner);
  EXPECT_EQ(trace[30], 1);
  auto current = test.start;
  auto departure = test.initial_owner;
  long double accumulated = 0;
  long double accumulated_length = 0;
  long double norm_squared = 0;
  for (auto const component : test.direction) {
    norm_squared += static_cast<long double>(component) * component;
  }
  auto const norm = std::sqrt(norm_squared);
  for (std::size_t index = 0; index < test.planes.size(); ++index) {
    auto const &plane = test.planes[index];
    auto const offset = 3 + (9 * index);
    std::uint32_t arrival = 0;
    std::uint32_t material_id = 0;
    if (plane.event == 1) {
      arrival = 0xFFFFFFFFU;
      material_id = 0xFFFFFFFFU;
    } else if (plane.event == 2) {
      arrival = 1;
      material_id = 7;
    }
    EXPECT_EQ(trace[offset], plane.event);
    EXPECT_EQ(trace[offset + 1], plane.faces);
    EXPECT_EQ(trace[offset + 2], departure);
    EXPECT_EQ(trace[offset + 3], arrival);
    EXPECT_EQ(trace[offset + 4], material_id);
    EXPECT_EQ(trace[offset + 8],
              particles::ToKernelParticleStatus(
                plane.event == 1 ? particles::GGEMSParticleStatus::EscapedWorld
                                 : particles::GGEMSParticleStatus::Alive));
    auto const segment = Parameter(plane, current, test.center, test.direction);
    auto const original =
      Parameter(plane, test.start, test.center, test.direction);
    accumulated += parameters[2 * index];
    accumulated_length += parameters[(2 * index) + 1];
    if (plane.event != 1) {
      errors.segment_parameter = std::max(
        errors.segment_parameter, std::abs(parameters[2 * index] - segment));
      errors.accumulated_parameter = std::max(errors.accumulated_parameter,
                                              std::abs(accumulated - original));
      errors.accumulated_length =
        std::max(errors.accumulated_length,
                 std::abs(accumulated_length - (original * norm)));
      EXPECT_LE(std::abs(parameters[2 * index] - segment), 0.1L);
      EXPECT_LE(std::abs(accumulated - original) * norm, 2.0L);
    }
    if (plane.event == 1 && test.qualify_world_parameter) {
      errors.world_parameter =
        std::max(errors.world_parameter, std::abs(accumulated - original));
      errors.world_length = std::max(
        errors.world_length, std::abs(accumulated_length - (original * norm)));
      EXPECT_LE(std::abs(accumulated - original) * norm, 2.0L);
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      auto const actual = trace[offset + 5 + axis];
      auto const expected = static_cast<std::int64_t>(
        std::round(static_cast<long double>(current[axis]) +
                   (segment * test.direction[axis])));
      EXPECT_LE(std::abs(actual - expected), 1);
      if (plane.event != 1) {
        long double const discrepancy =
          std::abs(static_cast<long double>(actual - test.start[axis]) -
                   (original * test.direction[axis]));
        errors.position = std::max(errors.position, discrepancy);
        EXPECT_LE(discrepancy, 2.0L);
      }
      if (plane.event == 1) {
        long double const discrepancy =
          std::abs(static_cast<long double>(actual - test.start[axis]) -
                   (original * test.direction[axis]));
        errors.world_position = std::max(errors.world_position, discrepancy);
        EXPECT_LE(discrepancy, 2.0L);
      }
      current[axis] = actual;
    }
    departure = arrival;
  }
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest,
       RotatedBoxAnalyticPlanesAndCanonicalContinuity) {
  auto &contexts = ocl::GGEMSOpenCL::GetInstance().GetContext();
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  for (auto &context : contexts) {
    SCOPED_TRACE(context.GetDevice().GetName());
    if (context.GetDevice().GetDoubleFpConfig() == 0U) {
      GTEST_SKIP() << "Selected device lacks FP64";
    }
    auto const options = std::format("-I{} -DGGEMS_ENABLE_FP64_NAVIGATION=1",
                                     root.generic_string());
    auto const &program = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "tests", "rotated_box_navigation_probe", options);
    EXPECT_TRUE(program.Matches(context, root / "tests",
                                "rotated_box_navigation_probe", options));
    EXPECT_FALSE(
      program.Matches(context, root / "tests", "rotated_box_navigation_probe",
                      options + " -DGGEMS_TEST_DIFFERENT_OPTIONS=1"));
    EXPECT_EQ(program.GetBuildOptions().find("-cl-fast-relaxed-math"),
              std::string::npos);
    Errors errors;
    auto const cases = Cases();
    for (auto const &test : cases) {
      CheckCase(context, program, test, errors);
    }
    std::cout << std::format(
      "Rotated Box: {} cases, max position {:.6f} pm, segment parameter {:.6f} "
      "pm, "
      "accumulated parameter {:.6f} pm, accumulated physical length {:.6f} "
      "pm\n",
      cases.size(), errors.position, errors.segment_parameter,
      errors.accumulated_parameter, errors.accumulated_length);
    std::cout << std::format(
      "Through World exit: max position {:.6f} pm; medical-scene accumulated "
      "parameter {:.6f} pm, length {:.6f} pm\n",
      errors.world_position, errors.world_parameter, errors.world_length);
    std::cout << "Device: " << context.GetDevice().GetName()
              << "; driver: " << context.GetDevice().GetDriverVersion()
              << "; options: " << program.GetBuildOptions() << '\n';
  }
}
