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
#include <chrono>
#include <cstdint>
#include <deque>
#include <string>
#include <thread>
/// \endcond

#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/render/GGEMSColourNames.hh"
#include "GGEMS/render/GGEMSTerminalFramebuffer.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"

namespace ggems::core {

class GGEMSProgressBar {
public:
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

    static char32_t ParticleSymbol(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return utf::Glyphs().gamma;
      case ParticleType::Proton:
        return utf::Glyphs().proton;
      case ParticleType::Electron:
        return utf::Glyphs().electron;
      case ParticleType::Positron:
        return utf::Glyphs().electron;
      case ParticleType::Neutron:
        return utf::Glyphs().neutron;
      case ParticleType::Alpha:
        return utf::Glyphs().alpha;
      case ParticleType::Aionino:
        return utf::Glyphs().aionino;
      }
    }

    static render::ColourKey ParticleColour(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return render::YELLOW_Gold_B;
      case ParticleType::Proton:
        return render::RED_Crimson_B;
      case ParticleType::Electron:
        return render::BLUE_Dodger_B;
      case ParticleType::Positron:
        return render::MAGENTA_Fuchsia_B;
      case ParticleType::Neutron:
        return render::CYAN_Frost_B;
      case ParticleType::Alpha:
        return render::CYAN_Marine_B;
      case ParticleType::Aionino:
        return render::BLUE_Ice_B;
      }
    }

    static constexpr std::u32string ParticleName(ParticleType p) {
      switch (p) {
      case ParticleType::Gamma:
        return U"gamma";
      case ParticleType::Proton:
        return U"proton";
      case ParticleType::Electron:
        return U"electron";
      case ParticleType::Positron:
        return U"positron";
      case ParticleType::Neutron:
        return U"neutron";
      case ParticleType::Alpha:
        return U"alpha";
      case ParticleType::Aionino:
        return U"aionino";
      }
    }

    static render::ColourKey StatusColour(Status status) {
      switch (status) {
      case Status::Pending:
        return render::CYAN_Frost;
      case Status::Running:
        return render::GREEN_Emerald;
      case Status::Finished:
        return render::BLUE_Azure;
      }
    }

    static std::u32string StatusName(Status status) {
      switch (status) {
      case Status::Pending:
        return U"pending";
      case Status::Running:
        return U"running";
      case Status::Finished:
        return U"finished";
      }
    }

    std::string name_;
    std::string kernel_name_;
    Status status_;
    ParticleType particle_type_{ParticleType::Gamma};
    bool is_gpu_{false};

    std::atomic<std::uint64_t> batches_done_{0};
    std::atomic<std::uint64_t> batches_total_{0};
    std::atomic<std::uint64_t> eta_ps_{0ULL};
    std::atomic<long double> bandwidth_byte_per_ps_{0.0};
    std::atomic<std::uint8_t> cpu_usage_percent_{0};
    std::atomic<std::uint8_t> gpu_usage_percent_{0};

    Slot &SetKernelName(std::string_view kernel) noexcept;
    Slot &SetStatus(Status status) noexcept;
    Slot &SetParticleType(ParticleType p) noexcept;
    Slot &SetBatchesDone(std::uint64_t done) noexcept;
    Slot &SetBatchesTotal(std::uint64_t total) noexcept;
    Slot &SetBandwidthBytesPerPicosecond(long double value) noexcept;
    Slot &SetETAPicoseconds(std::uint64_t eta_ps) noexcept;
    Slot &SetIsGPU(bool is_gpu) noexcept;
    Slot &SetCPUUsage(std::uint8_t value) noexcept;
    Slot &SetGPUUsage(std::uint8_t value) noexcept;
  };

public:
  GGEMSProgressBar() = default;
  ~GGEMSProgressBar();

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar const &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &&) = delete;

public:
  Slot &AddSlot(std::string_view name, bool is_gpu);
  Slot &GetSlot(std::size_t index) noexcept;

  void Start();
  void Stop();

  void SetFrameRate(std::chrono::milliseconds min_frame_time,
                    std::chrono::milliseconds max_frame_time) noexcept;

private:
  void RenderLoop(std::stop_token st);
  void PrepareFrame();
  void FlushFrame();

  void Draw();
  void DrawHeader();
  void DrawSlots();
  void DrawSingleSlot(std::size_t index, std::int16_t base_y);

  [[nodiscard]] static std::vector<char32_t> BuildBar(float progress);
  [[nodiscard]] static std::vector<char32_t>
  BuildPulse(Slot::ParticleType particle_type);

  [[nodiscard]] static std::u32string FormatPercentage(float progress);
  [[nodiscard]] static std::string FormatETA(std::uint64_t ps) noexcept;
  [[nodiscard]] static std::string
  FormatBandwidth(long double bytes_per_ps) noexcept;

  [[nodiscard]] std::chrono::milliseconds ComputeFrameTime() const noexcept;

  void DisableTerminal();
  void EnableTerminal();

private:
  // --- Concurrence / thread
  std::jthread worker_;
  std::atomic<bool> running_{false};

  // --- One slot each OpenCL context
  std::deque<Slot> slots_;

  // --- Layout
  std::int16_t content_width_{90};
  std::int16_t frame_height_{0};
  std::int16_t rows_per_slot_{5};
  std::int16_t header_rows_{4};
  std::int16_t footer_rows_{1};
  std::int16_t center_x_{0};
  std::int16_t center_y_{0};

  render::GGEMSTerminalFramebuffer framebuffer_;
  std::atomic<std::uint64_t> frame_counter_{0U};

  std::chrono::milliseconds min_frame_time_{1000};
  std::chrono::milliseconds max_frame_time_{300000};
};
} // namespace ggems::core
