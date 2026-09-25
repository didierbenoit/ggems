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

#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"

namespace ggems::ocl {

/*!
 * \brief Stores timestamp and duration information for one OpenCL kernel event.
 *
 * All integer values use canonical picoseconds. Timestamps refer to the device
 * profiling clock, not simulation time or the host steady clock. Nanosecond
 * values and differences saturate independently at uint64_t maximum when
 * conversion cannot fit; valid indicates successful queries and timestamp
 * ordering, not absence of saturation.
 */
struct GGEMSOpenCLKernelTiming {
  /*! \brief Event queued timestamp. */
  ggems::units::Time time_queued{0U};

  /*! \brief Event submission timestamp. */
  ggems::units::Time time_submit{0U};

  /*! \brief Event execution-start timestamp. */
  ggems::units::Time time_start{0U};

  /*! \brief Event execution-end timestamp. */
  ggems::units::Time time_end{0U};

  /*! \brief Queue-to-completion duration. */
  ggems::units::Duration command_time{0U};

  /*! \brief Kernel execution duration. */
  ggems::units::Duration kernel_time{0U};

  /*! \brief Whether kernel timing is valid. */
  bool valid{false};
};

/*!
 * \brief Measures host elapsed time and optional OpenCL kernel-event timing.
 */
class GGEMSOpenCLProfiler {
public:
  /*! \brief Constructs an empty profiler. */
  GGEMSOpenCLProfiler() = default;

  /*! \brief Destroys the profiler. */
  ~GGEMSOpenCLProfiler() = default;

  /*! \brief Disables copy construction. */
  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler const &) = delete;

  /*! \brief Disables move construction. */
  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler &&) = delete;

  /*! \brief Disables copy assignment. */
  auto operator=(GGEMSOpenCLProfiler const &) -> GGEMSOpenCLProfiler & = delete;

  /*! \brief Disables move assignment. */
  auto operator=(GGEMSOpenCLProfiler &&) -> GGEMSOpenCLProfiler & = delete;

  /*! \brief Clears all host and kernel timing state. */
  auto Reset() noexcept -> void;

  /*!
   * \brief Starts a fresh host measurement and clears previous timing results.
   */
  auto Start() noexcept -> void;

  /*!
   * \brief Completes a running host measurement; otherwise leaves it unchanged.
   */
  auto Stop() noexcept -> void;

  /*!
   * \brief Records profiling timestamps from a completed OpenCL event.
   *
   * \param[in] event OpenCL event whose profiling information is read.
   *
   * Requires a completed event with profiling information available. A failed
   * query or ordering check leaves the previous timing record unchanged.
   * Recording does not wait for the event.
   *
   * \throws ggems::core::GGEMSFatal If any profiling query fails.
   * \throws ggems::core::GGEMSRecoverable If queued, submitted, started, and
   * ended timestamps are not nondecreasing.
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
   * \return Completed host duration rounded to picoseconds, saturated on
   * conversion failure; zero before Stop() completes a measurement.
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
   * \return Completed host interval in seconds, or zero while running or before
   * a measurement.
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
   * \return Items processed per second, or zero when no valid elapsed time is
   * available.
   */
  [[nodiscard]] auto
  ComputeRatePerSecond(std::uint64_t item_count) const noexcept -> double;

  /*!
   * \brief Computes a kernel execution throughput.
   *
   * \param[in] item_count Number of processed items.
   * \return Items processed per kernel second, or zero when no valid kernel
   * time is available.
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
   * \brief Returns the host elapsed time in seconds using long-double
   * precision.
   *
   * \return Host elapsed time in seconds.
   */
  [[nodiscard]] auto GetElapsedSecondsRaw() const noexcept -> long double;

  /*! \brief Host timing start point. */
  std::chrono::steady_clock::time_point start_;

  /*! \brief Host timing stop point. */
  std::chrono::steady_clock::time_point stop_;

  /*! \brief Whether host timing is active. */
  bool running_{false};

  /*! \brief Whether a host measurement is available. */
  bool has_measurement_{false};

  /*! \brief Last kernel-event timing. */
  GGEMSOpenCLKernelTiming kernel_timing_{};
};

} // namespace ggems::ocl
