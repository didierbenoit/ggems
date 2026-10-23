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
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
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
#include "GGEMSBoxFP32Oracle.hh"
#include "GGEMSFP32TestOracle.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Position = std::array<std::int64_t, 3>;

/*! \brief A hand-selected physical face and event, independent of the device.
 */
struct ExpectedPlane {
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
};

/*! \brief Builds authored poses with independently selected crossing faces. */
auto Cases() -> std::vector<Case> {
  std::vector<Case> cases;
  Case axis{.name = "90-degree axis swap", .planes = {}};
  axis.planes = {
    {.faces = 8, .event = 2},
    {.faces = 4, .event = 3},
    {.faces = 2, .event = 1},
  };
  cases.push_back(axis);
  for (auto const sign : {-1.0F, 1.0F}) {
    for (auto const initial_y : {0, 1}) {
      Case half = axis;
      half.name = "signed fractional continuous motion";
      half.start = {
        -11,
        static_cast<std::int64_t>(-sign * static_cast<float>(initial_y)),
        0,
      };
      half.direction[1] = 0.5F * sign;
      cases.push_back(half);
    }
  }
  Case pathological = axis;
  pathological.name = "finite shallow incidence";
  // Replace the removed 2^-48 resolution-band contract with a separated,
  // exactly represented shallow hit through the local X planes.
  pathological.start = {-9, -50, 0};
  pathological.direction = {0.125F, 1.0F, 0.0F};
  pathological.planes = {
    {.faces = 1, .event = 2},
    {.faces = 2, .event = 3},
    {.faces = 8, .event = 1},
  };
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
  outward.planes = {{.faces = 1, .event = 1}};
  cases.push_back(outward);
  for (auto const height : {20, 21}) {
    Case miss = axis;
    miss.name = height == 20 ? "grazing face" : "no hit";
    miss.start[1] = height;
    miss.planes = {{.faces = 2, .event = 1}};
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
  diagonal.planes = {
    {.faces = 1, .event = 2},
    {.faces = 2, .event = 3},
    {.faces = 10, .event = 1},
  };
  cases.push_back(diagonal);
  Case tiny = axis;
  tiny.name = "positive entry with fractional displacement";
  tiny.half = {2, 5, 5};
  tiny.rotation[2] = 45.0_deg;
  tiny.start = {-3, 0, 0};
  tiny.planes = {
    {.faces = 1, .event = 2},
    {.faces = 2, .event = 3},
    {.faces = 2, .event = 1},
  };
  cases.push_back(tiny);
  tiny.name = "distinct FP32 entry and exit within one canonical pm";
  tiny.half = {1, 1, 1};
  tiny.start = {-1, 1, 0};
  tiny.planes = {
    {.faces = 8, .event = 2},
    {.faces = 2, .event = 3},
    {.faces = 2, .event = 1},
  };
  cases.push_back(tiny);
  Case spatial = axis;
  spatial.name = "three-angle oblique ray";
  spatial.half = {10, 30, 50};
  spatial.rotation = {30.0_deg, 45.0_deg, 30.0_deg};
  spatial.start = {-60, -40, -20};
  spatial.direction = {0.75F, 0.5F, 0.25F};
  spatial.planes = {
    {.faces = 1, .event = 2},
    {.faces = 2, .event = 3},
    {.faces = 2, .event = 1},
  };
  cases.push_back(spatial);
  for (auto const scale :
       {1LL, 100'000'000LL, 1'000'000'000LL, 15'000'000'000LL}) {
    Case medical = axis;
    medical.name = "3-meter World";
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
    medical.planes = {
      {.faces = 1, .event = 2},
      {.faces = 2, .event = 3},
      {.faces = 2, .event = 1},
    };
    cases.push_back(medical);
    if (scale == 1'000'000'000LL) {
      medical.name = "translated centimeter Box in 3-meter World";
      medical.world_half = 1'500'000'000'000LL;
      medical.center = {
        300'000'000'000LL,
        -200'000'000'000LL,
        100'000'000'000LL,
      };
      for (std::size_t axis_index = 0; axis_index < 3; ++axis_index) {
        medical.start[axis_index] += medical.center[axis_index];
      }
      cases.push_back(medical);
    }
  }
  return cases;
}

/*! \brief Records the separately measured position, parameter and length
 * errors. */
struct Errors {
  double position{0};
  double segment_parameter{0};
  double accumulated_parameter{0};
  double accumulated_length{0};
  double world_position{0};
  double world_parameter{0};
  double world_length{0};
};

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
  // Upload quantization is an exact conversion contract, separate from the
  // accumulated intersection/movement assertions below.
  for (std::size_t axis = 0; axis < 3; ++axis) {
    EXPECT_EQ(ggems::test::box_oracle::HalfExtents(volume.parameters.box)[axis],
              static_cast<float>(test.half[axis]));
    EXPECT_EQ(ggems::test::box_oracle::Center(pose)[axis],
              static_cast<float>(test.center[axis]));
  }
  auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
    auto result = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
    ocl::WriteSVMFromHost(result, value);
    return result;
  };
  auto world_buffer = buffer(world.BuildRecord());
  auto box_buffer = buffer(volume);
  auto pose_buffer = buffer(pose);
  particles::GGEMSParticleState particle;
  particle.position_x_pm = static_cast<float>(test.start[0]);
  particle.position_y_pm = static_cast<float>(test.start[1]);
  particle.position_z_pm = static_cast<float>(test.start[2]);
  particle.direction_x = test.direction[0];
  particle.direction_y = test.direction[1];
  particle.direction_z = test.direction[2];
  particle.energy_micro_eV = 123;
  particle.time_ps = 17;
  particle.status =
    particles::ToKernelParticleStatus(particles::GGEMSParticleStatus::Alive);
  auto particle_buffer = buffer(particle);
  std::array<std::int64_t, 31> trace{};
  std::array<float, 6> parameters{};
  std::array<std::uint64_t, 15> layout{};
  auto trace_buffer = buffer(trace);
  auto parameter_buffer = buffer(parameters);
  auto layout_buffer = buffer(layout);
  ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel("rotated_box_navigation_probe"),
    "rotated_box_navigation_probe"};
  std::array<void *, 7> pointers{
    world_buffer.GetData(),  box_buffer.GetData(),
    pose_buffer.GetData(),   particle_buffer.GetData(),
    trace_buffer.GetData(),  parameter_buffer.GetData(),
    layout_buffer.GetData(),
  };
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
    sizeof(geometry::GGEMSBoxRecord),
  };
  EXPECT_EQ(layout, expected_layout);
  ASSERT_EQ(trace[1], test.expected_status);
  if (test.expected_status != 0) {
    EXPECT_EQ(trace[2], 0);
    return;
  }
  ASSERT_EQ(trace[2], test.planes.size());
  EXPECT_EQ(trace[0], test.initial_owner);
  EXPECT_EQ(trace[30], 1);
  namespace oracle = ggems::test::box_oracle;
  namespace fp32 = ggems::test::fp32;
  auto const packed_start = oracle::PackedPoint(test.start);
  auto reference = packed_start;
  auto const world_record = world.BuildRecord();
  auto const &box_record = volume.parameters.box;
  auto departure = test.initial_owner;
  double accumulated = 0;
  double accumulated_length = 0;
  fp32::Scalar parameter_sum{};
  fp32::Scalar length_sum{};
  double maximum_position_error = 0;
  double maximum_position_budget = 0;
  double maximum_budget_fraction = 0;
  auto const norm = fp32::Sqrt(
    oracle::Dot(oracle::PackedPoint(test.direction), test.direction));
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
    auto const segment =
      oracle::Parameter(world_record, box_record, pose, reference,
                        test.direction, plane.faces, plane.event == 1);
    auto const original =
      oracle::Parameter(world_record, box_record, pose, packed_start,
                        test.direction, plane.faces, plane.event == 1);
    auto const length = fp32::Multiply(segment, norm);
    fp32::ExpectWithin(parameters[2 * index], segment);
    fp32::ExpectWithin(parameters[(2 * index) + 1], length);
    // These sums are host binary64 diagnostics, so propagate the per-segment
    // uncertainty without adding fictitious FP32 host-addition roundoff.
    parameter_sum.value += segment.value;
    parameter_sum.error += segment.error;
    length_sum.value += length.value;
    length_sum.error += length.error;
    accumulated += static_cast<double>(parameters[2 * index]);
    accumulated_length += static_cast<double>(parameters[(2 * index) + 1]);
    fp32::ExpectWithin(accumulated, {
                                      .value = original.value,
                                      .error = parameter_sum.error,
                                    });
    fp32::ExpectWithin(accumulated_length,
                       {
                         .value = original.value * norm.value,
                         .error = length_sum.error,
                       });
    errors.segment_parameter = std::max(
      errors.segment_parameter,
      std::abs(static_cast<double>(parameters[2 * index]) - segment.value));
    if (plane.event == 1) {
      errors.world_parameter = std::max(errors.world_parameter,
                                        std::abs(accumulated - original.value));
      errors.world_length =
        std::max(errors.world_length,
                 std::abs(accumulated_length - (original.value * norm.value)));
    } else {
      errors.accumulated_parameter = std::max(
        errors.accumulated_parameter, std::abs(accumulated - original.value));
      errors.accumulated_length =
        std::max(errors.accumulated_length,
                 std::abs(accumulated_length - (original.value * norm.value)));
    }
    reference = oracle::Advance(reference, test.direction, segment);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      double const actual = static_cast<double>(std::bit_cast<float>(
        static_cast<std::uint32_t>(trace[offset + 5 + axis])));
      fp32::ExpectWithin(actual, reference[axis]);
      double const discrepancy = std::abs(actual - reference[axis].value);
      maximum_position_error = std::max(maximum_position_error, discrepancy);
      maximum_position_budget =
        std::max(maximum_position_budget, reference[axis].error);
      if (reference[axis].error > 0) {
        maximum_budget_fraction = std::max(maximum_budget_fraction,
                                           discrepancy / reference[axis].error);
      }
      auto &maximum =
        plane.event == 1 ? errors.world_position : errors.position;
      maximum = std::max(maximum, discrepancy);
    }
    departure = arrival;
  }
  if (test.world_half >= 1'000'000'000LL) {
    std::cout << std::format(
      "Box oracle [{}]: max coordinate error {:.6f} pm ({:.6f} nm); "
      "max propagated bound {:.6f} pm; max error/bound {:.6f}\n",
      test.name, maximum_position_error, maximum_position_error / 1000.0,
      maximum_position_budget, maximum_budget_fraction);
  }
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest,
       RotatedBoxAnalyticPlanesAndCanonicalContinuity) {
  auto &contexts = ocl::GGEMSOpenCL::GetInstance().GetContext();
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  for (auto &context : contexts) {
    SCOPED_TRACE(context.GetDevice().GetName());
    auto const options = std::format("-I{}", root.generic_string());
    auto const &program = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "tests", "rotated_box_navigation_probe", options);
    EXPECT_TRUE(program.Matches(context, root / "tests",
                                "rotated_box_navigation_probe", options));
    EXPECT_FALSE(
      program.Matches(context, root / "tests", "rotated_box_navigation_probe",
                      options + " -DGGEMS_TEST_DIFFERENT_OPTIONS=1"));
    // Caller options contain only the include path: these are the default
    // program math options, not an explicit unsafe-math override test.
    for (auto const *flag : {
           "-cl-fast-relaxed-math",
           "-cl-unsafe-math-optimizations",
           "-cl-finite-math-only",
           "-cl-mad-enable",
           "-cl-denorms-are-zero",
         }) {
      EXPECT_EQ(program.GetBuildOptions().find(flag), std::string::npos)
        << flag;
    }
#ifndef GGEMS_DEBUG_MODE
    EXPECT_EQ(program.GetBuildOptions().find("-cl-opt-disable"),
              std::string::npos);
#endif
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
