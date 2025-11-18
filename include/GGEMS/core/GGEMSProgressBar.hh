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
#include "GGEMS/core/units/GGEMSBandwidthUnits.hh"
#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_TARGET_OPENCL_VERSION 300

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include <CL/opencl.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
/// \endcond

namespace ggems::core {

class GGEMSProgressBar {
public:
  /*!
   \class Slot
   \brief Shared state for a single device progress entry.
   \details
   A Slot is designed to be owned via std::shared_ptr and updated by the
   device worker thread while the GGEMSProgressBar periodically renders
   a textual representation of its fields.

   Only the atomic fields shall be modified concurrently with rendering.
  */
  class Slot {
  public:
    enum class ParticleType { Unknown = 0, Electron, Positron, Gamma, Proton };

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

  GGEMSProgressBar(bool use_colour = true,
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
};

} // namespace ggems::core
