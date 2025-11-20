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
  GGEMSProgressBar(std::size_t width_ = 100, std::size_t height = 30);
  ~GGEMSProgressBar();

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar const &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &&) = delete;

public:
  void Start();
  void Stop();

  struct Slot {
    enum class ParticleType {
      Gamma,
      Proton,
      Electron,
      Positron,
      Neutron,
      Alpha,
      Aionino
    };

    static std::string_view ParticleSymbol(ParticleType p) {
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

    std::string name_;
    std::string kernel_name_;
    std::string status_;
    ParticleType particle_type_;
    std::atomic<uint64_t> batches_done_{0};
    std::atomic<uint64_t> batches_total_{0};
    std::atomic<uint64_t> eta_ps_{0ULL};
    std::atomic<double> bandwidth_byte_per_ps_{0.0};
    std::atomic<bool> is_gpu_{false};
    std::atomic<int> phase_{0};

    Slot &SetBandwidth_bytes_per_ps(double bytes_per_ps);
    [[nodiscard]] inline double GetBandwidth_bytes_per_ps() const noexcept {
      return bandwidth_byte_per_ps_.load(std::memory_order_relaxed);
    }

    Slot &SetETA_ps(uint64_t eta_ps);
    [[nodiscard]] inline uint64_t GetETA_ps() const noexcept { return eta_ps_; }

    Slot &SetName(std::string_view name);
    [[nodiscard]] inline std::string GetName() const noexcept { return name_; }

    Slot &SetIsGPU(bool is_gpu);
    [[nodiscard]] inline bool IsGPU() const noexcept { return is_gpu_; }

    Slot &SetKernelName(std::string_view kernel);
    [[nodiscard]] inline std::string GetKernelName() const noexcept {
      return kernel_name_;
    }

    Slot &SetParticleType(ParticleType p);
    [[nodiscard]] inline ParticleType GetParticleType() const noexcept {
      return particle_type_;
    };

    Slot &SetBatchesTotal(uint64_t total);
    [[nodiscard]] inline uint64_t GetBatchesTotal() const noexcept {
      return batches_total_.load(std::memory_order_relaxed);
    }

    Slot &SetBatchesDone(uint64_t done);
    [[nodiscard]] inline uint64_t GetBatchesDone() const noexcept {
      return batches_done_.load(std::memory_order_relaxed);
    }

    Slot &SetStatus(std::string_view s);
    [[nodiscard]] std::string GetStatus() const noexcept { return status_; }
  };

  Slot &AddSlot(std::string_view name, bool is_gpu);
  Slot &GetSlot(std::size_t index) noexcept;
  inline std::size_t GetSlotCount() const noexcept { return slots_.size(); }

private:
  void RenderLoop(std::stop_token st);
  void Draw();
  [[nodiscard]] std::string BuildBar(float progress, bool is_gpu);
  [[nodiscard]] std::string BuildPulse(int phase);
  [[nodiscard]] std::string FormatETA(uint64_t ps) const noexcept;
  [[nodiscard]] std::string
  FormatBandwidth(long double bytes_per_ps) const noexcept;
  [[nodiscard]] std::size_t SlotBaseRow(std::size_t slot_index) const noexcept;

private:
  std::deque<Slot> slots_;
  std::jthread worker_;
  std::atomic<bool> running_{false};
  std::unique_ptr<render::GGEMSTerminalFramebuffer> framebuffer_;
  std::size_t width_;
  std::size_t height_;
  std::atomic<uint64_t> frame_counter_{0U};
};

/*public:
  class Slot {
  public:
    enum class ParticleType { Unknown = 0, Electron, Positron, Gamma, Proton
};

  public:
    Slot(std::string device_name, std::string kernel_name) noexcept;

    [[nodiscard]] std::string const &GetDeviceName() const noexcept;

    [[nodiscard]] std::string const &GetKernelName() const noexcept;

    void SetActive(bool active) noexcept;

    [[nodiscard]] bool IsActive() const noexcept;

    void SetBatches(int done, int total) noexcept;

    [[nodiscard]] int GetBatchesDone() const noexcept;

    [[nodiscard]] int GetBatchesTotal() const noexcept;

    void SetParticleType(ParticleType type) noexcept;

    [[nodiscard]] ParticleType GetParticleType() const noexcept;

    void SetBandwidthBytesPico(units::Bandwidth bw) noexcept;

    [[nodiscard]] units::Bandwidth GetBandwidthBytesPico() const noexcept;

    void SetDeviceType(cl_device_type t) noexcept {
      device_type_.store(t, std::memory_order_relaxed);
    }

    [[nodiscard]] cl_device_type GetDeviceType() const noexcept {
      return device_type_.load(std::memory_order_relaxed);
    }

  private:
    std::string device_name_;
    std::string kernel_name_;
    std::atomic<bool> active_{false};
    std::atomic<long double> bandwidth_bps_{0.0};
    std::atomic<int> batches_done_{0};
    std::atomic<int> batches_total_{0};
    std::atomic<ParticleType> particle_type_{ParticleType::Unknown};
    std::atomic<cl_device_type> device_type_{0};
  };

  using SlotPtr = std::shared_ptr<Slot>;
  using Clock = std::chrono::steady_clock;

  GGEMSProgressBar(std::size_t width = 100, std::size_t height = 40,
                   std::chrono::milliseconds frame_duration =
                       std::chrono::milliseconds{100});

  ~GGEMSProgressBar();

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar &&) = delete;

  [[nodiscard]] SlotPtr RegisterDevice(std::string device_name,
                                       std::string kernel_name);

  void SetEnabled(bool enabled) noexcept;

  [[nodiscard]] bool IsEnabled() const noexcept {
    return enabled_.load(std::memory_order_relaxed);
  }

  void Start();

  void Stop();

  void ClearScreenRegion();

private:
  void RenderLoop(std::stop_token stop_token);
  void RenderFrame(double t_seconds);
  static void EnableVirtualTerminalIfNeeded() noexcept;

  static std::string MakeClearScreen();
  static std::string MakeHeader();
  static std::string MakeFooter();

  static std::string FormatSlot(std::size_t index, Slot const &slot,
                                double t_seconds, bool use_colour);

  static std::string BuildBar(float progress, bool use_colour, bool is_gpu);

  static std::string BuildPulse(Slot const &slot, std::size_t device_index,
                                double t_seconds, bool use_colour);

private:
  using Slots = std::vector<SlotPtr>;

  std::mutex slots_mutex_;
  Slots slots_;

  std::chrono::milliseconds frame_duration_;
  bool first_frame_ = true;
  std::size_t last_rendered_height_ = 0;
  std::jthread worker_;
  std::atomic<bool> enabled_{true};
  std::atomic<bool> running_{false};
  bool use_colour_{true};
  std::unique_ptr<render::GGEMSAsciiFrameBuffer> framebuffer_;
};*/

} // namespace ggems::core
