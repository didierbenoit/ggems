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
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

#include "GGEMSBoxFixtures.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {

using namespace ggems;
using namespace ggems::core;

constexpr std::size_t k_trace_stride{8U};
constexpr std::size_t k_trace_values{1U + (3U * k_trace_stride)};
constexpr std::uint32_t k_world{0U};
constexpr std::uint32_t k_box{1U};
constexpr std::uint32_t k_exterior{0xFFFFFFFFU};
constexpr std::uint32_t k_event_world_exit{1U};
constexpr std::uint32_t k_event_volume_entry{2U};
constexpr std::uint32_t k_event_volume_exit{3U};

/*! \brief Replaces one unique occurrence or throws. */
auto ReplaceUnique(std::string &source, std::string const &before,
                   std::string const &after) -> void {
  auto const position = source.find(before);
  if (position == std::string::npos ||
      source.find(before, position + 1U) != std::string::npos) {
    throw std::runtime_error(
      std::format("Stream instrumentation site is not unique: '{}'", before));
  }
  source.replace(position, before.size(), after);
}

/*!
 * \brief Loads the production stream source and inserts the trace.
 *
 * The geometry loop receives a trace pointer; after every accepted
 * transaction it records departure owner, arrival owner, crossed planes,
 * committed position, event and arrival Material. Every anchor must be
 * unique in the production source. Nothing else changes.
 */
auto InstrumentStream(std::filesystem::path const &root) -> std::string {
  std::ifstream input{root / "transport/particle_stream_transport.cl"};
  std::string source{std::istreambuf_iterator<char>{input}, {}};

  ReplaceUnique(source,
                "  volatile __global GGEMSTransportCounters *counters) {",
                "  volatile __global GGEMSTransportCounters *counters,\n"
                "  __global long *trace) {");
  ReplaceUnique(source, "  uint box_count) {",
                "  uint box_count, __global long *trace) {");
  ReplaceUnique(source,
                "GGEMS_TransportGeometry(world, boxes, box_count, &particle,\n"
                "                                  counters)",
                "GGEMS_TransportGeometry(world, boxes, box_count, &particle,\n"
                "                                  counters, trace)");
  ReplaceUnique(
    source, "    uint const status = GGEMS_TransportToNextBoundary(",
    "    uint const departure_owner = particle->current_volume_id;\n"
    "    uint const status = GGEMS_TransportToNextBoundary(");
  ReplaceUnique(source, "    if (status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {",
                "    if (status == GGEMS_NAVIGATION_RESOLVED) {\n"
                "      uint const offset = 1U + 8U * event_index;\n"
                "      trace[0] = event_index + 1U;\n"
                "      trace[offset] = departure_owner;\n"
                "      trace[offset + 1U] = particle->current_volume_id;\n"
                "      trace[offset + 2U] = faces;\n"
                "      trace[offset + 3U] = particle->position_x_pm;\n"
                "      trace[offset + 4U] = particle->position_y_pm;\n"
                "      trace[offset + 5U] = particle->position_z_pm;\n"
                "      trace[offset + 6U] = event;\n"
                "      trace[offset + 7U] = particle->material_id;\n"
                "    }\n\n"
                "    if (status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {");
  return source;
}

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
                      BoxCase const &test, std::uint32_t box_material) -> void {
  auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
    auto result = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
    ocl::WriteSVMFromHost(result, value);
    return result;
  };

  auto rng = context.CreateSVMBuffer(units::Bytes{random.GetStateSize()});
  std::vector<std::byte> rng_before(random.GetStateSize());
  random.InitializeStates(0ULL, std::span<std::byte>{rng_before});
  ocl::WriteSVMFromHost(rng, std::span<std::byte const>{rng_before});

  geometry::GGEMSWorldRecord const world{.half_extent_x_pm = test.half[0],
                                         .half_extent_y_pm = test.half[1],
                                         .half_extent_z_pm = test.half[2]};
  auto world_buffer = buffer(world);
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
  auto source_buffer = buffer(source);
  auto ranges = buffer(sources::GGEMSSourceRunRange{
    .projection_primary_begin = 0ULL, .primary_count = 1ULL});
  auto populations = buffer(sources::GGEMSSourcePopulationRecord{});
  auto emissions = buffer(sources::GGEMSSourceEmissionRecord{});
  auto emission_ranges = buffer(sources::GGEMSSourceEmissionRange{});
  sources::GGEMSEnergyDistributionRecord energy{};
  energy.distribution_type = sources::ToKernelEnergyDistributionType(
    sources::GGEMSEnergyDistributionType::Mono);
  auto energies = buffer(energy);
  auto values = buffer(std::uint64_t{0});
  auto tickets = buffer(std::uint64_t{0});
  auto counters_buffer = buffer(transport::GGEMSTransportCounters{});
  observer::GGEMSObserverConfigRecord capture{};
  capture.enabled = 1U;
  capture.capture_first_primary_count_per_source = 1U;
  auto capture_buffer = buffer(capture);
  auto observed_counts = buffer(observer::GGEMSObserverCounters{});
  auto observations = context.CreateSVMBuffer(
    units::Bytes{2 * sizeof(observer::GGEMSObserverRecord)});
  ocl::FillSVMFromHost(observations, 2U, observer::GGEMSObserverRecord{});
  geometry::GGEMSBoxRecord const box{.lower_x_pm = test.lower[0],
                                     .lower_y_pm = test.lower[1],
                                     .lower_z_pm = test.lower[2],
                                     .upper_x_pm = test.upper[0],
                                     .upper_y_pm = test.upper[1],
                                     .upper_z_pm = test.upper[2],
                                     .volume_id = k_box,
                                     .material_id = box_material};
  auto box_buffer = buffer(box);
  std::array<std::int64_t, k_trace_values> trace{};
  auto trace_buffer = buffer(trace);

  ocl::GGEMSOpenCLKernel kernel{
    context, cl::Kernel(program, "particle_stream_transport"),
    "particle_stream_transport"};
  kernel.SetArgSVMPointer(0U, rng.GetData());
  kernel.SetArgSVMPointer(1U, counters_buffer.GetData());
  kernel.SetArgSVMPointer(2U, source_buffer.GetData());
  kernel.SetArgSVMPointer(3U, ranges.GetData());
  kernel.SetArg(4U, cl_uint{1});
  kernel.SetArg(5U, cl_uint{1});
  kernel.SetArg(6U, cl_ulong{0});
  kernel.SetArg(7U, cl_ulong{0});
  kernel.SetArgSVMPointer(8U, capture_buffer.GetData());
  kernel.SetArgSVMPointer(9U, observed_counts.GetData());
  kernel.SetArgSVMPointer(10U, observations.GetData());
  kernel.SetArg(11U, cl_uint{2});
  kernel.SetArg(12U, cl_ulong{0});
  kernel.SetArg(13U, cl_uint{1});
  kernel.SetArgSVMPointer(14U, energies.GetData());
  kernel.SetArgSVMPointer(15U, values.GetData());
  kernel.SetArgSVMPointer(16U, tickets.GetData());
  kernel.SetArgSVMPointer(17U, populations.GetData());
  kernel.SetArgSVMPointer(18U, emissions.GetData());
  kernel.SetArgSVMPointer(19U, emission_ranges.GetData());
  kernel.SetArgSVMPointer(20U, world_buffer.GetData());
  kernel.SetArgSVMPointer(21U, box_buffer.GetData());
  kernel.SetArg(22U, cl_uint{1});
  kernel.SetArgSVMPointer(23U, trace_buffer.GetData());
  kernel.Run({64U}, {64U});

  ocl::ReadSVMToHost(trace_buffer, std::span{trace});
  ASSERT_EQ(trace[0], test.count);
  for (std::size_t event = 0U; event < test.count; ++event) {
    SCOPED_TRACE(event);
    auto const &expected = test.events[event];
    auto const offset = 1U + (k_trace_stride * event);
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

  std::vector<std::byte> rng_after(rng_before.size());
  ocl::ReadSVMToHost(rng, std::span{rng_after});
  EXPECT_EQ(rng_after, rng_before);
  auto const counts =
    ocl::ReadSVMToHost<transport::GGEMSTransportCounters>(counters_buffer);
  EXPECT_EQ(counts.completed_history_count, 1U);
  EXPECT_EQ(counts.terminal_particle_count, 1U);
  EXPECT_EQ(counts.escaped_world_count, 1U);
  EXPECT_EQ(counts.outside_world_count, 0U);
  EXPECT_EQ(counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(counts.overflow_count, 0U);
  std::array<observer::GGEMSObserverRecord, 2> records{};
  ocl::ReadSVMToHost(observations, std::span{records});
  EXPECT_EQ(records[1].position_x_pm, test.events[test.count - 1].endpoint[0]);
  EXPECT_EQ(records[1].position_y_pm, test.events[test.count - 1].endpoint[1]);
  EXPECT_EQ(records[1].position_z_pm, test.events[test.count - 1].endpoint[2]);
  EXPECT_EQ(records[1].status, particles::ToKernelParticleStatus(
                                 particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(records[1].energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(records[1].time_ps, 17U);
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
