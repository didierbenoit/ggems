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

/*!
 \file GGEMSProgressBar.hh
 \brief Terminal pulse-based progress monitor for GGEMS devices.
 \author Julien Bert
 \author Didier Benoit
 \date 2025-10-29
 \version 2.0
 \copyright GNU GPL v3.0
*/

/// \cond
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

/*!
 \class GGEMSProgressBar
 \brief Terminal-based multi-device progress and pulse monitor.
 \details
 This class manages an optional background rendering thread that periodically
 draws an animated, ANSI-coloured “pulse” style progress view for multiple
 compute devices (CPU / GPU). Each device is represented by a \ref Slot
 instance, which exposes a minimal thread-safe interface for:
 - progress update in \f$[0,1]\f$,
 - effective bandwidth in GB/s,
 - logical activity state,
 - particle counters and type.

 Thread-safety:
 - Each \ref Slot exposes only atomic fields for concurrent updates.
 - Static metadata (device name, kernel name) are written before registration
   and are read-only afterwards, to avoid data races.
 - The internal container of slots is protected by a mutex.

 Lifetime and threading:
 - The constructor does \b not start any thread.
 - Call \ref Start() once the slots are registered and the run is about to
   begin; this spawns an internal std::jthread running the render loop.
 - Call \ref Stop() to terminate the render thread explicitly; the destructor
   calls \ref Stop() automatically if needed.
*/
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
    /*!
     \brief Type of primary particle associated with this slot.
    */
    enum class ParticleType { Unknown = 0, Electron, Positron, Gamma, Proton };

  public:
    /*!
     \brief Construct a slot with static metadata.
     \param device_name Human-readable device name.
     \param kernel_name Human-readable kernel name (optional).
    */
    Slot(std::string device_name, std::string kernel_name) noexcept;

    //! \return Device name.
    [[nodiscard]] std::string const &GetDeviceName() const noexcept;

    //! \return Kernel name.
    [[nodiscard]] std::string const &GetKernelName() const noexcept;

    /*!
     \brief Set the logical activity state.
     \param active Boolean flag indicating whether the device is currently
                   participating in the run.
    */
    void SetActive(bool active) noexcept;

    //! \return True if device is active.
    [[nodiscard]] bool IsActive() const noexcept;

    /*!
     \brief Set current progress in \f$[0,1]\f$.
     \param value Normalised progress value; values are clamped to \f$[0,1]\f$.
    */
    void SetProgress(float value) noexcept;

    //! \return Current progress in \f$[0,1]\f$.
    [[nodiscard]] float GetProgress() const noexcept;

    /*!
     \brief Set current and total particle counters for this device.
     \param done   Number of already simulated particles.
     \param total  Total number of particles to be simulated.
    */
    void SetParticles(std::uint64_t done, std::uint64_t total) noexcept;

    //! \return Number of already simulated particles.
    [[nodiscard]] std::uint64_t GetParticlesDone() const noexcept;

    //! \return Total number of particles to be simulated.
    [[nodiscard]] std::uint64_t GetParticlesTotal() const noexcept;

    /*!
     \brief Set the primary particle type associated with this slot.
    */
    void SetParticleType(ParticleType type) noexcept;

    /*!
     \brief Get the primary particle type associated with this slot.
    */
    [[nodiscard]] ParticleType GetParticleType() const noexcept;

    /*!
     \brief Set current effective bandwidth in GB/s.
     \param value Bandwidth value, expected to be non-negative.
    */
    void SetBandwidth(float value) noexcept;

    //! \return Current bandwidth in GB/s.
    [[nodiscard]] float GetBandwidth() const noexcept;

    /*!
     \brief Set the OpenCL device type for this slot.
     \details
     This is used to choose colouring (CPU vs GPU) in the HUD.
    */
    void SetDeviceType(cl_device_type t) noexcept {
      device_type_.store(t, std::memory_order_relaxed);
    }

    /*!
     \brief Get the OpenCL device type for this slot.
    */
    [[nodiscard]] cl_device_type GetDeviceType() const noexcept {
      return device_type_.load(std::memory_order_relaxed);
    }

  private:
    std::string device_name_;
    std::string kernel_name_;
    std::atomic<bool> active_{false};
    std::atomic<float> progress_{0.0F};
    std::atomic<float> bandwidth_gbs_{0.0F};
    std::atomic<std::uint64_t> particles_done_{0U};
    std::atomic<std::uint64_t> particles_total_{0U};
    std::atomic<ParticleType> particle_type_{ParticleType::Unknown};
    std::atomic<cl_device_type> device_type_{0};
  };

  using SlotPtr = std::shared_ptr<Slot>;
  using Clock = std::chrono::steady_clock;

  /*!
   \brief Construct a GGEMSProgressBar.
   \param use_colour Flag enabling ANSI colour output when true.
   \param frame_duration Refresh period for the progress display.
   \details
   The constructor does not start the internal rendering thread. Call
   \ref Start() explicitly when the run is about to begin.
  */
  GGEMSProgressBar(
      bool use_colour = true,
      std::chrono::milliseconds frame_duration = std::chrono::milliseconds{40});

  /*!
   \brief Destructor.
   \details
   Calls \ref Stop() to ensure the internal std::jthread, if any, is
   requested to stop and joined before destruction.
  */
  ~GGEMSProgressBar();

  GGEMSProgressBar(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar const &) = delete;
  GGEMSProgressBar(GGEMSProgressBar &&) = delete;
  GGEMSProgressBar &operator=(GGEMSProgressBar &&) = delete;

  /*!
   \brief Register a new device slot.
   \details
   This function is thread-safe and can be called from the main thread
   before launching worker threads.

   \param device_name Human-readable device label.
   \param kernel_name Initial kernel name (can be an empty string).
   \return Shared pointer to a Slot that can be updated by worker threads.
  */
  [[nodiscard]] SlotPtr RegisterDevice(std::string device_name,
                                       std::string kernel_name);

  /*!
   \brief Enable or disable rendering without stopping the thread.
   \details
   When disabled, the internal render loop stays alive but skips frame
   generation and only sleeps for the configured frame duration.
   \param enabled Boolean flag controlling rendering.
  */
  void SetEnabled(bool enabled) noexcept;

  /*!
   \brief Query whether rendering is currently enabled.
   \return True when rendering is active.
  */
  [[nodiscard]] bool IsEnabled() const noexcept {
    return enabled_.load(std::memory_order_relaxed);
  }

  /*!
   \brief Start the internal rendering thread.
   \details
   If the rendering thread is already running, this function is a no-op.
   Thread-safe with respect to repeated calls on the same instance when
   used from a single controlling thread (e.g. GGEMSRun).
  */
  void Start();

  /*!
   \brief Request stop and join the internal rendering thread.
   \details
   If no rendering thread is running, this function is a no-op. It is
   safe to call multiple times, including from the destructor.
  */
  void Stop();

  /*!
   \brief Clear the screen region occupied by the last rendered HUD.
   \details
   This utility method moves the cursor up and erases the number of
   lines corresponding to the last frame. It can be used after a run
   to clean up the terminal if desired.
  */
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

  /*!
   \brief Build a travelling pulse with seven points for a given slot.
   \param slot          Slot used to query particle type.
   \param device_index  Zero-based device index (phase offset).
   \param t_seconds     Global elapsed time in seconds.
   \param use_colour    Whether ANSI colours are enabled.
   \return ASCII pulse string, e.g. "●●●····".
  */
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
