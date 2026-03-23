#pragma once

/// \cond
#include <string>
#include <string_view>
#include <cstdint>
/// \endcond

#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::render {
class GGEMSProgressBar;
}

namespace ggems::core {
enum class OutputMode : std::uint8_t { Term = 0, Gui, Cluster };

OutputMode GetOutputMode() noexcept;
GGEMSOutputState &EnsureOutputState();
render::GGEMSProgressBar &EnsureProgressBar();

void SetOutputMode(OutputMode mode);
void SetOutputMode(std::string_view mode);

void EnsureOutputRuntime();
void RefreshOutput();
void FinaliseOutput(std::u32string_view message = U"Press Enter to exit...");
void StopOutputRuntime() noexcept;

[[nodiscard]] inline std::string ToString(OutputMode mode) {
  switch (mode) {
  case OutputMode::Term:
    return "term";
  case OutputMode::Cluster:
    return "cluster";
  case OutputMode::Gui:
    return "gui";
  }
  return "term";
}
} // namespace ggems::core
