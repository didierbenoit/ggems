#include "GGEMS/core/GGEMSSystemUtils.hh"

namespace ggems::core {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace system {
SystemUsage GetSystemUsage() noexcept;

CPUProcessUsage GetProcessUsage() noexcept;

GPUsage GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept;
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
