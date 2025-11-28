#include "GGEMS/core/GGEMSSystemUtils.hh"

/// \cond
#include "windows.h"
/// \endcond

namespace ggems::core::system {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GPUUsage QueryGPUUsage(std::array<std::uint8_t, 8> &luid_bytes) noexcept {
  (void)luid_bytes;
  return GPUUsage{};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<GPUUsage> QueryMultiGPUUsage(
    std::vector<std::array<std::uint8_t, 8>> const &luids) noexcept {
  (void)luids;
  std::vector<GPUUsage> out;
  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<SystemUsage> GetSystemUsage() noexcept { return std::nullopt; }
} // namespace ggems::core::system
