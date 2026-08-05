#include <atomic>
#include <cstddef>
#include <format>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include "GGEMS/core/detail/GGEMSLoggerMetadata.hh"

namespace ggems::core::logging::detail {
namespace {

[[nodiscard]] auto IsAsciiSpace(char const c) noexcept -> bool {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

[[nodiscard]] auto IsAsciiLower(char const c) noexcept -> bool {
  return c >= 'a' && c <= 'z';
}

[[nodiscard]] auto TrimRight(std::string_view text) noexcept
    -> std::string_view {
  while (!text.empty() && IsAsciiSpace(text.back())) {
    text.remove_suffix(1);
  }

  return text;
}

[[nodiscard]] auto FindLastParameterList(std::string_view text) noexcept
    -> std::size_t {
  std::size_t angle_depth = 0;
  std::size_t last = std::string_view::npos;

  for (std::size_t i = 0; i < text.size(); ++i) {
    char const c = text[i];

    if (c == '<') {
      ++angle_depth;
    } else if (c == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (c == '(' && angle_depth == 0) {
      last = i;
    }
  }

  return last;
}

[[nodiscard]] auto RemoveReturnType(std::string_view text) noexcept
    -> std::string_view {
  std::size_t angle_depth = 0;
  std::size_t last_space = std::string_view::npos;

  for (std::size_t i = 0; i < text.size(); ++i) {
    char const c = text[i];

    if (c == '<') {
      ++angle_depth;
    } else if (c == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (angle_depth == 0 && IsAsciiSpace(c)) {
      last_space = i;
    }
  }

  if (last_space != std::string_view::npos) {
    return text.substr(last_space + 1);
  }

  return text;
}

[[nodiscard]] auto RemoveTrailingEmptyCall(std::string_view text) noexcept
    -> std::string_view {
  if (text.ends_with("()")) {
    text.remove_suffix(2);
  }

  return text;
}

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
    char const c = text[i];

    if (c == '<') {
      ++angle_depth;
    } else if (c == '>' && angle_depth > 0) {
      --angle_depth;
    } else if (angle_depth == 0 && c == ':' && text[i + 1] == ':') {
      last = i;
      ++i;
    }
  }

  return last;
}

[[nodiscard]] auto
StripFunctionTemplateArguments(std::string_view text) noexcept
    -> std::string_view {
  auto const last_scope = FindLastTopLevelScope(text);
  std::size_t const start =
      last_scope == std::string_view::npos ? 0 : last_scope + 2;

  std::size_t angle_depth = 0;

  for (std::size_t i = start; i < text.size(); ++i) {
    char const c = text[i];

    if (c == '<') {
      if (angle_depth == 0) {
        return text.substr(0, i);
      }

      ++angle_depth;
    } else if (c == '>' && angle_depth > 0) {
      --angle_depth;
    }
  }

  return text;
}

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
  static std::mutex m;
  static std::unordered_map<std::thread::id, unsigned> map;

  thread_local unsigned idx = [&] {
    std::scoped_lock lock(m);
    auto [it, inserted] = map.emplace(std::this_thread::get_id(), next++);
    return it->second;
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
