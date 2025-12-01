#include "GGEMS/core/GGEMSSystemUtils.hh"

namespace ggems::core {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace system {
SystemUsage GetSystemUsage() noexcept;

std::optional<GPUsage>
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept;
} // namespace system

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

SystemUsage GetSystemUsage() noexcept { return system::GetSystemUsage(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::optional<GPUsage>
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept {
  return system::GetGPUsage(luid);
}
} // namespace ggems::core
