#include <memory>

#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

namespace {
OutputMode g_mode = OutputMode::Term;

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

  return OutputMode::Term; // unreachable
}

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

void ConfigureLoggerForMode(OutputMode mode) {
  auto &logger = GGEMSLogger::GetInstance();
  logger.ClearSinks();

  switch (mode) {
  case OutputMode::Term: {
    auto &st = EnsureOutputState();
    logger.AttachSink(std::make_unique<GGEMSOutputStateSink>(st));
    break;
  }
  case OutputMode::Gui: {
    auto &st = EnsureOutputState();
    logger.AttachSink(std::make_unique<GGEMSOutputStateSink>(st));
    break;
  }
  case OutputMode::Cluster: {
    logger.AttachSink(std::make_unique<FileSink>("ggems.log"));
    logger.SetForceColor(false);
    logger.SetForceEncoding(Encoding::Ascii);
    break;
  }
  }
}
} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

OutputMode GetOutputMode() noexcept { return g_mode; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(OutputMode mode) {
  if (mode == g_mode)
    return;

  ConfigureLoggerForMode(mode);

  g_mode = mode;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void SetOutputMode(std::string_view mode) { SetOutputMode(Parse(mode)); }

} // namespace ggems::core
