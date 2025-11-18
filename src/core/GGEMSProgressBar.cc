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
#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/core/units/GGEMSBandwidthUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#if defined(_WIN32)
#include <windows.h>
#endif
/// \endcond

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSProgressBar.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::core {

/* -------------------------------------------------------------------------*/
/* Slot implementation                                                      */
/* -------------------------------------------------------------------------*/

GGEMSProgressBar::Slot::Slot(std::string device_name,
                             std::string kernel_name) noexcept
    : device_name_{std::move(device_name)},
      kernel_name_{std::move(kernel_name)} {}

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

void GGEMSProgressBar::Slot::SetBandwidthBytesPico(
    units::Bandwidth bw) noexcept {
  bandwidth_bps_.store(bw.value, std::memory_order_relaxed);
}

units::Bandwidth
GGEMSProgressBar::Slot::GetBandwidthBytesPico() const noexcept {
  long double v = bandwidth_bps_.load(std::memory_order_relaxed);
  return units::Bandwidth{v};
}

/* --------------------------------*/

void GGEMSProgressBar::Slot::SetBatches(int const done,
                                        int const total) noexcept {
  batches_done_.store(done, std::memory_order_relaxed);
  batches_total_.store(total, std::memory_order_relaxed);
}

/* --------------------------------*/

int GGEMSProgressBar::Slot::GetBatchesDone() const noexcept {
  return batches_done_.load(std::memory_order_relaxed);
}

/* --------------------------------*/

int GGEMSProgressBar::Slot::GetBatchesTotal() const noexcept {
  return batches_total_.load(std::memory_order_relaxed);
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
  std::ostringstream oss;
  // Top
  oss << "╭────────────────────────────────────────────────────────────────────"
         "────────────╮\n";
  oss << "│                       === GGEMS - Particle Monitor ===             "
         "  "
         "          │\n";
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

  int batches_done = slot.GetBatchesDone();
  int batches_total = slot.GetBatchesTotal();

  float ratio = (batches_total > 0) ? static_cast<float>(batches_done) /
                                          static_cast<float>(batches_total)
                                    : 0.0f;

  // Status bar.
  oss << "Status: " << BuildBar(ratio, use_colour, is_gpu) << "  "
      << std::setw(3) << ratio * 100.0F << "%";

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

  units::Bandwidth bw = slot.GetBandwidthBytesPico();
  oss << "        ↳ Bandwidth: " << units::HumanReadable(bw);

  std::string const &kernel_name = slot.GetKernelName();
  if (!kernel_name.empty()) {
    oss << "  |  Kernel: " << kernel_name;
  }
  oss << "\n";

  if (batches_total > 0) {
    oss << "        ↳ Batches: " << batches_done << " / " << batches_total;

    double eta_seconds = -1.0;
    if (ratio > 1.0e-3 && t_seconds > 1.0e-3) {
      // Simple ETA estimate: t / p - t.
      eta_seconds = t_seconds * (1.0 / ratio - 1.0);
    }

    units::Time eta{static_cast<std::uint64_t>(eta_seconds * 10e12)};
    if (eta_seconds > 0.0) {
      oss << "  : " << units::HumanReadable(eta) << " ETA";
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

  // Discrete colour index (0–9)
  int const colour_index =
      static_cast<int>(std::floor(clamped * 10.0F)); // 10 colours
  int const idx = std::clamp(colour_index, 0, 9);

  // GPU: green gradient
  static constexpr std::array<const char *, 10> gpu_colours = {
      "\033[38;2;0;45;0m",  "\033[38;2;0;60;0m",  "\033[38;2;0;75;0m",
      "\033[38;2;0;90;0m",  "\033[38;2;0;110;0m", "\033[38;2;0;130;0m",
      "\033[38;2;0;160;0m", "\033[38;2;0;190;0m", "\033[38;2;0;220;0m",
      "\033[38;2;0;255;0m",
  };

  // CPU: blue gradient
  static constexpr std::array<const char *, 10> cpu_colours = {
      "\033[38;2;0;0;45m",  "\033[38;2;0;0;70m",  "\033[38;2;0;0;90m",
      "\033[38;2;0;0;115m", "\033[38;2;0;0;140m", "\033[38;2;0;0;165m",
      "\033[38;2;0;0;190m", "\033[38;2;0;0;210m", "\033[38;2;0;0;230m",
      "\033[38;2;0;0;255m",
  };

  char const *active_colour =
      (is_gpu ? gpu_colours[static_cast<std::size_t>(idx)]
              : cpu_colours[static_cast<std::size_t>(idx)]);

  constexpr char const *inactive_colour = "\033[38;2;70;70;70m"; // gris
  constexpr char const *reset_colour = "\033[0m";

  std::ostringstream oss;

  for (std::size_t i = 0; i < width; ++i) {
    bool const is_filled = (i < filled);
    if (use_colour) {
      if (is_filled) {
        oss << active_colour << "█";
      } else {
        oss << inactive_colour << "░";
      }
    } else {
      oss << (is_filled ? "█" : " ");
    }
  }

  if (use_colour) {
    oss << reset_colour;
  }

  return oss.str();
}

/* -------------------------------------------------------------------------*/

} // namespace ggems::core
