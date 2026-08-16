// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements the thread-safe GGEMS output-state ring buffer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <mutex>
#include <cstddef>
#include <utility>
#include <vector>
#include <algorithm>

/// \endcond
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSOutputState.hh"

namespace ggems::core {

// =============================================================================
// =============================================================================

auto GGEMSOutputState::SetRunStatus(RunStatus run_status) -> void {
  std::scoped_lock lock(mtx_);
  run_status_ = run_status;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::GetRunStatus() const -> RunStatus {
  std::scoped_lock lock(mtx_);
  return run_status_;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::SetLogCapacity(std::size_t capacity) -> void {
  std::scoped_lock lock(mtx_);

  if (capacity == 0) {
    capacity = 1;
  }

  log_capacity_ = capacity;

  log_ring_.clear();
  log_ring_.resize(log_capacity_);

  log_head_ = 0;
  log_size_ = 0;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::GetLogCapacity() const -> std::size_t {
  std::scoped_lock lock(mtx_);
  return log_capacity_;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::PushLogLine(RenderedLogLine rendered_line) -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOutputState::GetLastLogLinesSnapshot(std::size_t max_lines) const
    -> std::vector<RenderedLogLine> {
  std::scoped_lock lock(mtx_);

  if (log_size_ == 0 || max_lines == 0) {
    return {};
  }

  std::size_t const line_count = std::min(max_lines, log_size_);
  std::vector<RenderedLogLine> out;
  out.reserve(line_count);

  std::size_t start = (log_head_ + (log_size_ - line_count)) % log_capacity_;
  for (std::size_t i = 0; i < line_count; ++i) {
    std::size_t idx = (start + i) % log_capacity_;
    out.emplace_back(log_ring_[idx]);
  }

  return out;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::ClearLogs() -> void {
  std::scoped_lock lock(mtx_);
  log_size_ = 0;
  log_head_ = 0;
}

// -----------------------------------------------------------------------------

auto GGEMSOutputState::GetLogCount() const -> std::size_t {
  std::scoped_lock lock(mtx_);
  return log_size_;
}

} // namespace ggems::core
