#include <cmath>
#include <cstdint>
#include <numbers>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/radioactivity/GGEMSDecayStatistics.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSPoissonSampler.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/core/GGEMSTimeWindow.hh"

namespace ggems::core::radioactivity {
namespace {
constexpr long double k_picosecond_to_second{1.0e-12L};
} // namespace

// =============================================================================
// =============================================================================

auto ComputeExpectedDecayEventCount(units::Activity activity_at_reference,
                                    long double half_life_seconds,
                                    std::uint64_t reference_time_ps,
                                    GGEMSTimeWindow time_window)
    -> long double {
  GGEMS_CHECK_RECOVERABLE(std::isfinite(activity_at_reference.value),
                          "Activity at the reference time must be finite.");
  GGEMS_CHECK_RECOVERABLE(
      activity_at_reference.value >= 0.0L,
      "Activity at the reference time must be non-negative.");
  GGEMS_CHECK_RECOVERABLE(std::isfinite(half_life_seconds),
                          "Half-life must be finite.");
  GGEMS_CHECK_RECOVERABLE(half_life_seconds > 0.0L,
                          "Half-life must be strictly positive.");
  GGEMS_CHECK_RECOVERABLE(time_window.start_ps <= time_window.stop_ps,
                          "Decay time window start must not exceed its stop.");
  GGEMS_CHECK_RECOVERABLE(
      reference_time_ps <= time_window.start_ps,
      "Decay reference time must not exceed the window start.");

  if (activity_at_reference.value == 0.0L ||
      time_window.start_ps == time_window.stop_ps) {
    return 0.0L;
  }

  auto const elapsed_start_ps = time_window.start_ps - reference_time_ps;
  auto const duration_ps = time_window.stop_ps - time_window.start_ps;
  long double const elapsed_start_seconds =
      static_cast<long double>(elapsed_start_ps) * k_picosecond_to_second;
  long double const duration_seconds =
      static_cast<long double>(duration_ps) * k_picosecond_to_second;
  long double const decay_constant =
      std::numbers::ln2_v<long double> / half_life_seconds;

  long double mean{0.0L};

  if (decay_constant == 0.0L) {
    mean = activity_at_reference.value * duration_seconds;
  } else if (!std::isfinite(decay_constant)) {
    if (elapsed_start_seconds == 0.0L) {
      mean = activity_at_reference.value * half_life_seconds /
             std::numbers::ln2_v<long double>;
    }
  } else {
    long double const activity_at_start =
        activity_at_reference.value *
        std::exp(-decay_constant * elapsed_start_seconds);
    long double const decayed_fraction =
        -std::expm1(-decay_constant * duration_seconds);

    mean = activity_at_start * decayed_fraction / decay_constant;
  }

  GGEMS_CHECK_RECOVERABLE(std::isfinite(mean),
                          "Expected decay event count is not finite.");
  GGEMS_CHECK_RECOVERABLE(mean >= 0.0L,
                          "Expected decay event count is negative.");

  return mean;
}

// =============================================================================
// =============================================================================

auto SampleDecayEventCount(units::Activity activity_at_reference,
                           long double half_life_seconds,
                           std::uint64_t reference_time_ps,
                           GGEMSTimeWindow time_window,
                           random::GGEMSHostRandomStream &random_stream)
    -> std::uint64_t {
  return random::SamplePoisson(
      ComputeExpectedDecayEventCount(activity_at_reference, half_life_seconds,
                                     reference_time_ps, time_window),
      random_stream);
}
} // namespace ggems::core::radioactivity
