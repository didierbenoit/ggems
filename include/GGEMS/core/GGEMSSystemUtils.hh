#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSSystemUtils.hh
 * \brief Cross-platform system usage interfaces for CPU, RAM and GPU metrics.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 *
 * This header exposes lightweight platform-agnostic utilities for
 * retrieving system-level and per-process resource consumption metrics.
 * These inspections are intended for runtime profiling, monitoring of
 * GPU/CPU workloads, adaptive configuration, and high-performance
 * simulations running under GGEMS.
 *
 * Metrics are sampled at the moment of the call and provide a concise
 * snapshot of the current utilisation state. Availability depends on
 * the underlying operating system, and certain values may only be
 * supported on Windows at this stage.
 */

/// \cond
#include <array>
#include <cstdint>
#include <optional>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"

namespace ggems::core {
/*!
 * \enum OS
 * \brief Enumerates operating systems detected at compile/runtime.
 *
 * The returned value from \ref ggems::core::DetectOS enables the caller to
 * select appropriate inspection routines for CPU, RAM and GPU metrics. The
 * enumeration reflects platforms for which GGEMS is programmed to
 * provide dedicated monitoring logic.
 */
enum class OS : std::uint8_t {
  Win,   /*!< Microsoft Windows */
  Linux, /*!< GNU/Linux */
  Apple  /*!< Apple macOS */
};

/*!
 * \struct RAMUsage
 * \brief Global system memory statistics.
 *
 * All values represent physical RAM at the system scale, not the
 * memory footprint of the current process. Fields are expressed in
 * bytes, except \ref percent, which is a percentage in [0,100].
 */
struct RAMUsage {
  std::uint64_t total;     /*!< Physical RAM installed (bytes) */
  std::uint64_t available; /*!< Currently available RAM (bytes) */
  std::uint64_t used;      /*!< System RAM usage (bytes) */
  std::uint8_t percent;    /*!< RAM utilisation percentage (system-wide) */
};

/*!
 * \struct SystemUsage
 * \brief Global CPU utilisation, RAM usage, and CPU frequency (if available).
 *
 * Instances describe an instantaneous snapshot of total CPU load,
 * system-level RAM occupation, and nominal CPU frequency when retrievable.
 * CPU percentages span [0,100].
 */
struct SystemUsage {
  std::uint8_t cpu_percent; /*!< Global CPU load (0–100) */
  RAMUsage ram;             /*!< System RAM usage metrics */
  std::optional<std::uint32_t>
      cpu_frequency; /*!< Nominal CPU frequency in MHz */
};

/*!
 * \brief Returns global system utilisation metrics.
 *
 * The returned structure describes CPU load, memory occupation, and CPU
 * frequency (when retrievable) at the system-wide scale. These values do
 * not represent the footprint of the current process.
 *
 * \return A fully populated \ref ggems::core::SystemUsage instance describing
 * CPU/RAM state. Values are always provided.
 */
[[nodiscard]] SystemUsage GetSystemUsage() noexcept;

/*!
 * \brief Detects the operating system at compile-time.
 *
 * This function resolves the underlying OS through predefined compiler
 * macros. Detection is performed at compile-time, and the result determines
 * which system inspection routines are invoked.
 *
 * \return One of the enumerators of \ref ggems::core::OS : Win, Linux or Apple.
 */
[[nodiscard]] constexpr OS DetectOS() noexcept {
#if defined(_WIN32)
  return OS::Win;
#elif defined(__linux__)
  return OS::Linux;
#elif defined(__APPLE__)
  return OS::Apple;
#else
  return OS::Linux;
#endif
}
} // namespace ggems::core
