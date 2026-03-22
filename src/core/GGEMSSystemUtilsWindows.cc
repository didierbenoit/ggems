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
 * \file GGEMSSystemUtilsWindows.cc
 * \brief Windows-specific implementations for system CPU, RAM and GPU metrics.
 *
 * This translation unit provides the Win32 and DXGI/D3DKMT backends
 * used by \ref ggems::core::GetSystemUsage(),
 * \ref ggems::core::GetProcessUsage() and
 * \ref ggems::core::GetGPUsage().
 *
 * It extracts system-wide and per-process utilisation metrics
 * using native WinAPI calls and low-level driver interfaces.
 * No OS abstraction is performed here: all logic is Windows-only.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#if defined(_WIN32)
#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"

namespace ggems::core::system {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::uint8_t QueryCPUPercent() noexcept {
  static FILETIME last_idle{}, last_kernel{}, last_user{};
  static bool first_cpu = true;

  FILETIME idle{}, kernel{}, user{};
  if (!GetSystemTimes(&idle, &kernel, &user))
    return 0;

  if (first_cpu) {
    last_kernel = kernel;
    last_idle = idle;
    last_user = user;
    first_cpu = false;
    return 0;
  }

  auto diff = [](FILETIME a, FILETIME b) {
    ULARGE_INTEGER x{};
    x.LowPart = a.dwLowDateTime;
    x.HighPart = a.dwHighDateTime;

    ULARGE_INTEGER y{};
    y.LowPart = b.dwLowDateTime;
    y.HighPart = b.dwHighDateTime;

    return x.QuadPart - y.QuadPart;
  };

  std::uint64_t idle_d = diff(idle, last_idle);
  std::uint64_t kernel_d = diff(kernel, last_kernel);
  std::uint64_t user_d = diff(user, last_user);

  last_idle = idle;
  last_kernel = kernel;
  last_user = user;

  uint64_t total = kernel_d + user_d;
  if (total == 0) {
    return 0;
  }

  uint64_t busy = total - idle_d;
  return static_cast<uint8_t>((100ULL * busy) / total);
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static RAMUsage QueryRAMStatus() noexcept {
  MEMORYSTATUSEX mem{};
  mem.dwLength = sizeof(mem);
  if (!GlobalMemoryStatusEx(&mem))
    return {0, 0ULL, 0LL, 0LL};

  std::uint64_t total = mem.ullTotalPhys;
  std::uint64_t available = mem.ullAvailPhys;
  std::uint64_t used = total - available;

  std::uint8_t percent = static_cast<std::uint8_t>((100ULL * used) / total);

  return RAMUsage{total, available, used, percent};
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
std::optional<uint32_t> GetCPUFrequencyMHz() noexcept {
  HKEY key;
  DWORD mhz = 0, size = sizeof(mhz);

  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                    KEY_READ, &key) == ERROR_SUCCESS) {
    if (RegQueryValueExA(key, "~MHz", nullptr, nullptr,
                         reinterpret_cast<LPBYTE>(&mhz),
                         &size) == ERROR_SUCCESS) {
      RegCloseKey(key);
      return mhz;
    }
    RegCloseKey(key);
  }

  return std::nullopt;
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
SystemUsage GetSystemUsage() noexcept {
  return SystemUsage{QueryCPUPercent(), QueryRAMStatus(), GetCPUFrequencyMHz()};
}
/// \endcond
} // namespace ggems::core::system

#endif
