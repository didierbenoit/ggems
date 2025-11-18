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

/*!
 \file GGEMSProgressBar.cc
 \brief Implementation of terminal pulse-based progress monitor.
*/

/// \cond
#include "GGEMS/core/GGEMSCoreUtils.hh"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#if defined(_WIN32)
#include <windows.h>
#endif
/// \endcond

#include "GGEMS/core/GGEMSProgressBar.hh"

namespace ggems::core {

/* -------------------------------------------------------------------------*/
/* Slot implementation                                                      */
/* -------------------------------------------------------------------------*/

GGEMSProgressBar::Slot::Slot(std::string device_name,
                             std::string kernel_name) noexcept
    : device_name_{std::move(device_name)},
      kernel_name_{std::move(kernel_name)} {
  // Nothing else to do.
}

/* --------------------------------*/

std::string const &GGEMSProgressBar::Slot::GetDeviceName() const noexcept {
  return device_name_;
}

/* --------------------------------*/

std::string const &GGEMSProgressBar::Slot::GetKernelName() const noexcept {
  return kernel_name_;
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetActive(bool const active) noexcept {
  active_.store(active, std::memory_order_relaxed);
}

/* --------------------------------*/

bool GGEMSProgressBar::Slot::IsActive() const noexcept {
  return active_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetProgress(float value) noexcept {
  if (!std::isfinite(value)) {
    value = 0.0F;
  }

  value = std::clamp(value, 0.0F, 1.0F);
  progress_.store(value, std::memory_order_relaxed);
}

/* --------------------------------*/

float GGEMSProgressBar::Slot::GetProgress() const noexcept {
  return progress_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetBandwidth(float value) noexcept {
  if (!std::isfinite(value) || value < 0.0F) {
    value = 0.0F;
  }

  bandwidth_gbs_.store(value, std::memory_order_relaxed);
}

/* --------------------------------*/

float GGEMSProgressBar::Slot::GetBandwidth() const noexcept {
  return bandwidth_gbs_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetParticles(std::uint64_t const done,
                                          std::uint64_t const total) noexcept {
  particles_done_.store(done, std::memory_order_relaxed);
  particles_total_.store(total, std::memory_order_relaxed);
}

/* --------------------------------*/

std::uint64_t GGEMSProgressBar::Slot::GetParticlesDone() const noexcept {
  return particles_done_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

std::uint64_t GGEMSProgressBar::Slot::GetParticlesTotal() const noexcept {
  return particles_total_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetParticleType(ParticleType const type) noexcept {
  particle_type_.store(type, std::memory_order_relaxed);
}

/* --------------------------------*/

GGEMSProgressBar::Slot::ParticleType
GGEMSProgressBar::Slot::GetParticleType() const noexcept {
  return particle_type_.load(std::memory_order_relaxed);
}

/* -------------------------------------------------------------------------*/
/* GGEMSProgressBar implementation                                          */
/* -------------------------------------------------------------------------*/

GGEMSProgressBar::GGEMSProgressBar(bool const use_colour,
                                   std::chrono::milliseconds frame_duration)
    : frame_duration_{frame_duration}, worker_{}, use_colour_{use_colour} {
  EnableVirtualTerminalIfNeeded();
}

/* --------------------------------*/

GGEMSProgressBar::~GGEMSProgressBar() { Stop(); }

/* --------------------------------*/

GGEMSProgressBar::SlotPtr
GGEMSProgressBar::RegisterDevice(std::string device_name,
                                 std::string kernel_name) {
  auto slot =
      std::make_shared<Slot>(std::move(device_name), std::move(kernel_name));

  {
    std::scoped_lock lock{slots_mutex_};
    slots_.push_back(slot);
  }

  return slot;
}

/* --------------------------------*/

void GGEMSProgressBar::ClearScreenRegion() {
  // Move up and erase only the lines belonging to the last HUD frame.
  for (std::size_t i = 0; i < last_rendered_height_; ++i) {
    std::cout << "\033[1A"; // move cursor up by one line
    std::cout << "\033[2K"; // clear entire line
  }
}

/* --------------------------------*/

void GGEMSProgressBar::SetEnabled(bool const enabled) noexcept {
  enabled_.store(enabled, std::memory_order_relaxed);
}

/* --------------------------------*/

void GGEMSProgressBar::Start() {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true,
                                        std::memory_order_acq_rel)) {
    // Already running; nothing to do.
    return;
  }

  // In case a previous run left a thread alive, ensure it is joined.
  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  first_frame_ = true;
  last_rendered_height_ = 0;

  worker_ = std::jthread{[this](std::stop_token st) { RenderLoop(st); }};
}

/* --------------------------------*/

void GGEMSProgressBar::Stop() {
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false,
                                        std::memory_order_acq_rel)) {
    // Not running; nothing to do.
    return;
  }

  if (worker_.joinable()) {
    worker_.request_stop();
    worker_.join();
  }

  // After a stop, we consider that the next Start() will rebuild a fresh HUD.
  first_frame_ = true;
  last_rendered_height_ = 0;
}

/* --------------------------------*/

void GGEMSProgressBar::RenderLoop(std::stop_token const stop_token) {
  auto const t0 = Clock::now();

  while (!stop_token.stop_requested()) {
    if (enabled_.load(std::memory_order_relaxed)) {
      auto const now = Clock::now();
      double const t_seconds =
          std::chrono::duration_cast<std::chrono::duration<double>>(now - t0)
              .count();

      RenderFrame(t_seconds);
    }

    std::this_thread::sleep_for(frame_duration_);
  }
}

/* --------------------------------*/

void GGEMSProgressBar::RenderFrame(double const t_seconds) {
  Slots local_slots;
  {
    std::scoped_lock lock{slots_mutex_};
    local_slots = slots_;
  }

  std::ostringstream oss;
  oss << MakeHeader();

  if (local_slots.empty()) {
    oss << "\n(no devices registered)\n\n";
  } else {
    for (std::size_t i = 0; i < local_slots.size(); ++i) {
      if (!local_slots[i]) {
        continue;
      }
      oss << "\n";
      oss << FormatSlot(i, *local_slots[i], t_seconds, use_colour_);
      oss << "\n";
    }
  }

  oss << MakeFooter();

  std::string hud = oss.str();
  std::size_t hud_lines =
      static_cast<std::size_t>(std::count(hud.begin(), hud.end(), '\n'));

  if (hud_lines == 0) {
    hud_lines = 1;
  }

  // Erase the previous HUD frame only after the first real frame.
  if (!first_frame_) {
    ClearScreenRegion();
  }

  std::cout << hud << std::flush;

  last_rendered_height_ = hud_lines;
  first_frame_ = false;
}

/* --------------------------------*/

void GGEMSProgressBar::EnableVirtualTerminalIfNeeded() noexcept {
#if defined(_WIN32)
  HANDLE const handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle == INVALID_HANDLE_VALUE) {
    return;
  }

  DWORD mode = 0;
  if (!GetConsoleMode(handle, &mode)) {
    return;
  }

  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  (void)SetConsoleMode(handle, mode);
#endif
}

/* --------------------------------*/

std::string GGEMSProgressBar::MakeClearScreen() {
  // Position cursor at home and clear downwards; more stable on Windows
  // than a full 2J clear when mixed with other outputs.
  return "\033[H\033[J";
}

/* --------------------------------*/

std::string GGEMSProgressBar::MakeHeader() {
  constexpr std::size_t width = 80;

  std::ostringstream oss;

  // Top
  oss << "╭────────────────────────────────────────────────────────────────────"
         "────────────╮\n";

  oss << "│                       === GGEMS - Particle Monitor ===             "
         "  "
         "          │\n";

  // Separator
  oss << "├────────────────────────────────────────────────────────────────────"
         "────────────┤\n";

  return oss.str();
}

/* --------------------------------*/

std::string GGEMSProgressBar::MakeFooter() {
  return "╰────────────────────────────────────────────────────────────────────"
         "────────────╯\n";
}

/* --------------------------------*/

std::string GGEMSProgressBar::FormatSlot(std::size_t const index,
                                         Slot const &slot,
                                         double const t_seconds,
                                         bool const use_colour) {
  std::ostringstream oss;

  bool const active = slot.IsActive();
  float const progress = slot.GetProgress();
  float const bandwidth = slot.GetBandwidth();

  cl_device_type type = slot.GetDeviceType();
  bool const is_gpu = (type & CL_DEVICE_TYPE_GPU) != 0;
  bool const is_cpu = (!is_gpu) && ((type & CL_DEVICE_TYPE_CPU) != 0);

  std::string device_prefix;
  std::string colour_reset;
  if (use_colour) {
    if (is_gpu) {
      device_prefix = "\033[32m"; // green for GPU
    } else if (is_cpu) {
      device_prefix = "\033[34m"; // blue for CPU
    }
    colour_reset = "\033[0m";
  }

  // Thread-like tag + coloured device name.
  oss << "[T" << (index + 1U) << "] ";
  if (!device_prefix.empty()) {
    oss << device_prefix << slot.GetDeviceName() << colour_reset;
  } else {
    oss << slot.GetDeviceName();
  }
  oss << "\n";

  // Status bar.
  oss << "Status: " << BuildBar(progress, use_colour, is_gpu) << "  "
      << std::setw(3) << static_cast<int>(progress * 100.0F) << "%";

  if (active) {
    oss << "   (pulse ";

    // Seven-point travelling pulse.
    std::string const pulse_str =
        BuildPulse(slot, index, t_seconds, use_colour);
    oss << pulse_str;

    // Particle label outside the pulse, as (e+), (e-), (γ), (p) ...
    std::string particle_label;
    switch (slot.GetParticleType()) {
    case Slot::ParticleType::Electron:
      particle_label = "e-";
      break;
    case Slot::ParticleType::Positron:
      particle_label = "e+";
      break;
    case Slot::ParticleType::Gamma:
      particle_label = "γ";
      break;
    case Slot::ParticleType::Proton:
      particle_label = "p";
      break;
    case Slot::ParticleType::Unknown:
    default:
      particle_label = "?";
      break;
    }

    oss << " (" << particle_label << ")";
    oss << ")";
  } else {
    oss << "   (idle)";
  }

  oss << "\n";

  // Bandwidth + kernel.
  oss << "        ↳ Bandwidth: " << std::fixed << std::setprecision(1)
      << bandwidth << " GB/s";

  std::string const &kernel_name = slot.GetKernelName();
  if (!kernel_name.empty()) {
    oss << "  |  Kernel: " << kernel_name;
  }
  oss << "\n";

  // Optional particle statistics and ETA.
  std::uint64_t const particles_done = slot.GetParticlesDone();
  std::uint64_t const particles_total = slot.GetParticlesTotal();

  if (particles_total > 0U) {
    oss << "        ↳ Particles: " << particles_done << " / "
        << particles_total;

    double eta_seconds = -1.0;
    double const p = static_cast<double>(progress);
    if (p > 1.0e-3 && t_seconds > 1.0e-3) {
      // Simple ETA estimate: t / p - t.
      eta_seconds = t_seconds * (1.0 / p - 1.0);
    }

    if (eta_seconds > 0.0) {
      oss << "  : " << std::fixed << std::setprecision(2) << eta_seconds
          << " s ETA";
    } else {
      oss << "  : ETA --";
    }

    oss << "\n";
  }

  return oss.str();
}

/* --------------------------------*/

std::string GGEMSProgressBar::BuildPulse(Slot const &slot,
                                         std::size_t const device_index,
                                         double const t_seconds,
                                         bool const use_colour) {
  constexpr std::size_t width = 7U;

  double const speed = 6.0;
  double const phase = static_cast<double>(device_index) * 0.5;
  double const pos =
      std::fmod(speed * t_seconds + phase, static_cast<double>(width));
  std::size_t const head = static_cast<std::size_t>(pos);

  std::ostringstream oss;

  std::string prefix;
  std::string suffix;
  if (use_colour) {
    switch (slot.GetParticleType()) {
    case Slot::ParticleType::Electron:
      prefix = "\033[36m"; // cyan
      break;
    case Slot::ParticleType::Positron:
      prefix = "\033[35m"; // magenta
      break;
    case Slot::ParticleType::Gamma:
      prefix = "\033[33m"; // yellow
      break;
    case Slot::ParticleType::Proton:
      prefix = "\033[31m"; // red
      break;
    case Slot::ParticleType::Unknown:
    default:
      prefix = "\033[37m"; // grey
      break;
    }
    suffix = "\033[0m";
  }

  oss << prefix;

  for (std::size_t i = 0; i < width; ++i) {
    if (i < head) {
      oss << "●"; // trail
    } else if (i == head) {
      oss << "●"; // head
    } else {
      oss << "·"; // empty path
    }
  }

  oss << suffix;

  return oss.str();
}

/* --------------------------------*/

std::string GGEMSProgressBar::BuildBar(float const progress,
                                       bool const use_colour,
                                       bool const is_gpu) {
  constexpr std::size_t width = 73U;

  float const clamped = std::clamp(progress, 0.0F, 1.0F);
  std::size_t const filled =
      static_cast<std::size_t>(std::floor(clamped * static_cast<float>(width)));

  std::ostringstream oss;

  for (std::size_t i = 0; i < width; ++i) {
    bool const is_f = (i < filled);
    float const ratio = static_cast<float>(i) / static_cast<float>(width);

    if (use_colour) {
      if (is_f) {
        if (is_gpu) {
          if (ratio < 0.33f) {
            oss << "\033[1;32m";
          } else if (ratio < 0.66f) {
            oss << "\033[1;32m";
          } else {
            oss << "\033[1;32m";
          }
        } else {
          if (ratio < 0.33f) {
            oss << "\033[1;34m";
          } else if (ratio < 0.66f) {
            oss << "\033[1;34m";
          } else {
            oss << "\033[1;34m";
          }
        }
        oss << "█";
      } else {
        oss << "\033[90m░";
      }
    } else {
      oss << (is_f ? "█" : " ");
    }
  }

  if (use_colour) {
    oss << "\033[0m";
  }

  return oss.str();
}

/* -------------------------------------------------------------------------*/

} // namespace ggems::core
