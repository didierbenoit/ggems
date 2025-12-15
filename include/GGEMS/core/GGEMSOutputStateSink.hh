#pragma once

#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {
class GGEMSOutputStateSink final : public LogSink {
public:
  explicit GGEMSOutputStateSink(GGEMSOutputState &state) noexcept
      : state_(state) {}

  void Write(LogRecord const &rec, std::string const &) override {
    state_.PushLogs(rec);
  }

private:
  GGEMSOutputState &state_;
};
} // namespace ggems::core
