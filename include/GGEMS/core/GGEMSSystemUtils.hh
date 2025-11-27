#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
/// \endcond

namespace ggems::core {
enum class OS : std::uint8_t { Win, Linux, Apple };

struct SystemUsage {
  std::uint8_t cpu_percent_;
  std::uint8_t ram_percent_;
  std::uint64_t ram_total_bytes_;
  std::uint64_t ram_used_bytes_;
};

struct GPUUsage {
  std::optional<std::uint8_t> gpu_percent_;
  std::uint64_t vram_total_bytes_;
  std::uint64_t vram_used_bytes_;
};

[[nodiscard]] std::optional<SystemUsage> GetSystemUsage() noexcept;

[[nodiscard]] GPUUsage
QueryGPUUsage(std::array<std::uint8_t, 8> &luid_bytes) noexcept;

[[nodiscard]] std::vector<GPUUsage> QueryMultiGPUUsage(
    std::vector<std::array<std::uint8_t, 8>> const &luids) noexcept;

[[nodiscard]] std::optional<std::uint64_t> GetCPUFrequencyMHz() noexcept;

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
