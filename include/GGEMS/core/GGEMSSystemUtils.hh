#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <optional>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"

namespace ggems::core {
enum class OS : std::uint8_t { Win, Linux, Apple };

// ----------- Global system -----------------
struct RAMUsage {
  std::uint64_t total_;     // bytes
  std::uint64_t available_; // bytes
  std::uint64_t used_;      // bytes
  std::uint8_t percent_;    // 0–100
};

struct SystemUsage {
  std::uint8_t cpu_percent_; // 0-100
  RAMUsage ram_;
  std::optional<std::uint32_t> cpu_frequency_; // MHz
};

// ----------- Only Process ----------------------
// CPU
struct CPURAMProcessUsage {
  std::uint64_t working_set_size_; // bytes
  std::uint64_t private_;          // bytes
};

struct CPUProcessUsage {
  std::uint8_t cpu_percent_; // 0 - 100
  CPURAMProcessUsage ram_;
};

// GPU
struct GPURAMProcessUsage {
  std::uint64_t total_;
  std::uint64_t used_;
  std::uint8_t percent_;
};

struct GPUsage {
  std::uint8_t gpu_percent_;
  GPURAMProcessUsage ram_;
};

[[nodiscard]] SystemUsage GetSystemUsage() noexcept;

[[nodiscard]] CPUProcessUsage GetProcessUsage() noexcept;

[[nodiscard]] GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid_bytes) noexcept;

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
