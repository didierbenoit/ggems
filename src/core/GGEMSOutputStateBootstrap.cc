#include "GGEMS/core/GGEMSOutputStateBootstrap.hh"
#include "GGEMS/core/GGEMSOutputStateSink.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSOutputState &EnsureOutputState() {
  static std::mutex mtx;
  static std::unique_ptr<GGEMSOutputState> state;

  std::lock_guard<std::mutex> lock(mtx);

  if (!state) {
    state = std::make_unique<GGEMSOutputState>();
    GGEMSLogger::GetInstance().AttachSink(
        std::make_unique<GGEMSOutputStateSink>(*state));
  }

  return *state;
}
} // namespace ggems::core
