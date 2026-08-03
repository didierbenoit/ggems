#pragma once

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::test {
class ScopedLoggerEncoding final {
public:
  explicit ScopedLoggerEncoding(core::Encoding encoding)
      : logger_{core::GGEMSLogger::GetInstance()},
        previous_encoding_{logger_.GetEncoding()} {
    logger_.SetForceEncoding(encoding);
  }

  ~ScopedLoggerEncoding() noexcept {
    logger_.SetForceEncoding(previous_encoding_);
  }

  ScopedLoggerEncoding(ScopedLoggerEncoding const &) = delete;
  ScopedLoggerEncoding(ScopedLoggerEncoding &&) = delete;
  auto operator=(ScopedLoggerEncoding const &)
      -> ScopedLoggerEncoding & = delete;
  auto operator=(ScopedLoggerEncoding &&) -> ScopedLoggerEncoding & = delete;

private:
  core::GGEMSLogger &logger_;
  core::Encoding previous_encoding_;
};
} // namespace ggems::test
