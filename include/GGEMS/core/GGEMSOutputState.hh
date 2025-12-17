#pragma once

/// \cond
#include <cstdint>
#include <mutex>
#include <vector>
/// \endcond

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
  GGEMSOutputState &operator=(GGEMSOutputState &&) = delete;

public:
  void SetRunStatus(RunStatus run_status);

  [[nodiscard]] RunStatus GetRunStatus() const;

  void SetLogCapacity(std::size_t capacity);

  [[nodiscard]] std::size_t GetLogCapacity() const;

  void PushLogLine(std::string_view formatted_line);

  [[nodiscard]] std::vector<std::string>
  GetLastLogLinesSnapshot(std::size_t max_lines) const;

  void ClearLogs();

  [[nodiscard]] std::size_t GetLogCount() const;

private:
  mutable std::mutex mtx_{};
  RunStatus run_status_{RunStatus::Running};
  std::vector<std::string> log_ring_;
  std::size_t log_capacity_{2000};
  std::size_t log_head_{0};
  std::size_t log_size_{0};
};
} // namespace ggems::core
