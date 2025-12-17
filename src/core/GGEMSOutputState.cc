#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::SetRunStatus(RunStatus run_status) {
  std::lock_guard<std::mutex> lock(mtx_);
  run_status_ = run_status;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

RunStatus GGEMSOutputState::GetRunStatus() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return run_status_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::SetLogCapacity(std::size_t capacity) {
  std::lock_guard<std::mutex> lock(mtx_);

  if (capacity == 0)
    capacity = 1;

  log_capacity_ = capacity;

  log_ring_.clear();
  log_ring_.resize(log_capacity_);

  log_head_ = 0;
  log_size_ = 0;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSOutputState::GetLogCapacity() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return log_capacity_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::PushLogLine(std::string_view formatted_line) {
  std::lock_guard<std::mutex> lock(mtx_);

  if (log_ring_.empty()) {
    log_ring_.resize(log_capacity_);
    log_size_ = 0;
    log_head_ = 0;
  }

  if (log_size_ < log_capacity_) {
    std::size_t idx = (log_size_ + log_head_) % log_capacity_;
    log_ring_[idx].assign(formatted_line.data(), formatted_line.size());
    ++log_size_;
  } else {
    log_ring_[log_head_].assign(formatted_line.data(), formatted_line.size());
    log_head_ = (log_head_ + 1) % log_capacity_;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::string>
GGEMSOutputState::GetLastLogLinesSnapshot(std::size_t max_lines) const {
  std::lock_guard<std::mutex> lock(mtx_);

  if (log_size_ == 0 || max_lines == 0) {
    return {};
  }

  std::size_t n = std::min(max_lines, log_size_);
  std::vector<std::string> out;
  out.reserve(n);

  std::size_t start = (log_head_ + (log_size_ - n)) % log_capacity_;
  for (std::size_t i = 0; i < n; ++i) {
    std::size_t idx = (start + i) % log_capacity_;
    out.emplace_back(log_ring_[idx]);
  }

  return out;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::ClearLogs() {
  std::lock_guard<std::mutex> lock(mtx_);
  log_size_ = 0;
  log_head_ = 0;
}
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSOutputState::GetLogCount() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return log_size_;
}

} // namespace ggems::core
