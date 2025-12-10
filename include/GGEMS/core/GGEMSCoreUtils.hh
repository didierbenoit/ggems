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
 * \brief Small collection of lightweight utility functions for string
 *        manipulation, hashing, and logging support inside GGEMS.
 *
 * This header provides reusable helpers designed to support the core
 * engine: ASCII lowercasing, thread-safe tagging, function name
 * simplification for logs, and a 64-bit FNV-1a hashing routine useful
 * for identifiers, kernel names, or internal dictionaries.
 *
 * The utilities are intentionally minimal and header-only to avoid
 * overhead. They may be extended in future versions of GGEMS to support
 * Vulkan/OpenCL interop naming, symbol mangling analysis, or lightweight
 * compile-time introspection.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

/// \cond
#include <format>
#include <mutex>
#include <string>
#include <unordered_map>
#include <atomic>
#include <thread>
/// \endcond

namespace ggems::core {

/*!
 * \brief Converts an ASCII string to lowercase.
 *
 * Performs a simple lowercase transformation using `std::tolower`.
 * Intended for case-insensitive comparisons (e.g. extension lists,
 * identifiers, runtime interface names). Only ASCII characters
 * are modified; multibyte UTF-8 sequences remain untouched.
 *
 * \param s Input string (ASCII).
 * \return Lowercased string.
 */
[[nodiscard]] inline std::string Lower(std::string s) {
  for (char &c : s)
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return s;
}

/*!
 * \brief Produces a short thread tag suitable for logs.
 *
 * Retrieves the numerical value of `std::thread::id` and returns it
 * as an unsigned integral identifier. This is mainly used for run
 * tracing, debugging multi-threaded sections, and monitoring command
 * queue submissions.
 *
 * The implementation typically relies on `std::hash<std::thread::id>`,
 * which guarantees stable ordering during a process lifetime.
 *
 * \return Numeric tag representing the current thread.
 */
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

/*!
 * \brief Simplifies function names by removing common qualifiers.
 *
 * This helper trims compiler-added prefixes or qualifiers
 * (such as class scopes, return types, and calling conventions)
 * from `__FUNCTION__`, `__PRETTY_FUNCTION__`, or MSVC equivalents
 * to produce cleaner, shorter log output.
 *
 * The algorithm is intentionally lightweight and non-parsing:
 * it performs substring pruning using heuristics tailored for
 * GGEMS logging (removing trailing type descriptors or redundant
 * namespace information).
 *
 * \param full Raw function name string.
 * \return Simplified user-facing function name.
 */
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

/*!
 * \brief Computes a 64-bit FNV-1a hash of the input string.
 *
 * Implements the Fowler–Noll–Vo hash function (1a variant), which
 * performs a byte-wise XOR then multiplies by a large prime constant.
 *
 * Characteristics:
 *  - Fast and lightweight.
 *  - Good distribution for short strings (e.g. kernel names, tags).
 *  - Stable across platforms.
 *
 * This hash is useful for compact identifiers, map keys, or internal
 * routing of named resources; it is not cryptographically secure
 * and must not be used for authentication or hashing untrusted data.
 *
 * \param s Input string (any ASCII/UTF-8 view).
 * \return 64-bit FNV-1a hash.
 */
[[nodiscard]] inline std::uint64_t HashFNV1a(std::string_view s) noexcept {
  static constexpr std::uint64_t offset = 1469598103934665603ULL;
  static constexpr std::uint64_t prime = 1099511628211ULL;

  std::uint64_t h = offset;
  for (char ch : s) {
    unsigned char c = static_cast<unsigned char>(ch);
    h ^= c;
    h *= prime;
  }
  return h;
}
} // namespace ggems::core
