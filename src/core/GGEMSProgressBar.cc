/*
 * This file is part of GGEMS.
 *
 * GGEMS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * GGEMS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with GGEMS.  If not, see <https://www.gnu.org/licenses/>.
 */

/// \cond
#include <algorithm>
#include <atomic>
#include <memory>
/// \endcond

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSProgressBar.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::core {
using render::AsciiColour;
using render::GGEMSAsciiFrameBuffer;
using units::Bandwidth;
using units::HumanReadable;
using units::Time;

GGEMSProgressBar::GGEMSProgressBar(std::size_t width, std::size_t height)
    : framebuffer_{std::make_unique<GGEMSAsciiFrameBuffer>(width, height)},
      width_{width}, height_{height} {}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::~GGEMSProgressBar() { Stop(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Start() {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true,
                                        std::memory_order_acq_rel)) {
    return;
  }

  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  worker_ = std::jthread{[this](std::stop_token st) { RenderLoop(st); }};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Stop() {
  bool was_running = running_.exchange(false, std::memory_order_acq_rel);
  if (!was_running) {
    return;
  }

  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  Draw();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSProgressBar::AddSlot(std::string_view name, bool is_gpu) {
  slots_.emplace_back();
  auto &slot = slots_.back();
  slot.name_ = std::string{name};
  slot.is_gpu_.store(is_gpu, std::memory_order_relaxed);
  return slots_.size() - 1U;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::GetSlot(std::size_t index) noexcept {
  return slots_[index];
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t GGEMSProgressBar::GetSlotCount() const noexcept {
  return slots_.size();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::RenderLoop(std::stop_token st) {
  using namespace std::chrono_literals;

  while (!st.stop_requested()) {
    Draw();
    ++frame_counter_;
    std::this_thread::sleep_for(100ms);
  }

  Draw();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSProgressBar::Draw() {
  if (!framebuffer_) {
    return;
  }

  auto &fb = *framebuffer_;
  fb.Clear(' ', AsciiColour::Default);

  // Header
  std::string title = "=== GGEMS - Particule Monitor ===";
  std::size_t title_len = title.size();
  std::size_t title_x = (width_ > title_len) ? (width_ - title_len) / 2U : 0U;
  fb.DrawText(title_x, 0U, title, AsciiColour::Bright);
  // fb.DrawHLine(0U, 1U, width_, "─", AsciiColour::Grey);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::BuildBar(float progress, bool is_gpu) {
  constexpr std::size_t width = 50U;
  float clamped = std::clamp(progress, 0.0f, 1.0f);
  std::size_t filled =
      static_cast<std::size_t>(std::floor(clamped * static_cast<float>(width)));

  std::string bar;
  bar.reserve(width * 3);

  for (std::size_t i = 0; i < width; ++i) {
    bool is_filled = (i < filled);
    bar.append(is_filled ? "█" : "░");
  }

  return bar;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::BuildPulse(int phase) {
  switch (phase & 0x3) {
  case 0:
    return "pulse ●●●··· (γ)";
  case 1:
    return "pulse ·●●●·· (γ)";
  case 2:
    return "pulse ··●●●· (γ)";
  default:
    return "pulse ···●●● (γ)";
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::FormatETA(uint64_t ps) const noexcept {
  if (!std::isfinite(ps) || ps <= 0) {
    return "--";
  }

  return HumanReadable(Time{ps});
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string
GGEMSProgressBar::FormatBandwidth(long double bytes_per_ps) const noexcept {
  if (!(std::isfinite(bytes_per_ps) || bytes_per_ps <= 0)) {
    return "--";
  }

  return HumanReadable(Bandwidth{bytes_per_ps});
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t
GGEMSProgressBar::SlotBaseRow(std::size_t slot_index) const noexcept {
  constexpr std::size_t header_rows = 2U;
  constexpr std::size_t rows_per_slot = 5U;
  return header_rows + slot_index * rows_per_slot + 1U;
}
} // namespace ggems::core
