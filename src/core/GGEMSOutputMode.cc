/// \cond
#include <memory>
/// \endcond

#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"
#include "GGEMS/render/GGEMSBanner.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace {
OutputMode g_mode{OutputMode::Term};
bool g_configured{false};

std::unique_ptr<GGEMSOutputState> g_state{};
std::unique_ptr<render::GGEMSBanner> g_banner{};
std::unique_ptr<render::GGEMSProgressBar> g_progress_bar{};
std::unique_ptr<render::GGEMSTerminalRenderer> g_terminal_renderer{};

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

OutputMode Parse(std::string_view s) {
  std::string v = Lower(std::string(s));
  if (v == "term" || v == "terminal")
    return OutputMode::Term;
  if (v == "gui" || v == "imgui")
    return OutputMode::Gui;
  if (v == "cluster")
    return OutputMode::Cluster;

  Throw<GGEMSFatal>(
      "Unknown output mode. Expected: 'term', 'gui', or 'cluster'.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void ConfigureLoggerForMode(OutputMode mode) {
  auto &logger = GGEMSLogger::GetInstance();

  switch (mode) {
  case OutputMode::Term: {
    auto &st = EnsureOutputState();
    logger.SetSink(std::make_unique<GGEMSOutputStateSink>(st));
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Utf32);
    break;
  }
  case OutputMode::Gui: {
    auto &st = EnsureOutputState();
    logger.SetSink(std::make_unique<GGEMSOutputStateSink>(st));
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Utf32);
    break;
  }
  case OutputMode::Cluster: {
    logger.SetSink(std::make_unique<FileSink>("ggems.log"));
    logger.SetForceColor(false);
    logger.SetForceEncoding(Encoding::Ascii);
    break;
  }
  }

  g_configured = true;
}
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSOutputState &EnsureOutputState() {
  if (!g_state) {
    g_state = std::make_unique<GGEMSOutputState>();
  }
  return *g_state;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

render::GGEMSProgressBar &EnsureProgressBar() {
  if (!g_progress_bar) {
    g_progress_bar = std::make_unique<render::GGEMSProgressBar>();
  }
  return *g_progress_bar;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

OutputMode GetOutputMode() noexcept { return g_mode; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(OutputMode mode) {
  if (mode == g_mode && g_configured)
    return;

  if (mode != g_mode && g_configured) {
    Throw<GGEMSFatal>(
        "Output mode already configured; must be set before initialisation.");
  }

  ConfigureLoggerForMode(mode);
  g_mode = mode;

  if (g_mode == OutputMode::Term) {
    EnsureOutputRuntime();
    RefreshOutput();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(std::string_view mode) { SetOutputMode(Parse(mode)); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void EnsureOutputRuntime() {
  if (g_mode != OutputMode::Term) {
    return;
  }

  auto &st = EnsureOutputState();

  if (!g_banner) {
    g_banner = std::make_unique<render::GGEMSBanner>();
  }

  auto &progress_bar = EnsureProgressBar();

  if (!g_terminal_renderer) {
    g_terminal_renderer = std::make_unique<render::GGEMSTerminalRenderer>(
        *g_banner, progress_bar, st);
  }

  g_terminal_renderer->Start();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void RefreshOutput() {
  if (g_mode != OutputMode::Term) {
    return;
  }

  EnsureOutputRuntime();
  g_terminal_renderer->RenderOnce();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void FinaliseOutput(std::u32string_view message) {
  if (g_mode != OutputMode::Term) {
    return;
  }

  EnsureOutputRuntime();
  g_terminal_renderer->RunFinalScreen(message);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void StopOutputRuntime() noexcept {
  if (g_terminal_renderer) {
    g_terminal_renderer->Stop();
  }
}
} // namespace ggems::core
