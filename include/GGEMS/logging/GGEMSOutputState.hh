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
 * \brief Declares the thread-safe GGEMS output-state ring buffer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <mutex>
#include <vector>
#include <cstddef>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"

namespace ggems::core {
/*!
 * \brief Describes the lifecycle state of a GGEMS run for user-facing output.
 */
enum class RunStatus : std::uint8_t {
  /*!
   * \brief Run startup has begun.
   */
  Starting = 0,
  /*!
   * \brief Run configuration is in progress.
   */
  Configuring,
  /*!
   * \brief Run configuration is complete and ready to execute.
   */
  Ready,
  /*!
   * \brief Particle transport is running.
   */
  Running,
  /*!
   * \brief Run completed successfully.
   */
  Finished,
  /*!
   * \brief Run terminated with a failure.
   */
  Failed
};

/*!
 * \brief Stores thread-safe user-facing run status and recent rendered log lines.
 *
 * Log lines are retained in a bounded ring buffer. When capacity is reached,
 * newly pushed lines replace the oldest retained entries.
 */
class GGEMSOutputState {
public:
  /*!
   * \brief Constructs an empty output state with the default log capacity.
   */
  GGEMSOutputState() = default;
  /*!
   * \brief Destroys the output state.
   */
  ~GGEMSOutputState() = default;

  /*! \brief Copy construction is disabled. */
  GGEMSOutputState(GGEMSOutputState const &) = delete;
  /*! \brief Move construction is disabled. */
  GGEMSOutputState(GGEMSOutputState &&) = delete;
  /*! \brief Copy assignment is disabled. */
  auto operator=(GGEMSOutputState const &) -> GGEMSOutputState & = delete;
  /*! \brief Move assignment is disabled. */
  auto operator=(GGEMSOutputState &&) -> GGEMSOutputState & = delete;

  /*!
   * \brief Sets the current run status.
   *
   * \param[in] run_status New run status.
   */
  auto SetRunStatus(RunStatus run_status) -> void;
  /*!
   * \brief Returns the current run status.
   *
   * \return Current run status.
   */
  [[nodiscard]] auto GetRunStatus() const -> RunStatus;

  /*!
   * \brief Sets the retained-log capacity and clears all existing log lines.
   *
   * A requested capacity of zero is clamped to one.
   *
   * \param[in] capacity Requested maximum retained line count.
   */
  auto SetLogCapacity(std::size_t capacity) -> void;
  /*!
   * \brief Returns the current retained-log capacity.
   *
   * \return Maximum number of retained log lines.
   */
  [[nodiscard]] auto GetLogCapacity() const -> std::size_t;

  /*!
   * \brief Appends one rendered log line to the bounded ring buffer.
   *
   * When the buffer is full, the oldest retained line is replaced.
   *
   * \param[in] rendered_line Rendered line to retain.
   */
  void PushLogLine(RenderedLogLine rendered_line);

  /*!
   * \brief Copies the newest retained log lines in chronological order.
   *
   * \param[in] max_lines Maximum number of newest lines to return.
   * \return Snapshot ordered from the oldest to the newest selected line.
   */
  [[nodiscard]] auto GetLastLogLinesSnapshot(std::size_t max_lines) const
      -> std::vector<RenderedLogLine>;

  /*!
   * \brief Clears retained log lines without changing the configured capacity.
   */
  auto ClearLogs() -> void;

  /*!
   * \brief Returns the number of currently retained log lines.
   *
   * \return Number of retained log lines.
   */
  [[nodiscard]] auto GetLogCount() const -> std::size_t;

private:
  /*!
   * \brief Default number of rendered log lines retained by a new state.
   */
  static constexpr std::size_t k_default_log_capacity{2000U};

  /*! \brief Mutex protecting all mutable output-state data. */
  mutable std::mutex mtx_;
  /*! \brief Current user-facing run status. */
  RunStatus run_status_{RunStatus::Starting};
  /*! \brief Storage backing the bounded rendered-line ring buffer. */
  std::vector<RenderedLogLine> log_ring_;
  /*! \brief Maximum number of rendered lines retained. */
  std::size_t log_capacity_{k_default_log_capacity};
  /*! \brief Index of the oldest retained line when the ring is populated. */
  std::size_t log_head_{0};
  /*! \brief Number of valid rendered lines currently retained. */
  std::size_t log_size_{0};
};
} // namespace ggems::core
