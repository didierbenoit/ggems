#pragma once

// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSCoreUtils.hh
 * \brief
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-11-8
 * \copyright GNU General Public License v3.0
 * \version 2.0
 *
 */

/// \cond
#include <atomic>
#include <format>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#if defined(_WIN32)
#include <intrin.h>
#include <windows.h>
#elif defined(__linux__)
#include <fstream>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#endif
/// \endcond

namespace ggems::core {
[[nodiscard]]
inline std::string ThreadTag() {
  static std::atomic<unsigned> next{0};
  static std::mutex m;
  static std::unordered_map<std::thread::id, unsigned> map;

  thread_local unsigned idx = [&] {
    std::scoped_lock lock(m);
    auto [it, inserted] = map.emplace(std::this_thread::get_id(), next++);
    return it->second;
  }();

  return std::format("T{}", idx);
}

[[nodiscard]]
inline std::string_view SimplifyFunctionName(std::string_view full) noexcept {
  // Step 1: Remove parameters "(...)"
  if (auto pos = full.find('('); pos != std::string_view::npos) {
    full = full.substr(0, pos);
  }

  // Step 2: Remove "<...>"
  if (auto pos = full.find('<'); pos != std::string_view::npos) {
    full = full.substr(0, pos);
  }

  // Step 3: Remove calling convention (e.g. "__cdecl ")
  if (auto pos = full.rfind(' '); pos != std::string_view::npos) {
    full = full.substr(pos + 1);
  }

  // Step 3: Remove namespace
  constexpr std::string_view root = "ggems::";
  if (full.starts_with(root)) {
    full = full.substr(root.size());

    if (auto pos = full.find("::"); pos != std::string_view::npos) {
      full = full.substr(pos + 2);
    }
  }

  // Step 4: Keep only the last two identifiers (Class::Method)
  auto last = full.rfind("::");
  if (last == std::string_view::npos)
    return full;

  if (last < 2)
    return full; // Safety check

  auto prev = full.rfind("::", last - 2);
  if (prev == std::string_view::npos)
    return full;

  return full.substr(prev + 2);
}

[[nodiscard]] inline std::optional<std::uint32_t>
GetCPUFrequencyMHz() noexcept {
#if defined(_WIN32)
  // --- Windows ---
  HKEY key;
  DWORD mhz = 0;
  DWORD size = sizeof(mhz);
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

#elif defined(__linux__)
  // --- Linux ---
  std::ifstream cpuinfo("/proc/cpuinfo");
  std::string line;
  while (std::getline(cpuinfo, line)) {
    if (line.starts_with("cpu MHz")) {
      auto pos = line.find(':');
      if (pos != std::string::npos) {
        try {
          double freq = std::stod(line.substr(pos + 1));
          return static_cast<std::uint32_t>(freq);
        } catch (...) {
          return std::nullopt;
        }
      }
    }
  }
  return std::nullopt;

#elif defined(__APPLE__)
  // --- macOS ---
  std::uint64_t hz = 0;
  std::size_t size = sizeof(hz);
  if (sysctlbyname("hw.cpufrequency", &hz, &size, nullptr, 0) == 0)
    return static_cast<std::uint32_t>(hz / 1'000'000);
  return std::nullopt;
#else
  return std::nullopt;
#endif
}
} // namespace ggems::core
