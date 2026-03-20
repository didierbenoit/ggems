#if defined(__linux__)

/// \cond
#include <fstream>
#include <sstream>
/// \endcond

#include "GGEMS/platform/posix/GGEMSPosixCore.hh"
#include "GGEMS/core/GGEMSSystemUtils.hh"

namespace ggems::core::system {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
struct LinuxCPUStat {
  std::uint64_t user{0ULL};
  std::uint64_t nice{0ULL};
  std::uint64_t system{0ULL};
  std::uint64_t idle{0ULL};
  std::uint64_t iowait{0ULL};
  std::uint64_t irq{0ULL};
  std::uint64_t softirq{0ULL};
  std::uint64_t steal{0ULL};
};
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
struct LinuxProcessCPUTime {
  std::uint64_t utime_ticks{0ULL};
  std::uint64_t stime_ticks{0ULL};
};
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::optional<LinuxCPUStat> ReadLinuxCPUStat() noexcept {
  std::ifstream file("/proc/stat");
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string cpu_label;
  LinuxCPUStat stat;

  file >> cpu_label >> stat.user >> stat.nice >> stat.system >> stat.idle >>
      stat.iowait >> stat.irq >> stat.softirq >> stat.steal;

  if (!file || cpu_label != "cpu") {
    return std::nullopt;
  }

  return stat;
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::uint64_t TotalCPUTime(LinuxCPUStat const &stat) noexcept {
  return stat.user + stat.nice + stat.system + stat.idle + stat.iowait +
         stat.irq + stat.softirq + stat.steal;
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::uint64_t IdleCPUTime(LinuxCPUStat const &stat) noexcept {
  return stat.idle + stat.iowait;
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::uint8_t QueryCPUPercent() noexcept {
  static LinuxCPUStat last_stat{};
  static bool first_cpu{true};

  auto current_opt = ReadLinuxCPUStat();
  if (!current_opt.has_value()) {
    return 0;
  }

  LinuxCPUStat current = *current_opt;

  if (first_cpu) {
    last_stat = current;
    first_cpu = false;
    return 0;
  }

  std::uint64_t total_d = TotalCPUTime(current) - TotalCPUTime(last_stat);
  std::uint64_t idle_d = IdleCPUTime(current) - IdleCPUTime(last_stat);

  last_stat = current;

  if (total_d == 0) {
    return 0;
  }

  std::uint64_t busy_d = total_d - idle_d;
  return static_cast<std::uint8_t>((100ULL * busy_d) / total_d);
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::optional<std::uint32_t>
ReadKHzFileAsMHz(char const *path) noexcept {
  std::ifstream file(path);
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::uint64_t khz = 0;
  file >> khz;
  if (!file || khz == 0) {
    return std::nullopt;
  }

  return static_cast<std::uint32_t>(khz / 1000ULL);
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::optional<std::uint32_t> ReadProcCPUInfoMHz() noexcept {
  std::ifstream file("/proc/cpuinfo");
  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.rfind("cpu MHz", 0) == 0) {
      auto pos = line.find(':');
      if (pos == std::string::npos) {
        continue;
      }

      try {
        double mhz = std::stod(line.substr(pos + 1));
        if (mhz > 0.0) {
          return static_cast<std::uint32_t>(mhz);
        }
      } catch (...) {
        return std::nullopt;
      }
    }
  }

  return std::nullopt;
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static std::optional<uint32_t> GetCPUFrequencyMHz() noexcept {
  if (auto mhz = ReadKHzFileAsMHz(
          "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq")) {
    return mhz;
  }

  if (auto mhz = ReadKHzFileAsMHz(
          "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq")) {
    return mhz;
  }

  return ReadProcCPUInfoMHz();
}
/// \endcond

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/// \cond
static RAMUsage QueryRAMStatus() noexcept {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    return {0ULL, 0ULL, 0ULL, 0U};
  }

  std::uint64_t mem_total_kb{0ULL};
  std::uint64_t mem_available_kb{0ULL};

  std::string key;
  std::uint64_t value{0ULL};
  std::string unit;

  while (file >> key >> value >> unit) {
    if (key == "MemTotal:") {
      mem_total_kb = value;
    } else if (key == "MemAvailable:") {
      mem_available_kb = value;
    }

    if (mem_total_kb != 0ULL && mem_available_kb != 0LL) {
      break;
    }
  }

  if (mem_total_kb == 0ULL) {
    return {0ULL, 0ULL, 0ULL, 0U};
  }

  std::uint64_t total{mem_total_kb * 1024ULL};
  std::uint64_t available{mem_available_kb * 1024ULL};
  std::uint64_t used = (available <= total) ? (total - available) : 0ULL;

  std::uint8_t percent = static_cast<std::uint8_t>((100ULL * used) / total);

  return RAMUsage{total, available, used, percent};
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
