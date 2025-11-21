#pragma once

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
#include <atomic>
#include <cstdint>
#include <deque>
#include <string>
#include <thread>
/// \endcond

#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"

namespace ggems::core {

class GGEMSProgressBar {
public:
  GGEMSProgressBar(std::int16_t width_ = 80, std::int16_t height = 24);
  ~GGEMSProgressBar();

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar const &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &&) = delete;

public:
  void Start();
  void Stop();

  struct Slot {
    enum class ParticleType : std::uint8_t {
      Gamma,
      Proton,
      Electron,
      Positron,
      Neutron,
      Alpha,
      Aionino
    };

    enum class Status : std::uint8_t { Pending, Running, Finished };

    static std::string ParticleSymbol(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return "γ";
      case ParticleType::Proton:
        return "p";
      case ParticleType::Electron:
        return "e⁻";
      case ParticleType::Positron:
        return "e⁺";
      case ParticleType::Neutron:
        return "n";
      case ParticleType::Alpha:
        return "α";
      case ParticleType::Aionino:
        return "λ";
      }
    }

    static render::AsciiColour ParticleColour(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return render::AsciiColour::Green;
      case ParticleType::Proton:
        return render::AsciiColour::Red;
      case ParticleType::Electron:
        return render::AsciiColour::Blue;
      case ParticleType::Positron:
        return render::AsciiColour::Magenta;
      case ParticleType::Neutron:
        return render::AsciiColour::Grey;
      case ParticleType::Alpha:
        return render::AsciiColour::Cyan;
      case ParticleType::Aionino:
        return render::AsciiColour::Blue;
      }
    }

    static std::string ParticleName(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return "gamma";
      case ParticleType::Proton:
        return "proton";
      case ParticleType::Electron:
        return "electron";
      case ParticleType::Positron:
        return "positron";
      case ParticleType::Neutron:
        return "neutron";
      case ParticleType::Alpha:
        return "alpha";
      case ParticleType::Aionino:
        return "aionino";
      }
    }

    static render::AsciiColour StatusColour(Status status) {
      switch (status) {
      case Status::Pending:
        return render::AsciiColour::Grey;
      case Status::Running:
        return render::AsciiColour::Green;
      case Status::Finished:
        return render::AsciiColour::Red;
      }
    }

    static std::string StatusName(Status status) {
      switch (status) {
      case Status::Pending:
        return "pending";
      case Status::Running:
        return "running";
      case Status::Finished:
        return "finished";
      }
    }

    std::string name_;
    std::string kernel_name_;
    Status status_;
    ParticleType particle_type_;
    std::atomic<std::uint64_t> batches_done_{0};
    std::atomic<std::uint64_t> batches_total_{0};
    std::atomic<std::uint64_t> eta_ps_{0ULL};
    std::atomic<long double> bandwidth_byte_per_ps_{0.0};
    std::atomic<bool> is_gpu_{false};

    Slot &SetBandwidth_bytes_per_ps(long double bytes_per_ps);
    Slot &SetETA_ps(std::uint64_t eta_ps);
    Slot &SetName(std::string_view name);
    Slot &SetIsGPU(bool is_gpu);
    Slot &SetKernelName(std::string_view kernel);
    Slot &SetParticleType(ParticleType p);
    Slot &SetBatchesTotal(std::uint64_t total);
    Slot &SetBatchesDone(std::uint64_t done);
    Slot &SetStatus(Status status);
  };

  Slot &AddSlot(std::string_view name, bool is_gpu);
  Slot &GetSlot(std::size_t index) noexcept;

private:
  void RenderLoop(std::stop_token st);
  void Draw();
  [[nodiscard]] std::vector<std::string> BuildBar(float progress);
  [[nodiscard]] std::vector<std::string>
  BuildPulse(Slot::ParticleType particle_type);
  [[nodiscard]] std::string FormatETA(std::uint64_t ps) const noexcept;
  [[nodiscard]] std::string
  FormatBandwidth(long double bytes_per_ps) const noexcept;
  [[nodiscard]] std::size_t SlotBaseRow(std::size_t slot_index) const noexcept;

  void DisableTerminal();
  void EnableTerminal();
  void EnsureFramebufferSize();

private:
  std::deque<Slot> slots_;
  std::jthread worker_;
  std::atomic<bool> running_{false};
  std::unique_ptr<render::GGEMSTerminalFramebuffer> framebuffer_;
  std::int16_t width_;
  std::int16_t height_;
  std::atomic<std::uint64_t> frame_counter_{0U};
};
} // namespace ggems::core
