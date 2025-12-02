#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/platform/windows/GGEMSWindowsGPU.hh"
#include <combaseapi.h>
#include <dxgi.h>

namespace ggems::core::system {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static std::uint8_t QueryProcessCPUPercent() noexcept {
  static bool first{true};
  static FILETIME last_kernel{}, last_user{};
  static auto last_time = std::chrono::steady_clock::now();

  FILETIME creation, exit, kernel, user;

  if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user)) {
    return 0;
  }

  auto now = std::chrono::steady_clock::now();

  if (first) {
    first = false;
    last_kernel = kernel;
    last_user = user;
    last_time = now;
    return 0;
  }

  double dt = std::chrono::duration<double>(now - last_time).count();
  if (dt <= 0.0) {
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

  std::uint64_t dk = diff(kernel, last_kernel);
  std::uint64_t du = diff(user, last_user);

  last_kernel = kernel;
  last_user = user;
  last_time = now;

  // 100 ns → secondes
  std::uint64_t total = dk + du;
  double busy_seconds = static_cast<double>(total) * 1e-7;

  double pct = (busy_seconds / dt) * 100.0;
  if (pct < 0.0)
    pct = 0.0;
  if (pct > 100.0)
    pct = 100.0;

  return static_cast<std::uint8_t>(pct);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static CPURAMProcessUsage QueryProcessRAM() noexcept {
  PROCESS_MEMORY_COUNTERS_EX pmc{};
  if (GetProcessMemoryInfo(GetCurrentProcess(),
                           reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc),
                           sizeof(pmc))) {
    return CPURAMProcessUsage{pmc.WorkingSetSize, pmc.PrivateUsage};
  } else {
    return CPURAMProcessUsage{0ULL, 0LL};
  }
}

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

struct GPUStateCache {
  bool init = false;
  uint64_t last_running = 0;
  std::chrono::steady_clock::time_point last;
};

static GPUStateCache gpu_cache[16];

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static LUID
ToWindowsLUID(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept {
  LUID id{};
  std::memcpy(&id, luid.data(), sizeof(LUID));
  return id;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static std::uint8_t QueryGPUPercent_D3DKMT(LUID luid,
                                           GPUStateCache &cache) noexcept {
  D3DKMT_QUERYSTATISTICS s{};
  s.Type = D3DKMT_QUERYSTATISTICS_NODE;
  s.AdapterLuid = luid;
  s.hProcess = 0;
  s.QueryNode.NodeId = 0;

  if (D3DKMTQueryStatistics(&s) != 0)
    return 0;

  uint64_t now_run = static_cast<uint64_t>(
      s.QueryResult.NodeInformation.GlobalInformation.RunningTime.QuadPart);

  auto now = std::chrono::steady_clock::now();

  if (!cache.init) {
    cache.init = true;
    cache.last_running = now_run;
    cache.last = now;
    return 0;
  }

  double dt = std::chrono::duration<double>(now - cache.last).count();
  cache.last = now;

  if (dt <= 0.0)
    return 0;

  uint64_t delta = now_run - cache.last_running;
  cache.last_running = now_run;

  double busy = double(delta) * 1e-7; // 100ns → seconds
  double pct = (busy / dt) * 100.0;

  if (pct < 0.0)
    pct = 0.0;
  if (pct > 100.0)
    pct = 100.0;

  return static_cast<uint8_t>(pct);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static GPURAMProcessUsage QueryVRAMUsed_DXGI(LUID const &luid) noexcept {
  IDXGIFactory4 *factory = nullptr;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
    return GPURAMProcessUsage{0ULL, 0LL, 0};
#pragma clang diagnostic pop

  IDXGIAdapter3 *adapter3 = nullptr;

  for (UINT i = 0;; ++i) {
    IDXGIAdapter1 *ad1 = nullptr;
    if (factory->EnumAdapters1(i, &ad1) == DXGI_ERROR_NOT_FOUND)
      break;

    DXGI_ADAPTER_DESC1 desc{};
    ad1->GetDesc1(&desc);

    if (desc.AdapterLuid.LowPart == luid.LowPart &&
        desc.AdapterLuid.HighPart == luid.HighPart) {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
      if (SUCCEEDED(ad1->QueryInterface(IID_PPV_ARGS(&adapter3)))) {
        ad1->Release();
        break;
      }
#pragma clang diagnostic pop
    }
    ad1->Release();
  }

  factory->Release();

  if (!adapter3)
    return GPURAMProcessUsage{0LL, 0LL, 0};

  DXGI_QUERY_VIDEO_MEMORY_INFO info{};
  if (FAILED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL,
                                            &info))) {
    adapter3->Release();
    return GPURAMProcessUsage{0LL, 0LL, 0};
  }

  DXGI_ADAPTER_DESC1 desc_final{};
  if (FAILED(adapter3->GetDesc1(&desc_final))) {
    adapter3->Release();
    return GPURAMProcessUsage{0LL, 0LL, 0};
  }

  std::uint64_t used = info.CurrentUsage;
  std::uint64_t total =
      static_cast<std::uint64_t>(desc_final.DedicatedVideoMemory);
  std::uint8_t percent = static_cast<std::uint8_t>(
      static_cast<double>(used) * 100.0 / static_cast<double>(total));

  adapter3->Release();

  return GPURAMProcessUsage{total, used, percent};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GPUsage
GetGPUsage(std::array<cl_uchar, CL_LUID_SIZE_KHR> const &luid) noexcept {
  GPUsage out;

  // reinterpret id as LUID
  LUID luid_win = ToWindowsLUID(luid);
  std::uint32_t mix = static_cast<std::uint32_t>(luid_win.LowPart) ^
                      static_cast<std::uint32_t>(luid_win.HighPart);

  int index = static_cast<int>(mix & 15u);

  out.gpu_percent_ = QueryGPUPercent_D3DKMT(luid_win, gpu_cache[index]);
  out.ram_ = QueryVRAMUsed_DXGI(luid_win);

  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

CPUProcessUsage GetProcessUsage() noexcept {
  return CPUProcessUsage{QueryProcessCPUPercent(), QueryProcessRAM()};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

SystemUsage GetSystemUsage() noexcept {
  return SystemUsage{QueryCPUPercent(), QueryRAMStatus(), GetCPUFrequencyMHz()};
}
} // namespace ggems::core::system
