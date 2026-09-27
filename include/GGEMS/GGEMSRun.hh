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
 * \brief Coordinates source windows and diagnostic transport execution.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>

#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/particles/GGEMSPrimaryStream.hh"
#include "GGEMS/transport/GGEMSTransportWorkload.hh"

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::sources {
class GGEMSSourcePopulationPlanner;
}

namespace ggems::core::observer {
class GGEMSTransportObserver;
}

namespace ggems::core {

/*!
 * \brief Owns source chronology and per-device diagnostic transport workloads.
 *
 * Initialize() freezes population mode and energy configuration and prepares
 * OpenCL resources. Sequential Run() calls capture the current source poses and
 * execute one population window each. Current transport is diagnostic; this
 * class does not yet orchestrate physical navigation, interactions, or
 * acquisition.
 *
 * Concurrent Run() and ResetTime() calls on this object are rejected by a
 * shared guard. Configuration and source mutation require caller
 * synchronization; the guard does not make the entire configuration API
 * thread-safe.
 */
class GGEMSRun {
public:
  /*! \brief Constructs a run with one implicit default source. */
  GGEMSRun();

  /*! \brief Releases workloads, source snapshots, and shared attachments. */
  ~GGEMSRun();

  /*! \brief Disallows copying run state and device resources. */
  GGEMSRun(GGEMSRun const &) = delete;

  /*! \brief Disallows moving a run with attached execution state. */
  GGEMSRun(GGEMSRun &&) = delete;

  /*! \brief Disallows replacing run state by copy assignment. */
  auto operator=(GGEMSRun const &) -> GGEMSRun & = delete;

  /*! \brief Disallows replacing run state by move assignment. */
  auto operator=(GGEMSRun &&) -> GGEMSRun & = delete;

  /*!
   * \brief Prepares stable source and device state once.
   *
   * Requires an attached random engine and initialized OpenCL contexts.
   * Population mode and energy become immutable after successful
   * initialization; counts and poses may still change between runs.
   * Activity-driven sources require a time schedule.
   *
   * \throws GGEMSRecoverable If already initialized, required runtime resources
   * are missing, or source configuration is invalid.
   * \throws GGEMSInternal If an attached source is null.
   * \throws GGEMSFatal If an OpenCL resource operation fails.
   */
  auto Initialize() -> void;

  /*!
   * \brief Executes the next source population on the active devices.
   *
   * All devices use one source window and partition primary histories. The
   * final scheduled window may be shorter than the configured step. A scheduled
   * empty population still commits its snapshot and advances the clock. Static
   * mode requires a nonzero population. The clock and last successful snapshot
   * advance only after successful result checks; a failed run does not promise
   * RNG or history-identifier rollback.
   *
   * \throws GGEMSRecoverable If uninitialized, already running, exhausted, or
   * if population planning, observer selection, or transport result checks
   * fail.
   * \throws GGEMSFatal If a device operation fails.
   */
  auto Run() -> void;

  /*!
   * \brief Configures the half-open source schedule before initialization.
   *
   * \param[in] start_ps Inclusive initial simulation time in ps.
   * \param[in] stop_ps Exclusive final simulation time in ps.
   * \param[in] step_ps Positive maximum duration of one run window in ps.
   * \throws GGEMSRecoverable If initialized, start is not before stop, or step
   * is zero.
   */
  auto SetTimePicoSecond(std::uint64_t start_ps, std::uint64_t stop_ps,
                         std::uint64_t step_ps) -> void;

  /*!
   * \brief Rewinds the configured source clock, or sets static time to zero.
   *
   * Does not reset random streams, run identifiers, history reservations, or
   * the last successful snapshot.
   *
   * \throws GGEMSRecoverable If Run() or another ResetTime() owns the guard.
   */
  auto ResetTime() -> void;

  /*!
   * \brief Reports whether a source time schedule was configured.
   * \return True when SetTimePicoSecond() has succeeded.
   */
  [[nodiscard]] auto HasTimeConfiguration() const noexcept -> bool;

  /*!
   * \brief Reports whether another run window remains.
   * \return True in static mode, or while the clock precedes the scheduled
   * stop.
   */
  [[nodiscard]] auto HasNextTimeStep() const noexcept -> bool;

  /*!
   * \brief Reads the source clock in picoseconds.
   * \return Start of the next scheduled window, or zero in static mode.
   */
  [[nodiscard]] auto GetCurrentTimePicoSecond() const noexcept -> std::uint64_t;

  /*!
   * \brief Returns the next source window with its stop clipped to the
   * schedule.
   * \return Half-open ps interval; {0, 0} in static mode and {stop, stop} after
   * the schedule is exhausted.
   */
  [[nodiscard]] auto GetCurrentTimeWindowPicoSecond() const noexcept
    -> GGEMSTimeWindow;

  /*!
   * \brief Copies the last successfully committed source snapshot under a lock.
   * \return An owning snapshot copy, or no value before the first successful
   * run.
   */
  [[nodiscard]] auto GetLastSourceRunSnapshot() const
    -> std::optional<sources::GGEMSSourceRunSnapshot>;

  /*!
   * \brief Reports whether diagnostic observation is attached.
   * \return True when an observer object is attached.
   */
  [[nodiscard]] auto HasObserver() const noexcept -> bool;

  /*!
   * \brief Attaches shared ownership of the random engine before
   * initialization.
   * \param[in] random Non-null engine used for population and device streams.
   * \throws GGEMSRecoverable If random is null or the run is initialized.
   */
  auto SetRandom(std::shared_ptr<random::GGEMSRandom> random) -> void;

  /*!
   * \brief Appends a source, replacing the implicit source on the first call.
   * \param[in] source Source whose shared ownership is retained; must be
   * non-null when Initialize() is called.
   * \throws GGEMSRecoverable If the run is already initialized.
   */
  auto AddSource(std::shared_ptr<sources::GGEMSSource> source) -> void;

  /*!
   * \brief Attaches shared ownership of diagnostic observation before setup.
   * \param[in] observer Non-null observer receiving successful run results.
   * \throws GGEMSRecoverable If observer is null or the run is initialized.
   */
  auto SetObserver(std::shared_ptr<observer::GGEMSTransportObserver> observer)
    -> void;

private:
  /*! \brief Guard shared by Run() and ResetTime(). */
  std::atomic<bool> running_{false};

  /*! \brief Shared random engine retained for all run workloads. */
  std::shared_ptr<random::GGEMSRandom> random_{nullptr};

  /*! \brief Optional shared diagnostic observer. */
  std::shared_ptr<observer::GGEMSTransportObserver> observer_{nullptr};

  /*! \brief Ordered source slots retained through shared ownership. */
  std::vector<std::shared_ptr<sources::GGEMSSource>> sources_;

  /*! \brief Whether the first explicit source replaces the default source. */
  bool uses_implicit_default_source_{true};

  /*! \brief Protects publication and copying of the last source snapshot. */
  mutable std::mutex source_run_snapshot_mutex_;

  /*! \brief Source snapshot from the last successful run. */
  std::optional<sources::GGEMSSourceRunSnapshot> last_source_run_snapshot_;

  /*! \brief Reserves global history ranges across nonempty runs. */
  particles::GGEMSPrimaryStream primary_stream_;

  /*! \brief Whether stable runtime preparation completed successfully. */
  bool initialized_{false};

  /*! \brief Identifier reserved for the next execution attempt. */
  std::uint64_t next_run_id_{0ULL};

  /*! \brief Whether the source clock follows an explicit schedule. */
  bool has_time_configuration_{false};

  /*! \brief Configured inclusive schedule start in picoseconds. */
  std::uint64_t time_start_ps_{0ULL};

  /*! \brief Configured exclusive schedule stop in picoseconds. */
  std::uint64_t time_stop_ps_{0ULL};

  /*! \brief Configured maximum source-window duration in picoseconds. */
  std::uint64_t time_step_ps_{0ULL};

  /*! \brief Clock advanced only by a successful run or explicit reset. */
  std::atomic<std::uint64_t> current_time_ps_{0ULL};

  /*! \brief Immutable energy and emission configuration shared by runs. */
  sources::GGEMSSourceConfigurationSnapshotPtr source_configuration_snapshot_;

  /*! \brief Owns stable source slots and population-planning RNG state. */
  std::unique_ptr<sources::GGEMSSourcePopulationPlanner>
    source_population_planner_;

  /*! \brief Owns one persistent transport workload per active context. */
  std::vector<std::unique_ptr<transport::GGEMSTransportWorkload>>
    transport_workloads_;
};
} // namespace ggems::core
