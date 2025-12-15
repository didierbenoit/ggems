#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::SetRunStatus(RunStatus run_status) noexcept {
  std::lock_guard<std::mutex> lock(mtx_);
  run_status_ = run_status;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

RunStatus GGEMSOutputState::GetRunStatus() const noexcept {
  std::lock_guard<std::mutex> lock(mtx_);
  return run_status_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::PushLogs(LogRecord const &record) {
  std::lock_guard<std::mutex> lock(mtx_);
  logs_.push_back(record);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<LogRecord> GGEMSOutputState::GetLogsSnapshot() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return logs_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::ClearLogs() {
  std::lock_guard<std::mutex> lock(mtx_);
  logs_.clear();
}
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSOutputState::GetLogCount() const noexcept {
  std::lock_guard<std::mutex> lock(mtx_);
  return logs_.size();
}

} // namespace ggems::core
