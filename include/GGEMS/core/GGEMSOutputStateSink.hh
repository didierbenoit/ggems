#pragma once

#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
class GGEMSOutputStateSink final : public LogSink {
public:
  explicit GGEMSOutputStateSink(GGEMSOutputState &state) noexcept
      : state_(state) {}

  void Write(RenderedLogLine &&log_line) override {
    state_.PushLogLine(std::move(log_line));
  }

private:
  GGEMSOutputState &state_;
};
} // namespace ggems::core
