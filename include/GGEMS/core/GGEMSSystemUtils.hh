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
 * \struct CPURAMProcessUsage
 * \brief Per-process physical memory usage for the running application.
 *
 * These fields quantify memory pages attributable to the current process.
 * Both values are expressed in bytes.
 */
struct CPURAMProcessUsage {
  std::uint64_t working_set_size; /*!< Resident memory pages actively mapped
                                     into RAM (bytes) */
  std::uint64_t private_mem; /*!< Privately allocated RAM not shared with other
                                processes (bytes) */
};

/*!
 * \struct CPUProcessUsage
 * \brief CPU utilisation of the current process.
 *
 * CPU usage is expressed as a percentage in [0,100] and corresponds to
 * kernel-level statistics for the executing process.
 */
struct CPUProcessUsage {
  std::uint8_t cpu_percent; /*!< CPU load of the running process (0–100) */
  CPURAMProcessUsage ram;   /*!< RAM footprint of the same process */
};

/*!
 * \struct GPURAMProcessUsage
 * \brief GPU video memory usage attributable to the current process.
 *
 * All three fields describe memory reported by the graphics driver as
 * belonging to the process hosting GGEMS. Units are bytes, and percentage
 * spans [0,100].
 */
struct GPURAMProcessUsage {
  std::uint64_t total;  /*!< Total visible VRAM (bytes) */
  std::uint64_t used;   /*!< VRAM usage attributable to the process (bytes) */
  std::uint8_t percent; /*!< VRAM utilisation for the process (0–100) */
};

/*!
 * \struct GPUsage
 * \brief GPU utilisation metrics for the current process.
 *
 * Includes instantaneous GPU load and VRAM usage. Percentages are always
 * in the interval [0,100].
 */
struct GPUsage {
  std::uint8_t gpu_percent; /*!< GPU load attributed to the process (0–100) */
  GPURAMProcessUsage ram;   /*!< GPU VRAM usage belonging to the process */
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
 * \brief Returns CPU and RAM usage of the current process.
 *
 * Metrics represent CPU activity and resident memory space belonging to
 * the process hosting GGEMS. All values are instantaneous.
 *
 * \return A \ref ggems::core::CPUProcessUsage structure describing per-process
 * CPU and RAM usage.
 */
[[nodiscard]] CPUProcessUsage GetProcessUsage() noexcept;

/*!
 * \brief Returns GPU utilisation and VRAM occupancy of the current process.
 *
 * Values represent GPU workload attributed to the process executed by
 * GGEMS. VRAM metrics quantify memory space reserved by the same process.
 *
 * \param luid_bytes Logical Unique Identifier used to identify the GPU device.
 *                   This LUID is typically retrieved via OpenCL.
 *
 * \return A \ref ggems::core::GPUsage structure with GPU load and process VRAM
 * metrics. No field is optional: if not supported by the platform, zeroed
 *         fields are returned.
 */
[[nodiscard]] GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid_bytes) noexcept;

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
