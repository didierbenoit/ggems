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
 * \brief Shared test-only boundary trace of the production analytic worker.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRecord.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace ggems::test {

using namespace ggems::core;

inline constexpr std::size_t k_trace_stride{9U};
inline constexpr std::size_t k_initial_owner_index{1U + (33U * k_trace_stride)};
inline constexpr std::size_t k_trace_values{k_initial_owner_index + 3U};

/*! \brief Replaces one unique occurrence or throws. */
inline auto ReplaceUnique(std::string &source, std::string const &before,
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
 * committed position, event and arrival Material. The selected parameter is
 * captured from the actual transaction witness, without repeating its query.
 * Initial owner, Material and location status follow the 33 event slots.
 * Every instrumentation anchor must be unique. No production hook is added.
 */
inline auto InstrumentStream(std::filesystem::path const &root) -> std::string {
  std::ifstream input{root / "transport/particle_stream_transport.cl"};
  std::string source{std::istreambuf_iterator<char>{input}, {}};
  std::ifstream transaction_input{root / "transport/GGEMSTransport.clh"};
  std::string transaction{std::istreambuf_iterator<char>{transaction_input},
                          {}};
  ReplaceUnique(
    transaction, "  uint *faces, uint *previous_volume) {",
    "  uint *faces, uint *previous_volume, __global long *capture) {");
  ReplaceUnique(transaction, "  if (status != GGEMS_NAVIGATION_RESOLVED) {",
                "  capture[8] = (long)as_uint(witness.parameter);\n"
                "  if (status != GGEMS_NAVIGATION_RESOLVED) {");
  ReplaceUnique(source, "#include \"transport/GGEMSTransport.clh\"",
                transaction);

  ReplaceUnique(source,
                "  volatile __global GGEMSTransportCounters *counters) {",
                "  volatile __global GGEMSTransportCounters *counters,\n"
                "  __global long *trace) {");
  ReplaceUnique(source, "  __global void const *box_pose) {",
                "  __global void const *box_pose, __global long *trace) {");
  ReplaceUnique(
    source, "counters) != 0U)",
    std::format("counters, trace + {}UL * global_primary_id) != 0U)",
                k_trace_values));
  ReplaceUnique(
    source, "  if (locate_status != GGEMS_NAVIGATION_RESOLVED) {",
    std::format("  trace[{}] = (uint)(owner >> 32U);\n"
                "  trace[{}] = (uint)(owner & 0xFFFFFFFFUL);\n"
                "  trace[{}] = locate_status;\n"
                "  if (locate_status != GGEMS_NAVIGATION_RESOLVED) {{",
                k_initial_owner_index, k_initial_owner_index + 1U,
                k_initial_owner_index + 2U));
  ReplaceUnique(
    source, "    uint const status = GGEMS_TransportToNextBoundary(",
    "    uint const departure_owner = particle->current_volume_id;\n"
    "    uint const status = GGEMS_TransportToNextBoundary(");
  ReplaceUnique(source, "&previous_volume);",
                "&previous_volume, trace + 1U + 9U * event_index);");
  ReplaceUnique(
    source, "    if (status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {",
    "    if (status == GGEMS_NAVIGATION_RESOLVED) {\n"
    "      uint const offset = 1U + 9U * event_index;\n"
    "      trace[0] = event_index + 1U;\n"
    "      trace[offset] = departure_owner;\n"
    "      trace[offset + 1U] = particle->current_volume_id;\n"
    "      trace[offset + 2U] = faces;\n"
    "      trace[offset + 3U] = (long)as_uint(particle->position_x_pm);\n"
    "      trace[offset + 4U] = (long)as_uint(particle->position_y_pm);\n"
    "      trace[offset + 5U] = (long)as_uint(particle->position_z_pm);\n"
    "      trace[offset + 6U] = event;\n"
    "      trace[offset + 7U] = particle->material_id;\n"
    "    }\n\n"
    "    if (status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {");
  return source;
}

/*! \brief Evidence from one real worker history; no geometric oracle here. */
struct StreamResult {
  std::array<std::int64_t, k_trace_values> trace{};
  transport::GGEMSTransportCounters counts{};
  std::array<observer::GGEMSObserverRecord, 2> records{};
  observer::GGEMSObserverCounters observer_counts{};
  bool unchanged_rng{false};
};

/*! \brief Launches a production worker for one configured birth.
 *
 * The optional trace argument is set only for the instrumented program.
 */
inline auto
RunStream(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
          random::GGEMSRandom &random, geometry::GGEMSWorldRecord const &world,
          std::span<geometry::GGEMSAnalyticVolumeRecord const> volumes,
          sources::GGEMSSourceRecord const &source,
          geometry::GGEMSBoxPoseRecord const *pose = nullptr,
          bool capture_trace = true, std::span<std::byte const> auxiliary = {})
  -> StreamResult {
  auto buffer = [&context](auto const &value) -> ocl::GGEMSOpenCLSVMBuffer {
    auto result = context.CreateSVMBuffer(units::Bytes{sizeof(value)});
    ocl::WriteSVMFromHost(result, value);
    return result;
  };

  constexpr std::uint32_t workers{1};
  auto rng =
    context.CreateSVMBuffer(units::Bytes{random.GetStateSize() * workers});
  std::vector<std::byte> rng_before(random.GetStateSize() * workers);
  random.InitializeStates(0ULL, std::span<std::byte>{rng_before});
  ocl::WriteSVMFromHost(rng, std::span<std::byte const>{rng_before});

  auto world_buffer = buffer(world);
  auto source_buffer = buffer(sources::BuildSourceDeviceRecord(source));
  auto ranges = buffer(sources::GGEMSSourceRunRange{
    .projection_primary_begin = 0ULL,
    .primary_count = 1U,
  });
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
  auto volume_buffer =
    context.CreateSVMBuffer(units::Bytes{volumes.size_bytes()});
  ocl::WriteSVMFromHost(volume_buffer, volumes);
  std::array<std::int64_t, k_trace_values> trace{};
  auto trace_buffer =
    context.CreateSVMBuffer(units::Bytes{sizeof(std::int64_t) * trace.size()});
  ocl::WriteSVMFromHost(trace_buffer, std::span<std::int64_t const>{trace});
  std::optional<ocl::GGEMSOpenCLSVMBuffer> pose_buffer;
  if (!auxiliary.empty()) {
    pose_buffer.emplace(
      context.CreateSVMBuffer(units::Bytes{auxiliary.size()}));
    ocl::WriteSVMFromHost(*pose_buffer, auxiliary);
  } else if (pose != nullptr) {
    pose_buffer.emplace(buffer(*pose));
  }

  ocl::GGEMSOpenCLKernel kernel{
    context, cl::Kernel(program, "particle_stream_transport"),
    "particle_stream_transport"};
  kernel.SetArgSVMPointer(0U, rng.GetData());
  kernel.SetArgSVMPointer(1U, counters_buffer.GetData());
  kernel.SetArgSVMPointer(2U, source_buffer.GetData());
  kernel.SetArgSVMPointer(3U, ranges.GetData());
  kernel.SetArg(4U, cl_uint{1});
  kernel.SetArg(5U, cl_uint{1U});
  kernel.SetArg(6U, cl_ulong{0});
  kernel.SetArg(7U, cl_ulong{0});
  kernel.SetArgSVMPointer(8U, capture_buffer.GetData());
  kernel.SetArgSVMPointer(9U, observed_counts.GetData());
  kernel.SetArgSVMPointer(10U, observations.GetData());
  kernel.SetArg(11U, cl_uint{2});
  kernel.SetArg(12U, cl_ulong{0});
  kernel.SetArg(13U, cl_uint{workers});
  kernel.SetArgSVMPointer(14U, energies.GetData());
  kernel.SetArgSVMPointer(15U, values.GetData());
  kernel.SetArgSVMPointer(16U, tickets.GetData());
  kernel.SetArgSVMPointer(17U, populations.GetData());
  kernel.SetArgSVMPointer(18U, emissions.GetData());
  kernel.SetArgSVMPointer(19U, emission_ranges.GetData());
  kernel.SetArgSVMPointer(20U, world_buffer.GetData());
  kernel.SetArgSVMPointer(21U, volume_buffer.GetData());
  // Match production: every oriented analytic family shares the pose argument.
  kernel.SetArgSVMPointer(22U, pose_buffer ? pose_buffer->GetData()
                                           : volume_buffer.GetData());
  if (capture_trace) {
    kernel.SetArgSVMPointer(23U, trace_buffer.GetData());
  }
  kernel.Run({64U}, {64U});

  StreamResult result;
  ocl::ReadSVMToHost(trace_buffer, std::span{result.trace});
  result.counts =
    ocl::ReadSVMToHost<transport::GGEMSTransportCounters>(counters_buffer);
  ocl::ReadSVMToHost(observations, std::span{result.records});
  result.observer_counts =
    ocl::ReadSVMToHost<observer::GGEMSObserverCounters>(observed_counts);
  std::vector<std::byte> rng_after(rng_before.size());
  ocl::ReadSVMToHost(rng, std::span{rng_after});
  result.unchanged_rng = rng_before == rng_after;
  return result;
}

/*! \brief Launches the same worker with one analytic record. */
inline auto
RunStream(ocl::GGEMSOpenCLContext &context, cl::Program const &program,
          random::GGEMSRandom &random, geometry::GGEMSWorldRecord const &world,
          geometry::GGEMSAnalyticVolumeRecord const &volume,
          sources::GGEMSSourceRecord const &source,
          geometry::GGEMSBoxPoseRecord const *pose = nullptr,
          bool capture_trace = true, std::span<std::byte const> auxiliary = {})
  -> StreamResult {
  return RunStream(context, program, random, world, std::span{&volume, 1U},
                   source, pose, capture_trace, auxiliary);
}

} // namespace ggems::test
