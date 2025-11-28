#include "GGEMS/core/GGEMSSystemUtils.hh"

#if defined(__linux__)

namespace ggems::core::system {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<uint32_t> GetCPUFrequencyMHz() noexcept { return std::nullopt; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GPUUsage QueryGPUUsage(std::array<std::uint8_t, 8> &luid_bytes) noexcept {
  return GPUUsage{};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<GPUUsage> QueryMultiGPUUsage(
    std::vector<std::array<std::uint8_t, 8>> const &luids) noexcept {
  ;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<SystemUsage> GetSystemUsage() noexcept { return std::nullopt; }
return std::nullopt;
} // namespace ggems::core::system

#endif
