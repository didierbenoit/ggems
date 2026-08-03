#include <atomic>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"
#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/render/GGEMSColour.hh"

#if defined(_WIN32)
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
#endif

namespace ggems::core {

namespace {

// =============================================================================
// =============================================================================

OutputMode g_mode{OutputMode::Term};
bool g_configured{false};

std::optional<std::string> g_output_file_path{};

std::unique_ptr<GGEMSOutputState> g_state{};
std::unique_ptr<render::GGEMSBanner> g_banner{};

std::atomic<bool> g_output_running{false};

// =============================================================================
// =============================================================================

#if defined(_WIN32)
void EnableWindowsVirtualTerminal(DWORD standard_handle) noexcept {
  HANDLE handle = GetStdHandle(standard_handle);

  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return;
  }

  DWORD console_mode{0};
  if (GetConsoleMode(handle, &console_mode) == FALSE) {
    return;
  }

  console_mode |= ENABLE_PROCESSED_OUTPUT;
  console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

  static_cast<void>(SetConsoleMode(handle, console_mode));
}

// =============================================================================
// =============================================================================

void PrepareWindowsTerminal() noexcept {
  static_cast<void>(SetConsoleCP(CP_UTF8));
  static_cast<void>(SetConsoleOutputCP(CP_UTF8));

  EnableWindowsVirtualTerminal(STD_OUTPUT_HANDLE);
  EnableWindowsVirtualTerminal(STD_ERROR_HANDLE);
}
#endif

// =============================================================================
// =============================================================================

auto Parse(std::string_view mode) -> OutputMode {
  std::string value = Lower(std::string(mode));

  if (value == "term" || value == "terminal") {
    return OutputMode::Term;
  }

  if (value == "gui" || value == "imgui") {
    return OutputMode::Gui;
  }

  GGEMS_FATAL("Unknown output mode. Expected: 'term' or 'gui'.");
}

// =============================================================================
// =============================================================================

auto AddOptionalFileSink(GGEMSLogger &logger) -> void {
  if (g_output_file_path.has_value()) {
    logger.AddSink(std::make_unique<FileSink>(*g_output_file_path));
  }
}

// =============================================================================
// =============================================================================

auto ConfigureLoggerForMode(OutputMode mode) -> void {
  GGEMSLogger &logger = GGEMSLogger::GetInstance();

  logger.ClearSinks();

  switch (mode) {
  case OutputMode::Term:
    logger.AddSink(std::make_unique<StdoutSink>());
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Unicode);
    break;

  case OutputMode::Gui:
    logger.AddSink(std::make_unique<GGEMSOutputStateSink>(GetOutputState()));
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Unicode);
    break;
  }

  g_configured = true;
}

// =============================================================================
// =============================================================================

auto ToTerminalText(render::WrappedLine const &line) -> std::string {
  std::string out;

  bool const use_colour = GGEMSLogger::GetInstance().UseColour();
  bool wrote_colour{false};

  for (render::VisualSegment const &segment : line.segments) {
    if (use_colour) {
      render::AppendAnsiColour(out, segment.colour);
      wrote_colour = true;
    }

    out += utf::UTF32ToUTF8(segment.text);
  }

  if (wrote_colour) {
    render::AppendAnsiControl(out, render::AnsiControl::ResetColour);
  }

  return out;
}

// =============================================================================
// =============================================================================

auto EmitTerminalBanner() -> void {
  render::GGEMSBanner &banner = GetOutputBanner();

  std::vector<render::WrappedLine> lines = banner.BuildLines(banner.GetWidth());

  for (render::WrappedLine const &line : lines) {
    std::cout << ToTerminalText(line) << '\n';
  }

  std::cout << '\n';
}
} // namespace

// =============================================================================
// =============================================================================

auto GetOutputMode() noexcept -> OutputMode { return g_mode; }

// =============================================================================
// =============================================================================

auto IsOutputConfigured() noexcept -> bool { return g_configured; }

// =============================================================================
// =============================================================================

auto IsOutputRuntimeStarted() noexcept -> bool {
  return g_output_running.load(std::memory_order_relaxed);
}

// =============================================================================
// =============================================================================

auto GetOutputState() -> GGEMSOutputState & {
  if (!g_state) {
    g_state = std::make_unique<GGEMSOutputState>();
  }

  return *g_state;
}

// =============================================================================
// =============================================================================

auto GetOutputBanner() -> render::GGEMSBanner & {
  GGEMS_CHECK_FATAL(
      g_configured,
      "Output mode must be configured before requesting the banner. "
      "Call ggems.core.set_output_mode('term'|'gui') first.");

  if (!g_banner) {
    g_banner = std::make_unique<render::GGEMSBanner>();
  }

  return *g_banner;
}

// =============================================================================
// =============================================================================

auto SetOutputMode(OutputMode const mode) -> void {
  if (mode == g_mode && g_configured) {
    return;
  }

  GGEMS_CHECK_FATAL(
      !g_configured,
      "Output mode already configured; it must be set exactly once before "
      "starting GGEMS output runtime.");

  g_mode = mode;
  ConfigureLoggerForMode(g_mode);
}

// =============================================================================
// =============================================================================

auto SetOutputMode(std::string_view mode) -> void {
  SetOutputMode(Parse(mode));
}

// =============================================================================
// =============================================================================

auto SetOutputFile(std::string_view path) -> void {
  GGEMS_CHECK_FATAL(!path.empty(), "Output file path must not be empty.");

  GGEMS_CHECK_FATAL(
      !g_output_running.load(std::memory_order_relaxed),
      "Output file cannot be changed while output runtime is started.");

  g_output_file_path = std::string(path);

  if (g_configured) {
    ConfigureLoggerForMode(g_mode);
  }
}

// =============================================================================
// =============================================================================

auto ClearOutputFile() noexcept -> void {
  if (g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  g_output_file_path.reset();

  if (g_configured) {
    ConfigureLoggerForMode(g_mode);
  }
}

// =============================================================================
// =============================================================================

auto StartOutputRuntime() -> void {
  GGEMS_CHECK_FATAL(
      g_configured,
      "Output mode is not configured. "
      "Call ggems.core.set_output_mode('term'|'gui') before starting GGEMS "
      "output runtime.");

  if (g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  if (g_mode == OutputMode::Term) {
#if defined(_WIN32)
    PrepareWindowsTerminal();
#endif
  }

  g_output_running.store(true, std::memory_order_relaxed);

  if (g_mode == OutputMode::Term) {
    EmitTerminalBanner();
  }
}

// =============================================================================
// =============================================================================

auto StopOutputRuntime() noexcept -> void {
  g_output_running.store(false, std::memory_order_relaxed);
}
} // namespace ggems::core
