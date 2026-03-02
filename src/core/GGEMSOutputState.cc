#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::SetRunStatus(RunStatus run_status) {
  std::scoped_lock lock(mtx_);
  run_status_ = run_status;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

RunStatus GGEMSOutputState::GetRunStatus() const {
  std::scoped_lock lock(mtx_);
  return run_status_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::SetLogCapacity(std::size_t capacity) {
  std::scoped_lock lock(mtx_);

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
  std::scoped_lock lock(mtx_);
  return log_capacity_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSOutputState::PushLogLine(RenderedLogLine rendered_line) {
  std::scoped_lock lock(mtx_);

  if (log_ring_.empty()) {
    log_ring_.resize(log_capacity_);
    log_size_ = 0;
    log_head_ = 0;
  }

  if (log_size_ < log_capacity_) {
    std::size_t idx = (log_size_ + log_head_) % log_capacity_;
    log_ring_[idx] = std::move(rendered_line);
    ++log_size_;
  } else {
    log_ring_[log_head_] = std::move(rendered_line);
    log_head_ = (log_head_ + 1) % log_capacity_;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<RenderedLogLine>
GGEMSOutputState::GetLastLogLinesSnapshot(std::size_t max_lines) const {
  std::scoped_lock lock(mtx_);

  if (log_size_ == 0 || max_lines == 0) {
    return {};
  }

  std::size_t n = std::min(max_lines, log_size_);
  std::vector<RenderedLogLine> out;
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
  std::scoped_lock lock(mtx_);
  log_size_ = 0;
  log_head_ = 0;
}
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSOutputState::GetLogCount() const {
  std::scoped_lock lock(mtx_);
  return log_size_;
}

} // namespace ggems::core
