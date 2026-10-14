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
 * \brief Ordered owner transitions of Aionino histories through the
 * production stream kernel, traced in a test-side copy of the kernel source.
 *
 * The production source is loaded, a trace argument is appended and one
 * trace write is inserted right after the production Transport transaction;
 * no production hook, second navigator or Observer extension is involved. The
 * frozen fixtures were generated independently with Python Fraction (see the
 * fixture header).
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <numbers>
#include <optional>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSBoxFixtures.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {

using namespace ggems;
using namespace ggems::core;

using ggems::test::InstrumentStream;
using ggems::test::k_trace_stride;
using ggems::test::k_trace_values;
constexpr std::uint32_t k_world{0U};
constexpr std::uint32_t k_box{1U};
constexpr std::uint32_t k_exterior{0xFFFFFFFFU};
constexpr std::uint32_t k_event_world_exit{1U};
constexpr std::uint32_t k_event_volume_entry{2U};
constexpr std::uint32_t k_event_volume_exit{3U};

/*! \brief Event implied by an owner transition. */
auto ExpectedEvent(BoxEvent const &event) -> std::uint32_t {
  if (event.arrival == k_exterior) {
    return k_event_world_exit;
  }
  return event.arrival == k_box ? k_event_volume_entry : k_event_volume_exit;
}

/*! \brief Arrival Material implied by an owner and the Box Material. */
auto ExpectedMaterial(std::uint32_t arrival, std::uint32_t box_material)
  -> std::int64_t {
  if (arrival == k_exterior) {
    return static_cast<std::int64_t>(k_exterior);
  }
  return arrival == k_box ? box_material : 0LL;
}

/*!
 * \brief Runs one frozen history through the traced stream and checks the
 * ordered transitions, the untouched RNG, the counters and the terminal record.
 */
auto RunTracedHistory(ocl::GGEMSOpenCLContext &context,
                      cl::Program const &program, random::GGEMSRandom &random,
                      BoxCase const &test, std::uint32_t box_material,
                      geometry::GGEMSBoxPoseRecord const *pose = nullptr)
  -> void {
  geometry::GGEMSWorldRecord const world{.half_extent_x_pm = test.half[0],
                                         .half_extent_y_pm = test.half[1],
                                         .half_extent_z_pm = test.half[2]};
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = test.start[0];
  source.position_y_pm = test.start[1];
  source.position_z_pm = test.start[2];
  source.axis_z_x = test.direction[0];
  source.axis_z_y = test.direction[1];
  source.axis_z_z = test.direction[2];
  source.energy_micro_eV = 123456789ULL;
  source.time_start_ps = 17ULL;
  source.time_stop_ps = 17ULL;
  geometry::GGEMSBoxRecord const box{.lower_x_pm = test.lower[0],
                                     .lower_y_pm = test.lower[1],
                                     .lower_z_pm = test.lower[2],
                                     .upper_x_pm = test.upper[0],
                                     .upper_y_pm = test.upper[1],
                                     .upper_z_pm = test.upper[2],
                                     .volume_id = k_box,
                                     .material_id = box_material};
  geometry::GGEMSAnalyticVolumeRecord volume{};
  volume.kind = pose != nullptr ? geometry::GGEMSAnalyticKind::RotatedBox
                                : geometry::GGEMSAnalyticKind::Box;
  volume.parameters.box = box;
  auto const result = ggems::test::RunStream(context, program, random, world,
                                             volume, source, pose);
  auto const &trace = result.trace;
  for (std::size_t history = 0; history < 1U; ++history) {
    auto const base = k_trace_values * history;
    ASSERT_EQ(trace[base], test.count);
    for (std::size_t event = 0U; event < test.count; ++event) {
      SCOPED_TRACE(event);
      auto const &expected = test.events[event];
      auto const offset = base + 1U + (k_trace_stride * event);
      EXPECT_EQ(trace[offset], expected.departure);
      EXPECT_EQ(trace[offset + 1U], expected.arrival);
      EXPECT_EQ(trace[offset + 2U], expected.faces);
      EXPECT_EQ(trace[offset + 3U], expected.endpoint[0]);
      EXPECT_EQ(trace[offset + 4U], expected.endpoint[1]);
      EXPECT_EQ(trace[offset + 5U], expected.endpoint[2]);
      EXPECT_EQ(trace[offset + 6U], ExpectedEvent(expected));
      EXPECT_EQ(trace[offset + 7U],
                ExpectedMaterial(expected.arrival, box_material));
    }
  }

  EXPECT_TRUE(result.unchanged_rng);
  auto const &counts = result.counts;
  EXPECT_EQ(counts.completed_history_count, 1U);
  EXPECT_EQ(counts.terminal_particle_count, 1U);
  EXPECT_EQ(counts.escaped_world_count, 1U);
  EXPECT_EQ(counts.outside_world_count, 0U);
  EXPECT_EQ(counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(counts.overflow_count, 0U);
  auto const &records = result.records;
  EXPECT_EQ(records[1].position_x_pm, test.events[test.count - 1].endpoint[0]);
  EXPECT_EQ(records[1].position_y_pm, test.events[test.count - 1].endpoint[1]);
  EXPECT_EQ(records[1].position_z_pm, test.events[test.count - 1].endpoint[2]);
  EXPECT_EQ(records[1].status, particles::ToKernelParticleStatus(
                                 particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(records[1].energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(records[1].time_ps, 17U);
  EXPECT_FLOAT_EQ(records[1].direction_x, source.axis_z_x);
  EXPECT_FLOAT_EQ(records[1].direction_y, source.axis_z_y);
  EXPECT_FLOAT_EQ(records[1].direction_z, source.axis_z_z);
}

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, TracedStreamReportsOrderedOwnerTransitions) {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  std::string const instrumented = InstrumentStream(root);

  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());

    for (auto const *engine : {"philox", "pcg32", "jkiss"}) {
      SCOPED_TRACE(engine);
      random::GGEMSRandom random{};
      random.SetEngine(engine);
      random.SetSeed(91231ULL);
      auto const &production =
        ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
          context, root / "transport", "particle_stream_transport",
          std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                      root.generic_string(),
                      random.GetKernelBuildDefinition()));
      cl::Program program(context.GetContextNative(), instrumented);
      auto const device = context.GetDevice().GetDeviceNative();
      std::string const options{production.GetBuildOptions()};
      auto const error = program.build({device}, options.c_str());
      ASSERT_EQ(error, CL_SUCCESS)
        << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

      for (auto const &test : k_box_cases) {
        SCOPED_TRACE(test.name);
        RunTracedHistory(context, program, random, test, k_box);
      }

      // A Box sharing the World Material (identity 0) stays a distinct
      // physical volume: the owners still read World -> Box -> World ->
      // exterior while every arrival Material is 0 until the exit.
      for (auto const &test : k_box_cases) {
        if (test.count != 3U) {
          continue;
        }
        SCOPED_TRACE(std::format("{} (World Material)", test.name));
        RunTracedHistory(context, program, random, test, k_world);
      }
    }
  }
}

TEST_F(GGEMSNavigationDeviceTest, RotatedBoxTracedWorker) {
  using namespace ggems::units;
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  std::string const instrumented = InstrumentStream(root);
  geometry::GGEMSWorld world;
  auto const material = materials::builtins::BuildBuiltInMaterial("Vacuum");
  world.SetSize(200_mm, 200_mm, 200_mm).SetMaterial(material);
  geometry::GGEMSBox box;
  box.SetSize(40_mm, 60_mm, 80_mm)
    .SetRotation(0.0_deg, 0.0_deg, 30.0_deg)
    .SetMaterial(material);
  world.AddVolume(box);
  auto const pose = *world.BuildBoxPoseRecord();
  // Along global X, the local-X planes are x = +/- 20 mm / cos(30 degrees).
  auto const intercept = static_cast<std::int64_t>(
    std::llround(40'000'000'000.0L / std::numbers::sqrt3_v<long double>));
  BoxCase const test{
    .name = "rotated medical worker",
    .half = {100'000'000'000LL, 100'000'000'000LL, 100'000'000'000LL},
    .lower = {-20'000'000'000LL, -30'000'000'000LL, -40'000'000'000LL},
    .upper = {20'000'000'000LL, 30'000'000'000LL, 40'000'000'000LL},
    .start = {-80'000'000'000LL, 0, 0},
    .direction = {1.0F, 0.0F, 0.0F},
    .events = {BoxEvent{.departure = 0,
                        .arrival = 1,
                        .faces = 1,
                        .endpoint = {-intercept, 0, 0},
                        .parameter = 0},
               BoxEvent{.departure = 1,
                        .arrival = 0,
                        .faces = 2,
                        .endpoint = {intercept, 0, 0},
                        .parameter = 0},
               BoxEvent{.departure = 0,
                        .arrival = k_exterior,
                        .faces = 2,
                        .endpoint = {100'000'000'000LL, 0, 0},
                        .parameter = 0}},
    .count = 3};
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    if (context.GetDevice().GetDoubleFpConfig() == 0U) {
      GTEST_SKIP() << "Selected device lacks FP64";
    }
    random::GGEMSRandom random;
    random.SetEngine("philox").SetSeed(91231ULL);
    auto const options =
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1 "
                  "-DGGEMS_ENABLE_FP64_NAVIGATION=1",
                  root.generic_string(), random.GetKernelBuildDefinition());
    auto const &strict = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport", options);
    auto const device = context.GetDevice().GetDeviceNative();
    cl::Program strict_program(context.GetContextNative(), instrumented);
    ASSERT_EQ(strict_program.build(
                {device}, std::string{strict.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << strict_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    RunTracedHistory(context, strict_program, random, test, 1, &pose);
    ASSERT_FALSE(HasFailure());
  }
}
