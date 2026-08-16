// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements internal GGEMS logger metadata normalization.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <atomic>
#include <cstddef>
#include <format>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

/// \endcond
#include "GGEMS/core/detail/GGEMSLoggerMetadata.hh"

namespace ggems::core::logging::detail {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Tests whether a character is one of the ASCII whitespace characters used by the simplifier.
 *
 * \param[in] character Character to test.
 * \return True for space, tab, line feed, or carriage return.
 */
[[nodiscard]] auto IsAsciiSpace(char character) noexcept -> bool {
  return character == ' ' || character == '\t' || character == '\n' ||
         character == '\r';
}

// =============================================================================
// =============================================================================

/*!
 * \brief Tests whether a character is an ASCII lowercase letter.
 *
 * \param[in] character Character to test.
 * \return True for ``a`` through ``z``.
 */
[[nodiscard]] auto IsAsciiLower(char character) noexcept -> bool {
  return character >= 'a' && character <= 'z';
}

// =============================================================================
// =============================================================================

/*!
 * \brief Removes trailing ASCII whitespace from a string view.
 *
 * \param[in] text View to trim.
 * \return Trimmed subview of \p text.
 */
[[nodiscard]] auto TrimRight(std::string_view text) noexcept
    -> std::string_view {
  while (!text.empty() && IsAsciiSpace(text.back())) {
    text.remove_suffix(1);
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Finds the last top-level opening parenthesis outside template arguments.
 *
 * \param[in] text Function spelling to scan.
 * \return Position of the last candidate parameter list, or ``npos``.
 */
[[nodiscard]] auto FindLastParameterList(std::string_view text) noexcept
    -> std::size_t {
  std::size_t angle_depth = 0;
  std::size_t last = std::string_view::npos;

  for (std::size_t i = 0; i < text.size(); ++i) {
    char const character = text[i];

    if (character == '<') {
      ++angle_depth;
    } else if (character == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (character == '(' && angle_depth == 0) {
      last = i;
    }
  }

  return last;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Removes a compiler-provided return-type prefix from a function spelling.
 *
 * \param[in] text Function spelling to simplify.
 * \return Subview beginning after the last top-level ASCII space when present.
 */
[[nodiscard]] auto RemoveReturnType(std::string_view text) noexcept
    -> std::string_view {
  std::size_t angle_depth = 0;
  std::size_t last_space = std::string_view::npos;

  for (std::size_t i = 0; i < text.size(); ++i) {
    char const character = text[i];

    if (character == '<') {
      ++angle_depth;
    } else if (character == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (angle_depth == 0 && IsAsciiSpace(character)) {
      last_space = i;
    }
  }

  if (last_space != std::string_view::npos) {
    return text.substr(last_space + 1);
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Removes a trailing empty-call suffix from a function spelling.
 *
 * \param[in] text Function spelling to simplify.
 * \return Simplified subview.
 */
[[nodiscard]] auto RemoveTrailingEmptyCall(std::string_view text) noexcept
    -> std::string_view {
  if (text.ends_with("()")) {
    text.remove_suffix(2);
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Removes supported compiler lambda suffix spellings.
 *
 * \param[in] text Function spelling to simplify.
 * \return Simplified subview.
 */
[[nodiscard]] auto StripLambdaSuffix(std::string_view text) noexcept
    -> std::string_view {
  if (auto const pos = text.find("::<lambda"); pos != std::string_view::npos) {
    return RemoveTrailingEmptyCall(text.substr(0, pos));
  }

  if (auto const pos = text.find("::(lambda)"); pos != std::string_view::npos) {
    return RemoveTrailingEmptyCall(text.substr(0, pos));
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Finds the last top-level C++ scope separator before a limit.
 *
 * \param[in] text Function spelling to scan.
 * \param[in] limit Exclusive scan limit, or ``npos`` for the full view.
 * \return Position of the last top-level ``::`` separator, or ``npos``.
 */
[[nodiscard]] auto
FindLastTopLevelScope(std::string_view text,
                      std::size_t limit = std::string_view::npos) noexcept
    -> std::size_t {
  if (limit == std::string_view::npos || limit > text.size()) {
    limit = text.size();
  }

  std::size_t angle_depth = 0;
  std::size_t last = std::string_view::npos;

  for (std::size_t i = 0; i + 1 < limit; ++i) {
    char const character = text[i];

    if (character == '<') {
      ++angle_depth;
    } else if (character == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (angle_depth == 0 && character == ':' && text[i + 1] == ':') {
      last = i;
      ++i;
    }
  }

  return last;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Removes template arguments attached to the final function name.
 *
 * \param[in] text Function spelling to simplify.
 * \return Simplified subview.
 */
[[nodiscard]] auto
StripFunctionTemplateArguments(std::string_view text) noexcept
    -> std::string_view {
  auto const last_scope = FindLastTopLevelScope(text);
  std::size_t const start =
      last_scope == std::string_view::npos ? 0 : last_scope + 2;

  std::size_t angle_depth = 0;

  for (std::size_t i = start; i < text.size(); ++i) {
    char const character = text[i];

    if (character == '<') {
      if (angle_depth == 0) {
        return text.substr(0, i);
      }

      ++angle_depth;
    } else if (character == '>' && angle_depth > 0) {
      --angle_depth;
    }
  }

  return text;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Keeps the concise class/function scope used in log prefixes.
 *
 * \param[in] text Function spelling to simplify.
 * \return View containing the retained final scopes.
 */
[[nodiscard]] auto KeepRelevantScopes(std::string_view text) noexcept
    -> std::string_view {
  auto const last = FindLastTopLevelScope(text);
  if (last == std::string_view::npos) {
    return text;
  }

  auto const prev = FindLastTopLevelScope(text, last);

  if (prev == std::string_view::npos) {
    // Example: ocl::PrintInfo -> PrintInfo
    // But keep Class::Method if the first part looks like a class.
    auto const left = text.substr(0, last);
    if (!left.empty() && IsAsciiLower(left.front())) {
      return text.substr(last + 2);
    }

    return text;
  }

  return text.substr(prev + 2);
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto ThreadTag() -> std::string {
  static std::atomic<unsigned> next{0};
  static std::mutex mutex;
  static std::unordered_map<std::thread::id, unsigned> thread_tags;

  thread_local unsigned idx = [&]() -> unsigned {
    std::scoped_lock lock(mutex);
    auto const iterator =
        thread_tags.emplace(std::this_thread::get_id(), next++).first;
    return iterator->second;
  }();

  return std::format("T{}", idx);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto SimplifyFunctionName(std::string_view function_name) noexcept
    -> std::string_view {
  // Step 1: remove the final parameter list.
  // Use the last '(' to survive operator() and clang lambda spellings.
  if (auto const pos = FindLastParameterList(function_name);
      pos != std::string_view::npos) {
    function_name = function_name.substr(0, pos);
  }

  function_name = TrimRight(function_name);

  // Step 2: normalize lambda spellings.
  function_name = StripLambdaSuffix(function_name);
  function_name = TrimRight(function_name);

  // Step 3: prefer the GGEMS root namespace if present.
  constexpr std::string_view root = "ggems::";

  if (auto const pos = function_name.rfind(root);
      pos != std::string_view::npos) {
    function_name = function_name.substr(pos + root.size());
  } else {
    function_name = RemoveReturnType(function_name);
  }

  function_name = TrimRight(function_name);

  // Step 4: remove template arguments only from the function name.
  function_name = StripFunctionTemplateArguments(function_name);
  function_name = TrimRight(function_name);

  // Step 5: keep Class::Method, or plain free function name.
  return KeepRelevantScopes(function_name);
}

} // namespace ggems::core::logging::detail
