#pragma once

/// \cond
#include <string>
#include <string_view>
#include <cstdint>
/// \endcond

#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::render {
class GGEMSBanner;
} // namespace ggems::render

namespace ggems::core {

enum class OutputMode : std::uint8_t { Term = 0, Gui };

OutputMode GetOutputMode() noexcept;

bool IsOutputConfigured() noexcept;
bool IsOutputRuntimeStarted() noexcept;

void SetOutputMode(OutputMode mode);
void SetOutputMode(std::string_view mode);

void SetOutputFile(std::string_view path);
void ClearOutputFile() noexcept;

void StartOutputRuntime();
void WakeOutputRuntime() noexcept;
void ShowFinalOutputScreen(
    std::u32string_view message = U"Press Enter to exit...");
void StopOutputRuntime() noexcept;

GGEMSOutputState &GetOutputState();

render::GGEMSBanner &GetOutputBanner();

[[nodiscard]] inline std::string ToString(OutputMode mode) {
  switch (mode) {
  case OutputMode::Term:
    return "term";
  case OutputMode::Gui:
    return "gui";
  }
  return "term";
}
} // namespace ggems::core
