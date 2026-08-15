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
 * \brief Declares host and OpenCL kernel profiling utilities.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <chrono>
#include <cstdint>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::ocl {

/*!
 * \brief Stores timestamp and duration information for one OpenCL kernel event.
 */
struct GGEMSOpenCLKernelTiming {
  /*!
   * \brief OpenCL event queued timestamp.
   */
  ggems::units::Time time_queued{0U};
  /*!
   * \brief OpenCL event submission timestamp.
   */
  ggems::units::Time time_submit{0U};
  /*!
   * \brief OpenCL event execution-start timestamp.
   */
  ggems::units::Time time_start{0U};
  /*!
   * \brief OpenCL event execution-end timestamp.
   */
  ggems::units::Time time_end{0U};

  /*!
   * \brief Duration from queueing to completion.
   */
  ggems::units::Duration command_time{0U};
  /*!
   * \brief Kernel execution duration from start to end.
   */
  ggems::units::Duration kernel_time{0U};

  /*!
   * \brief Whether kernel timing information is valid.
   */
  bool valid{false};
};

/*!
 * \brief Measures host elapsed time and optional OpenCL kernel-event timing.
 */
class GGEMSOpenCLProfiler {
public:
  /*!
   * \brief Constructs an empty profiler.
   */
  GGEMSOpenCLProfiler() = default;
  /*!
   * \brief Destroys the profiler.
   */
  ~GGEMSOpenCLProfiler() = default;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler const &) = delete;
  /*!
   * \brief Disables move construction.
   */
  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler &&) = delete;
  /*!
   * \brief Disables copy assignment.
   *
   * \return Reference to this profiler.
   */
  auto operator=(GGEMSOpenCLProfiler const &) -> GGEMSOpenCLProfiler & = delete;
  /*!
   * \brief Disables move assignment.
   *
   * \return Reference to this profiler.
   */
  auto operator=(GGEMSOpenCLProfiler &&) -> GGEMSOpenCLProfiler & = delete;

  /*!
   * \brief Clears all host and kernel timing state.
   */
  auto Reset() noexcept -> void;
  /*!
   * \brief Starts host elapsed-time measurement.
   */
  auto Start() noexcept -> void;
  /*!
   * \brief Stops host elapsed-time measurement.
   */
  auto Stop() noexcept -> void;

  /*!
   * \brief Records profiling timestamps from a completed OpenCL event.
   *
   * \param[in] event OpenCL event whose profiling information is read.
   */
  auto RecordKernelEvent(cl::Event const &event) -> void;

  /*!
   * \brief Checks whether host timing is currently running.
   *
   * \return True while host timing is running.
   */
  [[nodiscard]] auto IsRunning() const noexcept -> bool { return running_; }

  /*!
   * \brief Checks whether a completed host measurement is available.
   *
   * \return True if a host measurement is available.
   */
  [[nodiscard]] auto HasMeasurement() const noexcept -> bool {
    return has_measurement_;
  }

  /*!
   * \brief Checks whether valid kernel-event timing is available.
   *
   * \return True if valid kernel-event timing is available.
   */
  [[nodiscard]] auto HasKernelTiming() const noexcept -> bool {
    return kernel_timing_.valid;
  }

  /*!
   * \brief Returns the measured host elapsed time.
   *
   * \return Measured host elapsed duration.
   */
  [[nodiscard]] auto GetElapsedTime() const noexcept -> ggems::units::Duration;
  /*!
   * \brief Returns the OpenCL kernel execution duration.
   *
   * \return Measured kernel execution duration.
   */
  [[nodiscard]] auto GetKernelTime() const noexcept -> ggems::units::Duration;
  /*!
   * \brief Returns the OpenCL command lifetime from queueing to completion.
   *
   * \return Measured command duration.
   */
  [[nodiscard]] auto GetCommandTime() const noexcept -> ggems::units::Duration;

  /*!
   * \brief Returns the measured host elapsed time in seconds.
   *
   * \return Host elapsed time in seconds.
   */
  [[nodiscard]] auto GetElapsedSeconds() const noexcept -> double;
  /*!
   * \brief Returns the measured kernel time in seconds.
   *
   * \return Kernel execution time in seconds.
   */
  [[nodiscard]] auto GetKernelSeconds() const noexcept -> double;

  /*!
   * \brief Computes a host elapsed-time throughput.
   *
   * \param[in] item_count Number of processed items.
   * \return Items processed per second, or zero when no valid elapsed time is available.
   */
  [[nodiscard]] auto
  ComputeRatePerSecond(std::uint64_t item_count) const noexcept -> double;

  /*!
   * \brief Computes a kernel execution throughput.
   *
   * \param[in] item_count Number of processed items.
   * \return Items processed per kernel second, or zero when no valid kernel time is available.
   */
  [[nodiscard]] auto
  ComputeKernelRatePerSecond(std::uint64_t item_count) const noexcept -> double;

  /*!
   * \brief Returns the stored kernel-event timing record.
   *
   * \return Kernel-event timing record.
   */
  [[nodiscard]] auto GetKernelTiming() const noexcept
      -> GGEMSOpenCLKernelTiming const & {
    return kernel_timing_;
  }

private:
  /*!
   * \brief Returns the host elapsed time in seconds using long-double precision.
   *
   * \return Host elapsed time in seconds.
   */
  [[nodiscard]] auto GetElapsedSecondsRaw() const noexcept -> long double;

  /*!
   * \brief Steady clock used for host elapsed-time measurements.
   */
  using Clock = std::chrono::steady_clock;

  /*!
   * \brief Host timing start point.
   */
  Clock::time_point start_;
  /*!
   * \brief Host timing stop point.
   */
  Clock::time_point stop_;

  /*!
   * \brief Whether host timing is currently active.
   */
  bool running_{false};
  /*!
   * \brief Whether a completed host measurement is available.
   */
  bool has_measurement_{false};

  /*!
   * \brief Most recently recorded kernel-event timing.
   */
  GGEMSOpenCLKernelTiming kernel_timing_{};
};

} // namespace ggems::ocl
