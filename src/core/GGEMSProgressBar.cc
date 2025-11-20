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
#include <cstdint>
#include <iostream>
#include <memory>
/// \endcond

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSProgressBar.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::core {
using render::AsciiColour;
using render::GGEMSTerminalFramebuffer;
using units::Bandwidth;
using units::HumanReadable;
using units::Time;

// ----- Slot ----------------------------------------------

GGEMSProgressBar::Slot &GGEMSProgressBar::AddSlot(std::string_view name,
                                                  bool is_gpu) {
  slots_.emplace_back();
  auto &slot = slots_.back();
  slot.SetName(name).SetIsGPU(is_gpu);
  return slot;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetName(std::string_view name) {
  name_ = name;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetBatchesDone(uint64_t done) {
  batches_done_.store(done, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetBatchesTotal(uint64_t total) {
  batches_total_.store(total, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetParticleType(ParticleType p) {
  particle_type_ = p;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &
GGEMSProgressBar::Slot::SetKernelName(std::string_view kernel_name) {
  kernel_name_ = kernel_name;
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetIsGPU(bool is_gpu) {
  is_gpu_.store(is_gpu, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetETA_ps(uint64_t eta_ps) {
  eta_ps_.store(eta_ps, std::memory_order_relaxed);
  return *this;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetBandwidth_bytes_per_ps(
    double bandwidth_byte_per_ps) {
  bandwidth_byte_per_ps_.store(bandwidth_byte_per_ps,
                               std::memory_order_relaxed);
  return *this;
}

GGEMSProgressBar::Slot &GGEMSProgressBar::Slot::SetStatus(std::string_view s) {
  status_ = s;
  return *this;
}

// ----- Progress Bar ------------------------------

GGEMSProgressBar::GGEMSProgressBar(std::size_t width, std::size_t height)
    : framebuffer_{std::make_unique<GGEMSTerminalFramebuffer>(width, height)},
      width_{width}, height_{height} {}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSProgressBar::Slot &GGEMSProgressBar::GetSlot(std::size_t index) noexcept {
  return slots_[index];
}

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
  std::cout << "\033[?1049h";
  std::cout << "\033[2J\033[H";
  std::cout << "\033[?12l";
  std::cout << "\033[?25l";
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
  std::cout << "\033[?1049l";
  std::cout << "\033[?12h";
  std::cout << "\033[?25h";
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
  fb.Clear(" ", AsciiColour::Default);

  // Header
  std::string title = "=== GGEMS - Particule Monitor ===";
  std::size_t title_len = title.size();
  std::size_t title_x = (width_ > title_len) ? (width_ - title_len) / 2U : 0U;
  fb.DrawString(title_x, 0U, title, AsciiColour::Bright);
  fb.DrawHLine(0U, 1U, width_, "─", AsciiColour::Grey);

  // Device slots
  std::size_t n_slots = slots_.size();
  for (std::size_t i = 0; i < n_slots; ++i) {
    Slot &slot = slots_[i];
    std::size_t base_y = SlotBaseRow(i);

    if (base_y + 4U >= height_) {
      break; // No more vertical space.
    }

    bool is_gpu = slot.is_gpu_.load(std::memory_order_relaxed);

    // Header line: [Tn] name
    std::string hdr = std::format("[T{}] {}", i + 1U, slot.name_);
    fb.DrawString(2U, base_y, hdr,
                  is_gpu ? AsciiColour::Green : AsciiColour::Blue);

    // Building progress bar
    std::uint64_t done = slot.batches_done_.load(std::memory_order_relaxed);
    std::uint64_t total = slot.batches_total_.load(std::memory_order_relaxed);

    float progress{0.0};
    if (total > 0) {
      progress = static_cast<float>(done) / static_cast<float>(total);
    } else {
      progress = 0.0;
    }

    std::string bar = BuildBar(progress, is_gpu);

    std::string perc =
        std::format("{:6.2f}%", std::clamp(progress, 0.0F, 1.0F) * 100.0F);

    int local_phase =
        static_cast<int>(frame_counter_.load(std::memory_order_relaxed) % 4U);
    std::string pulse = BuildPulse(local_phase);

    fb.DrawString(4U, base_y + 1U, "Status: ", AsciiColour::Default);
    fb.DrawString(13U, base_y + 1U, bar,
                  is_gpu ? AsciiColour::Green : AsciiColour::Blue);
    fb.DrawString(13U + bar.size() + 1U, base_y + 1U, perc,
                  AsciiColour::Bright);
    fb.DrawString(width_ > 18U ? width_ - 18U : 0U, base_y + 1U, pulse,
                  AsciiColour::Yellow);

    // Bandwidth and kernel info line
    long double bw_ps =
        slot.bandwidth_byte_per_ps_.load(std::memory_order_relaxed);
    std::string bw_text = FormatBandwidth(bw_ps);

    // Kernel name could be integrated later; for now we keep only bandwidth.
    fb.DrawString(8U, base_y + 2U, std::format("↳ Bandwidth: {}", bw_text),
                  AsciiColour::Cyan);

    uint64_t eta_s = slot.eta_ps_.load(std::memory_order_relaxed);

    std::string eta_text = FormatETA(eta_s);

    fb.DrawString(
        8U, base_y + 3U,
        std::format("↳ Batches: {} / {}  : ETA {}", done, total, eta_text),
        AsciiColour::Default);
  }

  // Optional footer line
  fb.DrawHLine(0U, height_ > 1U ? height_ - 1U : 0U, width_, "─",
               AsciiColour::Grey);

  // Present frame.
  std::string const frame = fb.Render();

  // Move cursor to home and print the frame.
  std::cout << "\033[H" << frame << std::flush;
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
  bar.reserve(width);

  for (std::size_t i = 0; i < width; ++i) {
    bool is_filled = (i < filled);
    bar.append(is_filled ? "#" : "-");
  }

  return bar;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string GGEMSProgressBar::BuildPulse(int phase) {
  switch (phase & 0x3) {
  case 0:
    return "pulse (γ)";
  case 1:
    return "pulse (γ)";
  case 2:
    return "pulse (γ)";
  default:
    return "pulse (γ)";
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
