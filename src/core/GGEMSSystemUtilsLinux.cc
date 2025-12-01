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

GPUUsage QueryGPUUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> &luid) noexcept {
  return GPUUsage{};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<SystemUsage> GetSystemUsage() noexcept { return std::nullopt; }
return std::nullopt;
} // namespace ggems::core::system

#endif
