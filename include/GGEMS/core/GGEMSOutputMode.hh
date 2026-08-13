#pragma once

#include <string>
#include <string_view>
#include <cstdint>

#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {

enum class OutputMode : std::uint8_t { Term = 0, Gui };

auto GetOutputMode() noexcept -> OutputMode;

auto IsOutputConfigured() noexcept -> bool;
auto IsOutputRuntimeStarted() noexcept -> bool;

auto SetOutputMode(OutputMode mode) -> void;
auto SetOutputMode(std::string_view mode) -> void;

auto SetOutputFile(std::string_view path) -> void;
auto ClearOutputFile() -> void;

auto StartOutputRuntime() -> void;
auto StopOutputRuntime() noexcept -> void;

auto GetOutputState() -> GGEMSOutputState &;

[[nodiscard]] inline auto ToString(OutputMode mode) -> std::string {
  switch (mode) {
  case OutputMode::Term:
    return "term";
  case OutputMode::Gui:
    return "gui";
  }
  return "term";
}
} // namespace ggems::core
