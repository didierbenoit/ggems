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
 * \brief Defines the legacy synthetic branching workload and its diagnostics.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::transport {

/*!
 * \brief Supplies one legacy synthetic branching dispatch.
 *
 * This workload exercises worker streams, bounded branching, and Observer
 * capture. Its fake steps and termination thresholds do not model physical
 * Processes, Production Cuts, or current analytical Navigation.
 */
struct GGEMSDummyTransportRunConfig {
  /*! \brief Run identifier copied into captured diagnostic records. */
  std::uint64_t run_id{0ULL};
  /*! \brief Positive number of primary histories assigned to this dispatch. */
  std::uint32_t total_primary_count{4096U};
  /*! \brief History-ID base shared by the device assignments. */
  std::uint64_t projection_history_offset{0ULL};
  /*! \brief First assigned primary index in the run population. */
  std::uint64_t device_primary_offset{0ULL};
  /*!
   * \brief Ordered source authoring records with exact positive Mono energy.
   */
  std::vector<sources::GGEMSSourceRecord> source_records{};
  /*!
   * \brief Source intervals in the run population, parallel to source_records.
   */
  std::vector<sources::GGEMSSourceRunRange> source_ranges{};
  /*! \brief Per-dispatch diagnostic primary-selection controls. */
  observer::GGEMSObserverConfigRecord observer_config{};
  /*! \brief Synthetic termination threshold in integer micro-eV. */
  std::uint64_t min_energy_micro_eV{10'000'000'000ULL};
  /*! \brief Maximum descendant generation admitted by synthetic branching. */
  std::uint32_t max_generation{8U};
  /*!
   * \brief Synthetic step-count termination threshold checked after each move.
   *
   * A zero threshold still permits the first fake step before termination.
   */
  std::uint32_t max_steps_per_track{32U};
};

/*!
 * \brief Returns copied counters, diagnostic records, and timing for one
 * dispatch.
 *
 * Timing and rates characterize this synthetic workload, not physical Monte
 * Carlo performance. Observer records may be truncated at device capacity.
 */
struct GGEMSDummyTransportRunReport {
  /*! \brief Active OpenCL context index supplied to the workload. */
  std::uint32_t context_index{0U};
  /*! \brief Device display name copied from the owning context. */
  std::string device_name{};
  /*! \brief Copied synthetic transport counters after kernel completion. */
  GGEMSTransportCounters counters{};
  /*! \brief Copied device capture counts, including rejected records. */
  observer::GGEMSObserverCounters observer_counters{};
  /*! \brief Owned records copied up to the device capture capacity. */
  std::vector<observer::GGEMSObserverRecord> observer_records{};
  /*! \brief Host-observed synchronous kernel-dispatch duration in ps. */
  ggems::units::Time host_time{0U};
  /*! \brief Device START-to-END profiling duration in ps. */
  ggems::units::Time kernel_time{0U};
  /*! \brief Device QUEUED-to-END profiling duration in ps. */
  ggems::units::Time command_time{0U};
  /*! \brief Completed histories divided by host-observed dispatch time. */
  double host_histories_per_second{0.0};
  /*! \brief Completed histories divided by device kernel time. */
  double kernel_histories_per_second{0.0};
  /*! \brief Terminal synthetic particles per second of host dispatch time. */
  double host_terminal_particles_per_second{0.0};
  /*! \brief Terminal synthetic particles per second of device kernel time. */
  double kernel_terminal_particles_per_second{0.0};
};

/*!
 * \brief Owns SVM storage for the legacy synthetic branching transport kernel.
 *
 * The OpenCL context and random-engine configuration are borrowed and must
 * outlive this nonmovable workload. The selected random engine must remain
 * unchanged: construction allocates its state layout, while Run() rereads its
 * kernel definition. Worker RNG states persist between calls; changing the
 * borrowed configuration's seed does not reinitialize those states. Counters,
 * final worker states, and capture records reset each dispatch. Calls must be
 * serialized because they mutate the same SVM buffers.
 * This workload is diagnostic and does not implement physical Process
 * transport.
 */
class GGEMSDummyTransportWorkload {
public:
  /*!
   * \brief Allocates device workload storage and initializes worker RNG
   * streams.
   *
   * \param[in] context Borrowed context owning all SVM allocations.
   * \param[in] kernel_root Root containing the transport kernel source and
   * shared headers.
   * \param[in] random Borrowed initialized random-engine configuration.
   * \param[in] worker_count Positive count of persistent worker streams.
   * \param[in] source_count Positive stable number of source slots.
   * \param[in] random_stream_offset First worker stream ID assigned to this
   * workload.
   * \param[in] context_index Active context index recorded in reports.
   * \param[in] observer_record_capacity Positive maximum number of captured
   * device records.
   * \throws GGEMSRecoverable If a count is zero or the engine rejects the
   * stream range.
   */
  GGEMSDummyTransportWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                              std::filesystem::path kernel_root,
                              random::GGEMSRandom const &random,
                              std::uint32_t worker_count,
                              std::uint32_t source_count,
                              std::uint64_t random_stream_offset = 0ULL,
                              std::uint32_t context_index = 0U,
                              std::uint32_t observer_record_capacity = 1U);

  /*! \brief Releases workload-owned SVM buffers. */
  ~GGEMSDummyTransportWorkload() = default;

  /*! \brief Disallows copying owned device execution state. */
  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload const &) = delete;
  /*! \brief Disallows moving bound device execution state. */
  GGEMSDummyTransportWorkload(GGEMSDummyTransportWorkload &&) = delete;
  /*! \brief Disallows copy assignment of device execution state. */
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload const &) = delete;
  /*! \brief Disallows move assignment of device execution state. */
  GGEMSDummyTransportWorkload &
  operator=(GGEMSDummyTransportWorkload &&) = delete;

public:
  /*!
   * \brief Executes one synthetic branching dispatch and copies its
   * diagnostics.
   *
   * Source array sizes must match each other and the constructor's stable
   * source count. Every source must carry exact positive Mono energy. Active
   * source records are packed to FP32 before upload. Births copy the stored
   * origin, window start, axis_z direction, Mono energy, and particle species;
   * configured position and direction distributions are not sampled. The
   * unchanged selected engine and Observer flag determine the cached OpenCL
   * program variant; RNG states are not reset.
   *
   * \param[in] config Coherent source arrays and positive primary count for
   * this dispatch.
   * \return Owned counters, bounded Observer records, and measured dispatch
   * timing.
   * \throws GGEMSRecoverable If the primary count, source arrays, energy, or
   * source packing is invalid.
   */
  GGEMSDummyTransportRunReport Run(GGEMSDummyTransportRunConfig const &config);

  /*!
   * \brief Copies completed synthetic transport counters to host storage.
   *
   * \return The current per-dispatch device counters.
   */
  [[nodiscard]] GGEMSTransportCounters ReadCountersFromSVM();

  /*!
   * \brief Copies completed diagnostic capture counters to host storage.
   *
   * \return Current device record, overflow, and captured-primary counts.
   */
  [[nodiscard]] observer::GGEMSObserverCounters ReadObserverCountersFromSVM();

  /*!
   * \brief Copies the requested prefix of completed diagnostic records.
   *
   * \param[in] record_count Requested number of records; clamped to
   * capture-buffer capacity.
   * \return Owned host copies of the bounded prefix, empty when the request is
   * zero.
   */
  [[nodiscard]] std::vector<observer::GGEMSObserverRecord>
  ReadObserverRecordsFromSVM(std::uint32_t record_count);

  /*!
   * \brief Returns the number of persistent workers allocated at construction.
   *
   * \return Worker-stream count used by dispatch and RNG storage.
   */
  [[nodiscard]] std::uint32_t GetWorkerCount() const noexcept {
    return worker_count_;
  }

private:
  /*!
   * \brief Initializes the allocated worker stream range from the selected
   * engine.
   */
  void InitializeRandomStatesInSVM();
  /*! \brief Clears per-dispatch transport counters before kernel execution. */
  void ResetCountersInSVM();
  /*! \brief Resets the synthetic final-state slot for every worker. */
  void ClearWorkerFinalStatesInSVM();
  /*!
   * \brief Clears capture counters and diagnostic record storage before
   * dispatch.
   */
  void ResetObserverInSVM();
  /*!
   * \brief Uploads the diagnostic primary selectors for the next dispatch.
   *
   * \param[in] observer_config Capture flags and source-local selectors to
   * copy.
   */
  void WriteObserverConfigToSVM(
    observer::GGEMSObserverConfigRecord const &observer_config);

private:
  /*! \brief Borrowed OpenCL context that outlives this workload. */
  ggems::ocl::GGEMSOpenCLContext *context_{nullptr};
  /*! \brief Root directory used to find kernel source and shared headers. */
  std::filesystem::path kernel_root_{};
  /*! \brief Borrowed engine configuration used to initialize worker states. */
  random::GGEMSRandom const *random_{nullptr};

  /*! \brief Number of persistent workers and random-stream records. */
  std::uint32_t worker_count_{0U};
  /*! \brief Stable source-slot count used to size uploaded arrays. */
  std::uint32_t source_count_{0U};
  /*! \brief First stream ID in this workload's assigned RNG range. */
  std::uint64_t random_stream_offset_{0ULL};

  /*! \brief Active context index included in dispatch reports. */
  std::uint32_t context_index_{0U};
  /*! \brief Cached display name of the context's OpenCL device. */
  std::string device_name_{};

  /*! \brief Maximum captured records in the device buffer. */
  std::uint32_t observer_record_capacity_{1U};

  /*! \brief Owned SVM containing persistent per-worker RNG states. */
  ggems::ocl::GGEMSOpenCLSVMBuffer random_states_buffer_;
  /*! \brief Owned SVM containing one synthetic final particle per worker. */
  ggems::ocl::GGEMSOpenCLSVMBuffer worker_final_states_buffer_;
  /*! \brief Owned SVM containing per-dispatch transport counters. */
  ggems::ocl::GGEMSOpenCLSVMBuffer counters_buffer_;
  /*! \brief Owned SVM containing packed FP32 source birth descriptors. */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_records_buffer_;
  /*! \brief Owned SVM containing source primary intervals for the dispatch. */
  ggems::ocl::GGEMSOpenCLSVMBuffer source_ranges_buffer_;
  /*! \brief Owned SVM containing diagnostic capture selectors. */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_config_buffer_;
  /*! \brief Owned SVM containing device diagnostic capture counters. */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_counters_buffer_;
  /*! \brief Owned SVM containing the bounded diagnostic record array. */
  ggems::ocl::GGEMSOpenCLSVMBuffer observer_records_buffer_;
};

} // namespace ggems::core::transport
