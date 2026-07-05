#include <atomic>
#include <iostream>
#include <memory>
#include <optional>

#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/render/GGEMSColour.hh"

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

OutputMode Parse(std::string_view mode) {
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

void AddOptionalFileSink(GGEMSLogger &logger) {
  if (g_output_file_path.has_value()) {
    logger.AddSink(std::make_unique<FileSink>(*g_output_file_path));
  }
}

// =============================================================================
// =============================================================================

void ConfigureLoggerForMode(OutputMode mode) {
  GGEMSLogger &logger = GGEMSLogger::GetInstance();

  logger.ClearSinks();

  switch (mode) {
  case OutputMode::Term:
    logger.AddSink(std::make_unique<StdoutSink>());
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Utf32);
    break;

  case OutputMode::Gui:
    logger.AddSink(std::make_unique<GGEMSOutputStateSink>(GetOutputState()));
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Utf32);
    break;
  }

  g_configured = true;
}

// =============================================================================
// =============================================================================

std::string ToTerminalText(render::WrappedLine const &line) {
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

void EmitTerminalBanner() {
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

OutputMode GetOutputMode() noexcept { return g_mode; }

// =============================================================================
// =============================================================================

bool IsOutputConfigured() noexcept { return g_configured; }

// =============================================================================
// =============================================================================

bool IsOutputRuntimeStarted() noexcept {
  return g_output_running.load(std::memory_order_relaxed);
}

// =============================================================================
// =============================================================================

GGEMSOutputState &GetOutputState() {
  if (!g_state) {
    g_state = std::make_unique<GGEMSOutputState>();
  }

  return *g_state;
}

// =============================================================================
// =============================================================================

render::GGEMSBanner &GetOutputBanner() {
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

void SetOutputMode(OutputMode const mode) {
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

void SetOutputMode(std::string_view mode) { SetOutputMode(Parse(mode)); }

// =============================================================================
// =============================================================================

void SetOutputFile(std::string_view path) {
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

void ClearOutputFile() noexcept {
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

void StartOutputRuntime() {
  GGEMS_CHECK_FATAL(
      g_configured,
      "Output mode is not configured. "
      "Call ggems.core.set_output_mode('term'|'gui') before starting GGEMS "
      "output runtime.");

  if (g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  g_output_running.store(true, std::memory_order_relaxed);

  if (g_mode == OutputMode::Term) {
    EmitTerminalBanner();
  }
}

// =============================================================================
// =============================================================================

void WakeOutputRuntime() noexcept {}

// =============================================================================
// =============================================================================

void ShowFinalOutputScreen(std::u32string_view) {}

// =============================================================================
// =============================================================================

void StopOutputRuntime() noexcept {
  g_output_running.store(false, std::memory_order_relaxed);
}
} // namespace ggems::core
