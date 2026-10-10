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
 * \brief Owns per-device diagnostic transport resources and run reports.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <string>
#include <vector>
#include <memory>

#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceEmissionRange.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::sources {
class GGEMSSourceConfigurationSnapshot;
}

namespace ggems::core::transport {

/*!
 * \brief Owns one workload's run inputs and its assigned primary interval.
 *
 * Source arrays describe the whole run; total_primary_count is only this
 * device's assigned count. A chunk primary index becomes a run-primary index by
 * adding device_primary_offset, then a global history ID by adding
 * projection_history_offset. Per-run records must match the stable
 * configuration uploaded when the workload was constructed.
 */
struct GGEMSTransportRunConfig {
  /*! \brief Run identifier attached to diagnostic capture records. */
  std::uint64_t run_id{0ULL};

  /*!
   * \brief Primary count assigned to this device workload, not the full run.
   */
  std::uint64_t total_primary_count{0ULL};

  /*! \brief Global history base reserved for the entire run. */
  std::uint64_t projection_history_offset{0ULL};

  /*!
   * \brief First assigned primary index within the concatenated run population.
   */
  std::uint64_t device_primary_offset{0ULL};

  /*! \brief Owned current records for every run source slot. */
  std::vector<sources::GGEMSSourceRecord> source_records;

  /*! \brief Owned per-source population modes and radioactive time laws. */
  std::vector<sources::GGEMSSourcePopulationRecord> source_population_records;

  /*! \brief Owned run-wide primary intervals parallel to source slots. */
  std::vector<sources::GGEMSSourceRunRange> source_ranges;

  /*!
   * \brief Owned source-local primary intervals parallel to stable emissions.
   */
  std::vector<sources::GGEMSSourceEmissionRange> source_emission_ranges;

  /*! \brief Diagnostic capture controls copied to the workload device. */
  observer::GGEMSObserverConfigRecord observer_config{};
};

/*!
 * \brief Accumulates per-launch transport diagnostics in host uint64 values.
 *
 * Additive counters sum across launches; max_stack_depth is a maximum. The
 * logical next_primary_id sums assigned chunk counts after checking each device
 * cursor, so it excludes the extra atomic termination probes present in raw
 * counters.
 */
struct GGEMSTransportLogicalCounters {
  /*! \brief Sum of assigned chunk counts after successful raw-cursor checks. */
  std::uint64_t next_primary_id{0ULL};

  /*! \brief Primaries admitted to source initialization across all chunks. */
  std::uint64_t consumed_primary_count{0ULL};

  /*!
   * \brief Histories reaching successful World exit or diagnostic termination
   * across all chunks.
   */
  std::uint64_t completed_history_count{0ULL};

  /*! \brief Particles marked terminal by the diagnostic kernel. */
  std::uint64_t terminal_particle_count{0ULL};

  /*! \brief Created secondaries; zero in the current diagnostic kernel. */
  std::uint64_t created_secondary_count{0ULL};

  /*! \brief Synthetic Aionino-to-Gamma events; currently zero. */
  std::uint64_t aionino_to_gamma_count{0ULL};

  /*! \brief Synthetic Gamma-to-Electron events; currently zero. */
  std::uint64_t gamma_to_electron_count{0ULL};

  /*! \brief Synthetic Electron branching events; currently zero. */
  std::uint64_t electron_to_electron_count{0ULL};

  /*! \brief Accumulated operation failures or dropped diagnostic captures. */
  std::uint64_t overflow_count{0ULL};

  /*! \brief Maximum launch stack depth; currently zero. */
  std::uint64_t max_stack_depth{0ULL};

  /*! \brief Accumulated synthetic steps; currently zero. */
  std::uint64_t total_fake_step_count{0ULL};

  /*! \brief Aionino births rejected outside the World. */
  std::uint64_t outside_world_count{0ULL};

  /*! \brief Aionino queries that could not resolve a physical boundary. */
  std::uint64_t unresolved_geometry_count{0ULL};

  /*! \brief Aionino histories completed at the World boundary. */
  std::uint64_t escaped_world_count{0ULL};
};

/*! \brief Retains uint64 capture totals across all launches of one workload. */
struct GGEMSObserverLogicalCounters {
  /*! \brief Number of diagnostic records retained in the host report. */
  std::uint64_t record_count{0ULL};

  /*! \brief Accumulated operation failures or dropped diagnostic captures. */
  std::uint64_t overflow_count{0ULL};

  /*! \brief Primaries selected for diagnostic capture across all chunks. */
  std::uint64_t captured_primary_count{0ULL};
};

/*!
 * \brief Owns accumulated workload diagnostics, captured records, and timings.
 *
 * Timing durations sum the per-launch measurements in canonical ps. Host time
 * measures synchronous launch execution and excludes source upload and later
 * counter/record collection. Kernel and command durations use device profiling.
 * Observer capture is bounded diagnostic output, not a scientific completeness
 * measure; the uint32 summary counters saturate where logical totals exceed
 * them.
 */
struct GGEMSTransportRunReport {
  /*! \brief Active OpenCL context index that produced this report. */
  std::uint32_t context_index{0U};

  /*! \brief Owned display name of the device that executed the workload. */
  std::string device_name;

  /*! \brief Accumulated transport counts, with maximum stack depth. */
  GGEMSTransportLogicalCounters counters{};

  /*!
   * \brief Legacy uint32 observer summary, saturating logical totals if needed.
   */
  observer::GGEMSObserverCounters observer_counters{};

  /*! \brief Owned bounded capture records concatenated in launch order. */
  std::vector<observer::GGEMSObserverRecord> observer_records;

  /*! \brief Full-width totals for retained and dropped diagnostic captures. */
  GGEMSObserverLogicalCounters logical_observer_counters{};

  /*! \brief Sum of host launch intervals in canonical ps. */
  ggems::units::Time host_time{0U};

  /*! \brief Sum of device kernel execution durations in canonical ps. */
  ggems::units::Time kernel_time{0U};

  /*! \brief Sum of device queued-to-complete durations in canonical ps. */
  ggems::units::Time command_time{0U};

  /*!
   * \brief Completed histories divided by host launch time; zero at zero time.
   */
  double host_histories_per_second{0.0};

  /*! \brief Completed histories divided by kernel time; zero at zero time. */
  double kernel_histories_per_second{0.0};

  /*!
   * \brief Terminal particles divided by host launch time; zero at zero time.
   */
  double host_terminal_particles_per_second{0.0};

  /*! \brief Terminal particles divided by kernel time; zero at zero time. */
  double kernel_terminal_particles_per_second{0.0};
};

/*!
 * \brief Checks run arrays and interval consistency against stable source
 * assets.
 *
 * Checks nonzero workload count, contiguous source/group ranges, supported
 * population modes, diagnostic endpoint envelopes, and containment in the
 * source population. Callers own arithmetic representability and stable
 * energy-table consistency.
 *
 * \param[in] config Device assignment with full-run source arrays.
 * \param[in] stable_source_count Number of source slots uploaded during
 * construction.
 * \param[in] stable_emission_count Number of immutable radioactive emission
 * descriptors.
 * \param[in] worker_count Reserved argument, currently unused by this
 * validator.
 * \param[in] launch_primary_count_limit Reserved argument, currently unused by
 * this validator.
 * \throws GGEMSRecoverable If an input shape, interval, population, or
 * diagnostic endpoint check fails.
 * \throws GGEMSInternal If the built-in diagnostic direction bound cannot be
 * formed.
 */
auto ValidateTransportRunConfig(GGEMSTransportRunConfig const &config,
                                std::uint32_t stable_source_count,
                                std::uint32_t stable_emission_count,
                                std::uint32_t worker_count,
                                std::uint32_t launch_primary_count_limit)
  -> void;

/*!
 * \brief Owns persistent OpenCL resources for one device's diagnostic workload.
 *
 * The context is borrowed and must outlive this object. Construction copies
 * stable source assets into owned SVM buffers and initializes one random state
 * per worker. Sequential Run() calls reuse those states and split their
 * assigned primaries into uint32-bounded chunks. The current kernel initializes
 * each primary, advances Aionino to the World boundary, and keeps the
 * diagnostic one-meter projection for other species. No interactions or time of
 * flight. Calls using the same workload must be serialized by the owner.
 */
class GGEMSTransportWorkload {
public:
  /*!
   * \brief Allocates workload buffers and uploads stable source assets.
   * \param[in,out] context Borrowed initialized OpenCL context that outlives
   * this object.
   * \param[in] kernel_root Root containing the transport kernel and its
   * includes.
   * \param[in] random Engine and seed used to initialize device worker streams.
   * \param[in] worker_count Positive persistent worker count with a usable
   * launch limit.
   * \param[in] source_configuration Coherent immutable assets copied into owned
   * SVM.
   * \param[in] random_stream_offset First device stream identifier assigned to
   * workers.
   * \param[in] context_index Active context index reported in diagnostics.
   * \param[in] observer_record_capacity Positive maximum retained diagnostic
   * records.
   * \param[in] launch_primary_count_limit Requested positive chunk limit, or
   * zero to select the largest limit preserving atomic-cursor headroom.
   * \param[in] world Optional World copied to SVM; required for Aionino.
   * \param[in] boxes Immutable Box occurrence records copied to SVM.
   * \throws GGEMSRecoverable If counts, random-stream range, or launch limit
   * are invalid.
   * \throws GGEMSInternal If energy and cumulative-ticket table lengths
   * disagree.
   * \throws GGEMSFatal If kernel creation, SVM allocation, or an OpenCL
   * operation fails.
   */
  GGEMSTransportWorkload(
    ggems::ocl::GGEMSOpenCLContext &context, std::filesystem::path kernel_root,
    random::GGEMSRandom const &random, std::uint32_t worker_count,
    sources::GGEMSSourceConfigurationSnapshot const &source_configuration,
    std::uint64_t random_stream_offset = 0ULL, std::uint32_t context_index = 0U,
    std::uint32_t observer_record_capacity = 1U,
    std::uint32_t launch_primary_count_limit = 0U,
    geometry::GGEMSWorld const *world = nullptr,
    std::span<geometry::GGEMSBoxRecord const> boxes = {});

  ~GGEMSTransportWorkload() = default;

  /*! \brief Disallows copy construction of owned execution state. */
  GGEMSTransportWorkload(GGEMSTransportWorkload const &) = delete;

  /*! \brief Disallows move construction of owned execution state. */
  GGEMSTransportWorkload(GGEMSTransportWorkload &&) = delete;

  /*! \brief Disallows copy assignment of owned execution state. */
  auto operator=(GGEMSTransportWorkload const &)
    -> GGEMSTransportWorkload & = delete;

  /*! \brief Disallows move assignment of owned execution state. */
  auto operator=(GGEMSTransportWorkload &&)
    -> GGEMSTransportWorkload & = delete;

  /*!
   * \brief Executes a validated device assignment as sequential diagnostic
   * chunks.
   *
   * Does not call ValidateRunConfig() itself. Caller serializes access and
   * supplies the same stable source layout. Worker RNG states persist across
   * chunks and runs; failures do not rewind them.
   *
   * \param[in] config Configuration already accepted by ValidateRunConfig().
   * \return Owning counters, captured records, and accumulated launch timings.
   * \throws GGEMSInternal If padded launch geometry, cursor progress, or
   * retained capture bounds are inconsistent.
   * \throws GGEMSFatal If an OpenCL operation fails.
   * \throws GGEMSRecoverable If device profiling timestamps are not ordered.
   */
  auto Run(GGEMSTransportRunConfig const &config) -> GGEMSTransportRunReport;

  /*!
   * \brief Validates a configuration against this workload stable counts.
   *
   * \param[in] config Proposed run assignment.
   * \throws GGEMSRecoverable If source arrays, ranges, or endpoint envelopes
   * are invalid.
   * \throws GGEMSInternal If a built-in diagnostic bound cannot be formed.
   */
  auto ValidateRunConfig(GGEMSTransportRunConfig const &config) const -> void;

  /*!
   * \brief Reads the latest raw transport counters from the device.
   *
   * Requires serialized access after the producing launch completes.
   *
   * \return One launch uint32 counter record.
   * \throws GGEMSFatal If SVM host access fails.
   */
  [[nodiscard]] auto ReadCountersFromSVM() -> GGEMSTransportCounters;

  /*!
   * \brief Reads the latest raw observer counters from the device.
   *
   * Requires serialized access after the producing launch completes.
   *
   * \return One launch uint32 capture counter record.
   * \throws GGEMSFatal If SVM host access fails.
   */
  [[nodiscard]] auto ReadObserverCountersFromSVM()
    -> observer::GGEMSObserverCounters;

  /*!
   * \brief Copies the bounded prefix of the device diagnostic record buffer.
   *
   * \param[in] record_count Requested record count, clipped to allocated
   * capacity.
   * \return Owned record copy, empty for zero requested records.
   * \throws GGEMSFatal If SVM host access fails.
   */
  [[nodiscard]] auto ReadObserverRecordsFromSVM(std::uint32_t record_count)
    -> std::vector<observer::GGEMSObserverRecord>;

  /*!
   * \brief Reads the number of persistent RNG-backed workers.
   *
   * \return Configured worker count, independent of the primary population.
   */
  [[nodiscard]] auto GetWorkerCount() const noexcept -> std::uint32_t {
    return worker_count_;
  }

private:
  /*!
   * \brief Initializes each persistent worker random state through mapped SVM.
   *
   * \param[in] random Engine supplying state layout and seed.
   * \param[in] random_stream_offset First worker stream identifier.
   * \throws GGEMSRecoverable If random-state initialization rejects the range.
   * \throws GGEMSFatal If SVM mapping or unmapping fails.
   */
  auto InitializeRandomStatesInSVM(random::GGEMSRandom const &random,
                                   std::uint64_t random_stream_offset) -> void;

  /*!
   * \brief Clears raw transport counters before a launch.
   *
   * \throws GGEMSFatal If SVM host access fails.
   */
  auto ResetCountersInSVM() -> void;

  /*!
   * \brief Clears raw capture counters before a launch.
   *
   * \throws GGEMSFatal If SVM host access fails.
   */
  auto ResetObserverCountersInSVM() -> void;

  /*!
   * \brief Copies diagnostic selection controls into device-visible storage.
   *
   * \param[in] observer_config Prepared per-run capture configuration.
   * \throws GGEMSFatal If SVM host access fails.
   */
  auto WriteObserverConfigToSVM(
    observer::GGEMSObserverConfigRecord const &observer_config) -> void;

  /*! \brief Borrowed context that must outlive all workload resources. */
  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};

  /*! \brief Persistent workers, each owning a device random-stream state. */
  std::uint32_t worker_count_{0U};

  /*! \brief Stable count of source slots in the uploaded configuration. */
  std::uint32_t source_count_{0U};

  /*! \brief Stable number of radioactive emission descriptors. */
  std::uint32_t emission_count_{0U};

  /*! \brief Active context index copied into run reports. */
  std::uint32_t context_index_{0U};

  /*! \brief Cached device display name. */
  std::string device_name_;

  /*! \brief Maximum retained capture records for one workload run. */
  std::uint32_t observer_record_capacity_{1U};

  /*! \brief Primary limit preserving uint32 atomic-cursor headroom. */
  std::uint32_t launch_primary_count_limit_{0U};

  /*!
   * \brief Borrowed immutable World used for static source preflight, or null.
   *
   * The owner (GGEMSRun) keeps the World alive for the workload lifetime.
   */
  geometry::GGEMSWorld const *world_{nullptr};

  /*! \brief Immutable origin-centered World box, or zero record if absent. */
  ggems::ocl::GGEMSOpenCLSVMBuffer world_buffer_;

  /*! \brief Number of immutable Box occurrences uploaded to the device. */
  std::uint32_t box_count_{0U};

  /*! \brief Immutable Box occurrence records; one zero record if absent. */
  ggems::ocl::GGEMSOpenCLSVMBuffer box_records_buffer_;

  /*! \brief Owns SVM storage for persistent per-worker random states. */
  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;

  /*!
   * \brief Owns SVM storage for raw uint32 transport counters reset before each
   * launch.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;

  /*!
   * \brief Owns SVM storage for per-run source poses and analytic birth laws.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_records_buffer_;

  /*!
   * \brief Owns SVM storage for per-run source modes and scaled radioactive
   * decay.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_population_records_buffer_;

  /*!
   * \brief Owns SVM storage for per-run source intervals in the complete run
   * population.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_ranges_buffer_;

  /*!
   * \brief Owns SVM storage for immutable radioactive particle and energy
   * descriptors.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_emission_records_buffer_;

  /*!
   * \brief Owns SVM storage for per-run source-local primary intervals for
   * emissions.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_emission_ranges_buffer_;

  /*!
   * \brief Owns SVM storage for immutable descriptors locating each conditional
   * energy law.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer energy_distribution_records_buffer_;

  /*!
   * \brief Owns SVM storage for immutable canonical line energies or
   * spectrum-bin centers.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer energy_values_buffer_;

  /*!
   * \brief Owns SVM storage for immutable exclusive energy-selection ticket
   * bounds.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer cumulative_ticket_upper_buffer_;

  /*! \brief Owns SVM storage for per-run diagnostic selection controls. */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_config_buffer_;

  /*!
   * \brief Owns SVM storage for raw uint32 capture counters reset before each
   * launch.
   */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_counters_buffer_;

  /*! \brief Owns SVM storage for bounded device diagnostic record storage. */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_records_buffer_;

  /*! \brief Owned kernel wrapper with workload SVM arguments bound. */
  std::unique_ptr<ggems::ocl::GGEMSOpenCLKernel> kernel_;
};

} // namespace ggems::core::transport
