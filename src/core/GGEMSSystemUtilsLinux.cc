#if defined(__linux__)

#include "GGEMS/core/GGEMSSystemUtils.hh"

namespace ggems::core::system {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept {
  (void)luid;
  return GPUsage{0, GPURAMProcessUsage{0LL, 0LL, 0}};
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
CPUProcessUsage GetProcessUsage() noexcept {
  return CPUProcessUsage{0, CPURAMProcessUsage{0ULL, 0ULL}};
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
SystemUsage GetSystemUsage() noexcept {
  return SystemUsage{0, RAMUsage{0ULL, 0ULL, 0ULL, 0}, 0};
}
/// \endcond
} // namespace ggems::core::system

#endif
