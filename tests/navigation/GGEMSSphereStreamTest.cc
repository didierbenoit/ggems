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
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
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
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using Vec = std::array<long double, 3>;
using Point = std::array<std::int64_t, 3>;
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};

/*! \brief Cases and ownership are prescribed independently of device output. */
struct SphereCase {
  char const *name;
  std::int64_t radius;
  Point local_start;
  std::array<float, 3> direction{1, 0, 0};
  Point center{};
  std::uint32_t initial_owner{0};
  std::uint32_t event_count{3};
  bool unresolved{false};
  bool medical{true};
};

auto Cases() -> std::vector<SphereCase> {
  constexpr std::int64_t mm{1'000'000'000};
  constexpr std::int64_t cm{10 * mm};
  constexpr std::int64_t meter{1000 * mm};
  constexpr std::int64_t shift{1LL << 60};
  return {
    {.name = "rounded outward arrival",
     .radius = 1,
     .local_start = {-3, 0, 0},
     .direction = {1, 0.35F, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 3,
     .unresolved = false,
     .medical = false},
    {.name = "mm diameter", .radius = mm, .local_start = {-2 * mm, 0, 0}},
    {.name = "mm off-axis chord",
     .radius = mm,
     .local_start = {-2 * mm, mm / 2, mm / 3}},
    {.name = "negative center",
     .radius = mm,
     .local_start = {-3 * mm, mm / 4, 0},
     .direction = {1, 0, 0},
     .center = {-15 * mm, 7 * mm, -3 * mm}},
    {.name = "cm actual nonunit D",
     .radius = cm,
     .local_start = {-2 * cm, 3 * cm / 5, 0},
     .direction = {0.75F, 0, 0}},
    {.name = "meter chord",
     .radius = meter,
     .local_start = {-2 * meter, meter / 2, meter / 4},
     .direction = {0.5F, 0, 0}},
    {.name = "oblique stored D",
     .radius = 2 * cm,
     .local_start = {-4 * cm, -2 * cm, 0},
     .direction = {0.5F, 0.25F, 0.125F}},
    {.name = "float norm not one",
     .radius = mm - 1,
     .local_start = {-3 * mm, 0, 0},
     .direction = {0x1.000002p0F, 0, 0}},
    {.name = "miss by one pm",
     .radius = mm,
     .local_start = {-2 * mm, mm + 1, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 1},
    {.name = "exact tangent",
     .radius = mm,
     .local_start = {-2 * mm, mm, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 1},
    {.name = "nearby positive chord",
     .radius = mm,
     .local_start = {-2 * mm, mm - 1, 0}},
    {.name = "center birth",
     .radius = cm,
     .local_start = {0, 0, 0},
     .direction = {0.5F, 0.25F, 0.125F},
     .center = {},
     .initial_owner = 1,
     .event_count = 2},
    {.name = "surface inward",
     .radius = mm,
     .local_start = {mm, 0, 0},
     .direction = {-1, 0, 0},
     .center = {},
     .initial_owner = 1,
     .event_count = 2},
    {.name = "surface outward",
     .radius = mm,
     .local_start = {mm, 0, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 1},
    {.name = "surface tangent",
     .radius = mm,
     .local_start = {0, mm, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 1},
    {.name = "just outside",
     .radius = cm,
     .local_start = {cm + 1, 0, 0},
     .direction = {-1, 0, 0}},
    {.name = "just inside",
     .radius = cm,
     .local_start = {cm - 1, 0, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 1,
     .event_count = 2},
    {.name = "negative direction",
     .radius = mm,
     .local_start = {2 * mm, 0, 0},
     .direction = {-0.5F, 0, 0}},
    {.name = "entry rounds to same point",
     .radius = 3,
     .local_start = {3, 1, 0},
     .direction = {-1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 3,
     .unresolved = false,
     .medical = false},
    {.name = "one pm radius",
     .radius = 1,
     .local_start = {-2, 0, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 3,
     .unresolved = false,
     .medical = false},
    {.name = "odd pm radius",
     .radius = 7,
     .local_start = {-20, 2, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 3,
     .unresolved = false,
     .medical = false},
    {.name = "large common positive origin",
     .radius = mm + 3,
     .local_start = {-3 * mm, mm / 2, 0},
     .direction = {1, 0, 0},
     .center = {shift, shift, -shift}},
    {.name = "large common negative origin",
     .radius = mm,
     .local_start = {2 * mm, mm / 4, 0},
     .direction = {-0.75F, 0, 0},
     .center = {-shift, shift, -shift}},
    {.name = "small oblique surface inward",
     .radius = 5,
     .local_start = {3, 4, 0},
     .direction = {-1, 0, 0},
     .center = {},
     .initial_owner = 1,
     .event_count = 2,
     .unresolved = false,
     .medical = false},
    {.name = "medical oblique surface inward",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0},
     .direction = {-1, 0, 0},
     .initial_owner = 1,
     .event_count = 2},
    {.name = "medical oblique surface outward",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0},
     .event_count = 1},
    {.name = "medical oblique surface tangent",
     .radius = 5 * cm,
     .local_start = {3 * cm, 4 * cm, 0},
     .direction = {-4, 3, 0},
     .event_count = 1},
    {.name = "sub-pm radial outside birth",
     .radius = mm,
     .local_start = {mm, 1, 0},
     .direction = {-1, 0, 0}},
    {.name = "compensated integer profile limit",
     .radius = 1LL << 50,
     .local_start = {1LL << 50, 1, 0},
     .direction = {-1, 0, 0},
     .medical = false},
    {.name = "unresolved rounded grazing chord",
     .radius = 1,
     .local_start = {-2, 0, 0},
     .direction = {1, 0.49F, 0},
     .event_count = 1,
     .unresolved = true,
     .medical = false},
    {.name = "unresolved extreme grazing",
     .radius = 1LL << 55,
     .local_start = {-(1LL << 56), (1LL << 55) - 1, 0},
     .direction = {1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 0,
     .unresolved = true,
     .medical = false},
    {.name = "unresolved extreme radial birth",
     .radius = 1LL << 55,
     .local_start = {1LL << 55, 1, 0},
     .direction = {-1, 0, 0},
     .center = {},
     .initial_owner = 0,
     .event_count = 0,
     .unresolved = true,
     .medical = false},
  };
}

/*! \brief Geometric closest-point/chord oracle in extended host precision.
 * The implementation instead uses cross products and stable quadratic roots.
 * Inputs are integer local coordinates and the actual promoted binary32 D. */
auto Roots(Vec const &point, Vec const &direction, long double radius)
  -> std::array<long double, 2> {
  long double norm2{0};
  long double projection{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    norm2 += direction[axis] * direction[axis];
    projection += point[axis] * direction[axis];
  }
  auto const closest = -projection / norm2;
  long double impact2{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    auto const transverse = point[axis] + (closest * direction[axis]);
    impact2 += transverse * transverse;
  }
  auto const half_parameter = std::sqrt(((radius * radius) - impact2) / norm2);
  return {closest - half_parameter, closest + half_parameter};
}

struct Errors {
  Vec position{};
  long double segment{0};
  long double flight{0};
};

/*! \brief Checks ownership, integer commits and norm-aware segment evidence.
 * Budgets fixed before execution: 2 pm per coordinate and physical segment,
 * 2 pm accumulated Sphere flight for the representative cases. */
auto CheckCase(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
               random::GGEMSRandom &random, SphereCase const &test,
               std::uint32_t material, Errors &medical_errors) -> void {
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
  world.Validate();
  auto const volume = world.BuildAnalyticVolumeRecord(material);
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  Point start{};
  Vec initial{};
  Vec direction{};
  long double norm2{0};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    start[axis] = test.center[axis] + test.local_start[axis];
    initial[axis] = static_cast<long double>(test.local_start[axis]);
    direction[axis] = test.direction[axis];
    norm2 += direction[axis] * direction[axis];
  }
  source.position_x_pm = start[0];
  source.position_y_pm = start[1];
  source.position_z_pm = start[2];
  source.axis_z_x = test.direction[0];
  source.axis_z_y = test.direction[1];
  source.axis_z_z = test.direction[2];
  source.energy_micro_eV = 123456789;
  source.time_start_ps = 17;
  source.time_stop_ps = 17;
  auto const result = test::RunStream(context, program, random,
                                      world.BuildRecord(), volume, source);
  auto const &counts = result.counts;
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(counts.overflow_count, 0U);
  EXPECT_EQ(counts.outside_world_count, 0U);
  ASSERT_EQ(result.trace[0], test.event_count);
  EXPECT_EQ(counts.unresolved_geometry_count, test.unresolved ? 1U : 0U);
  EXPECT_EQ(counts.escaped_world_count, test.unresolved ? 0U : 1U);
  if (test.unresolved) {
    EXPECT_EQ(counts.completed_history_count, 0U);
    if (test.event_count == 1) {
      EXPECT_EQ(result.trace[1], 0); // Accepted entry precedes the failed exit.
      EXPECT_EQ(result.trace[2], 1);
      EXPECT_EQ(result.trace[7], 2);
      EXPECT_EQ(result.trace[8], material);
      EXPECT_EQ((Point{result.trace[4], result.trace[5], result.trace[6]}),
                (Point{-1, 1, 0}));
    }
    return;
  }
  EXPECT_EQ(counts.completed_history_count, 1U);
  EXPECT_EQ(counts.terminal_particle_count, 1U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_x),
            std::bit_cast<std::uint32_t>(source.axis_z_x));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_y),
            std::bit_cast<std::uint32_t>(source.axis_z_y));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_z),
            std::bit_cast<std::uint32_t>(source.axis_z_z));
  auto const ideal = test.event_count > 1
                       ? Roots(initial, direction, test.radius)
                       : std::array<long double, 2>{};
  Vec anchor = initial;
  long double flight{0};
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
    Point endpoint{result.trace[offset + 3], result.trace[offset + 4],
                   result.trace[offset + 5]};
    if (!world_exit) {
      EXPECT_EQ(result.trace[offset + 2],
                0); // A curved surface has no planar face.
      auto const roots = Roots(anchor, direction, test.radius);
      auto const expected = roots[owner == 0 ? 0 : 1];
      auto const measured = std::bit_cast<double>(result.trace[offset + 8]);
      if (std::string{test.name} == "rounded outward arrival" && index == 1) {
        EXPECT_EQ(measured, 0.0);
        EXPECT_EQ(endpoint, (Point{0, 1, 0}));
      }
      auto const segment_error =
        std::abs((static_cast<long double>(measured) - expected) * norm);
      flight += measured * norm;
      auto const ideal_parameter = ideal[owner == 0 ? 0 : 1];
      auto const flight_error = std::abs(flight - (ideal_parameter * norm));
      EXPECT_LE(segment_error, 2.0L);
      EXPECT_LE(flight_error, 2.0L);
      for (std::size_t axis = 0; axis < 3; ++axis) {
        auto const local =
          static_cast<long double>(endpoint[axis] - test.center[axis]);
        auto const error = std::abs(
          local - (initial[axis] + (ideal_parameter * direction[axis])));
        EXPECT_LE(error, 2.0L);
        if (test.medical) {
          medical_errors.position[axis] =
            std::max(medical_errors.position[axis], error);
        }
        anchor[axis] = local;
      }
      if (test.medical) {
        medical_errors.segment =
          std::max(medical_errors.segment, segment_error);
        medical_errors.flight = std::max(medical_errors.flight, flight_error);
      }
      if (std::string{test.name} == "entry rounds to same point" &&
          index == 0) {
        EXPECT_EQ(endpoint, start);
        EXPECT_GT(measured, 0);
      }
    } else {
      // Independent exact World face condition; the unchanged integer World
      // fixture suite checks every endpoint component, ties and masks.
      bool on_outward_face{false};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        EXPECT_LE(std::abs(endpoint[axis]), half[axis]);
        on_outward_face |=
          (direction[axis] > 0 && endpoint[axis] == half[axis]) ||
          (direction[axis] < 0 && endpoint[axis] == -half[axis]);
      }
      EXPECT_TRUE(on_outward_face);
      EXPECT_NE(result.trace[offset + 2], 0);
    }
    owner = arrival;
  }
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest, SphereProductionWorkerGeometryAndContinuity) {
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
}
)cl";
  bool exercised_fp64{false};
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    if (context.GetDevice().GetDoubleFpConfig() == 0U) {
      continue;
    }
    exercised_fp64 = true;
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231);
    auto const options =
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1 "
                  "-DGGEMS_ENABLE_FP64_NAVIGATION=1",
                  root.generic_string(), random.GetKernelBuildDefinition());
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport", options);
    EXPECT_EQ(production.GetBuildOptions().find("-cl-fast-relaxed-math"),
              std::string::npos);
    cl::Program program{context.GetContextNative(), source};
    auto const device = context.GetDevice().GetDeviceNative();
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    std::array<std::uint64_t, 10> layout{};
    auto layout_buffer = context.CreateSVMBuffer(units::Bytes{sizeof(layout)});
    ocl::GGEMSOpenCLKernel layout_kernel{
      context, cl::Kernel(program, "analytic_layout"), "analytic_layout"};
    layout_kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
    layout_kernel.Run({1U}, {1U});
    ocl::ReadSVMToHost(layout_buffer, std::span{layout});
    EXPECT_EQ(layout, (std::array<std::uint64_t, 10>{64, 8, 40, 0, 8, 16, 24,
                                                     32, 36, 112}));
    Errors errors;
    for (auto const &test : Cases()) {
      CheckCase(context, program, random, test, 7U, errors);
    }
    CheckCase(context, program, random, Cases()[1], 0U, errors);
    std::cout << std::format(
      "Sphere: {} worker histories, 3 expected UNRESOLVED; medical max XYZ "
      "({:.6f}, {:.6f}, {:.6f}) pm, segment {:.6f} pm, accumulated Sphere "
      "flight {:.6f} pm\nOptions: {}\n",
      Cases().size() + 1, errors.position[0], errors.position[1],
      errors.position[2], errors.segment, errors.flight,
      production.GetBuildOptions());
  }
  if (!exercised_fp64) {
    GTEST_SKIP() << "No selected device supports FP64 Sphere navigation.";
  }
}

TEST_F(GGEMSNavigationDeviceTest,
       RunOwnsSphereSnapshotAndDispatchesItsPredicate) {
  using namespace ggems::units;
  bool requires_rejection{false};
  for (auto const &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    requires_rejection |= context.GetDevice().GetDoubleFpConfig() == 0U;
  }
  for (bool const ambiguous_birth : {false, true}) {
    auto source = std::make_shared<sources::GGEMSSource>();
    source->SetAnalytic()
      .SetPrimaryCount(1)
      .SetEmittedParticleType(particles::GGEMSParticleType::Aionino)
      .SetPositionPicoMeter(ambiguous_birth ? -2 : -2'000'000'000, 0, 0)
      .SetDirection(1, ambiguous_birth ? 0.49 : 0, 0);
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
      world.SetSize(20_mm, 20_mm, 20_mm)
        .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
      geometry::GGEMSSphere sphere;
      sphere.SetRadius(ambiguous_birth ? 1_pm : 1_mm)
        .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
      world.AddVolume(sphere);
      run.SetWorld(world);
      sphere.SetRadius(2_mm);
      world.SetSize(40_mm, 40_mm, 40_mm);
    }
    if (requires_rejection) {
      // Run must reject the complete selected device set if any device lacks
      // FP64; it must not silently execute only the compatible devices.
      EXPECT_THROW(run.Initialize(), GGEMSRecoverable);
      EXPECT_TRUE(observer->GetRecords().empty());
      return;
    }
    ASSERT_NO_THROW(run.Initialize());
    if (ambiguous_birth) {
      // A World-only dispatch would silently succeed: this proves Run uploaded
      // and selected the Sphere predicate, including its bounded diagnostic.
      EXPECT_THROW(run.Run(), GGEMSRecoverable);
    } else {
      ASSERT_NO_THROW(run.Run());
      auto const records = observer->GetRecords();
      ASSERT_EQ(records.size(), 2U);
      EXPECT_EQ(records.back().position_x_pm, 10'000'000'000);
    }
  }
}
