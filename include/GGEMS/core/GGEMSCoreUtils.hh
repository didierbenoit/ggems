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
#include <string_view>
/// \endcond

namespace ggems::core {
[[nodiscard]]
inline std::size_t GetThreadIndex() noexcept {
  static std::atomic_size_t counter{0};
  static thread_local std::size_t id = counter++;
  return id;
}

[[nodiscard]]
inline std::string_view SimplifyFunctionName(std::string_view full) noexcept {
  // Step 1: Remove parameters "(...)"
  if (auto pos = full.find('('); pos != std::string_view::npos) {
    full = full.substr(0, pos);
  }

  // Step 2: Remove calling convention (e.g. "__cdecl ")
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
} // namespace ggems::core
