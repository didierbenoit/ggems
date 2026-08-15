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
 * \brief Implements host and OpenCL kernel profiling utilities.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <limits>
#include <cstdint>
#include <chrono>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::ocl {

namespace {

/*!
 * \brief Number of picoseconds in one nanosecond.
 */
constexpr std::uint64_t k_picoseconds_per_nanosecond{1000ULL};

// =============================================================================
// =============================================================================

/*!
 * \brief Converts a nonnegative duration in seconds to GGEMS picoseconds.
 *
 * \param[in] seconds Duration in seconds.
 * \return GGEMS duration rounded to picoseconds and saturated to its storage range.
 */
auto MakeDurationFromSeconds(long double seconds) noexcept
    -> ggems::units::Duration {
  if (seconds < 0.0L) {
    return ggems::units::Duration{0U};
  }

  auto const duration =
      ggems::units::TryMakeQuantity<ggems::units::Duration>(seconds, "s");

  return duration.value_or(
      ggems::units::Duration{std::numeric_limits<std::uint64_t>::max()});
}

// =============================================================================
// =============================================================================

/*!
 * \brief Converts an OpenCL nanosecond timestamp to a GGEMS picosecond time point.
 *
 * \param[in] nanoseconds OpenCL profiling timestamp in nanoseconds.
 * \return GGEMS time point in picoseconds, saturated to its storage range.
 */
auto MakeTimeFromNanoseconds(cl_ulong nanoseconds) noexcept
    -> ggems::units::Time {
  if (nanoseconds > std::numeric_limits<std::uint64_t>::max() /
                        k_picoseconds_per_nanosecond) {
    return ggems::units::Time{std::numeric_limits<std::uint64_t>::max()};
  }

  auto const time =
      ggems::units::TryMakeQuantity<ggems::units::Time>(nanoseconds, "ns");

  return time.value_or(
      ggems::units::Time{std::numeric_limits<std::uint64_t>::max()});
}

} // namespace

// =============================================================================
// =============================================================================

auto GGEMSOpenCLProfiler::Reset() noexcept -> void {
  start_ = Clock::time_point{};
  stop_ = Clock::time_point{};
  running_ = false;
  has_measurement_ = false;
  kernel_timing_ = GGEMSOpenCLKernelTiming{};
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::Start() noexcept -> void {
  start_ = Clock::now();
  stop_ = Clock::time_point{};
  running_ = true;
  has_measurement_ = false;
  kernel_timing_ = GGEMSOpenCLKernelTiming{};
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::Stop() noexcept -> void {
  if (!running_) {
    return;
  }

  stop_ = Clock::now();
  running_ = false;
  has_measurement_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::RecordKernelEvent(cl::Event const &event) -> void {
  if (running_) {
    throw ggems::core::GGEMSRecoverable(
        "OpenCL kernel event profiling must be recorded after Stop() when host "
        "timing is active.");
  }

  cl_int error{0};

  cl_ulong queued = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(&error);
  CheckCLError(error, "Failed to get OpenCL command queue timestamp.");

  cl_ulong submit = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>(&error);
  CheckCLError(error, "Failed to get OpenCL command submit timestamp.");

  cl_ulong start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&error);
  CheckCLError(error, "Failed to get OpenCL command start timestamp.");

  cl_ulong end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&error);
  CheckCLError(error, "Failed to get OpenCL command end timestamp.");

  if (queued > submit || submit > start || start > end) {
    throw ggems::core::GGEMSRecoverable(
        "Invalid OpenCL profiling timestamps ordering.");
  }

  kernel_timing_.time_queued = MakeTimeFromNanoseconds(queued);
  kernel_timing_.time_submit = MakeTimeFromNanoseconds(submit);
  kernel_timing_.time_start = MakeTimeFromNanoseconds(start);
  kernel_timing_.time_end = MakeTimeFromNanoseconds(end);

  kernel_timing_.command_time = MakeTimeFromNanoseconds(end - queued);
  kernel_timing_.kernel_time = MakeTimeFromNanoseconds(end - start);

  kernel_timing_.valid = true;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetElapsedSecondsRaw() const noexcept -> long double {
  if (!has_measurement_) {
    return 0.0L;
  }

  auto elapsed = std::chrono::duration<long double>(stop_ - start_);

  return elapsed.count();
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetElapsedTime() const noexcept
    -> ggems::units::Duration {
  return MakeDurationFromSeconds(GetElapsedSecondsRaw());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetKernelTime() const noexcept
    -> ggems::units::Duration {
  return kernel_timing_.kernel_time;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetCommandTime() const noexcept
    -> ggems::units::Time {
  return kernel_timing_.command_time;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetElapsedSeconds() const noexcept -> double {
  return static_cast<double>(GetElapsedSecondsRaw());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::GetKernelSeconds() const noexcept -> double {
  if (!kernel_timing_.valid) {
    return 0.0;
  }

  auto const seconds =
      ggems::units::TryConvertTo(kernel_timing_.kernel_time, "s");

  return seconds.has_value() ? static_cast<double>(*seconds) : 0.0;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::ComputeRatePerSecond(
    std::uint64_t item_count) const noexcept -> double {
  double elapsed_seconds = GetElapsedSeconds();

  if (elapsed_seconds <= 0.0) {
    return 0.0;
  }

  return static_cast<double>(item_count) / elapsed_seconds;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLProfiler::ComputeKernelRatePerSecond(
    std::uint64_t item_count) const noexcept -> double {
  double const kernel_seconds = GetKernelSeconds();

  if (kernel_seconds <= 0.0) {
    return 0.0;
  }

  return static_cast<double>(item_count) / kernel_seconds;
}
} // namespace ggems::ocl
