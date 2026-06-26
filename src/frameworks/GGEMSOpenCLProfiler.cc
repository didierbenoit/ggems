#include <limits>

#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace ggems::ocl {

namespace {
ggems::units::Time MakeTimeFromSeconds(long double seconds) noexcept {
  if (seconds <= 0.0L) {
    return ggems::units::Time{0U};
  }

  long double picoseconds = seconds * 1.0e12L;

  if (picoseconds >=
      static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
    return ggems::units::Time{std::numeric_limits<std::uint64_t>::max()};
  }

  return ggems::units::Time{static_cast<std::uint64_t>(picoseconds + 0.5L)};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Time MakeTimeFromNanoseconds(cl_ulong nanoseconds) noexcept {
  if (nanoseconds >= std::numeric_limits<std::uint64_t>::max() / 1000ULL) {
    return ggems::units::Time{std::numeric_limits<std::uint64_t>::max()};
  }

  return ggems::units::Time{static_cast<std::uint64_t>(nanoseconds) * 1000ULL};
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
  cl_ulong queued = event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
  cl_ulong submit = event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>();
  cl_ulong start = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
  cl_ulong end = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

  GGEMS_CHECK_RECOVERABLE(queued <= submit && submit <= start && start <= end,
                          "Invalid OpenCL profiling timestamps ordering.");

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

ggems::units::Time GGEMSOpenCLProfiler::GetElapsedTime() const noexcept {
  return MakeTimeFromSeconds(GetElapsedSecondsRaw());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Time GGEMSOpenCLProfiler::GetKernelTime() const noexcept {
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

  return static_cast<double>(kernel_timing_.kernel_time.value) * 1.0e-12;
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
