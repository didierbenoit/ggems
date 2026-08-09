#include <limits>

#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::ocl {

namespace {
auto MakeDurationFromSeconds(long double seconds) noexcept
    -> ggems::units::Duration {
  if (seconds <= 0.0L) {
    return ggems::units::Duration{0U};
  }

  auto const duration =
      ggems::units::TryMakeQuantity<ggems::units::Duration>(seconds, "s");

  return duration.value_or(
      ggems::units::Duration{std::numeric_limits<std::uint64_t>::max()});
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Time MakeTimeFromNanoseconds(cl_ulong nanoseconds) noexcept {
  if (nanoseconds >= std::numeric_limits<std::uint64_t>::max() / 1000ULL) {
    return ggems::units::Time{std::numeric_limits<std::uint64_t>::max()};
  }

  auto const time =
      ggems::units::TryMakeQuantity<ggems::units::Time>(nanoseconds, "ns");
  return time.value_or(
      ggems::units::Time{std::numeric_limits<std::uint64_t>::max()});
}

} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSOpenCLProfiler::Reset() noexcept {
  start_ = Clock::time_point{};
  stop_ = Clock::time_point{};
  running_ = false;
  has_measurement_ = false;
  kernel_timing_ = GGEMSOpenCLKernelTiming{};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSOpenCLProfiler::Start() noexcept {
  start_ = Clock::now();
  stop_ = Clock::time_point{};
  running_ = true;
  has_measurement_ = false;
  kernel_timing_ = GGEMSOpenCLKernelTiming{};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSOpenCLProfiler::Stop() noexcept {
  stop_ = Clock::now();
  running_ = false;
  has_measurement_ = true;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSOpenCLProfiler::RecordKernelEvent(cl::Event const &event) {
  if (running_) {
    throw ggems::core::GGEMSRecoverable(
        "OpenCL kernel event profiling must be recorded after Stop() when host "
      "timing is active.");
  }

  cl_ulong queued = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
  cl_ulong submit = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>();
  cl_ulong start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
  cl_ulong end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

  if (!(queued <= submit && submit <= start && start <= end)) {
    throw ggems::core::GGEMSRecoverable("Invalid OpenCL profiling timestamps ordering.");
  }

  kernel_timing_.time_queued = MakeTimeFromNanoseconds(queued);
  kernel_timing_.time_submit = MakeTimeFromNanoseconds(submit);
  kernel_timing_.time_start = MakeTimeFromNanoseconds(start);
  kernel_timing_.time_end = MakeTimeFromNanoseconds(end);

  kernel_timing_.command_time = MakeTimeFromNanoseconds(end - queued);
  kernel_timing_.kernel_time = MakeTimeFromNanoseconds(end - start);

  kernel_timing_.valid = true;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

long double GGEMSOpenCLProfiler::GetElapsedSecondsRaw() const noexcept {
  if (!has_measurement_) {
    return 0.0L;
  }

  auto elapsed = std::chrono::duration<long double>(stop_ - start_);

  return elapsed.count();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

auto GGEMSOpenCLProfiler::GetElapsedTime() const noexcept
    -> ggems::units::Duration {
  return MakeDurationFromSeconds(GetElapsedSecondsRaw());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

auto GGEMSOpenCLProfiler::GetKernelTime() const noexcept
    -> ggems::units::Duration {
  return kernel_timing_.kernel_time;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Time GGEMSOpenCLProfiler::GetCommandTime() const noexcept {
  return kernel_timing_.command_time;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

double GGEMSOpenCLProfiler::GetElapsedSeconds() const noexcept {
  return static_cast<double>(GetElapsedSecondsRaw());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

double GGEMSOpenCLProfiler::GetKernelSeconds() const noexcept {
  if (!kernel_timing_.valid) {
    return 0.0;
  }

  auto const seconds =
      ggems::units::TryConvertTo(kernel_timing_.kernel_time, "s");
  return seconds.has_value() ? static_cast<double>(*seconds) : 0.0;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

double GGEMSOpenCLProfiler::ComputeRatePerSecond(
    std::uint64_t item_count) const noexcept {
  double elapsed_seconds = GetElapsedSeconds();

  if (elapsed_seconds <= 0.0) {
    return 0.0;
  }

  return static_cast<double>(item_count) / elapsed_seconds;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

double GGEMSOpenCLProfiler::ComputeKernelRatePerSecond(
    std::uint64_t item_count) const noexcept {
  double const kernel_seconds = GetKernelSeconds();

  if (kernel_seconds <= 0.0) {
    return 0.0;
  }

  return static_cast<double>(item_count) / kernel_seconds;
}

} // namespace ggems::ocl
