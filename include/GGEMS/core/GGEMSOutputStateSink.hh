#pragma once

#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
class GGEMSOutputStateSink final : public LogSink {
public:
  explicit GGEMSOutputStateSink(GGEMSOutputState &state) noexcept
      : state_(state) {}

  void Write(LogRecord const &rec, std::string const &formatted) override {
    (void)rec;
    state_.PushLogLine(formatted);
  }

private:
  GGEMSOutputState &state_;
};
} // namespace ggems::core
