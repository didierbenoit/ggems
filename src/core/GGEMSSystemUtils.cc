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
 * \file GGEMSSystemUtils.cc
 * \brief Delegation layer for cross-platform system inspection routines.
 *
 * This implementation forwards requests to the platform-specific
 * backend defined under the \c system namespace. All logic related
 * to CPU, RAM and GPU utilisation metrics is performed in the
 * corresponding OS-dependent translation units.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/core/GGEMSSystemUtils.hh"

namespace ggems::core {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace system {
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
SystemUsage GetSystemUsage() noexcept;

/*!
 * \brief Returns CPU and RAM usage of the current process.
 *
 * Metrics represent CPU activity and resident memory space belonging to
 * the process hosting GGEMS. All values are instantaneous.
 *
 * \return A \ref ggems::core::CPUProcessUsage structure describing per-process
 * CPU and RAM usage.
 */
CPUProcessUsage GetProcessUsage() noexcept;

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
GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid_bytes) noexcept;
} // namespace system

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

SystemUsage GetSystemUsage() noexcept { return system::GetSystemUsage(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

CPUProcessUsage GetProcessUsage() noexcept { return system::GetProcessUsage(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept {
  return system::GetGPUsage(luid);
}
} // namespace ggems::core
