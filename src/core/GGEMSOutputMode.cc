/// \cond
#include <memory>
/// \endcond

#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace {
OutputMode g_mode{OutputMode::Term};
bool g_configured{false};
std::unique_ptr<GGEMSOutputState> g_state;

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
    //    logger.SetForceEncoding(Encoding::Utf32);
    logger.SetForceEncoding(Encoding::Ascii);
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
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(std::string_view mode) { SetOutputMode(Parse(mode)); }

} // namespace ggems::core
