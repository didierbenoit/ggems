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
 * \brief Qualifies Sphere through the common production Aionino worker.
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
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSFP32TestOracle.hh"
#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
namespace fp = ggems::test::fp32;
using Vec = std::array<double, 3>;
using Point = std::array<std::int64_t, 3>;
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};
constexpr std::int64_t k_mm{1'000'000'000};
constexpr std::int64_t k_cm{10 * k_mm};

/*! \brief Expected owners and event counts are independent of device output. */
struct SphereCase {
  char const *name;
  std::int64_t radius;
  Point local_start;
  std::array<float, 3> direction{1, 0, 0};
  Point center{};
  std::uint32_t initial_owner{0};
  std::uint32_t event_count{3};
  bool predicate_witness{false};
};

auto OrdinaryCases() -> std::vector<SphereCase> {
  return {
    {
      .name = "one mm radius axial chord",
      .radius = k_mm,
      .local_start = {-2 * k_mm, 0, 0},
    },
    {
      .name = "mm off-axis chord",
      .radius = k_mm,
      .local_start = {-2 * k_mm, k_mm / 2, k_mm / 3},
    },
    {
      .name = "negative center",
      .radius = k_mm,
      .local_start = {-3 * k_mm, k_mm / 4, 0},
      .direction = {1, 0, 0},
      .center = {-15 * k_mm, 7 * k_mm, -3 * k_mm},
    },
    {
      .name = "cm actual nonunit D",
      .radius = k_cm,
      .local_start = {-2 * k_cm, 3 * k_cm / 5, 0},
      .direction = {0.75F, 0, 0},
    },
    // A 375 mm radius fits a nominal 3 m centered World (full width 8R).
    {
      .name = "375 mm chord in three meter World",
      .radius = 375 * k_mm,
      .local_start = {-750 * k_mm, 375 * k_mm / 2, 375 * k_mm / 4},
      .direction = {0.5F, 0, 0},
    },
    {
      .name = "oblique stored D",
      .radius = 2 * k_cm,
      .local_start = {-4 * k_cm, -2 * k_cm, 0},
      .direction = {0.5F, 0.25F, 0.125F},
    },
    {
      .name = "float norm not one",
      .radius = k_mm - 1,
      .local_start = {-3 * k_mm, 0, 0},
      .direction = {0x1.000002p0F, 0, 0},
    },
    {
      .name = "separated miss",
      .radius = k_mm,
      .local_start = {-2 * k_mm, 5 * k_mm / 4, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
    {
      .name = "resolved positive chord",
      .radius = k_mm,
      .local_start = {-2 * k_mm, 3 * k_mm / 4, 0},
    },
    {
      .name = "center birth",
      .radius = k_cm,
      .local_start = {0, 0, 0},
      .direction = {0.5F, 0.25F, 0.125F},
      .center = {},
      .initial_owner = 1,
      .event_count = 2,
    },
    {
      .name = "separated outside birth",
      .radius = k_cm,
      .local_start = {5 * k_cm / 4, 0, 0},
      .direction = {-1, 0, 0},
    },
    {
      .name = "separated inside birth",
      .radius = k_cm,
      .local_start = {3 * k_cm / 4, 0, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 1,
      .event_count = 2,
    },
    {
      .name = "negative direction",
      .radius = k_mm,
      .local_start = {2 * k_mm, 0, 0},
      .direction = {-0.5F, 0, 0},
    },
    {
      .name = "medical oblique interior birth",
      .radius = 5 * k_cm,
      .local_start = {15 * k_mm, 20 * k_mm, 0},
      .direction = {-1, 0, 0},
      .center = {},
      .initial_owner = 1,
      .event_count = 2,
    },
    {
      .name = "medical separated outward miss",
      .radius = 5 * k_cm,
      .local_start = {6 * k_cm, 8 * k_cm, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
    {
      .name = "medical separated transverse miss",
      .radius = 5 * k_cm,
      .local_start = {6 * k_cm, 8 * k_cm, 0},
      .direction = {-4, 3, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
  };
}

/*! \brief Independent closest-point/chord reference in binary64.
 * All inputs are promoted from actual uploaded FP32 records. This geometric
 * construction describes the nominal continuum interval only for separated
 * crossings; the surface-band topology has its own discrete tests.
 * Windows long double supplies no additional precision and is not used. */
auto Roots(Vec const &point, Vec const &direction, double radius)
  -> std::array<double, 2> {
  double norm2{0};
  double projection{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    norm2 += direction[axis] * direction[axis];
    projection += point[axis] * direction[axis];
  }
  auto const closest = -projection / norm2;
  double impact2{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const transverse = point[axis] + (closest * direction[axis]);
    impact2 += transverse * transverse;
  }
  auto const half_parameter = std::sqrt(((radius * radius) - impact2) / norm2);
  return {closest - half_parameter, closest + half_parameter};
}

/*! \brief Propagates operand-specific rounding through the implemented stages.
 * This supplies an error envelope, never the geometric expected value.
 * Three products/two additions per dot, center subtraction, closest-approach
 * projection, shared reciprocal, radius square, half-chord scaling, sqrt and
 * root sum are
 * counted. Shared Scalar operations admit standard OpenCL divide/sqrt accuracy
 * and separate operations or contraction. Conditioning enters at sqrt/division;
 * an infinite/invalid envelope is rejected, not converted to a passing test. */
auto RootBudget(std::array<fp::Scalar, 3> const &anchor, Vec const &center,
                double radius, Vec const &direction, bool exit) -> fp::Scalar {
  fp::Scalar a{};
  fp::Scalar b{};
  std::array<fp::Scalar, 3> offset{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const q =
      fp::Subtract(anchor[axis], {.value = center[axis], .error = 0});
    fp::Scalar const d{.value = direction[axis], .error = 0};
    offset[axis] = q;
    a = axis == 0 ? fp::Multiply(d, d) : fp::Add(a, fp::Multiply(d, d));
    b = axis == 0 ? fp::Multiply(q, d) : fp::Add(b, fp::Multiply(q, d));
  }
  auto const inverse_a = fp::Divide({.value = 1.0, .error = 0.0}, a);
  auto const projection =
    fp::Multiply({.value = -b.value, .error = b.error}, inverse_a);
  fp::Scalar closest_squared{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const closest =
      fp::Add(offset[axis],
              fp::Multiply(projection, {.value = direction[axis], .error = 0}));
    auto const squared = fp::Multiply(closest, closest);
    closest_squared = axis == 0 ? squared : fp::Add(closest_squared, squared);
  }
  auto const half_chord_squared =
    fp::Multiply(fp::Subtract(fp::Multiply({.value = radius, .error = 0},
                                           {.value = radius, .error = 0}),
                              closest_squared),
                 inverse_a);
  auto const half_chord = fp::Sqrt(half_chord_squared);
  return exit ? fp::Add(projection, half_chord)
              : fp::Subtract(projection, half_chord);
}

struct Errors {
  double position{0};
  double segment{0};
  double flight{0};
  double position_budget{0};
  double segment_budget{0};
  double flight_budget{0};
};

auto CheckCase(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
               random::GGEMSRandom &random, SphereCase const &test,
               std::uint32_t material, Errors &errors) -> void {
  SCOPED_TRACE(test.name);
  geometry::GGEMSWorld world;
  Point half{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    half[axis] = std::abs(test.center[axis]) + (4 * test.radius) + 100;
  }
  auto const vacuum = materials::builtins::BuildBuiltInMaterial("Vacuum");
  world
    .SetSize(units::Length{static_cast<std::uint64_t>(2 * half[0])},
             units::Length{static_cast<std::uint64_t>(2 * half[1])},
             units::Length{static_cast<std::uint64_t>(2 * half[2])})
    .SetMaterial(vacuum);
  geometry::GGEMSSphere sphere;
  sphere.SetRadius(units::Length{static_cast<std::uint64_t>(test.radius)})
    .SetPosition(
      geometry::MakePositionPM(test.center[0], test.center[1], test.center[2]))
    .SetMaterial(material == 0
                   ? vacuum
                   : materials::builtins::BuildBuiltInMaterial("Water"));
  world.AddVolume(sphere);
  // The World owns its Sphere copy, including the shape parameters.
  sphere.SetRadius(units::Length{static_cast<std::uint64_t>(2 * test.radius)});
  world.Validate();
  auto const volume = world.BuildAnalyticVolumeRecord(material);
  auto const world_record = world.BuildRecord();
  auto const &packed_sphere = volume.parameters.sphere;
  EXPECT_EQ(volume.kind, geometry::GGEMSAnalyticKind::Sphere);
  EXPECT_EQ(packed_sphere.radius_pm, static_cast<float>(test.radius));
  Vec const center{
    packed_sphere.center_x_pm,
    packed_sphere.center_y_pm,
    packed_sphere.center_z_pm,
  };
  double const radius = packed_sphere.radius_pm;
  Vec const extents{
    world_record.half_extent_x_pm,
    world_record.half_extent_y_pm,
    world_record.half_extent_z_pm,
  };
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = test.center[0] + test.local_start[0];
  source.position_y_pm = test.center[1] + test.local_start[1];
  source.position_z_pm = test.center[2] + test.local_start[2];
  source.axis_z_x = test.direction[0];
  source.axis_z_y = test.direction[1];
  source.axis_z_z = test.direction[2];
  source.energy_micro_eV = 123456789;
  source.time_start_ps = 17;
  source.time_stop_ps = 17;
  auto const packed_source = sources::BuildSourceDeviceRecord(source);
  Vec const start{
    packed_source.position_x_pm,
    packed_source.position_y_pm,
    packed_source.position_z_pm,
  };
  Vec const direction{
    packed_source.axis_z_x,
    packed_source.axis_z_y,
    packed_source.axis_z_z,
  };
  Vec initial{};
  double norm2{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    initial[axis] = start[axis] - center[axis];
    norm2 += direction[axis] * direction[axis];
  }
  auto const result =
    test::RunStream(context, program, random, world_record, volume, source);
  auto const coordinate = [&](std::size_t index) -> double {
    return std::bit_cast<float>(
      static_cast<std::uint32_t>(result.trace[index]));
  };
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.observer_counts.record_count, 2U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  EXPECT_EQ(result.trace[test::k_initial_owner_index], test.initial_owner);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U],
            test.initial_owner == 0U ? 0U : material);
  EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
  // Print actual topology before a fatal count mismatch; other test cases and
  // device contexts still run because this assertion belongs to this helper.
  std::cout << std::format("Sphere case '{}' on {}: events {}, completed {}, "
                           "unresolved {}\n",
                           test.name, context.GetDevice().GetName(),
                           result.trace[0],
                           result.counts.completed_history_count,
                           result.counts.unresolved_geometry_count);
  if (std::cmp_not_equal(result.trace[0], test.event_count)) {
    for (std::size_t index = 0;
         index <
         std::min<std::size_t>(3, static_cast<std::size_t>(result.trace[0]));
         ++index) {
      auto const offset = 1 + (test::k_trace_stride * index);
      std::cout << std::format(
        "  boundary {}: owner {} -> {}, face {}, event {}, material {}, "
        "point ({:.6f}, {:.6f}, {:.6f}) pm, parameter {:.6f} pm\n",
        index, result.trace[offset], result.trace[offset + 1],
        result.trace[offset + 2], result.trace[offset + 6],
        result.trace[offset + 7], coordinate(offset + 3),
        coordinate(offset + 4), coordinate(offset + 5), coordinate(offset + 8));
    }
  }
  ASSERT_EQ(result.trace[0], test.event_count);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  EXPECT_EQ(result.counts.terminal_particle_count, 0U); // Derived by Workload.
  auto const &terminal = result.records[1];
  EXPECT_EQ(result.records[0].record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Source));
  EXPECT_EQ(terminal.record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Terminal));
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_EQ(terminal.particle_type, source.emitted_particle_type);
  EXPECT_EQ(terminal.global_primary_id, result.records[0].global_primary_id);
  EXPECT_EQ(terminal.source_local_primary_id,
            result.records[0].source_local_primary_id);
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_x),
            std::bit_cast<std::uint32_t>(source.axis_z_x));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_y),
            std::bit_cast<std::uint32_t>(source.axis_z_y));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_z),
            std::bit_cast<std::uint32_t>(source.axis_z_z));
  EXPECT_EQ(result.records[0].position_valid, 1U);
  EXPECT_EQ(result.records[0].position_x_pm, std::llround(start[0]));
  EXPECT_EQ(result.records[0].position_y_pm, std::llround(start[1]));
  EXPECT_EQ(result.records[0].position_z_pm, std::llround(start[2]));
  auto const ideal = test.event_count > 1 ? Roots(initial, direction, radius)
                                          : std::array<double, 2>{};
  Vec anchor = start;
  std::array<fp::Scalar, 3> propagated_anchor{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    propagated_anchor[axis] = {.value = start[axis], .error = 0};
  }
  double flight{0};
  double previous_parameter{0};
  double accumulated_budget{0};
  auto const norm = std::sqrt(norm2);
  auto owner = test.initial_owner;
  for (std::size_t index = 0; index < test.event_count; ++index) {
    auto const offset = 1 + (test::k_trace_stride * index);
    bool const world_exit = index + 1 == test.event_count;
    auto const opposite_owner = owner == 0 ? 1U : 0U;
    auto const arrival = world_exit ? k_exterior : opposite_owner;
    EXPECT_EQ(result.trace[offset], owner);
    EXPECT_EQ(result.trace[offset + 1], arrival);
    EXPECT_EQ(result.trace[offset + 6],
              world_exit ? 1U : (arrival == 1 ? 2U : 3U));
    EXPECT_EQ(result.trace[offset + 7],
              world_exit ? k_exterior : (arrival == 1 ? material : 0U));
    Vec const endpoint{
      coordinate(offset + 3),
      coordinate(offset + 4),
      coordinate(offset + 5),
    };
    auto const measured = coordinate(offset + 8);
    ASSERT_TRUE(std::isfinite(measured));
    fp::Scalar parameter{};
    fp::Scalar propagated_parameter{};
    double expected{0};
    double ideal_parameter{0};
    if (!world_exit) {
      EXPECT_EQ(result.trace[offset + 2], 0); // Sphere has no planar face.
      Vec local{};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        local[axis] = anchor[axis] - center[axis];
      }
      expected = Roots(local, direction, radius)[owner == 0 ? 0 : 1];
      ideal_parameter = ideal[owner == 0 ? 0 : 1];
      std::array<fp::Scalar, 3> exact_anchor{};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        exact_anchor[axis] = {.value = anchor[axis], .error = 0};
      }
      parameter =
        RootBudget(exact_anchor, center, radius, direction, owner != 0);
      propagated_parameter =
        RootBudget(propagated_anchor, center, radius, direction, owner != 0);
    } else {
      expected = std::numeric_limits<double>::infinity();
      ideal_parameter = expected;
      std::uint32_t faces{0};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        if (direction[axis] == 0) {
          continue;
        }
        auto const plane = std::copysign(extents[axis], direction[axis]);
        auto const candidate = (plane - anchor[axis]) / direction[axis];
        ideal_parameter =
          std::min(ideal_parameter, (plane - start[axis]) / direction[axis]);
        auto const face = 1U << ((2U * axis) + (direction[axis] > 0 ? 1U : 0U));
        if (candidate < expected) {
          expected = candidate;
          faces = face;
          parameter =
            fp::Divide(fp::Subtract({.value = plane, .error = 0},
                                    {.value = anchor[axis], .error = 0}),
                       {.value = direction[axis], .error = 0});
          propagated_parameter = fp::Divide(
            fp::Subtract({.value = plane, .error = 0}, propagated_anchor[axis]),
            {.value = direction[axis], .error = 0});
        } else if (candidate == expected) {
          faces |= face;
        }
      }
      EXPECT_EQ(result.trace[offset + 2], faces);
    }
    ASSERT_TRUE(std::isfinite(expected));
    ASSERT_TRUE(std::isfinite(parameter.error));
    ASSERT_GE(parameter.error, 0);
    // The expected parameter is independently geometric. Difference between
    // the two binary64 constructions is included only as host evaluation error.
    parameter.error += std::abs(parameter.value - expected);
    parameter.value = expected;
    fp::ExpectWithin(measured, parameter);
    double const ideal_step = ideal_parameter - previous_parameter;
    propagated_parameter.error +=
      std::abs(propagated_parameter.value - ideal_step);
    propagated_parameter.value = ideal_step;
    ASSERT_TRUE(std::isfinite(propagated_parameter.error));
    fp::ExpectWithin(measured, propagated_parameter);
    auto const segment_error = std::abs((measured - expected) * norm);
    flight += measured * norm;
    // Propagate uncertainty from the uploaded inputs through every accepted
    // movement. No measured discrepancy enters this accumulated path budget.
    accumulated_budget += propagated_parameter.error * norm;
    auto const flight_budget = accumulated_budget;
    EXPECT_LE(std::abs(flight - ideal_parameter * norm), flight_budget);
    errors.segment = std::max(errors.segment, segment_error);
    errors.segment_budget =
      std::max(errors.segment_budget, parameter.error * norm);
    errors.flight =
      std::max(errors.flight, std::abs(flight - (ideal_parameter * norm)));
    errors.flight_budget = std::max(errors.flight_budget, flight_budget);
    for (std::size_t axis = 0; axis < 3; ++axis) {
      auto const moved =
        fp::Add(propagated_anchor[axis],
                fp::Multiply(propagated_parameter,
                             {.value = direction[axis], .error = 0}));
      auto const reference = start[axis] + (ideal_parameter * direction[axis]);
      auto const budget = moved.error + std::abs(moved.value - reference);
      ASSERT_TRUE(std::isfinite(budget));
      ASSERT_TRUE(std::isfinite(endpoint[axis]));
      EXPECT_LE(std::abs(endpoint[axis] - reference), budget);
      errors.position =
        std::max(errors.position, std::abs(endpoint[axis] - reference));
      errors.position_budget = std::max(errors.position_budget, budget);
      propagated_anchor[axis] = {.value = reference, .error = budget};
    }
    if (test.predicate_witness && !world_exit) {
      // A bounding Box crosses at +/-R; a Sphere crosses at +/-sqrt(7)R/4.
      // This macroscopic separation rejects World-only and Box dispatch.
      EXPECT_GT(std::abs(endpoint[0] - center[0]), radius / 2);
      EXPECT_LT(std::abs(endpoint[0] - center[0]), 3 * radius / 4);
    }
    anchor = endpoint;
    previous_parameter = ideal_parameter;
    owner = arrival;
  }
  // Observer conversion is tested at its own boundary, separate from geometry.
  EXPECT_EQ(terminal.position_valid, 1U);
  EXPECT_EQ(terminal.position_x_pm, std::llround(anchor[0]));
  EXPECT_EQ(terminal.position_y_pm, std::llround(anchor[1]));
  EXPECT_EQ(terminal.position_z_pm, std::llround(anchor[2]));
}

/*! \brief Compiles only test-owned capture around the unchanged real worker. */
auto RunCases(std::vector<SphereCase> const &cases, bool same_material = false)
  -> void {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const source = test::InstrumentStream(root) + R"cl(
__kernel void analytic_layout(__global ulong *layout) {
  layout[0] = sizeof(GGEMSAnalyticVolumeRecord);
  layout[1] = __builtin_offsetof(GGEMSAnalyticVolumeRecord, parameters);
  layout[2] = sizeof(GGEMSSphereRecord);
  layout[3] = __builtin_offsetof(GGEMSSphereRecord, center_x_pm);
  layout[4] = __builtin_offsetof(GGEMSSphereRecord, center_y_pm);
  layout[5] = __builtin_offsetof(GGEMSSphereRecord, center_z_pm);
  layout[6] = __builtin_offsetof(GGEMSSphereRecord, radius_pm);
  layout[7] = __builtin_offsetof(GGEMSSphereRecord, volume_id);
  layout[8] = __builtin_offsetof(GGEMSSphereRecord, material_id);
  layout[9] = sizeof(GGEMSParticleState);
  layout[10] = __builtin_offsetof(GGEMSSphereRecord, tolerance_pm);
  layout[11] = sizeof(GGEMSWorldRecord);
  layout[12] = __builtin_offsetof(GGEMSWorldRecord, tolerance_pm);
  layout[13] = sizeof(GGEMSBoxRecord);
  layout[14] = __builtin_offsetof(GGEMSBoxRecord, tolerance_pm);
}
)cl";
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231);
    auto const options =
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                  root.generic_string(), random.GetKernelBuildDefinition());
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport", options);
    for (auto const *flag : {
           "-cl-fast-relaxed-math",
           "-cl-unsafe-math-optimizations",
           "-cl-finite-math-only",
           "-cl-mad-enable",
           "-cl-denorms-are-zero",
         }) {
      EXPECT_EQ(production.GetBuildOptions().find(flag), std::string::npos)
        << flag;
    }
#ifndef GGEMS_DEBUG_MODE
    EXPECT_EQ(production.GetBuildOptions().find("-cl-opt-disable"),
              std::string::npos);
    EXPECT_EQ(production.GetBuildOptions().find("-g"), std::string::npos);
#endif
    cl::Program program{context.GetContextNative(), source};
    auto const device = context.GetDevice().GetDeviceNative();
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    std::array<std::uint64_t, 15> layout{};
    auto layout_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    ocl::GGEMSOpenCLKernel layout_kernel{
      context, cl::Kernel(program, "analytic_layout"), "analytic_layout"};
    layout_kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
    layout_kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(layout_buffer, std::span{layout});
    EXPECT_EQ(layout,
              (std::array<std::uint64_t, 15>{84, 4, 28, 0, 4, 8, 12, 16, 20, 80,
                                             24, 28, 12, 24, 20}));
    Errors errors;
    auto const run_case = [&](SphereCase const &test,
                              std::uint32_t material) -> void {
      Errors current;
      CheckCase(context, program, random, test, material, current);
      std::cout << std::format(
        "Sphere budget '{}': max error/budget position {:.6f}/{:.6f} pm, "
        "segment {:.6f}/{:.6f} pm, flight {:.6f}/{:.6f} pm\n",
        test.name, current.position, current.position_budget, current.segment,
        current.segment_budget, current.flight, current.flight_budget);
      errors.position = std::max(errors.position, current.position);
      errors.segment = std::max(errors.segment, current.segment);
      errors.flight = std::max(errors.flight, current.flight);
      errors.position_budget =
        std::max(errors.position_budget, current.position_budget);
      errors.segment_budget =
        std::max(errors.segment_budget, current.segment_budget);
      errors.flight_budget =
        std::max(errors.flight_budget, current.flight_budget);
    };
    for (auto const &test : cases) {
      run_case(test, 7U);
    }
    if (same_material) {
      run_case(cases.front(), 0U);
    }
    std::cout << std::format(
      "Sphere {}: {} histories; maximum error/budget position {:.6f}/{:.6f} "
      "pm, "
      "segment {:.6f}/{:.6f} pm, accumulated flight {:.6f}/{:.6f} pm\nOptions: "
      "{}\n",
      context.GetDevice().GetName(), cases.size() + (same_material ? 1U : 0U),
      errors.position, errors.position_budget, errors.segment,
      errors.segment_budget, errors.flight, errors.flight_budget,
      production.GetBuildOptions());
  }
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest, SphereProductionWorkerGeometryAndContinuity) {
  RunCases(OrdinaryCases(), true);
}

// The configured surface band treats tangency as touch, with no Sphere owner
// or physical material interval. These discrete lifecycle assertions remain
// exact even though sub-tolerance continuum geometry is intentionally
// unresolved.
TEST_F(GGEMSNavigationDeviceTest, SphereToleranceTangentRemainsInWorld) {
  RunCases({
    {
      .name = "exact tangent",
      .radius = k_mm,
      .local_start = {-2 * k_mm, k_mm, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest, SphereToleranceSurfaceOutwardRemainsInWorld) {
  RunCases({
    {
      .name = "surface outward",
      .radius = k_mm,
      .local_start = {k_mm, 0, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest, SphereToleranceSurfaceTangentRemainsInWorld) {
  RunCases({
    {
      .name = "surface tangent",
      .radius = k_mm,
      .local_start = {0, k_mm, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 1,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest, SphereToleranceSurfaceInwardUsesSphere) {
  RunCases({
    {
      .name = "surface inward",
      .radius = k_mm,
      .local_start = {k_mm, 0, 0},
      .direction = {-1, 0, 0},
      .center = {},
      .initial_owner = 1,
      .event_count = 2,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest,
       SpherePredicateDispatchHasDistinctOffAxisChord) {
  RunCases({
    {
      .name = "ten mm predicate witness",
      .radius = k_cm,
      .local_start = {-2 * k_cm, 3 * k_cm / 4, 0},
      .direction = {1, 0, 0},
      .center = {},
      .initial_owner = 0,
      .event_count = 3,
      .predicate_witness = true,
    },
  });
}

TEST_F(GGEMSNavigationDeviceTest,
       RunOwnsSphereSnapshotAndDispatchesItsPredicate) {
  using namespace ggems::units;
  auto source = std::make_shared<sources::GGEMSSource>();
  source->SetAnalytic()
    .SetPrimaryCount(1)
    .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
    .SetPositionPicoMeter(-20 * k_mm, 15 * k_mm / 2, 0)
    .SetDirection(1, 0, 0);
  auto random = std::make_shared<random::GGEMSRandom>();
  random->SetSeed(3);
  auto observer = std::make_shared<observer::GGEMSTransportObserver>();
  observer->Enable().SetRecordCapacity(2).CaptureFirstPrimaries(1);
  GGEMSRun run;
  run.SetRandom(random);
  run.AddSource(source);
  run.SetObserver(observer);
  {
    geometry::GGEMSWorld world;
    world.SetSize(80_mm, 80_mm, 80_mm)
      .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
    geometry::GGEMSSphere sphere;
    sphere.SetRadius(10_mm).SetMaterial(
      materials::builtins::BuildBuiltInMaterial("Water"));
    world.AddVolume(sphere);
    run.SetWorld(world);
    sphere.SetRadius(2_mm);
    world.SetSize(160_mm, 160_mm, 160_mm);
  }
  ASSERT_NO_THROW(run.Initialize());
  ASSERT_NO_THROW(run.Run());
  auto const records = observer->GetRecords();
  ASSERT_EQ(records.size(), 2U);
  EXPECT_EQ(records[0].record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Source));
  EXPECT_EQ(records[1].record_kind,
            observer::ToKernelObserverRecordKind(
              observer::GGEMSObserverRecordKind::Terminal));
  EXPECT_EQ(records[1].status, particles::ToKernelParticleStatus(
                                 particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(records[0].global_primary_id, records[1].global_primary_id);
  // The retained public Run test proves World snapshot lifetime and ordinary
  // transport. Public source/terminal records cannot reveal the intermediate
  // Sphere chord; the companion instrumented production-worker test above
  // explicitly proves its distinct chord and owner/material dispatch.
  double const packed_face = static_cast<float>(40 * k_mm);
  fp::ExpectWithin(
    static_cast<double>(records[1].position_x_pm),
    {.value = packed_face, .error = fp::RoundoffBound(packed_face, 3)}, 0.5);
  constexpr std::int64_t transverse_pm{7'500'000'000};
  EXPECT_EQ(records[1].position_y_pm,
            std::llround(static_cast<float>(transverse_pm)));
  EXPECT_EQ(records[1].position_z_pm, 0);
}
