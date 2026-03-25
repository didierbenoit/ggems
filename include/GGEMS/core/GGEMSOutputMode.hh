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

bool IsOutputConfigured() noexcept;
bool IsOutputRuntimeStarted() noexcept;
bool IsProgressBarAvailable() noexcept;

void SetOutputMode(OutputMode mode);
void SetOutputMode(std::string_view mode);

void SetClusterOutputFile(std::string_view path);

void StartOutputRuntime();
void WakeOutputRuntime() noexcept;
void ShowFinalOutputScreen(
    std::u32string_view message = U"Press Enter to exit...");
void StopOutputRuntime() noexcept;

GGEMSOutputState &GetOutputState();
render::GGEMSProgressBar &GetProgressBar();

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
