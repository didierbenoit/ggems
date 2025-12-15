#pragma once

/// \cond
#include <cstdint>
#include <mutex>
#include <vector>
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
enum class RunStatus : std::uint8_t {
  Starting = 0,
  Configuring,
  Ready,
  Running,
  Finished,
  Failed
};

class GGEMSOutputState {
public:
  GGEMSOutputState() = default;
  ~GGEMSOutputState() = default;

  GGEMSOutputState(GGEMSOutputState &) = delete;
  GGEMSOutputState(GGEMSOutputState &&) = delete;
  GGEMSOutputState &operator=(GGEMSOutputState const &) = delete;
  GGEMSOutputState &operator=(GGEMSOutputState const &&) = delete;

public:
  void SetRunStatus(RunStatus run_status) noexcept;
  [[nodiscard]] RunStatus GetRunStatus() const noexcept;

  void PushLogs(LogRecord const &record);
  [[nodiscard]] std::vector<LogRecord> GetLogsSnapshot() const;
  void ClearLogs();
  [[nodiscard]] std::size_t GetLogCount() const noexcept;

private:
  mutable std::mutex mtx_;
  RunStatus run_status_{RunStatus::Running};
  std::vector<LogRecord> logs_;
};
} // namespace ggems::core
