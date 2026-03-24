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

std::thread g_output_thread{};
std::atomic<bool> g_output_running{false};
std::atomic<bool> g_output_stop_requested{false};
std::atomic<bool> g_output_final_requested{false};
std::atomic<bool> g_output_final_done{false};

std::mutex g_output_mtx{};
std::condition_variable g_output_cv{};
std::u32string g_final_message{U"Press Enter to exit..."};

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

  GGEMS_FATAL("Unknown output mode. Expected: 'term', 'gui', or 'cluster'.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void ConfigureLoggerForMode(OutputMode mode) {
  auto &logger = GGEMSLogger::GetInstance();

  switch (mode) {
  case OutputMode::Term: {
    auto &st = GetOutputState();
    logger.SetSink(std::make_unique<GGEMSOutputStateSink>(st));
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Utf32);
    break;
  }
  case OutputMode::Gui: {
    auto &st = GetOutputState();
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void EnsureTerminalObjects() {
  auto &st = GetOutputState();

  if (!g_banner) {
    g_banner = std::make_unique<render::GGEMSBanner>();
  }

  auto &progress_bar = GetProgressBar();

  if (!g_terminal_renderer) {
    g_terminal_renderer = std::make_unique<render::GGEMSTerminalRenderer>(
        *g_banner, progress_bar, st);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void OutputThreadLoop() {
  while (!g_output_stop_requested.load(std::memory_order_relaxed)) {
    if (g_mode == OutputMode::Term && g_terminal_renderer) {
      g_terminal_renderer->RenderOnce();
    }

    if (g_output_final_requested.load(std::memory_order_relaxed)) {
      std::u32string message;
      {
        std::scoped_lock lock(g_output_mtx);
        message = g_final_message;
      }

      if (g_mode == OutputMode::Term && g_terminal_renderer) {
        g_terminal_renderer->RunFinalScreen(message);
      }

      g_output_final_requested.store(false, std::memory_order_relaxed);
      g_output_final_done.store(true, std::memory_order_relaxed);
      g_output_cv.notify_one();
      continue;
    }

    std::unique_lock<std::mutex> lock(g_output_mtx);
    g_output_cv.wait_for(lock, std::chrono::milliseconds(50), [] {
      return g_output_stop_requested.load(std::memory_order_relaxed) ||
             g_output_final_requested.load(std::memory_order_relaxed);
    });
  }

  g_output_running.store(false, std::memory_order_relaxed);
}
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

OutputMode GetOutputMode() noexcept { return g_mode; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool IsOutputConfigured() noexcept { return g_configured; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool IsOutputRuntimeStarted() noexcept {
  return g_output_running.load(std::memory_order_relaxed);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSOutputState &GetOutputState() {
  if (!g_state) {
    g_state = std::make_unique<GGEMSOutputState>();
  }
  return *g_state;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

render::GGEMSProgressBar &GetProgressBar() {
  GGEMS_CHECK_FATAL(
      g_configured,
      "Output mode must be configured before requesting the progress bar. "
      "Call ggems.core.set_output_mode('term'|'gui'|'cluster') first.");

  GGEMS_CHECK_FATAL(g_mode != OutputMode::Cluster,
                    "Progress bar is not available in cluster mode.");

  if (!g_progress_bar) {
    g_progress_bar = std::make_unique<render::GGEMSProgressBar>();
  }
  return *g_progress_bar;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(OutputMode mode) {
  if (mode == g_mode && g_configured) {
    return;
  }

  GGEMS_CHECK_FATAL(
      !g_configured,
      "Output mode already configured; it must be set exactly once before "
      "starting GGEMS output runtime.");

  ConfigureLoggerForMode(mode);
  g_mode = mode;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(std::string_view mode) { SetOutputMode(Parse(mode)); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void StartOutputRuntime() {
  GGEMS_CHECK_FATAL(
      g_configured,
      "Output mode is not configured. "
      "Call ggems.core.set_output_mode('term'|'gui'|'cluster') before "
      "starting GGEMS output runtime.");

  if (g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  switch (g_mode) {
  case OutputMode::Term: {
    EnsureTerminalObjects();
    g_terminal_renderer->Start();

    g_output_stop_requested.store(false, std::memory_order_relaxed);
    g_output_final_requested.store(false, std::memory_order_relaxed);
    g_output_running.store(true, std::memory_order_relaxed);
    g_output_final_done.store(false, std::memory_order_relaxed);

    g_output_thread = std::thread(OutputThreadLoop);
    break;
  }

  case OutputMode::Gui: {
    g_output_running.store(true, std::memory_order_relaxed);
    break;
  }

  case OutputMode::Cluster: {
    g_output_running.store(true, std::memory_order_relaxed);
    break;
  }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void WakeOutputRuntime() noexcept {
  if (!g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  if (g_mode == OutputMode::Term) {
    g_output_cv.notify_one();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void ShowFinalOutputScreen(std::u32string_view message) {
  if (!g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  if (g_mode == OutputMode::Cluster) {
    return;
  }

  if (g_mode == OutputMode::Gui) {
    return;
  }

  {
    std::scoped_lock lock(g_output_mtx);
    g_final_message = std::u32string(message);
  }

  g_output_final_done.store(false, std::memory_order_relaxed);
  g_output_final_requested.store(true, std::memory_order_relaxed);
  g_output_cv.notify_one();

  std::unique_lock<std::mutex> lock(g_output_mtx);
  g_output_cv.wait(
      lock, [] { return g_output_final_done.load(std::memory_order_relaxed); });
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void StopOutputRuntime() noexcept {
  g_output_stop_requested.store(true, std::memory_order_relaxed);
  g_output_cv.notify_one();

  if (g_output_thread.joinable()) {
    g_output_thread.join();
  }

  if (g_terminal_renderer) {
    g_terminal_renderer->Stop();
  }

  g_output_running.store(false, std::memory_order_relaxed);
}
} // namespace ggems::core
