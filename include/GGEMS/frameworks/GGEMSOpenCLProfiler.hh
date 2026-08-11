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
  auto operator=(GGEMSOpenCLProfiler const &) -> GGEMSOpenCLProfiler & = delete;
  auto operator=(GGEMSOpenCLProfiler &&) -> GGEMSOpenCLProfiler & = delete;

  auto Reset() noexcept -> void;
  auto Start() noexcept -> void;
  auto Stop() noexcept -> void;

  auto RecordKernelEvent(cl::Event const &event) -> void;

  [[nodiscard]] auto IsRunning() const noexcept -> bool { return running_; }

  [[nodiscard]] auto HasMeasurement() const noexcept -> bool {
    return has_measurement_;
  }

  [[nodiscard]] auto HasKernelTiming() const noexcept -> bool {
    return kernel_timing_.valid;
  }

  [[nodiscard]] auto GetElapsedTime() const noexcept -> ggems::units::Duration;
  [[nodiscard]] auto GetKernelTime() const noexcept -> ggems::units::Duration;
  [[nodiscard]] auto GetCommandTime() const noexcept -> ggems::units::Duration;

  [[nodiscard]] auto GetElapsedSeconds() const noexcept -> double;
  [[nodiscard]] auto GetKernelSeconds() const noexcept -> double;

  [[nodiscard]] auto
  ComputeRatePerSecond(std::uint64_t item_count) const noexcept -> double;

  [[nodiscard]] auto
  ComputeKernelRatePerSecond(std::uint64_t item_count) const noexcept -> double;

  [[nodiscard]] auto GetKernelTiming() const noexcept
      -> GGEMSOpenCLKernelTiming const & {
    return kernel_timing_;
  }

private:
  [[nodiscard]] auto GetElapsedSecondsRaw() const noexcept -> long double;

  using Clock = std::chrono::steady_clock;

  Clock::time_point start_;
  Clock::time_point stop_;

  bool running_{false};
  bool has_measurement_{false};

  GGEMSOpenCLKernelTiming kernel_timing_{};
};

} // namespace ggems::ocl
