#pragma once

#include <chrono>
#include <cstdint>

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::ocl {

struct GGEMSOpenCLKernelTiming {
  ggems::units::Time time_queued{0U};
  ggems::units::Time time_submit{0U};
  ggems::units::Time time_start{0U};
  ggems::units::Time time_end{0U};

  ggems::units::Duration command_time{0U};
  ggems::units::Duration kernel_time{0U};

  bool valid{false};
};

class GGEMSOpenCLProfiler {
public:
  GGEMSOpenCLProfiler() = default;
  ~GGEMSOpenCLProfiler() = default;

  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler const &) = delete;
  GGEMSOpenCLProfiler(GGEMSOpenCLProfiler &&) = delete;
  GGEMSOpenCLProfiler &operator=(GGEMSOpenCLProfiler const &) = delete;
  GGEMSOpenCLProfiler &operator=(GGEMSOpenCLProfiler &&) = delete;

public:
  void Reset() noexcept;
  void Start() noexcept;
  void Stop() noexcept;

  void RecordKernelEvent(cl::Event const &event);

  [[nodiscard]] bool IsRunning() const noexcept { return running_; }

  [[nodiscard]] bool HasMeasurement() const noexcept {
    return has_measurement_;
  }

  [[nodiscard]] bool HasKernelTiming() const noexcept {
    return kernel_timing_.valid;
  }

  [[nodiscard]] auto GetElapsedTime() const noexcept -> ggems::units::Duration;
  [[nodiscard]] auto GetKernelTime() const noexcept -> ggems::units::Duration;
  [[nodiscard]] auto GetCommandTime() const noexcept -> ggems::units::Duration;

  [[nodiscard]] double GetElapsedSeconds() const noexcept;
  [[nodiscard]] double GetKernelSeconds() const noexcept;

  [[nodiscard]] double
  ComputeRatePerSecond(std::uint64_t item_count) const noexcept;

  [[nodiscard]] double
  ComputeKernelRatePerSecond(std::uint64_t item_count) const noexcept;

  [[nodiscard]] GGEMSOpenCLKernelTiming const &
  GetKernelTiming() const noexcept {
    return kernel_timing_;
  }

private:
  [[nodiscard]] long double GetElapsedSecondsRaw() const noexcept;

private:
  using Clock = std::chrono::steady_clock;

  Clock::time_point start_{};
  Clock::time_point stop_{};

  bool running_{false};
  bool has_measurement_{false};

  GGEMSOpenCLKernelTiming kernel_timing_{};
};

} // namespace ggems::ocl
